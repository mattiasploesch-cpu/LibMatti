// Port of net.neoforged.neoforgespi.transformation.ProcessorName.

#include "libmatti/net/neoforged/neoforgespi/transformation/ProcessorName.h"

#include "libmatti/cpw/modlauncher/LogManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final Pattern NAMESPACE_PATTERN = Pattern.compile("^[a-z0-9_.-]+$")
static int namespace_matches(const char *s)
{
    if (*s == '\0') return 0;
    for (; *s != '\0'; s++)
    {
        char c = *s;
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '.' || c == '-') continue;
        return 0;
    }
    return 1;
}

// Java: private static final Pattern PATH_PATTERN = Pattern.compile("^[a-z0-9_./-]+$")
static int path_matches(const char *s)
{
    if (*s == '\0') return 0;
    for (; *s != '\0'; s++)
    {
        char c = *s;
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '.' || c == '/' || c == '-') continue;
        return 0;
    }
    return 1;
}

LIBMATTI_NEOFORGESPI_ProcessorName *LIBMATTI_NEOFORGESPI_ProcessorName_New(const char *namespace, const char *path)
{
    // Java: Objects.requireNonNull(namespace, "namespace")
    if (namespace == NULL)
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), NULL, "NullPointerException: namespace");
        return NULL;
    }
    // Java: Objects.requireNonNull(path, "path")
    if (path == NULL)
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), NULL, "NullPointerException: path");
        return NULL;
    }
    // Java: if (!NAMESPACE_PATTERN.asMatchPredicate().test(namespace)) throw new IllegalArgumentException(...)
    if (!namespace_matches(namespace))
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), NULL,
                                 "Invalid namespace for processor name: {}", namespace);
        return NULL;
    }
    // Java: if (!PATH_PATTERN.asMatchPredicate().test(path)) throw new IllegalArgumentException(...)
    if (!path_matches(path))
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), NULL,
                                 "Invalid path for processor name: {}", path);
        return NULL;
    }

    LIBMATTI_NEOFORGESPI_ProcessorName *name = calloc(1, sizeof(LIBMATTI_NEOFORGESPI_ProcessorName));
    name->namespace = strdup(namespace);
    name->path = strdup(path);
    return name;
}

LIBMATTI_NEOFORGESPI_ProcessorName *LIBMATTI_NEOFORGESPI_ProcessorName_Parse(const char *fullName)
{
    // Java: var parts = fullName.split(":", 2); if (parts.length != 2) throw new IllegalArgumentException(...)
    const char *separator = strchr(fullName, ':');
    if (separator == NULL)
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), NULL, "Invalid processor name: {}", fullName);
        return NULL;
    }

    char *namespace = strndup(fullName, (size_t) (separator - fullName));
    LIBMATTI_NEOFORGESPI_ProcessorName *name =
        LIBMATTI_NEOFORGESPI_ProcessorName_New(namespace, separator + 1);
    free(namespace);
    return name;
}

void LIBMATTI_NEOFORGESPI_ProcessorName_Free(LIBMATTI_NEOFORGESPI_ProcessorName *name)
{
    if (name == NULL) return;
    free(name->namespace);
    free(name->path);
    free(name);
}

char *LIBMATTI_NEOFORGESPI_ProcessorName_ToString(const LIBMATTI_NEOFORGESPI_ProcessorName *name)
{
    // Java: return namespace + ":" + path
    size_t length = strlen(name->namespace) + 1 + strlen(name->path);
    char *result = malloc(length + 1);
    snprintf(result, length + 1, "%s:%s", name->namespace, name->path);
    return result;
}

int LIBMATTI_NEOFORGESPI_ProcessorName_CompareTo(const LIBMATTI_NEOFORGESPI_ProcessorName *name,
                                                 const LIBMATTI_NEOFORGESPI_ProcessorName *other)
{
    // Java: Comparator.comparing(ProcessorName::namespace).thenComparing(ProcessorName::path)
    int namespaceResult = strcmp(name->namespace, other->namespace);
    if (namespaceResult != 0) return namespaceResult;
    return strcmp(name->path, other->path);
}
