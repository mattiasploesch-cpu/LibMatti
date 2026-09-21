#include "libmatti/net/neoforged/fml/jarmoduleinfo/ModuleDescriptorFactory.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/nio/file/Path.h"
#include "libmatti/net/neoforged/fml/jarmoduleinfo/JlsConstants.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final Logger LOGGER (Java throws InvalidModuleDescriptorException instead)
static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// ---------------------------------------------------------------------------
// String set helpers (Java: HashSet<String>)
// ---------------------------------------------------------------------------

typedef struct
{
    char **values;
    size_t count;
} StringSet;

static int setContains(const StringSet *set, const char *value)
{
    for (size_t i = 0; i < set->count; i++)
        if (strcmp(set->values[i], value) == 0)
            return 1;
    return 0;
}

static void setAdd(StringSet *set, const char *value)
{
    if (setContains(set, value))
        return;
    set->values = realloc(set->values, sizeof(char *) * (set->count + 1));
    set->values[set->count++] = strdup(value);
}

static void setFree(StringSet *set)
{
    for (size_t i = 0; i < set->count; i++)
        free(set->values[i]);
    free(set->values);
    set->values = NULL;
    set->count = 0;
}

static int isDigit(char c)
{
    return c >= '0' && c <= '9';
}

static int isAlphaNumeric(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || isDigit(c);
}

// ---------------------------------------------------------------------------
// Pattern equivalents from jdk.internal.module.ModulePath
// ---------------------------------------------------------------------------

// Java: private static final Pattern DASH_VERSION = Pattern.compile("-([.\\d]+)")
static const char *findDashVersion(const char *name)
{
    for (const char *c = name; *c != '\0'; c++)
    {
        if (*c != '-')
            continue;
        if (!(isDigit(c[1]) || c[1] == '.'))
            continue;

        const char *end = c + 1;
        while (isDigit(*end) || *end == '.')
            end++;
        return c;
    }
    return NULL;
}

// Java: private static final Pattern MODULE_VERSION = Pattern.compile("(?<=^|-)([\\d][.\\d]*)")
static const char *findModuleVersion(const char *ver)
{
    for (const char *c = ver; *c != '\0'; c++)
    {
        if (c != ver && c[-1] != '-')
            continue;
        if (!isDigit(*c))
            continue;
        return c;
    }
    return NULL;
}

// Java: private static String safeParseVersion(String ver, String filename)
static char *safeParseVersion(const char *ver, const char *filename)
{
    size_t length = strlen(ver);
    if (length == 0)
    {
        // Java: throw new IllegalArgumentException("Error parsing version info from " + filename + ": Empty Version String")
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL,
                                 "Error parsing version info from {}: Empty Version String", filename);
        return NULL;
    }

    // Java: attempt to filter out the common wrong file names
    char *trimmed = strdup(ver);
    char last = trimmed[length - 1];
    if (last == '.' || last == '+' || last == '-')
    {
        if (length == 1)
        {
            LIBMATTI_ML_Logger_Error(LOGGER(), NULL,
                                     "Error parsing version info from {}: Invalid version \"{}\"", filename, ver);
            free(trimmed);
            return NULL;
        }
        trimmed[length - 1] = '\0';
    }

    // Java: ModuleDescriptor.Version.parse(ver).toString()
    LIBMATTI_JL_ModuleDescriptor_Version *parsed = LIBMATTI_JL_ModuleDescriptor_Version_Parse(trimmed);
    free(trimmed);

    if (parsed == NULL)
    {
        // Java: catch (IllegalArgumentException e) { throw new IllegalArgumentException("Error parsing version info from " + filename + " (" + ver + "): " + e.getMessage(), e); }
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Error parsing version info from {} ({})", filename, ver);
        return NULL;
    }

    char *result = strdup(LIBMATTI_JL_ModuleDescriptor_Version_ToString(parsed));
    LIBMATTI_JL_ModuleDescriptor_Version_Free(parsed);
    return result;
}

// Java: private static final Pattern NON_ALPHANUM = Pattern.compile("[^A-Za-z0-9]") -> replaceAll(".")
static char *replaceNonAlphanumeric(const char *name)
{
    char *result = strdup(name);
    for (char *c = result; *c != '\0'; c++)
        if (!isAlphaNumeric(*c))
            *c = '.';
    return result;
}

