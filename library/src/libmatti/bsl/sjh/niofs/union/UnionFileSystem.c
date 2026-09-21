//
// Created by administrator on 09.09.26.
//

#include "UnionFileSystem.h"

#include "libmatti/bsl/sjh/niofs/union/UnionFileSystemProvider.h"
#include "libmatti/bsl/sjh/niofs/union/UnionPath.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// ---------------------------------------------------------------------------
// .so parsing (in Java the JDK zip filesystem reads the artifact; here the ELF
// section reader below reads the .so).
// A mod is an ELF shared object; its manifest lives in the ELF section
// ".matti_manifest" and is exposed as the entry "META-INF/MANIFEST.MF".
// ---------------------------------------------------------------------------

// The ELF section holding the mod manifest
static const char *MANIFEST_SECTION = ".matti_manifest";
// The entry name the section is exposed under
static const char *MANIFEST_ENTRY = "META-INF/MANIFEST.MF";

typedef struct
{
    char *name;                // entry name, e.g. "META-INF/MANIFEST.MF"
    const unsigned char *data; // points into the file buffer
    size_t size;
    int isDirectory;
} ElfEntry;

struct LIBMATTI_UNION_EmbeddedElf
{
    char *path; // the base path of the .so file
    unsigned char *data;
    size_t dataLength;
    ElfEntry *entries;
    size_t entryCount;
};

static uint16_t elf_rd16(const unsigned char *p, int bigEndian)
{
    if (bigEndian) return (uint16_t)((p[0] << 8) | p[1]);
    return (uint16_t)(p[0] | (p[1] << 8));
}

static uint32_t elf_rd32(const unsigned char *p, int bigEndian)
{
    if (bigEndian)
        return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];

    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint64_t elf_rd64(const unsigned char *p, int bigEndian)
{
    if (bigEndian) return ((uint64_t)elf_rd32(p, 1) << 32) | elf_rd32(p + 4, 1);
    return ((uint64_t)elf_rd32(p + 4, 0) << 32) | elf_rd32(p, 0);
}

// Bounded copy of a section name; returns its length, or 0 when the offset is out of range.
static size_t elf_section_name(const unsigned char *names, uint64_t tableSize, uint32_t offset, char *buffer,
                               size_t bufferSize)
{
    if (offset >= tableSize) return 0;

    uint64_t remaining = tableSize - offset;
    size_t length = 0;

    while (length < remaining && names[offset + length] != '\0') length++;

    if (length == 0 || length >= bufferSize) return 0;

    memcpy(buffer, names + offset, length);
    buffer[length] = '\0';
    return length;
}

