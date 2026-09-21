#include "libmatti/net/neoforged/fml/loading/EarlyServiceDiscovery.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/nio/file/Files.h"
#include "libmatti/net/neoforged/fml/jarcontents/JarContents.h"
#include "libmatti/net/neoforged/fml/jarmoduleinfo/JarModuleInfo.h"
#include "libmatti/net/neoforged/fml/loading/FMLLoader.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/readers/JarModsDotTomlModFileReader.h"
#include "libmatti/net/neoforged/fml/startup/FatalStartupException.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

// Java: the artifact is a jar; in this port a mod is an ELF shared object (.so)
static const char *MOD_SUFFIX = ".so";
static const size_t MOD_SUFFIX_LENGTH = 3;

// Java: private static final Logger LOGGER = LogManager.getLogger(EarlyServiceDiscovery.class);
static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: private static final Set<Class<?>> SERVICES = Set.of(...)
static const char *SERVICES[] = {
    "net.neoforged.neoforgespi.locating.IModFileCandidateLocator",
    "net.neoforged.neoforgespi.locating.IModFileReader",
    "net.neoforged.neoforgespi.locating.IDependencyLocator",
    "net.neoforged.neoforgespi.earlywindow.GraphicsBootstrapper",
    "net.neoforged.neoforgespi.earlywindow.ImmediateWindowProvider",
};
static const size_t SERVICE_COUNT = sizeof(SERVICES) / sizeof(SERVICES[0]);

// Java: var candidates = new HashSet<Path>()
typedef struct
{
    char **paths;
    size_t count;
} CandidateSet;

// Java: candidates.add(path)
static void addCandidate(CandidateSet *candidates, const char *path)
{
    for (size_t i = 0; i < candidates->count; i++)
        if (strcmp(candidates->paths[i], path) == 0)
            return;

    candidates->paths = realloc(candidates->paths, sizeof(char *) * (candidates->count + 1));
    candidates->paths[candidates->count] = strdup(path);
    candidates->count++;
}

typedef struct
{
    CandidateSet *candidates;
} VisitFileUserdata;

// Java: public FileVisitResult visitFile(Path file, BasicFileAttributes attrs)
static LIBMATTI_JNF_FileVisitResult visitFile(void *self, const char *file, int isRegularFile, long size)
{
    VisitFileUserdata *userdata = self;
    size_t length = strlen(file);
    // Java: if (file.getFileName().toString().endsWith(".jar") && attrs.isRegularFile() && attrs.size() > 0)
    if (length >= MOD_SUFFIX_LENGTH && strcmp(file + length - MOD_SUFFIX_LENGTH, MOD_SUFFIX) == 0 && isRegularFile &&
        size > 0)
        addCandidate(userdata->candidates, file);
    return LIBMATTI_JNF_FileVisitResult_CONTINUE;
}

// Java: private static void findClasspathServices(StartupArgs startupArgs, Set<Path> candidates)
static void findClasspathServices(LIBMATTI_FML_StartupArgs *startupArgs, CandidateSet *candidates)
{
    // Java: for (var file : startupArgs.unclaimedClassPathEntries()) if (file.isFile()) candidates.add(file.toPath());
    for (size_t i = 0; i < startupArgs->unclaimedClassPathEntryCount; i++)
        if (LIBMATTI_JNF_Files_IsRegularFile(startupArgs->unclaimedClassPathEntries[i]))
            addCandidate(candidates, startupArgs->unclaimedClassPathEntries[i]);
}

// Java: private static ModFile createEarlyServiceModFile(Path path)
static LIBMATTI_FML_ModFile *createEarlyServiceModFile(const char *path)
{
    LIBMATTI_FML_JarContents *contents = LIBMATTI_FML_JarContents_OfPath(path);

    // Java: return (ModFile) IModFile.create(contents, JarModuleInfo.from(contents), JarModsDotTomlModFileReader::manifestParser);
    LIBMATTI_FML_JarModuleInfo *moduleInfo = LIBMATTI_FML_JarModuleInfo_From(contents);
    LIBMATTI_ML_Logger_Debug(LOGGER(), NULL, "{} is the automatic module {}", path,
                             LIBMATTI_FML_JarModuleInfo_Name(moduleInfo));

    return LIBMATTI_FML_ModFile_AsConcrete(LIBMATTI_NEOFORGESPI_IModFile_CreateWithMetadata(
        contents, moduleInfo, LIBMATTI_FML_JarModsDotTomlModFileReader_ManifestParserAdapter, NULL));
}