// Java: REPEATING_DOTS = Pattern.compile("(\\.)(\\1)+") -> replaceAll(".")
static void collapseRepeatingDots(char *name)
{
    char *write = name;
    for (char *read = name; *read != '\0'; read++)
    {
        if (*read == '.' && read != name && read[-1] == '.')
            continue;
        *write++ = *read;
    }
    *write = '\0';
}

// Java: private static String cleanModuleName(String mn)
char *LIBMATTI_FML_ModuleDescriptorFactory_CleanModuleName(const char *name)
{
    char *mn = replaceNonAlphanumeric(name);

    // Java: collapse repeating dots
    collapseRepeatingDots(mn);

    // Java: drop leading dots
    char *start = mn;
    while (*start == '.')
        start++;

    // Java: drop trailing dots
    size_t length = strlen(start);
    while (length > 0 && start[length - 1] == '.')
        start[--length] = '\0';

    // Java: NUMBERLIKE_PARTS = Pattern.compile("(?<=^|\\.)([0-9]+)") -> replaceAll("_$1")
    //       KEYWORD_PARTS = Pattern.compile("(?<=^|\\.)(kw)(?=\\.|$)") -> replaceAll("_$1")
    char *result = calloc(1, strlen(start) * 2 + 1);
    size_t resultLength = 0;

    const char *segment = start;
    while (1)
    {
        const char *dot = strchr(segment, '.');
        size_t segmentLength = dot != NULL ? (size_t)(dot - segment) : strlen(segment);

        // Java: a segment made up entirely of digits gets an underscore prefix
        int allDigits = segmentLength > 0;
        for (size_t i = 0; i < segmentLength; i++)
            if (!isDigit(segment[i]))
                allDigits = 0;

        // Java: a segment that is a Java keyword gets an underscore prefix
        char *segmentCopy = malloc(segmentLength + 1);
        memcpy(segmentCopy, segment, segmentLength);
        segmentCopy[segmentLength] = '\0';
        int illegalKeyword = 0;
        for (size_t i = 0; i < LIBMATTI_FML_JlsConstants_RESERVED_KEYWORD_COUNT; i++)
            if (strcmp(segmentCopy, LIBMATTI_FML_JlsConstants_RESERVED_KEYWORDS[i]) == 0)
                illegalKeyword = 1;
        free(segmentCopy);

        if ((allDigits || illegalKeyword) && segmentLength > 0)
            result[resultLength++] = '_';

        memcpy(result + resultLength, segment, segmentLength);
        resultLength += segmentLength;

        if (dot == NULL)
            break;

        result[resultLength++] = '.';
        segment = dot + 1;
    }

    result[resultLength] = '\0';
    free(mn);
    return result;
}

// Java: static NameAndVersion computeNameAndVersion(Path path)
LIBMATTI_FML_NameAndVersion *LIBMATTI_FML_ModuleDescriptorFactory_ComputeNameAndVersion(const char *path)
{
    // Java: detect Maven-like paths
    char *versionMaybe = LIBMATTI_JNF_Path_GetParent(path);
    if (versionMaybe != NULL)
    {
        char *artifactMaybe = LIBMATTI_JNF_Path_GetParent(versionMaybe);
        if (artifactMaybe != NULL)
        {
            char *artifactNameMaybe = LIBMATTI_JNF_Path_GetFileName(artifactMaybe);
            char *fileName = LIBMATTI_JNF_Path_GetFileName(path);
            char *versionName = LIBMATTI_JNF_Path_GetFileName(versionMaybe);

            if (artifactNameMaybe != NULL && fileName != NULL && versionName != NULL)
            {
                size_t prefixLength = strlen(artifactNameMaybe) + 1 + strlen(versionName);
                char *prefix = malloc(prefixLength + 1);
                snprintf(prefix, prefixLength + 1, "%s-%s", artifactNameMaybe, versionName);

                if (strncmp(fileName, prefix, prefixLength) == 0)
                {
                    // Java: var mat = MODULE_VERSION.matcher(ver); if (mat.find()) { potential = ver.substring(mat.start()); ... }
                    const char *match = findModuleVersion(versionName);
                    LIBMATTI_FML_NameAndVersion *result = calloc(1, sizeof(LIBMATTI_FML_NameAndVersion));
                    result->name = LIBMATTI_FML_ModuleDescriptorFactory_CleanModuleName(artifactNameMaybe);

                    if (match != NULL)
                        result->version = safeParseVersion(match, fileName);
                    // Java: else return new NameAndVersion(cleanModuleName(name), null)

                    free(prefix);
                    free(fileName);
                    free(versionName);
                    free(artifactNameMaybe);
                    free(artifactMaybe);
                    free(versionMaybe);
                    return result;
                }

                free(prefix);
            }

            free(artifactNameMaybe);
            free(fileName);
            free(versionName);
        }
        free(artifactMaybe);
    }
    free(versionMaybe);

    // Java: fallback parsing
    char *fileName = LIBMATTI_JNF_Path_GetFileName(path);
    if (fileName == NULL)
        fileName = strdup(path);

    // Java: strip extension if possible
    char *dot = strrchr(fileName, '.');
    if (dot != NULL && dot != fileName)
        *dot = '\0';

    LIBMATTI_FML_NameAndVersion *result = calloc(1, sizeof(LIBMATTI_FML_NameAndVersion));

    const char *dashVersion = findDashVersion(fileName);
    if (dashVersion != NULL)
    {
        result->version = safeParseVersion(dashVersion + 1, fileName);

        // Java: var name = mat.replaceAll("") - remove every dash-version match
        char *name = strdup(fileName);
        char *dash = (char *)findDashVersion(name);
        if (dash != NULL)
            *dash = '\0';
        result->name = LIBMATTI_FML_ModuleDescriptorFactory_CleanModuleName(name);
        free(name);
    }
    else
    {
        result->name = LIBMATTI_FML_ModuleDescriptorFactory_CleanModuleName(fileName);
    }

    free(fileName);
    return result;
}

