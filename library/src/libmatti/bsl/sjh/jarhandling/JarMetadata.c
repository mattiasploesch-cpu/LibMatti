//
// Created by administrator on 09.09.26.
//

#include "JarMetadata.h"

#include "libmatti/bsl/sjh/jarhandling/impl/ModuleJarMetadata.h"
#include "libmatti/bsl/sjh/jarhandling/impl/SimpleJarMetadata.h"
#include "libmatti/java/lang/module/ModuleDescriptor.h"

#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// pattern equivalents (Java: jdk.internal.module.ModulePath regexes in JarMetadata)
// ---------------------------------------------------------------------------

static int is_dot_or_digit(char c)
{
    return (c >= '0' && c <= '9') || c == '.';
}

// Java: DASH_VERSION = "-([.\\d]+)" - first match for the version, all matches removed for the name
static long find_dash_version(const char *s, size_t *runLength)
{
    for (size_t i = 0; s[i] != '\0'; i++)
    {
        if (s[i] == '-' && is_dot_or_digit(s[i + 1]))
        {
            size_t j = i + 1;
            while (is_dot_or_digit(s[j])) j++;
            *runLength = j - i - 1;
            return (long)i;
        }
    }
    return -1;
}

static char *remove_dash_versions(const char *fn)
{
    // Java: mat.replaceAll("") - removes every "-" followed by [.\\d]+
    size_t len = strlen(fn);
    char *out = malloc(len + 1);
    size_t o = 0;

    for (size_t i = 0; i < len;)
    {
        if (fn[i] == '-' && is_dot_or_digit(fn[i + 1]))
        {
            size_t j = i + 1;
            while (j < len && is_dot_or_digit(fn[j])) j++;
            i = j;
        }
        else
        {
            out[o++] = fn[i++];
        }
    }
    out[o] = '\0';
    return out;
}

