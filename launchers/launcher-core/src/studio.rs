use std::path::{Path, PathBuf};
use std::process::Command;

use crate::metadata::{ModMetadata, METADATA_FILE};
use crate::{slugify, Error, Result};

pub const MDK_REPOSITORY: &str = "https://github.com/mattiasploesch-cpu/matti-ndk.git";
pub const PACKAGE_DIR: &str = "mods/import";

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct StudioProject {
    pub root: PathBuf,
    pub metadata: ModMetadata,
    pub mdk: Option<PathBuf>,
    pub notes: Vec<String>,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct BuildOutput {
    pub library: PathBuf,
    pub package: PathBuf,
    pub metadata: PathBuf,
}

pub fn create_project(
    root: &Path,
    metadata: ModMetadata,
    mdk_url: &str,
    mdk_source: Option<&Path>,
) -> Result<StudioProject> {
    metadata.validate()?;
    if metadata.name.trim().is_empty() {
        return Err(Error::Invalid("the mod needs a name".into()));
    }
    std::fs::create_dir_all(root)?;
    let mut notes = Vec::new();
    let mdk = if let Some(source) = mdk_source {
        let target = root.join("mdk");
        copy_tree(source, &target)?;
        notes.push(format!("MDK copied from {}", source.display()));
        Some(target)
    } else if !mdk_url.trim().is_empty() {
        let target = root.join("mdk");
        match clone_mdk(mdk_url, &target) {
            Ok(()) => {
                notes.push(format!("MDK cloned from {mdk_url}"));
                Some(target)
            }
            Err(error) => {
                notes.push(format!("MDK clone skipped: {error}"));
                None
            }
        }
    } else {
        None
    };
    metadata.save(root)?;
    let src = root.join("src");
    std::fs::create_dir_all(&src)?;
    let resources = root.join("resources");
    std::fs::create_dir_all(&resources)?;
    let source_path = src.join(format!("{}.c", c_identifier(&metadata.id)));
    if !source_path.exists() {
        std::fs::write(&source_path, source_template(&metadata))?;
    }
    let cmake = root.join("CMakeLists.txt");
    if !cmake.exists() {
        std::fs::write(&cmake, cmake_template(&metadata, &mdk))?;
    }
    if !root.join("metadata.matti").is_file() {
        return Err(Error::Invalid(format!(
            "{} was not written",
            root.join(METADATA_FILE).display()
        )));
    }
    Ok(StudioProject {
        root: root.to_path_buf(),
        metadata,
        mdk,
        notes,
    })
}

pub fn clone_mdk(url: &str, target: &Path) -> Result<()> {
    if target.exists() && target.read_dir()?.next().is_some() {
        return Err(Error::Invalid(format!(
            "{} already exists and is not empty",
            target.display()
        )));
    }
    if let Some(parent) = target.parent() {
        std::fs::create_dir_all(parent)?;
    }
    let output = Command::new("git")
        .arg("clone")
        .arg("--depth")
        .arg("1")
        .arg(url)
        .arg(target)
        .output()
        .map_err(|error| Error::Invalid(format!("could not run git: {error}")))?;
    if !output.status.success() {
        return Err(Error::Invalid(format!(
            "git clone failed: {}",
            String::from_utf8_lossy(&output.stderr).trim()
        )));
    }
    Ok(())
}

pub fn build_project(
    project: &StudioProject,
    repository_root: &Path,
    output_dir: &Path,
) -> Result<BuildOutput> {
    project.metadata.validate()?;
    if !project.root.join(METADATA_FILE).is_file() {
        return Err(Error::Invalid(format!(
            "{} is required before building",
            project.root.join(METADATA_FILE).display()
        )));
    }
    std::fs::create_dir_all(output_dir)?;
    let build_dir = project.root.join("build");
    let helper = repository_root.join("mods/cmake/MattiMod.cmake");
    if !helper.is_file() {
        return Err(Error::NotFound(format!(
            "mod CMake helper {}",
            helper.display()
        )));
    }
    let mut configure = Command::new("cmake");
    configure
        .arg("-S")
        .arg(&project.root)
        .arg("-B")
        .arg(&build_dir)
        .arg(format!(
            "-DMATTI_LIBRARY_DIR={}",
            repository_root.join("library/src").display()
        ))
        .arg(format!("-DMATTI_MODS_DIR={}", output_dir.display()))
        .arg(format!("-DMATTI_MOD_CMAKE={}", helper.display()));
    run(configure, "configure the mod project")?;
    let mut build = Command::new("cmake");
    build.arg("--build").arg(&build_dir).arg("-j").arg("2");
    run(build, "build the mod project")?;

    let library = find_library(output_dir, &project.metadata)?;
    let package = output_dir.join(format!(
        "{}-{}.matti",
        project.metadata.id, project.metadata.version
    ));
    let metadata = pack(project, &library, &package)?;
    Ok(BuildOutput {
        library,
        package,
        metadata,
    })
}

pub fn pack(project: &StudioProject, library: &Path, package: &Path) -> Result<PathBuf> {
    if !library.is_file() {
        return Err(Error::NotFound(format!("built mod {}", library.display())));
    }
    let metadata_path = project.root.join(METADATA_FILE);
    let metadata = ModMetadata::load(&project.root)?;
    if metadata.id != project.metadata.id || metadata.version != project.metadata.version {
        return Err(Error::Invalid(
            "metadata.matti changed after the project was opened".into(),
        ));
    }
    let tmp = std::env::temp_dir().join(format!(
        "matticraft-pack-{}-{}",
        std::process::id(),
        slugify(&metadata.id)
    ));
    let _ = std::fs::remove_dir_all(&tmp);
    std::fs::create_dir_all(&tmp)?;
    let pack = tmp.join("pack");
    std::fs::write(
        &pack,
        format!(
            "Matti-Pack-Format: 1\nMatti-Mod-Id: {}\nMatti-Mod-Version: {}\n\n",
            metadata.id, metadata.version
        ),
    )?;
    if let Some(parent) = package.parent() {
        std::fs::create_dir_all(parent)?;
    }
    std::fs::copy(library, package)?;
    let mut command = Command::new("objcopy");
    command
        .arg("--add-section")
        .arg(format!(".matti_pack={}", pack.display()))
        .arg("--set-section-flags")
        .arg(".matti_pack=contents,readonly")
        .arg("--add-section")
        .arg(format!(".matti_metadata={}", metadata_path.display()))
        .arg("--set-section-flags")
        .arg(".matti_metadata=contents,readonly");
    let source_files = source_files(&project.root)?;
    if !source_files.is_empty() {
        let list = tmp.join("source-list");
        let list_text = source_files
            .iter()
            .map(|path| {
                path.strip_prefix(&project.root)
                    .unwrap_or(path)
                    .to_string_lossy()
                    .replace('\\', "/")
            })
            .collect::<Vec<_>>()
            .join("\n");
        std::fs::write(&list, list_text)?;
        command
            .arg("--add-section")
            .arg(format!("matti_src_list={}", list.display()))
            .arg("--set-section-flags")
            .arg("matti_src_list=contents,readonly");
        for path in &source_files {
            let relative = path
                .strip_prefix(&project.root)
                .unwrap_or(path)
                .to_string_lossy()
                .replace('\\', "/");
            command
                .arg("--add-section")
                .arg(format!("matti_src/{relative}={}", path.display()))
                .arg("--set-section-flags")
                .arg(format!("matti_src/{relative}=contents,readonly"));
        }
    }
    let cmake = project.root.join("CMakeLists.txt");
    if cmake.is_file() {
        command
            .arg("--add-section")
            .arg(format!("matti_cmake={}", cmake.display()))
            .arg("--set-section-flags")
            .arg("matti_cmake=contents,readonly");
    }
    command.arg(package);
    run(command, "pack the .matti file")?;
    std::fs::copy(&metadata_path, package.with_extension("metadata.matti"))?;
    let _ = std::fs::remove_dir_all(&tmp);
    Ok(package.to_path_buf())
}

pub fn stage_import(repository_root: &Path, package: &Path) -> Result<PathBuf> {
    if !package.is_file() {
        return Err(Error::NotFound(format!("package {}", package.display())));
    }
    let target = repository_root.join(PACKAGE_DIR);
    std::fs::create_dir_all(&target)?;
    for entry in std::fs::read_dir(&target)? {
        let entry = entry?;
        if entry.path().extension().and_then(|value| value.to_str()) == Some("matti") {
            std::fs::remove_file(entry.path())?;
        }
    }
    let destination = target.join(
        package
            .file_name()
            .ok_or_else(|| Error::Invalid("package has no file name".into()))?,
    );
    std::fs::copy(package, &destination)?;
    let metadata = package.with_extension("metadata.matti");
    if metadata.is_file() {
        std::fs::copy(metadata, target.join("metadata.matti"))?;
    }
    Ok(destination)
}

pub fn list_projects(root: &Path) -> Result<Vec<StudioProject>> {
    if !root.is_dir() {
        return Ok(Vec::new());
    }
    let mut projects = Vec::new();
    for entry in std::fs::read_dir(root)? {
        let entry = entry?;
        if !entry.file_type()?.is_dir() {
            continue;
        }
        let path = entry.path();
        if let Ok(metadata) = ModMetadata::load(&path) {
            projects.push(StudioProject {
                root: path,
                metadata,
                mdk: None,
                notes: Vec::new(),
            });
        }
    }
    projects.sort_by(|left, right| left.metadata.name.cmp(&right.metadata.name));
    Ok(projects)
}

fn find_library(dir: &Path, metadata: &ModMetadata) -> Result<PathBuf> {
    let expected = format!("{}-{}", metadata.id, metadata.version);
    let mut candidates = Vec::new();
    for entry in std::fs::read_dir(dir)? {
        let entry = entry?;
        let path = entry.path();
        if !path.is_file() {
            continue;
        }
        let name = path.file_name().and_then(|name| name.to_str()).unwrap_or("");
        if name.ends_with(".so") || name.ends_with(".dylib") || name.ends_with(".dll") {
            if name.starts_with(&expected) {
                return Ok(path);
            }
            candidates.push(path);
        }
    }
    candidates
        .into_iter()
        .next()
        .ok_or_else(|| Error::NotFound(format!("compiled mod in {}", dir.display())))
}

fn source_files(root: &Path) -> Result<Vec<PathBuf>> {
    let mut files = Vec::new();
    collect_files(root, &mut files)?;
    files.retain(|path| {
        let relative = path.strip_prefix(root).unwrap_or(path);
        let first = relative
            .components()
            .next()
            .map(|component| component.as_os_str().to_string_lossy().to_string())
            .unwrap_or_default();
        if matches!(first.as_str(), "build" | "out" | ".git" | "mdk" | "target") {
            return false;
        }
        matches!(
            path.extension().and_then(|value| value.to_str()),
            Some("c" | "h" | "cmake" | "txt" | "json" | "toml" | "md" | "matti")
        )
    });
    files.sort();
    Ok(files)
}

fn collect_files(root: &Path, files: &mut Vec<PathBuf>) -> Result<()> {
    for entry in std::fs::read_dir(root)? {
        let entry = entry?;
        let path = entry.path();
        if entry.file_type()?.is_dir() {
            if path.file_name().and_then(|name| name.to_str()) == Some(".git") {
                continue;
            }
            collect_files(&path, files)?;
        } else if entry.file_type()?.is_file() {
            files.push(path);
        }
    }
    Ok(())
}

fn copy_tree(source: &Path, target: &Path) -> Result<()> {
    if !source.is_dir() {
        return Err(Error::NotFound(format!("MDK {}", source.display())));
    }
    std::fs::create_dir_all(target)?;
    for entry in std::fs::read_dir(source)? {
        let entry = entry?;
        let name = entry.file_name();
        if name == ".git" {
            continue;
        }
        let from = entry.path();
        let to = target.join(&name);
        if entry.file_type()?.is_dir() {
            copy_tree(&from, &to)?;
        } else if entry.file_type()?.is_file() {
            std::fs::copy(&from, &to)?;
        }
    }
    Ok(())
}

fn run(mut command: Command, action: &str) -> Result<()> {
    let output = command
        .output()
        .map_err(|error| Error::Invalid(format!("{action}: {error}")))?;
    if output.status.success() {
        return Ok(());
    }
    let stderr = String::from_utf8_lossy(&output.stderr);
    let stdout = String::from_utf8_lossy(&output.stdout);
    Err(Error::Invalid(format!(
        "{action} failed: {}{}{}",
        stderr.trim(),
        if stderr.trim().is_empty() { "" } else { " " },
        stdout.trim()
    )))
}

fn c_identifier(id: &str) -> String {
    id.chars()
        .map(|ch| if ch.is_ascii_alphanumeric() { ch } else { '_' })
        .collect()
}

fn source_template(metadata: &ModMetadata) -> String {
    let symbol = c_identifier(&metadata.id);
    format!(
        "#include \"libmatti/cpw/modlauncher/LogManager.h\"\n\
         #include \"libmatti/net/neoforged/fml/javafmlmod/FMLModContainer.h\"\n\n\
         void {symbol}_init(const LIBMATTI_FML_FMLModContainer_ConstructorArgs *args)\n\
         {{\n\
         \x20   LIBMATTI_ML_Logger_Info(LIBMATTI_ML_LogManager_GetLogger(), NULL,\n\
         \x20       \"{id}: loaded for %s\",\n\
         \x20       args->dist == LIBMATTI_DIST_CLIENT ? \"the client\" : \"the dedicated server\");\n\
         }}\n"
        ,
        symbol = symbol,
        id = metadata.id
    )
}

fn cmake_template(metadata: &ModMetadata, mdk: &Option<PathBuf>) -> String {
    let helper = mdk
        .as_ref()
        .map(|path| path.join("cmake/MattiMod.cmake"))
        .filter(|path| path.is_file())
        .map(|path| path.display().to_string())
        .unwrap_or_else(|| "${MATTI_MOD_CMAKE}".to_string());
    format!(
        "cmake_minimum_required(VERSION 3.20)\n\
         project({id}_mod C)\n\n\
         set(CMAKE_C_STANDARD 11)\n\
         if(NOT MATTI_MOD_CMAKE)\n\
         \x20   set(MATTI_MOD_CMAKE \"{helper}\")\n\
         endif()\n\
         include(\"${{MATTI_MOD_CMAKE}}\")\n\n\
         matti_add_mod({id}\n\
         \x20       MOD_ID {id}\n\
         \x20       MOD_VERSION {version}\n\
         \x20       SOURCES src/{symbol}.c\n\
         \x20       RESOURCES resources\n\
         )\n",
        id = metadata.id,
        symbol = c_identifier(&metadata.id),
        version = metadata.version,
        helper = helper
    )
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn creates_required_files() {
        let root = crate::temp_dir("studio");
        let metadata = ModMetadata::new("studio-mod", "Studio Mod", "1.0.0").unwrap();
        let project = create_project(&root, metadata, "", None).unwrap();
        assert!(project.root.join("metadata.matti").is_file());
        assert!(project.root.join("CMakeLists.txt").is_file());
        assert!(project.root.join("src/studio_mod.c").is_file());
        let _ = std::fs::remove_dir_all(root);
    }
}