// Java: private static void parseServiceFile(String serviceName, BufferedReader reader, Set<String> packageNames, ModuleDescriptor.Builder builder)
static void parseServiceFile(const char *serviceName, const char *content, StringSet *packageNames,
                             LIBMATTI_JL_ModuleDescriptor *builder)
{
    char **providerClasses = NULL;
    size_t providerClassCount = 0;

    char *copy = strdup(content != NULL ? content : "");
    char *saveptr;
    char *line = strtok_r(copy, "\n", &saveptr);

    while (line != NULL)
    {
        // Java: strip comments
        char *comment = strchr(line, '#');
        if (comment != NULL)
            *comment = '\0';

        // Java: line = line.trim() - trim whitespace *after* removing the comment
        char *start = line;
        while (*start != '\0' && *start <= ' ')
            start++;
        char *end = start + strlen(start);
        while (end > start && *(end - 1) <= ' ')
            end--;
        *end = '\0';

        if (*start != '\0')
        {
            char *packageName = LIBMATTI_FML_JlsConstants_GetPackageName(start);
            if (!setContains(packageNames, packageName))
            {
                // Java: throw new InvalidModuleDescriptorException("Service provider file " + serviceName + " contains service that is not in this Jar file: " + line)
                LIBMATTI_ML_Logger_Error(
                    LOGGER(), NULL,
                    "Service provider file {} contains service that is not in this Jar file: {}", serviceName,
                    start);
                free(packageName);
                break;
            }
            free(packageName);

            providerClasses = realloc(providerClasses, sizeof(char *) * (providerClassCount + 1));
            providerClasses[providerClassCount++] = strdup(start);
        }

        line = strtok_r(NULL, "\n", &saveptr);
    }
    free(copy);

    if (providerClassCount > 0)
    {
        // Java: builder.provides(serviceName, providerClasses)
        LIBMATTI_JL_ModuleDescriptor_AddProvides(builder, serviceName, providerClasses, providerClassCount);
    }

    for (size_t i = 0; i < providerClassCount; i++)
        free(providerClasses[i]);
    free(providerClasses);
}

typedef struct
{
    StringSet *packageNames;
    // Java: Map<String, JarResource> serviceProviderFiles
    char **serviceNames;
    char **serviceContents;
    size_t serviceCount;
} ScanContext;

