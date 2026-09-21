//
// Created by administrator on 09.09.26.
//
// Port of cpw.mods.bootstraplauncher.BootstrapLauncher to C.
// The orchestration logic mirrors the Java file 1:1; components that live in
// external libraries in Java (jarhandling, module class loader) are the ported sjh.

#include "BootstrapLauncher.h"

#include "libmatti/bsl/sjh/cl/JarModuleFinder.h"
#include "libmatti/bsl/sjh/jarhandling/JarContents.h"
#include "libmatti/bsl/sjh/jarhandling/JarContentsBuilder.h"
#include "libmatti/bsl/sjh/jarhandling/JarMetadata.h"
#include "libmatti/bsl/sjh/jarhandling/SecureJar.h"
#include "libmatti/java/lang/module/Configuration.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// ---------------------------------------------------------------------------
// small helpers
// ---------------------------------------------------------------------------

static char *path_get_filename(const char *path)
{
    const char *slash = strrchr(path, '/');
    return strdup(slash != NULL ? slash + 1 : path);
}

static int path_exists(const char *path)
{
    return access(path, F_OK) == 0;
}

static int string_list_contains(char **list, size_t count, const char *value)
{
    for (size_t i = 0; i < count; i++)
    {
        if (strcmp(list[i], value) == 0) return 1;
    }
    return 0;
}

// Add value to the list if not already present (set semantics, like the
// HashSet/Set.copyOf usage in BootstrapLauncher.java)
static int string_set_add(char ***list, size_t *count, const char *value)
{
    if (string_list_contains(*list, *count, value)) return 0;

    char **grown = realloc(*list, sizeof(char *) * (*count + 1));
    if (grown == NULL) return 1;

    grown[*count] = strdup(value);
    if (grown[*count] == NULL) return 1;

    *list = grown;
    (*count)++;
    return 0;
}

static void string_list_free(char **list, size_t count)
{
    for (size_t i = 0; i < count; i++) free(list[i]);
    free(list);
}

// First index of name inside order, or -1 (order.indexOf in Java)
static long order_index(char **order, size_t orderCount, const char *name)
{
    for (size_t i = 0; i < orderCount; i++)
    {
        if (strcmp(order[i], name) == 0) return (long)i;
    }
    return -1;
}

// ---------------------------------------------------------------------------
// map helpers (Java: HashMap/LinkedHashMap usage in BootstrapLauncher.java)
// ---------------------------------------------------------------------------

static const char *filename_map_get(const LIBMATTI_BSL_FilenameMap *map, const char *filename)
{
    for (size_t i = 0; i < map->count; i++)
    {
        if (strcmp(map->entries[i].filename, filename) == 0) return map->entries[i].moduleNumber;
    }
    return NULL;
}

// Java: filenameMap.put(...) - a later entry for the same filename wins
static int filename_map_put(LIBMATTI_BSL_FilenameMap *map, const char *filename, const char *moduleNumber)
{
    for (size_t i = 0; i < map->count; i++)
    {
        if (strcmp(map->entries[i].filename, filename) == 0)
        {
            char *replacement = strdup(moduleNumber);
            if (replacement == NULL) return 1;

            free(map->entries[i].moduleNumber);
            map->entries[i].moduleNumber = replacement;
            return 0;
        }
    }

    LIBMATTI_BSL_FilenameMapEntry *grown = realloc(map->entries, sizeof(LIBMATTI_BSL_FilenameMapEntry) * (map->count + 1));
    if (grown == NULL) return 1;

    map->entries = grown;
    map->entries[map->count].filename = strdup(filename);
    map->entries[map->count].moduleNumber = strdup(moduleNumber);

    if (map->entries[map->count].filename == NULL || map->entries[map->count].moduleNumber == NULL) return 1;

    map->count++;
    return 0;
}

// Java: pathLookup.computeIfAbsent(path, k -> ...)
static int path_lookup_put(LIBMATTI_BSL_PathLookup *lookup, const char *path, const char *jarname)
{
    LIBMATTI_BSL_PathLookupEntry *grown = realloc(lookup->entries, sizeof(LIBMATTI_BSL_PathLookupEntry) * (lookup->count + 1));
    if (grown == NULL) return 1;

    lookup->entries = grown;
    lookup->entries[lookup->count].path = strdup(path);
    lookup->entries[lookup->count].jarname = strdup(jarname);

    if (lookup->entries[lookup->count].path == NULL || lookup->entries[lookup->count].jarname == NULL) return 1;

    lookup->count++;
    return 0;
}