static LIBMATTI_UNION_EmbeddedElf *embedded_elf_open(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL) return NULL;

    fseek(file, 0, SEEK_END);
    long fileLength = ftell(file);
    fseek(file, 0, SEEK_SET);

    // smallest possible ELF header (32 bit)
    if (fileLength < 52)
    {
        fclose(file);
        return NULL;
    }

    unsigned char *data = malloc((size_t)fileLength);
    if (fread(data, 1, (size_t)fileLength, file) != (size_t)fileLength)
    {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);

    // ELF magic 0x7f 'E' 'L' 'F'
    if (data[0] != 0x7f || data[1] != 'E' || data[2] != 'L' || data[3] != 'F')
    {
        free(data);
        return NULL;
    }

    int is64 = data[4] == 2;      // EI_CLASS: 1 = 32 bit, 2 = 64 bit
    int bigEndian = data[5] == 2; // EI_DATA: 1 = little endian, 2 = big endian

    if (is64 && fileLength < 64)
    {
        free(data);
        return NULL;
    }

    uint64_t sectionHeaderOffset = is64 ? elf_rd64(data + 0x28, bigEndian) : elf_rd32(data + 0x20, bigEndian);
    uint16_t sectionHeaderSize = elf_rd16(data + (is64 ? 0x3A : 0x2E), bigEndian);
    uint16_t sectionCount = elf_rd16(data + (is64 ? 0x3C : 0x30), bigEndian);
    uint16_t nameTableIndex = elf_rd16(data + (is64 ? 0x3E : 0x32), bigEndian);

    if (sectionHeaderOffset == 0 || sectionHeaderSize == 0 || sectionCount == 0 ||
        nameTableIndex >= sectionCount ||
        sectionHeaderOffset + (uint64_t)sectionHeaderSize * sectionCount > (uint64_t)fileLength)
    {
        free(data);
        return NULL;
    }

    const unsigned char *sections = data + sectionHeaderOffset;

    // the section header string table
    const unsigned char *nameTableHeader = sections + (uint64_t)nameTableIndex * sectionHeaderSize;
    uint64_t nameTableOffset = is64 ? elf_rd64(nameTableHeader + 24, bigEndian) : elf_rd32(nameTableHeader + 16, bigEndian);
    uint64_t nameTableSize = is64 ? elf_rd64(nameTableHeader + 32, bigEndian) : elf_rd32(nameTableHeader + 20, bigEndian);

    if (nameTableOffset + nameTableSize > (uint64_t)fileLength)
    {
        free(data);
        return NULL;
    }

    const unsigned char *nameTable = data + nameTableOffset;

    LIBMATTI_UNION_EmbeddedElf *elf = calloc(1, sizeof(LIBMATTI_UNION_EmbeddedElf));
    elf->path = strdup(path);
    elf->data = data;
    elf->dataLength = (size_t)fileLength;

    for (uint16_t i = 0; i < sectionCount; i++)
    {
        const unsigned char *section = sections + (uint64_t)i * sectionHeaderSize;

        uint32_t nameOffset = elf_rd32(section, bigEndian);
        uint64_t sectionOffset = is64 ? elf_rd64(section + 24, bigEndian) : elf_rd32(section + 16, bigEndian);
        uint64_t sectionSize = is64 ? elf_rd64(section + 32, bigEndian) : elf_rd32(section + 20, bigEndian);

        char sectionName[256];
        if (elf_section_name(nameTable, nameTableSize, nameOffset, sectionName, sizeof(sectionName)) == 0) continue;
        if (sectionOffset + sectionSize > (uint64_t)fileLength) continue;

        // The manifest section is exposed under its jar entry name.
        // Every other section whose name is not a toolchain name (".text", ".rodata", ...)
        // is exposed as the entry of exactly that path ("META-INF/services/x.Y", "mods.toml", ...).
        const char *entryName;
        if (strcmp(sectionName, MANIFEST_SECTION) == 0)
            entryName = MANIFEST_ENTRY;
        else if (sectionName[0] != '.')
            entryName = sectionName;
        else
            continue;

        ElfEntry *grown = realloc(elf->entries, sizeof(ElfEntry) * (elf->entryCount + 1));
        if (grown == NULL) continue;

        elf->entries = grown;
        ElfEntry *entry = &elf->entries[elf->entryCount];
        entry->name = strdup(entryName);
        entry->data = data + sectionOffset;
        entry->size = (size_t)sectionSize;
        entry->isDirectory = 0;
        elf->entryCount++;
    }

    return elf;
}

static void embedded_elf_free(LIBMATTI_UNION_EmbeddedElf *elf)
{
    if (elf == NULL) return;

    for (size_t i = 0; i < elf->entryCount; i++) free(elf->entries[i].name);
    free(elf->entries);
    free(elf->data);
    free(elf->path);
    free(elf);
}

// Exact entry lookup by relative name ("META-INF/MANIFEST.MF")
static ElfEntry *embedded_elf_find(LIBMATTI_UNION_EmbeddedElf *elf, const char *name)
{
    for (size_t i = 0; i < elf->entryCount; i++)
    {
        if (strcmp(elf->entries[i].name, name) == 0) return &elf->entries[i];
    }
    return NULL;
}

// Java: Files.exists on a mod path. Entries are stored as files; a directory
// also exists if entries live under it.
static int embedded_elf_exists(LIBMATTI_UNION_EmbeddedElf *elf, const char *name)
{
    if (embedded_elf_find(elf, name) != NULL) return 1;

    size_t len = strlen(name);
    if (len == 0 || name[len - 1] == '/') return 0;

    for (size_t i = 0; i < elf->entryCount; i++)
    {
        const char *entryName = elf->entries[i].name;
        if (strncmp(entryName, name, len) == 0 && entryName[len] == '/') return 1;
    }
    return 0;
}