// Java: NON_ALPHANUM = "[^A-Za-z0-9]" -> "."
static char *replace_non_alphanum(const char *s)
{
    char *out = strdup(s);

    for (char *c = out; *c != '\0'; c++)
    {
        char ch = *c;
        if (!((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')))
            *c = '.';
    }
    return out;
}

// Java: REPEATING_DOTS = "(\\.)(\\1)+" -> "."
static char *collapse_repeating_dots(const char *s)
{
    size_t len = strlen(s);
    char *out = malloc(len + 1);
    size_t o = 0;
    int previousDot = 0;

    for (size_t i = 0; i < len; i++)
    {
        if (s[i] == '.')
        {
            if (!previousDot) out[o++] = '.';
            previousDot = 1;
        }
        else
        {
            out[o++] = s[i];
            previousDot = 0;
        }
    }
    out[o] = '\0';
    return out;
}

// Java: LEADING_DOTS = "^\\." and TRAILING_DOTS = "\\.$" - in place
static void trim_dots(char *s)
{
    size_t len = strlen(s);
    size_t start = 0;

    while (start < len && s[start] == '.') start++;
    if (start > 0)
    {
        memmove(s, s + start, len - start + 1);
        len -= start;
    }
    while (len > 0 && s[len - 1] == '.') len--;
    s[len] = '\0';
}

// Java: NUMBERLIKE_PARTS = "(?<=^|\\.)([0-9]+)" -> "_$1"
static char *fix_numberlike_parts(const char *s)
{
    size_t len = strlen(s);
    char *out = malloc(len * 2 + 2);
    size_t o = 0;

    for (size_t i = 0; s[i] != '\0';)
    {
        if ((i == 0 || s[i - 1] == '.') && s[i] >= '0' && s[i] <= '9')
        {
            out[o++] = '_';
            while (s[i] >= '0' && s[i] <= '9') out[o++] = s[i++];
        }
        else
        {
            out[o++] = s[i++];
        }
    }
    out[o] = '\0';
    return out;
}

// Java: ILLEGAL_KEYWORDS + KEYWORD_PARTS = "(?<=^|\\.)(keyword)(?=\\.|$)" -> "_$1"
static const char *ILLEGAL_KEYWORDS[] = {
    "abstract", "continue", "for", "new", "switch", "assert",
    "default", "goto", "package", "synchronized", "boolean",
    "do", "if", "private", "this", "break", "double", "implements",
    "protected", "throw", "byte", "else", "import", "public", "throws",
    "case", "enum", "instanceof", "return", "transient", "catch",
    "extends", "int", "short", "try", "char", "final", "interface",
    "static", "void", "class", "finally", "long", "strictfp",
    "volatile", "const", "float", "native", "super", "while"
};

static char *fix_keyword_parts(const char *s)
{
    size_t len = strlen(s);
    char *out = malloc(len * 2 + 2);
    size_t o = 0;

    for (size_t i = 0; s[i] != '\0';)
    {
        int matched = 0;

        if (i == 0 || s[i - 1] == '.')
        {
            for (size_t k = 0; k < sizeof(ILLEGAL_KEYWORDS) / sizeof(ILLEGAL_KEYWORDS[0]); k++)
            {
                size_t keywordLength = strlen(ILLEGAL_KEYWORDS[k]);
                if (strncmp(s + i, ILLEGAL_KEYWORDS[k], keywordLength) == 0)
                {
                    char after = s[i + keywordLength];
                    if (after == '\0' || after == '.')
                    {
                        out[o++] = '_';
                        memcpy(out + o, s + i, keywordLength);
                        o += keywordLength;
                        i += keywordLength;
                        matched = 1;
                        break;
                    }
                }
            }
        }

        if (!matched) out[o++] = s[i++];
    }
    out[o] = '\0';
    return out;
}

static char *clean_module_name(const char *mn)
{
    char *nonAlphanum = replace_non_alphanum(mn);
    char *noRepeatingDots = collapse_repeating_dots(nonAlphanum);
    free(nonAlphanum);

    trim_dots(noRepeatingDots);

    char *noNumberlike = fix_numberlike_parts(noRepeatingDots);
    free(noRepeatingDots);

    char *noKeywords = fix_keyword_parts(noNumberlike);
    free(noNumberlike);

    return noKeywords;
}

// Java: MODULE_VERSION = "(?<=^|-)([\\d][.\\d]*)"
static int find_module_version(const char *s, size_t *start)
{
    for (size_t i = 0; s[i] != '\0'; i++)
    {
        if ((i == 0 || s[i - 1] == '-') && s[i] >= '0' && s[i] <= '9')
        {
            *start = i;
            return 1;
        }
    }
    return 0;
}

// Java: safeParseVersion - strip one trailing '.', '+' or '-', then Version.parse(...).toString()
// Returns NULL on failure (Java: IllegalArgumentException).
static char *safe_parse_version(const char *ver, const char *filename)
{
    (void)filename;

    size_t len = strlen(ver);
    if (len == 0) return NULL;

    char last = ver[len - 1];
    if (last == '.' || last == '+' || last == '-')
    {
        if (len == 1) return NULL;
        len--;
    }

    char *trimmed = malloc(len + 1);
    memcpy(trimmed, ver, len);
    trimmed[len] = '\0';

    LIBMATTI_JL_ModuleDescriptor_Version *version = LIBMATTI_JL_ModuleDescriptor_Version_Parse(trimmed);
    char *result = strdup(LIBMATTI_JL_ModuleDescriptor_Version_ToString(version));
    LIBMATTI_JL_ModuleDescriptor_Version_Free(version);
    free(trimmed);

    return result;
}

// ---------------------------------------------------------------------------
// computeNameAndVersion (Java: private static NameAndVersion computeNameAndVersion(Path path))
// ---------------------------------------------------------------------------

LIBMATTI_JH_NameAndVersion *LIBMATTI_JH_JarMetadata_ComputeNameAndVersion(const char *path)
{
    // detect Maven-like paths: .../artifact/version/artifact-version.jar
    char *copy = strdup(path);
    char *segments[64];
    size_t segmentCount = 0;
    char *saveptr;

    for (char *token = strtok_r(copy, "/", &saveptr); token != NULL && segmentCount < 64; token = strtok_r(NULL, "/", &saveptr))
        segments[segmentCount++] = token;

    if (segmentCount >= 3)
    {
        const char *filename = segments[segmentCount - 1];
        const char *versionMaybe = segments[segmentCount - 2];
        const char *artifactNameMaybe = segments[segmentCount - 3];

        size_t artifactLength = strlen(artifactNameMaybe);
        size_t versionLength = strlen(versionMaybe);

        // Java: filename.startsWith(artifactNameMaybe + "-" + versionMaybe)
        size_t filenameLength = strlen(filename);
        if (filenameLength >= artifactLength + 1 + versionLength &&
            strncmp(filename, artifactNameMaybe, artifactLength) == 0 &&
            filename[artifactLength] == '-' &&
            strncmp(filename + artifactLength + 1, versionMaybe, versionLength) == 0)
        {
            size_t moduleVersionStart;
            if (find_module_version(versionMaybe, &moduleVersionStart))
            {
                // Java: potential = ver.substring(mat.start())
                char *potential = strdup(versionMaybe + moduleVersionStart);
                char *version = safe_parse_version(potential, filename);
                free(potential);

                if (version == NULL)
                {
                    free(copy);
                    return NULL;
                }

                char *name = clean_module_name(artifactNameMaybe);
                LIBMATTI_JH_NameAndVersion *result = LIBMATTI_JH_NameAndVersion_Create(name, version);
                free(name);
                free(version);
                free(copy);
                return result;
            }
            else
            {
                char *name = clean_module_name(artifactNameMaybe);
                LIBMATTI_JH_NameAndVersion *result = LIBMATTI_JH_NameAndVersion_Create(name, NULL);
                free(name);
                free(copy);
                return result;
            }
        }
    }
    free(copy);

    // fallback parsing
    const char *lastSlash = strrchr(path, '/');
    const char *filename = lastSlash != NULL ? lastSlash + 1 : path;

    char *fn = strdup(filename);
    char *lastDot = strrchr(fn, '.');
    if (lastDot != NULL && lastDot != fn) *lastDot = '\0'; // strip extension

    size_t runLength;
    long dash = find_dash_version(fn, &runLength);

    if (dash >= 0)
    {
        char *potential = strdup(fn + dash + 1);
        char *version = safe_parse_version(potential, filename);
        free(potential);

        if (version == NULL)
        {
            free(fn);
            return NULL;
        }

        // Java: name = mat.replaceAll("")
        char *nameWithoutVersion = remove_dash_versions(fn);
        char *name = clean_module_name(nameWithoutVersion);
        free(nameWithoutVersion);

        LIBMATTI_JH_NameAndVersion *result = LIBMATTI_JH_NameAndVersion_Create(name, version);
        free(name);
        free(version);
        free(fn);
        return result;
    }
    else
    {
        char *name = clean_module_name(fn);
        LIBMATTI_JH_NameAndVersion *result = LIBMATTI_JH_NameAndVersion_Create(name, NULL);
        free(name);
        free(fn);
        return result;
    }
}

// ---------------------------------------------------------------------------
// from (Java: static JarMetadata from(JarContents jar))
// ---------------------------------------------------------------------------

LIBMATTI_JH_JarMetadata *LIBMATTI_JH_JarMetadata_From(LIBMATTI_JH_JarContents *contents)
{
    LIBMATTI_JN_URI *moduleInfo = NULL;

    if (LIBMATTI_JH_JarContents_FindFile(contents, "module-info.class", &moduleInfo))
    {
        LIBMATTI_JN_URI_Free(moduleInfo);
        // Java: new ModuleJarMetadata(() -> contents.getFileBytes("module-info.class"), targets)
        size_t descriptorByteCount = 0;
        unsigned char *descriptorBytes =
            LIBMATTI_JH_JarContents_OpenFile(contents, "module-info.class", &descriptorByteCount);
        if (descriptorBytes == NULL)
            return NULL;

        size_t providerCount = 0;
        LIBMATTI_JH_Provider *providers =
            LIBMATTI_JH_JarContents_GetMetaInfServices(contents, &providerCount);

        LIBMATTI_JH_ModuleJarMetadata *moduleMetadata = LIBMATTI_JH_ModuleJarMetadata_Create(
            descriptorBytes, descriptorByteCount, providers, providerCount);
        free(descriptorBytes);
        return (LIBMATTI_JH_JarMetadata *) moduleMetadata;
    }

    const char *primaryPath = LIBMATTI_JH_JarContents_GetPrimaryPath(contents);
    LIBMATTI_JH_NameAndVersion *nav = LIBMATTI_JH_JarMetadata_ComputeNameAndVersion(primaryPath);
    if (nav == NULL) return NULL;

    // take ownership of the name/version strings from the record
    char *name = nav->name;
    char *version = nav->version;
    nav->name = NULL;
    nav->version = NULL;

    // Java: manifest Automatic-Module-Name overrides the derived name
    const char *automaticModuleName = LIBMATTI_JU_Manifest_GetMainValue(LIBMATTI_JH_JarContents_GetManifest(contents), "Automatic-Module-Name");
    if (automaticModuleName != NULL)
    {
        free(name);
        name = strdup(automaticModuleName);
    }

    size_t providerCount;
    LIBMATTI_JH_Provider *providers = LIBMATTI_JH_JarContents_GetMetaInfServices(contents, &providerCount);

    LIBMATTI_JH_JarMetadata *metadata = (LIBMATTI_JH_JarMetadata *)LIBMATTI_JH_SimpleJarMetadata_Create(
        name, version, contents, providers, providerCount);

    free(name);
    free(version);
    LIBMATTI_JH_NameAndVersion_Free(nav);

    return metadata;
}