// Java: mergeMap.computeIfAbsent(jarname, k -> new ArrayList<>())
static LIBMATTI_BSL_MergeMapEntry *merge_map_get_or_create(LIBMATTI_BSL_MergeMap *map, const char *name)
{
    for (size_t i = 0; i < map->count; i++)
    {
        if (strcmp(map->entries[i].name, name) == 0) return &map->entries[i];
    }

    LIBMATTI_BSL_MergeMapEntry *grown = realloc(map->entries, sizeof(LIBMATTI_BSL_MergeMapEntry) * (map->count + 1));
    if (grown == NULL) return NULL;

    map->entries = grown;
    LIBMATTI_BSL_MergeMapEntry *entry = &map->entries[map->count];

    entry->name = strdup(name);
    entry->paths = NULL;
    entry->pathCount = 0;

    if (entry->name == NULL) return NULL;

    map->count++;
    return entry;
}

// ---------------------------------------------------------------------------
// mergeModules handling (Java: private static getMergeFilenameMap)
// ---------------------------------------------------------------------------

// `mergeModules` is a semicolon-separated set of comma-separated filenames,
// where each group of filenames is combined into a single module.
// example: filename1.jar,filename2.jar;filename3.jar
int LIBMATTI_BSL_FS_GetMergeFilenameMap(LIBMATTI_BSL_FilenameMap **map)
{
    *map = NULL;

    LIBMATTI_BSL_FilenameMap *result = calloc(1, sizeof(LIBMATTI_BSL_FilenameMap));
    if (result == NULL) return 1;

    const char *mergeModules = getenv("mergeModules");

    // Java: System.getProperty("mergeModules") == null -> Map.of()
    if (mergeModules == NULL)
    {
        *map = result;
        return 0;
    }

    char *copy = strdup(mergeModules);
    if (copy == NULL)
    {
        free(result);
        return 1;
    }

    char moduleNumber[32];
    int group = 0;

    char *merge = strtok(copy, ";");
    while (merge != NULL)
    {
        snprintf(moduleNumber, sizeof(moduleNumber), "%d", group);

        char *target = strtok(merge, ",");
        while (target != NULL)
        {
            if (filename_map_put(result, target, moduleNumber) != 0)
            {
                for (size_t i = 0; i < result->count; i++)
                {
                    free(result->entries[i].filename);
                    free(result->entries[i].moduleNumber);
                }
                free(result->entries);
                free(result);
                free(copy);
                return 1;
            }
            target = strtok(NULL, ",");
        }

        group++;
        merge = strtok(NULL, ";");
    }

    free(copy);
    *map = result;
    return 0;
}

// ---------------------------------------------------------------------------
// jar metadata (Java: JarContents.of + JarMetadata.from from cpw jarhandling)
// ---------------------------------------------------------------------------

int LIBMATTI_BSL_JAR_GetModuleName(const char *path, char **name)
{
    // Java: try (var jarContent = JarContents.of(path)) {
    //          moduleName = JarMetadata.from(jarContent).name(); }
    //      catch (UncheckedIOException | IOException e) { skip; }
    *name = NULL;

    LIBMATTI_JH_JarContents *contents = LIBMATTI_JH_JarContents_Of(path);
    if (contents == NULL)
    {
        *name = strdup("");
        return 1;
    }

    LIBMATTI_JH_JarMetadata *metadata = LIBMATTI_JH_JarMetadata_From(contents);
    if (metadata == NULL)
    {
        // Java: JarMetadata.from returns NULL on a malformed module-info.class or an
        // unparseable version - the file is skipped
        LIBMATTI_JH_JarContents_Close(contents);
        *name = strdup("");
        return 1;
    }

    const char *moduleName = LIBMATTI_JH_JarMetadata_Name(metadata);
    *name = strdup(moduleName != NULL ? moduleName : "");

    LIBMATTI_JH_JarMetadata_Free(metadata);
    LIBMATTI_JH_JarContents_Close(contents);
    return 0;
}