static int embedded_elf_attributes(LIBMATTI_UNION_EmbeddedElf *elf, const char *name, LIBMATTI_UNION_BasicFileAttributes *out)
{
    ElfEntry *entry = embedded_elf_find(elf, name);
    if (entry != NULL)
    {
        out->isDirectory = entry->isDirectory;
        out->isRegularFile = !entry->isDirectory;
        out->size = (long)entry->size;
        return 1;
    }

    // directory implied by a prefix of other entries
    size_t len = strlen(name);
    if (len > 0 && name[len - 1] != '/')
    {
        for (size_t i = 0; i < elf->entryCount; i++)
        {
            const char *entryName = elf->entries[i].name;
            if (strncmp(entryName, name, len) == 0 && entryName[len] == '/')
            {
                out->isDirectory = 1;
                out->isRegularFile = 0;
                out->size = 0;
                return 1;
            }
        }
    }
    return 0;
}

static int embedded_elf_read(LIBMATTI_UNION_EmbeddedElf *elf, ElfEntry *entry,
                             unsigned char **outBytes, size_t *outLength)
{
    (void)elf;

    unsigned char *bytes = malloc(entry->size > 0 ? entry->size : 1);

    if (entry->size > 0) memcpy(bytes, entry->data, entry->size);

    *outBytes = bytes;
    *outLength = entry->size;
    return 1;
}

// Collect the names of the entries under dirName (immediate children, or all
// descendants when recursive). Names are relative to the mod root.
static void embedded_elf_collect(LIBMATTI_UNION_EmbeddedElf *elf, const char *dirName, int recursive,
                                 char ***outNames, int **outIsDir, size_t *outCount)
{
    size_t dirLength = strlen(dirName);
    int hasPrefix = dirLength == 0 || dirName[dirLength - 1] == '/';

    for (size_t i = 0; i < elf->entryCount; i++)
    {
        const char *entryName = elf->entries[i].name;
        int isDir = elf->entries[i].isDirectory;

        // skip the directory itself
        if (hasPrefix && strcmp(entryName, dirName) == 0) continue;

        int under = 0;
        if (hasPrefix)
        {
            under = strncmp(entryName, dirName, dirLength) == 0;
        }
        else
        {
            under = strncmp(entryName, dirName, dirLength) == 0 && entryName[dirLength] == '/';
        }
        if (!under) continue;

        if (!recursive)
        {
            // immediate children only: no further '/' after the prefix
            const char *slash = strchr(entryName + dirLength, '/');
            if (slash != NULL && slash[1] != '\0') continue;
        }

        *outNames = realloc(*outNames, sizeof(char *) * (*outCount + 1));
        *outIsDir = realloc(*outIsDir, sizeof(int) * (*outCount + 1));
        (*outNames)[*outCount] = strdup(entryName);
        (*outIsDir)[*outCount] = isDir;
        (*outCount)++;
    }
}

// ---------------------------------------------------------------------------
// UnionFileSystem
// ---------------------------------------------------------------------------