// Java: content -> if (relativePath.startsWith("META-INF/services/")) ... else if (relativePath.endsWith(".class")) ...
static void scanAutomaticModuleVisit(const char *relativePath, LIBMATTI_FML_JarResource *resource, void *userdata)
{
    ScanContext *context = userdata;

    if (strncmp(relativePath, "META-INF/services/", 18) == 0)
    {
        const char *filename = strrchr(relativePath, '/');
        filename = filename != NULL ? filename + 1 : relativePath;

        // Java: ignore files in META-INF/services/ whose filenames are not valid Java class names
        if (!LIBMATTI_FML_JlsConstants_IsTypeName(filename))
            return;

        size_t length = 0;
        unsigned char *bytes = LIBMATTI_FML_JarResource_ReadAllBytes(resource, &length);
        if (bytes == NULL)
            return;

        // Java: resource.retain() - the port copies the content of the retained resource
        char *content = calloc(1, length + 1);
        memcpy(content, bytes, length);
        free(bytes);

        context->serviceNames = realloc(context->serviceNames, sizeof(char *) * (context->serviceCount + 1));
        context->serviceContents = realloc(context->serviceContents, sizeof(char *) * (context->serviceCount + 1));
        context->serviceNames[context->serviceCount] = strdup(filename);
        context->serviceContents[context->serviceCount] = content;
        context->serviceCount++;
    }
    else if (strlen(relativePath) >= 6 && strcmp(relativePath + strlen(relativePath) - 6, ".class") == 0)
    {
        // Java: in automatic modules, only packages with .class files are considered
        const char *lastSeparator = strrchr(relativePath, '/');
        if (lastSeparator == NULL || lastSeparator == relativePath)
            return;

        size_t relativeDirLength = (size_t)(lastSeparator - relativePath);
        char *packageName = malloc(relativeDirLength + 1);
        for (size_t i = 0; i < relativeDirLength; i++)
            packageName[i] = relativePath[i] == '/' ? '.' : relativePath[i];
        packageName[relativeDirLength] = '\0';

        if (LIBMATTI_FML_JlsConstants_IsTypeName(packageName))
            setAdd(context->packageNames, packageName);

        free(packageName);
    }
}

// Java: public static void scanAutomaticModule(JarContents jar, ModuleDescriptor.Builder builder, String... excludedRootDirectories)
void LIBMATTI_FML_ModuleDescriptorFactory_ScanAutomaticModule(LIBMATTI_FML_JarContents *jar,
                                                             LIBMATTI_JL_ModuleDescriptor *builder,
                                                             const char **excludedRootDirectories,
                                                             size_t excludedRootDirectoryCount)
{
    // Java: Set<String> ignoredRootDirs = Set.of(excludedRootDirectories) - unused in the Java method too
    (void) excludedRootDirectories;
    (void) excludedRootDirectoryCount;

    StringSet packageNames = {0};
    ScanContext context = {&packageNames, NULL, NULL, 0};

    LIBMATTI_FML_JarContents_VisitContent(jar, scanAutomaticModuleVisit, &context);

    // Java: builder.packages(packageNames)
    for (size_t i = 0; i < packageNames.count; i++)
        LIBMATTI_JL_ModuleDescriptor_AddPackage(builder, packageNames.values[i]);

    for (size_t i = 0; i < context.serviceCount; i++)
    {
        parseServiceFile(context.serviceNames[i], context.serviceContents[i], &packageNames, builder);
        free(context.serviceNames[i]);
        free(context.serviceContents[i]);
    }
    free(context.serviceNames);
    free(context.serviceContents);
    setFree(&packageNames);
}

typedef struct
{
    StringSet *packageNames;
} ScanModulePackagesContext;

// Java: jar.visitContent((relativePath, resource) -> { ... })
static void scanModulePackagesVisit(const char *relativePath, LIBMATTI_FML_JarResource *resource, void *userdata)
{
    (void) resource;
    ScanModulePackagesContext *context = userdata;

    const char *lastSeparator = strrchr(relativePath, '/');
    if (lastSeparator == NULL || lastSeparator == relativePath)
        return; // Java: file is in the default package

    size_t relativeDirLength = (size_t)(lastSeparator - relativePath);
    char *packageName = malloc(relativeDirLength + 1);
    for (size_t i = 0; i < relativeDirLength; i++)
        packageName[i] = relativePath[i] == '/' ? '.' : relativePath[i];
    packageName[relativeDirLength] = '\0';

    if (LIBMATTI_FML_JlsConstants_IsTypeName(packageName))
        setAdd(context->packageNames, packageName);

    free(packageName);
}

// Java: public static Set<String> scanModulePackages(JarContents jar)
char **LIBMATTI_FML_ModuleDescriptorFactory_ScanModulePackages(LIBMATTI_FML_JarContents *jar, size_t *count)
{
    StringSet packageNames = {0};
    ScanModulePackagesContext context = {&packageNames};

    LIBMATTI_FML_JarContents_VisitContent(jar, scanModulePackagesVisit, &context);

    *count = packageNames.count;
    return packageNames.values;
}