// ---------------------------------------------------------------------------
// PackageTracker (Java: private record PackageTracker implements UnionPathFilter)
// ---------------------------------------------------------------------------

static void package_tracker_free(LIBMATTI_BSL_PackageTracker *tracker);

// Java: new PackageTracker(Set.copyOf(previousPackages), pathsArray)
// The packages are copied so the tracker owns them (Set.copyOf semantics).
static LIBMATTI_BSL_PackageTracker *package_tracker_create(char **packages, size_t packageCount,
                                                           char **paths, size_t pathCount)
{
    LIBMATTI_BSL_PackageTracker *tracker = calloc(1, sizeof(LIBMATTI_BSL_PackageTracker));
    if (tracker == NULL) return NULL;

    if (packageCount > 0)
    {
        tracker->packages = calloc(packageCount, sizeof(char *));
        if (tracker->packages == NULL)
        {
            free(tracker);
            return NULL;
        }

        for (size_t i = 0; i < packageCount; i++)
        {
            tracker->packages[i] = strdup(packages[i]);
            if (tracker->packages[i] == NULL)
            {
                package_tracker_free(tracker);
                return NULL;
            }
        }
        tracker->packageCount = packageCount;
    }

    if (pathCount > 0)
    {
        tracker->paths = calloc(pathCount, sizeof(char *));
        if (tracker->paths == NULL)
        {
            package_tracker_free(tracker);
            return NULL;
        }

        for (size_t i = 0; i < pathCount; i++)
        {
            tracker->paths[i] = strdup(paths[i]);
            if (tracker->paths[i] == NULL)
            {
                package_tracker_free(tracker);
                return NULL;
            }
        }
        tracker->pathCount = pathCount;
    }

    return tracker;
}

static void package_tracker_free(LIBMATTI_BSL_PackageTracker *tracker)
{
    if (tracker == NULL) return;

    string_list_free(tracker->packages, tracker->packageCount);
    string_list_free(tracker->paths, tracker->pathCount);
    free(tracker);
}

// Java: PackageTracker.test(String path, Path basePath). The basePath parameter
// is part of the UnionPathFilter contract but unused in the logic, like in Java.
// Returns 1 if the given path is allowed within the module, 0 if it is filtered
// out (its package is already claimed by a previous module).
static int package_tracker_test(const LIBMATTI_BSL_PackageTracker *tracker, const char *path, const char *basePath)
{
    (void)basePath; // UnionPathFilter contract, unused like in Java

    if (tracker->packageCount == 0 || strncmp(path, "META-INF/", 9) == 0)
        return 1;

    const char *lastSlash = strrchr(path, '/');

    if (lastSlash == NULL) return 1;    // resources at the root may co-exist
    if (lastSlash[1] == '\0') return 1; // directories may co-exist

    size_t len = (size_t)(lastSlash - path);
    char *package = malloc(len + 1);
    if (package == NULL) return 0;

    for (size_t i = 0; i < len; i++) package[i] = (path[i] == '/') ? '.' : path[i];
    package[len] = '\0';

    int allowed = !string_list_contains(tracker->packages, tracker->packageCount, package);

    free(package);
    return allowed;
}

// Java: the PackageTracker is passed to the JarContentsBuilder as the
// UnionPathFilter (pathFilter)
static int package_tracker_filter_test(const char *entry, const char *basePath, void *userdata)
{
    return package_tracker_test((LIBMATTI_BSL_PackageTracker *)userdata, entry, basePath);
}

// ---------------------------------------------------------------------------
// launch consumer (Java: ServiceLoader lookup for BootstrapLaunchConsumer)
// ---------------------------------------------------------------------------

static LIBMATTI_BSL_LaunchConsumer launchConsumer = NULL;

void LIBMATTI_BSL_SetLaunchConsumer(LIBMATTI_BSL_LaunchConsumer consumer)
{
    launchConsumer = consumer;
}

// ---------------------------------------------------------------------------
// public API
// ---------------------------------------------------------------------------