static int path_is_directory(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static int path_exists_real(const char *path)
{
    return access(path, F_OK) == 0;
}

static LIBMATTI_UNION_EmbeddedElf *find_embedded_elf(const LIBMATTI_UNION_UnionFileSystem *fileSystem, const char *basePath)
{
    for (size_t i = 0; i < fileSystem->embeddedElfCount; i++)
    {
        if (strcmp(fileSystem->embeddedElfs[i]->path, basePath) == 0)
            return fileSystem->embeddedElfs[i];
    }
    return NULL;
}

// Java: toRealPath(basePath, path) - returns the mod entry name or basePath + resolvepath
static char *to_real_path(const LIBMATTI_UNION_UnionFileSystem *fileSystem, const char *basePath,
                          const LIBMATTI_UNION_UnionPath *path)
{
    LIBMATTI_UNION_UnionPath *relative = path->absolute
        ? LIBMATTI_UNION_UnionPath_Relativize(LIBMATTI_UNION_UnionFileSystem_GetRoot(fileSystem), path)
        : (LIBMATTI_UNION_UnionPath *)path;

    LIBMATTI_UNION_UnionPath *normalized = LIBMATTI_UNION_UnionPath_Normalize(relative);
    char *resolvePath = LIBMATTI_UNION_UnionPath_ToString(normalized);

    if (relative != path) LIBMATTI_UNION_UnionPath_Free(relative);

    if (find_embedded_elf(fileSystem, basePath) != NULL)
    {
        // .so base: the resolve path is the entry name
        return resolvePath;
    }

    size_t total = strlen(basePath) + 1 + strlen(resolvePath) + 1;
    char *realPath = malloc(total);
    snprintf(realPath, total, "%s/%s", basePath, resolvePath);
    free(resolvePath);
    return realPath;
}

// The relative name of a real path for the filter (Java: testFilter standardizes)
static const char *relative_name(const char *basePath, const char *realPath, int isElfBase)
{
    if (isElfBase) return realPath;

    size_t baseLength = strlen(basePath);
    const char *rel = realPath + baseLength;
    while (*rel == '/') rel++;
    return rel;
}

// Java: testFilter - standardize the path and call the filter.
// Directories get a trailing '/', then the filter is invoked with the relative name.
static int test_filter(const LIBMATTI_UNION_UnionFileSystem *fileSystem, const char *basePath,
                       const char *realPath, int isElfBase, int isDirectory)
{
    if (fileSystem->pathFilter == NULL) return 1;

    const char *rel = relative_name(basePath, realPath, isElfBase);

    size_t total = strlen(rel) + (isDirectory ? 2 : 1);
    char *standardized = malloc(total);
    strcpy(standardized, rel);
    if (isDirectory) strcat(standardized, "/");

    int allowed = fileSystem->pathFilter->test(standardized, basePath, fileSystem->pathFilter->userdata);
    free(standardized);
    return allowed;
}

// Java: fastPathExists(realPath)
static int fast_path_exists(const LIBMATTI_UNION_UnionFileSystem *fileSystem, const char *basePath, const char *realPath)
{
    LIBMATTI_UNION_EmbeddedElf *elf = find_embedded_elf(fileSystem, basePath);
    if (elf != NULL)
        return embedded_elf_exists(elf, realPath);

    return path_exists_real(realPath);
}

// Fetch attributes of a real path (mod entry or file system entry)
static int real_attributes(const LIBMATTI_UNION_UnionFileSystem *fileSystem, const char *basePath,
                           const char *realPath, LIBMATTI_UNION_BasicFileAttributes *out)
{
    LIBMATTI_UNION_EmbeddedElf *elf = find_embedded_elf(fileSystem, basePath);
    if (elf != NULL)
        return embedded_elf_attributes(elf, realPath, out);

    struct stat st;
    if (stat(realPath, &st) == 0)
    {
        out->isDirectory = S_ISDIR(st.st_mode);
        out->isRegularFile = S_ISREG(st.st_mode);
        out->size = (long)st.st_size;
        return 1;
    }
    return 0;
}

LIBMATTI_UNION_UnionFileSystem *LIBMATTI_UNION_UnionFileSystem_New(LIBMATTI_UNION_UnionFileSystemProvider *provider,
                                                                   LIBMATTI_UNION_UnionPathFilter *pathFilter,
                                                                   const char *key, const char **paths, size_t pathCount)
{
    LIBMATTI_UNION_UnionFileSystem *fileSystem = calloc(1, sizeof(LIBMATTI_UNION_UnionFileSystem));

    fileSystem->provider = provider;
    fileSystem->key = strdup(key);
    fileSystem->pathFilter = pathFilter;

    // Java: flip the list so later elements are first in search order, filter exists
    char **basePaths = calloc(pathCount, sizeof(char *));
    size_t baseCount = 0;

    for (size_t i = 0; i < pathCount; i++)
    {
        const char *candidate = paths[pathCount - i - 1];
        if (path_exists_real(candidate)) basePaths[baseCount++] = strdup(candidate);
    }

    fileSystem->basePaths = basePaths;
    fileSystem->basePathCount = baseCount;
    fileSystem->lastElementIndex = (int)baseCount - 1;

    // Java: open embedded file systems for non-directory base paths
    fileSystem->embeddedElfs = calloc(baseCount, sizeof(LIBMATTI_UNION_EmbeddedElf *));
    for (size_t i = 0; i < baseCount; i++)
    {
        if (!path_is_directory(basePaths[i]))
        {
            LIBMATTI_UNION_EmbeddedElf *elf = embedded_elf_open(basePaths[i]);
            if (elf == NULL)
            {
                // Java: UncheckedIOException - propagate as failure
                for (size_t j = 0; j < fileSystem->embeddedElfCount; j++) embedded_elf_free(fileSystem->embeddedElfs[j]);
                free(fileSystem->embeddedElfs);
                for (size_t j = 0; j < baseCount; j++) free(basePaths[j]);
                free(basePaths);
                free(fileSystem->key);
                free(fileSystem);
                return NULL;
            }
            fileSystem->embeddedElfs[fileSystem->embeddedElfCount++] = elf;
        }
    }

    const char *rootPart = "/";
    fileSystem->root = LIBMATTI_UNION_UnionPath_New(fileSystem, &rootPart, 1);

    return fileSystem;
}

void LIBMATTI_UNION_UnionFileSystem_Close(LIBMATTI_UNION_UnionFileSystem *fileSystem)
{
    if (fileSystem == NULL) return;

    if (fileSystem->provider != NULL)
        LIBMATTI_UNION_UnionFileSystemProvider_RemoveFileSystem(fileSystem->provider, fileSystem);

    for (size_t i = 0; i < fileSystem->embeddedElfCount; i++) embedded_elf_free(fileSystem->embeddedElfs[i]);
    free(fileSystem->embeddedElfs);

    for (size_t i = 0; i < fileSystem->basePathCount; i++) free(fileSystem->basePaths[i]);
    free(fileSystem->basePaths);

    LIBMATTI_UNION_UnionPath_Free(fileSystem->root);
    free(fileSystem->key);
    free(fileSystem);
}

const char *LIBMATTI_UNION_UnionFileSystem_GetPrimaryPath(const LIBMATTI_UNION_UnionFileSystem *fileSystem)
{
    // Java: basepaths.get(basepaths.size() - 1) - the first of the original input paths
    return fileSystem->basePaths[fileSystem->basePathCount - 1];
}

LIBMATTI_UNION_UnionPathFilter *LIBMATTI_UNION_UnionFileSystem_GetFilesystemFilter(const LIBMATTI_UNION_UnionFileSystem *fileSystem)
{
    return fileSystem->pathFilter;
}

char *LIBMATTI_UNION_UnionFileSystem_GetKey(const LIBMATTI_UNION_UnionFileSystem *fileSystem)
{
    return strdup(fileSystem->key);
}

LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionFileSystem_GetRoot(const LIBMATTI_UNION_UnionFileSystem *fileSystem)
{
    return fileSystem->root;
}

LIBMATTI_UNION_UnionPath *LIBMATTI_UNION_UnionFileSystem_GetPath(const LIBMATTI_UNION_UnionFileSystem *fileSystem,
                                                                 const char **parts, size_t partCount)
{
    return LIBMATTI_UNION_UnionPath_New((LIBMATTI_UNION_UnionFileSystem *)fileSystem, parts, partCount);
}

// Java: findFirstFiltered - first base (filter + existence), then the last base (filter only)
static long find_first_filtered(const LIBMATTI_UNION_UnionFileSystem *fileSystem, const LIBMATTI_UNION_UnionPath *path,
                                char **outRealPath)
{
    for (int i = 0; i < fileSystem->lastElementIndex; i++)
    {
        const char *basePath = fileSystem->basePaths[i];
        int isElfBase = find_embedded_elf(fileSystem, basePath) != NULL;
        char *realPath = to_real_path(fileSystem, basePath, path);

        if (test_filter(fileSystem, basePath, realPath, isElfBase, 0))
        {
            if (fast_path_exists(fileSystem, basePath, realPath))
            {
                *outRealPath = realPath;
                return i;
            }
        }
        free(realPath);
    }

    if (fileSystem->lastElementIndex >= 0)
    {
        int last = fileSystem->lastElementIndex;
        const char *basePath = fileSystem->basePaths[last];
        int isElfBase = find_embedded_elf(fileSystem, basePath) != NULL;
        char *realPath = to_real_path(fileSystem, basePath, path);

        // filter only, no existence check (Java: "without checking its existence")
        if (test_filter(fileSystem, basePath, realPath, isElfBase, 0))
        {
            *outRealPath = realPath;
            return last;
        }
        free(realPath);
    }

    return -1;
}

int LIBMATTI_UNION_UnionFileSystem_Exists(const LIBMATTI_UNION_UnionFileSystem *fileSystem, const LIBMATTI_UNION_UnionPath *path)
{
    char *realPath;
    long base = find_first_filtered(fileSystem, path, &realPath);
    if (base < 0) return 0;

    int result = fast_path_exists(fileSystem, fileSystem->basePaths[base], realPath);
    free(realPath);
    return result;
}

int LIBMATTI_UNION_UnionFileSystem_ReadAttributesIfExists(const LIBMATTI_UNION_UnionFileSystem *fileSystem,
                                                          const LIBMATTI_UNION_UnionPath *path,
                                                          LIBMATTI_UNION_BasicFileAttributes *out)
{
    for (size_t i = 0; i < fileSystem->basePathCount; i++)
    {
        const char *basePath = fileSystem->basePaths[i];
        int isElfBase = find_embedded_elf(fileSystem, basePath) != NULL;
        char *realPath = to_real_path(fileSystem, basePath, path);

        LIBMATTI_UNION_BasicFileAttributes attributes;
        int found = real_attributes(fileSystem, basePath, realPath, &attributes);
        int allowed = found && test_filter(fileSystem, basePath, realPath, isElfBase, attributes.isDirectory);

        free(realPath);

        if (allowed)
        {
            *out = attributes;
            return 1;
        }
    }
    return 0;
}

int LIBMATTI_UNION_UnionFileSystem_ReadAllBytes(const LIBMATTI_UNION_UnionFileSystem *fileSystem,
                                                const LIBMATTI_UNION_UnionPath *path,
                                                unsigned char **outBytes, size_t *outLength)
{
    char *realPath;
    long base = find_first_filtered(fileSystem, path, &realPath);
    if (base < 0) return 0;

    const char *basePath = fileSystem->basePaths[base];
    LIBMATTI_UNION_EmbeddedElf *elf = find_embedded_elf(fileSystem, basePath);

    int result = 0;
    if (elf != NULL)
    {
        ElfEntry *entry = embedded_elf_find(elf, realPath);
        if (entry != NULL)
            result = embedded_elf_read(elf, entry, outBytes, outLength);
    }
    else
    {
        FILE *file = fopen(realPath, "rb");
        if (file != NULL)
        {
            fseek(file, 0, SEEK_END);
            long length = ftell(file);
            fseek(file, 0, SEEK_SET);

            unsigned char *bytes = malloc(length > 0 ? (size_t)length : 1);
            if (fread(bytes, 1, (size_t)length, file) == (size_t)length)
            {
                *outBytes = bytes;
                *outLength = (size_t)length;
                result = 1;
            }
            else
            {
                free(bytes);
            }
            fclose(file);
        }
    }

    free(realPath);
    return result;
}

int LIBMATTI_UNION_UnionFileSystem_Walk(const LIBMATTI_UNION_UnionFileSystem *fileSystem, const char *dirRel, int recursive,
                                        int (*callback)(const char *relativePath, int isDirectory, void *userdata),
                                        void *userdata)
{
    // Collect candidates across all base paths, then filter + deduplicate
    // (Java: newDirStream merges with stream.distinct())
    char **candidates = NULL;
    int *candidateIsDir = NULL;
    const char **candidateBase = NULL;
    size_t candidateCount = 0;

    for (size_t i = 0; i < fileSystem->basePathCount; i++)
    {
        const char *basePath = fileSystem->basePaths[i];
        LIBMATTI_UNION_EmbeddedElf *elf = find_embedded_elf(fileSystem, basePath);

        if (elf != NULL)
        {
            size_t before = candidateCount;
            embedded_elf_collect(elf, dirRel, recursive, &candidates, &candidateIsDir, &candidateCount);
            candidateBase = realloc(candidateBase, sizeof(char *) * candidateCount);
            for (size_t c = before; c < candidateCount; c++) candidateBase[c] = basePath;
        }
        else
        {
            // walk the directory tree
            size_t baseLength = strlen(basePath);
            char *dirPath = malloc(baseLength + 1 + strlen(dirRel) + 1);
            snprintf(dirPath, baseLength + 1 + strlen(dirRel) + 1, "%s/%s", basePath, dirRel);

            char **stack = NULL;
            size_t stackCount = 0;
            stack = realloc(stack, sizeof(char *) * (stackCount + 1));
            stack[stackCount++] = strdup(dirPath);

            while (stackCount > 0)
            {
                char *current = stack[--stackCount];
                DIR *dir = opendir(current);
                if (dir != NULL)
                {
                    struct dirent *de;
                    while ((de = readdir(dir)) != NULL)
                    {
                        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;

                        char *child = malloc(strlen(current) + 1 + strlen(de->d_name) + 1);
                        snprintf(child, strlen(current) + 1 + strlen(de->d_name) + 1, "%s/%s", current, de->d_name);

                        struct stat st;
                        stat(child, &st);
                        int isDir = S_ISDIR(st.st_mode);

                        if (isDir)
                        {
                            if (recursive)
                            {
                                stack = realloc(stack, sizeof(char *) * (stackCount + 1));
                                stack[stackCount++] = strdup(child);
                            }
                        }
                        else
                        {
                            // relative name
                            const char *rel = child + baseLength;
                            while (*rel == '/') rel++;

                            candidates = realloc(candidates, sizeof(char *) * (candidateCount + 1));
                            candidateIsDir = realloc(candidateIsDir, sizeof(int) * (candidateCount + 1));
                            candidateBase = realloc(candidateBase, sizeof(char *) * (candidateCount + 1));
                            candidates[candidateCount] = strdup(rel);
                            candidateIsDir[candidateCount] = 0;
                            candidateBase[candidateCount] = basePath;
                            candidateCount++;
                        }

                        free(child);
                    }
                    closedir(dir);
                }
                free(current);
            }

            for (size_t s = 0; s < stackCount; s++) free(stack[s]);
            free(stack);
            free(dirPath);
        }
    }

    // filter + deduplicate + call the callback
    char **visited = NULL;
    size_t visitedCount = 0;

    for (size_t i = 0; i < candidateCount; i++)
    {
        int seen = 0;
        for (size_t v = 0; v < visitedCount; v++)
        {
            if (strcmp(visited[v], candidates[i]) == 0) { seen = 1; break; }
        }
        if (seen) continue;

        // Java: filter(p -> testFilter(p, bp, null))
        int isElfBase = find_embedded_elf(fileSystem, candidateBase[i]) != NULL;

        char *realPath;
        if (isElfBase)
        {
            realPath = candidates[i];
        }
        else
        {
            size_t total = strlen(candidateBase[i]) + 1 + strlen(candidates[i]) + 1;
            realPath = malloc(total);
            snprintf(realPath, total, "%s/%s", candidateBase[i], candidates[i]);
        }

        int allowed = test_filter(fileSystem, candidateBase[i], realPath, isElfBase, candidateIsDir[i]);
        if (!isElfBase) free(realPath);
        if (!allowed) continue;

        visited = realloc(visited, sizeof(char *) * (visitedCount + 1));
        visited[visitedCount++] = strdup(candidates[i]);

        if (callback(candidates[i], candidateIsDir[i], userdata) == 0) break;
    }

    for (size_t i = 0; i < candidateCount; i++) free(candidates[i]);
    free(candidates);
    free(candidateIsDir);
    free(candidateBase);
    for (size_t v = 0; v < visitedCount; v++) free(visited[v]);
    free(visited);

    return 0;
}