// Java: private static ModFile getEarlyServiceModFile(Path path)
static LIBMATTI_FML_ModFile *getEarlyServiceModFile(const char *path)
{
    // Java: try (var jarFile = new JarFile(path.toFile(), false, JarFile.OPEN_READ))
    LIBMATTI_FML_JarContents *contents = LIBMATTI_FML_JarContents_OfPath(path);
    if (contents == NULL)
    {
        // Java: LOGGER.error("Failed to read Jar file {} in mods directory: {}", path, e)
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Failed to read Jar file {} in mods directory: {}", path,
                                 "UncheckedIOException");
        return NULL;
    }

    for (size_t i = 0; i < SERVICE_COUNT; i++)
    {
        char servicePath[512];
        snprintf(servicePath, sizeof(servicePath), "META-INF/services/%s", SERVICES[i]);
        // Java: if (jarFile.getEntry("META-INF/services/" + serviceClass) != null)
        LIBMATTI_JN_URI *uri = NULL;
        if (LIBMATTI_FML_JarContents_FindFile(contents, servicePath, &uri))
        {
            LIBMATTI_ML_Logger_Debug(LOGGER(), NULL, "{} contains early service {}", path, SERVICES[i]);
            LIBMATTI_JN_URI_Free(uri);
            LIBMATTI_FML_JarContents_Close(contents);
            return createEarlyServiceModFile(path);
        }
    }

    LIBMATTI_FML_JarContents_Close(contents);
    return NULL;
}

// Java: the return is List<ModFile>; the port exposes it as void** so the FMLLoader field stays opaque
// Java: public static List<ModFile> findEarlyServiceJars(StartupArgs startupArgs, Path directory)
void **LIBMATTI_FML_EarlyServiceDiscovery_FindEarlyServiceJars(LIBMATTI_FML_StartupArgs *startupArgs,
                                                               const char *directory, size_t *count)
{
    *count = 0;

    // Java: if (!Files.exists(directory)) return List.of(); // Skip if the mods dir doesn't exist yet
    if (!LIBMATTI_JNF_Files_Exists(directory))
        return NULL;

    struct timeval start;
    gettimeofday(&start, NULL);

    CandidateSet candidates = {0};
    VisitFileUserdata userdata = {&candidates};
    LIBMATTI_JNF_SimpleFileVisitor visitor = {.self = &userdata, .visitFile = visitFile};

    // Java: Files.walkFileTree(directory, Set.of(FileVisitOption.FOLLOW_LINKS), 1, ...)
    if (!LIBMATTI_JNF_Files_WalkFileTree(directory, 1, 1, &visitor))
    {
        // Java: throw new FatalStartupException("Failed to find early startup services: " + e, startupArgs, e);
        // The walk's IOException detail has no C channel; the exception carries the message and
        // the startupArgs so the entrypoint's catch clause reads it back through the FMLLoader
        // exception channel (Java: the throw unwinds out of FMLLoader.create).
        LIBMATTI_JL_Throwable *ioException = LIBMATTI_JL_Throwable_NewNamed("java.io.IOException",
                                                                            "Failed to walk the mods directory");
        LIBMATTI_FML_FMLLoader_SetLastException(
            &LIBMATTI_FML_FatalStartupException_NewWithCause("Failed to find early startup services", startupArgs,
                                                             ioException)->base);
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Failed to find early startup services");
        for (size_t i = 0; i < candidates.count; i++)
            free(candidates.paths[i]);
        free(candidates.paths);
        return NULL;
    }

    findClasspathServices(startupArgs, &candidates);

    void **earlyServiceJars = calloc(candidates.count > 0 ? candidates.count : 1, sizeof(void *));
    for (size_t i = 0; i < candidates.count; i++)
    {
        LIBMATTI_FML_ModFile *modFile = getEarlyServiceModFile(candidates.paths[i]);
        if (modFile != NULL)
            earlyServiceJars[(*count)++] = modFile;
    }

    struct timeval end;
    gettimeofday(&end, NULL);
    long elapsed = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;

    // Java: LOGGER.info("Found {} early service jars (out of {}) in {}ms", earlyServiceJars.size(), candidates.size(), elapsed);
    // log4j formats the numbers; the port's logger only substitutes strings
    char foundCount[32];
    snprintf(foundCount, sizeof(foundCount), "%zu", *count);
    char candidateCount[32];
    snprintf(candidateCount, sizeof(candidateCount), "%zu", candidates.count);
    char elapsedMillis[32];
    snprintf(elapsedMillis, sizeof(elapsedMillis), "%ld", elapsed);
    LIBMATTI_ML_Logger_Info(LOGGER(), NULL, "Found {} early service jars (out of {}) in {}ms", foundCount,
                            candidateCount, elapsedMillis);

    for (size_t i = 0; i < candidates.count; i++)
        free(candidates.paths[i]);
    free(candidates.paths);
    return earlyServiceJars;
}