int LIBMATTI_BSL_RUN(int isolation, int argc, char *argv[], int debug)
{
    char **legacyClasspath = NULL;
    size_t legacyClasspathCount = 0;
    char *joined = NULL;
    LIBMATTI_BSL_BoostrapLauncher_Module **loadedModules = NULL;
    char **ignores = NULL;
    size_t ignoreCount = 0;
    char **previousPackages = NULL;
    size_t previousPackageCount = 0;
    LIBMATTI_JH_SecureJar **jars = NULL;
    size_t jarCount = 0;
    LIBMATTI_BSL_PackageTracker **trackers = NULL;
    size_t trackerCount = 0;
    LIBMATTI_BSL_PathLookup *pathLookup = NULL;
    LIBMATTI_BSL_FilenameMap *filenameMap = NULL;
    LIBMATTI_BSL_MergeMap *mergeMap = NULL;
    char **order = NULL;
    size_t orderCount = 0;
    LIBMATTI_CL_JarModuleFinder *finder = NULL;
    LIBMATTI_CL_JarModuleReference **refs = NULL;
    size_t refCount = 0;
    char **allTargets = NULL;
    size_t allTargetsCount = 0;
    LIBMATTI_JL_Configuration *configuration = NULL;
    LIBMATTI_CL_ModuleClassLoader *moduleClassLoader = NULL;

    // Load the legacy class path (loadLegacyClassPath in BootstrapLauncher.java)
    if (LIBMATTI_BSL_CLASSPATH_LoadLegacyClasspath(&legacyClasspath) != 0)
        goto cleanup;

    while (legacyClasspath[legacyClasspathCount] != NULL) legacyClasspathCount++;

    // Mirror System.setProperty("legacyClassPath", ...) for backwards
    // compatibility if somebody reads this value later on
    size_t length = 1;
    for (size_t i = 0; i < legacyClasspathCount; i++)
        length += strlen(legacyClasspath[i]) + 1;

    joined = malloc(length);
    if (joined == NULL) goto cleanup;

    joined[0] = '\0';
    for (size_t i = 0; i < legacyClasspathCount; i++)
    {
        if (i > 0) strcat(joined, ":");
        strcat(joined, legacyClasspath[i]);
    }

    if (setenv("legacyClassPath", joined, 1) != 0)
        goto cleanup;

    // Loaded modules (name -> fs location), mirroring findLoadedModules()
    if (LIBMATTI_BSL_MODULES_FindLoadedModules(&loadedModules) != 0)
        goto cleanup;

    // The ignore list exempts files whose name starts with one of the keywords
    // from being turned into modules (e.g. existing modules)
    const char *ignoreList = getenv("ignoreList");
    if (ignoreList == NULL) ignoreList = "asm,securejarhandler";

    char *copy = strdup(ignoreList);
    if (copy != NULL)
    {
        char *token = strtok(copy, ",");
        while (token != NULL)
        {
            char **grown = realloc(ignores, sizeof(char *) * (ignoreCount + 1));
            if (grown != NULL)
            {
                ignores = grown;
                ignores[ignoreCount] = strdup(token);
                if (ignores[ignoreCount] != NULL) ignoreCount++;
            }
            token = strtok(NULL, ",");
        }
        free(copy);
    }

    // Tracks all previously encountered packages so that subsequent modules
    // cannot include packages from previous modules (disallowed by the module
    // system)
    pathLookup = calloc(1, sizeof(LIBMATTI_BSL_PathLookup));
    mergeMap = calloc(1, sizeof(LIBMATTI_BSL_MergeMap));

    if (pathLookup == NULL || mergeMap == NULL)
        goto cleanup;

    // Java: getMergeFilenameMap() - the mergeModules property handling
    if (LIBMATTI_BSL_FS_GetMergeFilenameMap(&filenameMap) != 0)
        goto cleanup;

    // Main loop over the legacy class path (outer: in BootstrapLauncher.java)
    for (size_t i = 0; i < legacyClasspathCount; i++)
    {
        const char *legacy = legacyClasspath[i];
        char *filename = path_get_filename(legacy);

        // Skip files whose name starts with an ignore keyword
        int ignored = 0;
        for (size_t f = 0; f < ignoreCount; f++)
        {
            if (strncmp(filename, ignores[f], strlen(ignores[f])) == 0)
            {
                if (debug)
                    printf("bsl: file '%s' ignored because filename starts with '%s'\n", legacy, ignores[f]);
                ignored = 1;
                break;
            }
        }

        if (ignored)
        {
            free(filename);
            continue;
        }

        if (debug)
            printf("bsl: encountered path '%s'\n", legacy);

        // Skip entries that do not exist (Files.notExists in Java)
        if (!path_exists(legacy))
        {
            free(filename);
            continue;
        }

        // Compute the module name for this artifact
        // (JarContents.of + JarMetadata.from(...).name() in Java)
        char *moduleName = NULL;
        if (LIBMATTI_BSL_JAR_GetModuleName(legacy, &moduleName) != 0)
        {
            if (debug)
                printf("bsl: skipping '%s' due to an IO error\n", legacy);
            free(moduleName);
            free(filename);
            continue;
        }

        if (moduleName[0] == '\0')
        {
            free(moduleName);
            free(filename);
            continue;
        }

        // If a module of the same name is already loaded, skip it; if it is
        // loaded from a different location, that is a hard error
        const LIBMATTI_BSL_BoostrapLauncher_Module *existing = NULL;
        for (size_t m = 0; loadedModules[m] != NULL; m++)
        {
            if (strcmp(loadedModules[m]->name, moduleName) == 0)
            {
                existing = loadedModules[m];
                break;
            }
        }

        if (existing != NULL)
        {
            if (strcmp(existing->location, legacy) != 0)
            {
                fprintf(stderr,
                        "Module named %s was already on the JVMs module path loaded from %s but class-path contains it at location %s\n",
                        moduleName, existing->location, legacy);
                free(moduleName);
                free(filename);
                goto cleanup;
            }

            if (debug)
                printf("bsl: skipping '%s' because it is already loaded on boot-path as %s\n", legacy, moduleName);
            free(moduleName);
            free(filename);
            continue;
        }

        // jarname = pathLookup.computeIfAbsent(path, k -> filenameMap.getOrDefault(filename, moduleName))
        const char *jarname = NULL;

        for (size_t p = 0; p < pathLookup->count; p++)
        {
            if (strcmp(pathLookup->entries[p].path, legacy) == 0)
            {
                jarname = pathLookup->entries[p].jarname;
                break;
            }
        }

        if (jarname == NULL)
        {
            const char *mapped = filename_map_get(filenameMap, filename);
            jarname = (mapped != NULL) ? mapped : moduleName;

            if (path_lookup_put(pathLookup, legacy, jarname) != 0)
            {
                free(moduleName);
                free(filename);
                goto cleanup;
            }
        }

        // order.add(jarname)
        char **grownOrder = realloc(order, sizeof(char *) * (orderCount + 1));
        if (grownOrder == NULL)
        {
            free(moduleName);
            free(filename);
            goto cleanup;
        }
        order = grownOrder;
        order[orderCount] = strdup(jarname);
        if (order[orderCount] == NULL)
        {
            free(moduleName);
            free(filename);
            goto cleanup;
        }
        orderCount++;

        // mergeMap.computeIfAbsent(jarname, k -> new ArrayList<>()).add(path)
        LIBMATTI_BSL_MergeMapEntry *mergeEntry = merge_map_get_or_create(mergeMap, jarname);
        if (mergeEntry == NULL)
        {
            free(moduleName);
            free(filename);
            goto cleanup;
        }

        char **grownPaths = realloc(mergeEntry->paths, sizeof(char *) * (mergeEntry->pathCount + 1));
        if (grownPaths == NULL)
        {
            free(moduleName);
            free(filename);
            goto cleanup;
        }
        mergeEntry->paths = grownPaths;
        mergeEntry->paths[mergeEntry->pathCount] = strdup(legacy);
        if (mergeEntry->paths[mergeEntry->pathCount] == NULL)
        {
            free(moduleName);
            free(filename);
            goto cleanup;
        }
        mergeEntry->pathCount++;

        free(moduleName);
        free(filename);
    }

    // Merge phase: combine all paths sharing the same jarname into one module,
    // sorted by first occurrence in the order list
    // (Java: mergeMap.entrySet().stream().sorted(Comparator.comparingInt(
    //          e -> order.indexOf(e.getKey()))))
    for (size_t i = 1; i < mergeMap->count; i++)
    {
        LIBMATTI_BSL_MergeMapEntry key = mergeMap->entries[i];
        long keyIndex = order_index(order, orderCount, key.name);
        size_t j = i;

        while (j > 0 && order_index(order, orderCount, mergeMap->entries[j - 1].name) > keyIndex)
        {
            mergeMap->entries[j] = mergeMap->entries[j - 1];
            j--;
        }
        mergeMap->entries[j] = key;
    }

    for (size_t i = 0; i < mergeMap->count; i++)
    {
        LIBMATTI_BSL_MergeMapEntry *entry = &mergeMap->entries[i];

        // Skip empty paths (a single path that does not exist)
        if (entry->pathCount == 1 && !path_exists(entry->paths[0]))
            continue;

        // Java: new PackageTracker(Set.copyOf(previousPackages), pathsArray)
        LIBMATTI_BSL_PackageTracker *tracker = package_tracker_create(previousPackages, previousPackageCount,
                                                                      entry->paths, entry->pathCount);
        if (tracker == NULL) goto cleanup;

        // Java: new JarContentsBuilder().paths(pathsArray).pathFilter(tracker).build()
        LIBMATTI_UNION_UnionPathFilter filter = {package_tracker_filter_test, tracker};
        LIBMATTI_JH_JarContentsBuilder *builder = LIBMATTI_JH_JarContentsBuilder_New();
        if (builder == NULL)
        {
            package_tracker_free(tracker);
            goto cleanup;
        }
        LIBMATTI_JH_JarContentsBuilder_Paths(builder, (const char **)entry->paths, entry->pathCount);
        LIBMATTI_JH_JarContentsBuilder_PathFilter(builder, &filter);
        LIBMATTI_JH_JarContents *contents = LIBMATTI_JH_JarContentsBuilder_Build(builder);
        LIBMATTI_JH_JarContentsBuilder_Free(builder);

        if (contents == NULL)
        {
            package_tracker_free(tracker);
            goto cleanup;
        }

        // Java: SecureJar.from(jarContents)
        LIBMATTI_JH_SecureJar *jar = LIBMATTI_JH_SecureJar_FromContents(contents);
        if (jar == NULL)
        {
            // Java: JarMetadata.from returned NULL (malformed module descriptor)
            LIBMATTI_JH_JarContents_Close(contents);
            package_tracker_free(tracker);
            goto cleanup;
        }

        // Java: jar.moduleDataProvider().descriptor().packages()
        // The descriptor is owned and cached by the metadata (LazyJarMetadata);
        // it must not be freed here.
        LIBMATTI_JH_ModuleDataProvider *provider = LIBMATTI_JH_SecureJar_ModuleDataProvider(jar);
        LIBMATTI_JL_ModuleDescriptor *descriptor = provider->descriptor(provider);
        size_t packageCount = 0;
        char **packages = LIBMATTI_JL_ModuleDescriptor_Packages(descriptor, &packageCount);

        if (debug)
        {
            printf("bsl: the following paths are merged together in module %s\n", entry->name);
            for (size_t p = 0; p < entry->pathCount; p++)
                printf("bsl:    %s\n", entry->paths[p]);
            printf("bsl: list of packages for module %s\n", entry->name);
            for (size_t p = 0; p < packageCount; p++)
                printf("bsl:    %s\n", packages[p]);
        }

        // previousPackages.addAll(packages)
        for (size_t p = 0; p < packageCount; p++)
        {
            if (string_set_add(&previousPackages, &previousPackageCount, packages[p]) != 0)
            {
                package_tracker_free(tracker);
                goto cleanup;
            }
        }

        // Java: the tracker stays alive as the filesystem's pathFilter; keep it
        // until cleanup (the jar's contents reference it via the filesystem)
        LIBMATTI_BSL_PackageTracker **grownTrackers = realloc(trackers, sizeof(LIBMATTI_BSL_PackageTracker *) * (trackerCount + 1));
        if (grownTrackers == NULL)
        {
            package_tracker_free(tracker);
            goto cleanup;
        }
        trackers = grownTrackers;
        trackers[trackerCount++] = tracker;

        LIBMATTI_JH_SecureJar **grownJars = realloc(jars, sizeof(LIBMATTI_JH_SecureJar *) * (jarCount + 1));
        if (grownJars == NULL) goto cleanup;
        jars = grownJars;
        jars[jarCount++] = jar;
    }

    // Java: var jarModuleFinder = JarModuleFinder.of(secureJarsArray);
    finder = LIBMATTI_CL_JarModuleFinder_Of(jars, jarCount);
    if (finder == NULL) goto cleanup;

    refs = LIBMATTI_CL_JarModuleFinder_FindAll(finder, &refCount);

    // Java: allTargets = Arrays.stream(secureJarsArray).map(SecureJar::name).toList()
    for (size_t i = 0; i < jarCount; i++)
    {
        char **grown = realloc(allTargets, sizeof(char *) * (allTargetsCount + 1));
        if (grown == NULL) goto cleanup;
        allTargets = grown;
        allTargets[allTargetsCount] = strdup(LIBMATTI_JH_SecureJar_Name(jars[i]));
        if (allTargets[allTargetsCount] == NULL) goto cleanup;
        allTargetsCount++;
    }

    // Java: bootModuleConfiguration.resolveAndBind(jarModuleFinder, ModuleFinder.ofSystem(), allTargets)
    // The binding pass pulls providers of used services into the graph. The 'after'
    // finder (ModuleFinder.ofSystem(), JVM boot modules) is external and omitted.
    configuration = LIBMATTI_JL_Configuration_ResolveAndBind((LIBMATTI_JL_ModuleReference **)refs, refCount,
                                                             (const char **)allTargets, allTargetsCount);
    if (configuration == NULL) goto cleanup;

    // Java: new ModuleClassLoader("MC-BOOTSTRAP", bootstrapConfiguration, List.of(ModuleLayer.boot()), parentLoader)
    // The parentLoader (controlled by the isolation flag) feeds the JVM-external
    // fallback class loader and is therefore omitted here.
    (void)isolation;
    moduleClassLoader = LIBMATTI_CL_ModuleClassLoader_New("MC-BOOTSTRAP", configuration);
    if (moduleClassLoader == NULL) goto cleanup;

    // Java: Thread.currentThread().setContextClassLoader(moduleClassLoader) - no C equivalent

    // Java: ServiceLoader.load(layer.layer(), Consumer.class).stream().findFirst().orElseThrow().get().accept(args)
    // The mod-launcher BootstrapLaunchConsumer is external; the host registers the
    // C equivalent via LIBMATTI_BSL_SetLaunchConsumer.
    if (launchConsumer != NULL)
        launchConsumer(moduleClassLoader, argc, argv);
    else if (debug)
        printf("bsl: no launch consumer registered; resolved %zu module(s)\n", refCount);

cleanup:
    if (loadedModules != NULL)
    {
        for (size_t i = 0; loadedModules[i] != NULL; i++)
        {
            free(loadedModules[i]->name);
            free(loadedModules[i]->location);
            free(loadedModules[i]);
        }
        free(loadedModules);
    }

    string_list_free(ignores, ignoreCount);
    string_list_free(previousPackages, previousPackageCount);
    string_list_free(order, orderCount);
    string_list_free(legacyClasspath, legacyClasspathCount);

    if (moduleClassLoader != NULL) LIBMATTI_CL_ModuleClassLoader_Free(moduleClassLoader);
    if (configuration != NULL) LIBMATTI_JL_Configuration_Free(configuration);
    string_list_free(allTargets, allTargetsCount);
    free(refs);
    if (finder != NULL) LIBMATTI_CL_JarModuleFinder_Free(finder);

    if (jars != NULL)
    {
        for (size_t i = 0; i < jarCount; i++)
            LIBMATTI_JH_SecureJar_Close(jars[i]);
        free(jars);
    }

    if (trackers != NULL)
    {
        for (size_t i = 0; i < trackerCount; i++)
            package_tracker_free(trackers[i]);
        free(trackers);
    }

    if (pathLookup != NULL)
    {
        for (size_t i = 0; i < pathLookup->count; i++)
        {
            free(pathLookup->entries[i].path);
            free(pathLookup->entries[i].jarname);
        }
        free(pathLookup->entries);
        free(pathLookup);
    }

    if (filenameMap != NULL)
    {
        for (size_t i = 0; i < filenameMap->count; i++)
        {
            free(filenameMap->entries[i].filename);
            free(filenameMap->entries[i].moduleNumber);
        }
        free(filenameMap->entries);
        free(filenameMap);
    }

    if (mergeMap != NULL)
    {
        for (size_t i = 0; i < mergeMap->count; i++)
        {
            free(mergeMap->entries[i].name);
            string_list_free(mergeMap->entries[i].paths, mergeMap->entries[i].pathCount);
        }
        free(mergeMap->entries);
        free(mergeMap);
    }

    free(joined);
    return 0;
}

int LIBMATTI_BSL_MODULES_FindLoadedModules(LIBMATTI_BSL_BoostrapLauncher_Module ***modules)
{
    size_t count = 0;

    // Java: ModuleLayer.boot().configuration().modules() - the modules already on the JVM's
    // boot module path. The port's launcher runs before any boot layer exists (the process
    // IS the boot), so the set is empty exactly like a plain -cp launch.
    *modules = calloc(count + 1, sizeof(LIBMATTI_BSL_BoostrapLauncher_Module *));
    if (*modules == NULL) return 1;

    return 0;
}

int LIBMATTI_BSL_CLASSPATH_LoadLegacyClasspath(char ***classpath)
{
    *classpath = NULL;

    // legacyClassPath.file -> one path per line
    // (Java: System.getProperty("legacyClassPath.file"))
    const char *legacyCpPath = getenv("legacyClassPath.file");

    if (legacyCpPath != NULL)
    {
        FILE *file = fopen(legacyCpPath, "r");

        if (file == NULL) return 1;

        size_t count = 0;
        char buffer[4096];

        while (fgets(buffer, sizeof(buffer), file) != NULL) count++;
        rewind(file);

        char **entries = calloc(count + 1, sizeof(char *));
        if (entries == NULL)
        {
            fclose(file);
            return 1;
        }

        size_t i = 0;
        while (fgets(buffer, sizeof(buffer), file) != NULL)
        {
            buffer[strcspn(buffer, "\r\n")] = '\0';

            // Skip empty lines; Java's Files.readAllLines does not produce them
            if (buffer[0] == '\0') continue;

            entries[i] = strdup(buffer);
            if (entries[i] == NULL)
            {
                for (size_t j = 0; j < i; j++) free(entries[j]);
                free(entries);
                fclose(file);
                return 1;
            }
            i++;
        }

        fclose(file);
        entries[i] = NULL;
        *classpath = entries;
        return 0;
    }

    // Fallback: legacyClassPath, split by the path separator
    // (Java: System.getProperty("legacyClassPath", ...) split by File.pathSeparator)
    const char *legacyClasspath = getenv("legacyClassPath");

    if (legacyClasspath == NULL || legacyClasspath[0] == '\0')
    {
        char **empty = calloc(1, sizeof(char *));
        if (empty == NULL) return 1;

        *classpath = empty;
        return 0;
    }

    char *copy = strdup(legacyClasspath);
    if (copy == NULL) return 1;

    char **entries = NULL;
    size_t count = 0;
    char *token = strtok(copy, ":");

    while (token != NULL)
    {
        char **grown = realloc(entries, sizeof(char *) * (count + 1));
        if (grown == NULL)
        {
            string_list_free(entries, count);
            free(copy);
            return 1;
        }

        entries = grown;
        entries[count] = strdup(token);
        if (entries[count] == NULL)
        {
            string_list_free(entries, count);
            free(copy);
            return 1;
        }
        count++;
        token = strtok(NULL, ":");
    }

    char **grown = realloc(entries, sizeof(char *) * (count + 1));
    if (grown == NULL)
    {
        string_list_free(entries, count);
        free(copy);
        return 1;
    }

    entries = grown;
    entries[count] = NULL;

    free(copy);
    *classpath = entries;
    return 0;
}