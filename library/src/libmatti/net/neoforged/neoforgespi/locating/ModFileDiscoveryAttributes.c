#include "libmatti/net/neoforged/neoforgespi/locating/ModFileDiscoveryAttributes.h"

#include "libmatti/net/neoforged/neoforgespi/locating/IModFile.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFileCandidateLocator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: public static final ModFileDiscoveryAttributes DEFAULT
LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_Default(void)
{
    LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes attributes = {0};
    return attributes;
}

// Java: public ModFileDiscoveryAttributes withParent(IModFile parent)
LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_WithParent(
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes, LIBMATTI_NEOFORGESPI_IModFile *parent)
{
    LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes result = *attributes;
    result.parent = parent;
    return result;
}

// Java: public ModFileDiscoveryAttributes withReader(IModFileReader reader)
LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_WithReader(
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes, LIBMATTI_NEOFORGESPI_IModFileReader *reader)
{
    LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes result = *attributes;
    result.reader = reader;
    return result;
}

// Java: public ModFileDiscoveryAttributes withLocator(IModFileCandidateLocator locator)
LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_WithLocator(
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes,
    LIBMATTI_NEOFORGESPI_IModFileCandidateLocator *locator)
{
    LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes result = *attributes;
    result.locator = locator;
    return result;
}

// Java: public ModFileDiscoveryAttributes withDependencyLocator(IDependencyLocator dependencyLocator)
LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_WithDependencyLocator(
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes,
    LIBMATTI_NEOFORGESPI_IDependencyLocator *dependencyLocator)
{
    LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes result = *attributes;
    result.dependencyLocator = dependencyLocator;
    return result;
}

// Java: public ModFileDiscoveryAttributes merge(ModFileDiscoveryAttributes attributes)
LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_Merge(
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *other)
{
    LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes result = *attributes;
    if (other->parent != NULL) result.parent = other->parent;
    if (other->reader != NULL) result.reader = other->reader;
    if (other->locator != NULL) result.locator = other->locator;
    if (other->dependencyLocator != NULL) result.dependencyLocator = other->dependencyLocator;
    return result;
}

// Java: @Override public String toString()
char *LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_ToString(
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes)
{
    char *result = strdup("[");
    size_t length = 1;

    if (attributes->parent != NULL)
    {
        // Java: result.append("parent: "); result.append(parent.getFilePath().getFileName());
        const char *filePath = LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(attributes->parent);
        const char *fileName = filePath != NULL ? strrchr(filePath, '/') : NULL;
        fileName = fileName != NULL ? fileName + 1 : filePath;

        const char *label = "parent: ";
        size_t addition = strlen(label) + strlen(fileName != NULL ? fileName : "null");
        result = realloc(result, length + addition + 1);
        snprintf(result + length, addition + 1, "%s%s", label, fileName != NULL ? fileName : "null");
        length += addition;
    }

    // Java: if (locator != null) { if (result.length() > 1) result.append(", "); result.append("locator: "); result.append(locator); }
    if (attributes->locator != NULL)
    {
        char *locatorString = LIBMATTI_NEOFORGESPI_IModFileCandidateLocator_ToString(attributes->locator);
        const char *label = length > 1 ? ", locator: " : "locator: ";
        size_t addition = strlen(label) + strlen(locatorString);
        result = realloc(result, length + addition + 1);
        snprintf(result + length, addition + 1, "%s%s", label, locatorString);
        length += addition;
        free(locatorString);
    }
    if (attributes->dependencyLocator != NULL)
    {
        // Java: result.append(dependencyLocator); - the ordered provider carries no toString,
        //       so the port prints the pointer identity like Java's default Object.toString.
        char identity[2 * sizeof(void *) + 3];
        snprintf(identity, sizeof(identity), "@%zx", (size_t) attributes->dependencyLocator);
        const char *label = length > 1 ? ", dependencyLocator: " : "dependencyLocator: ";
        size_t addition = strlen(label) + strlen(identity);
        result = realloc(result, length + addition + 1);
        snprintf(result + length, addition + 1, "%s%s", label, identity);
        length += addition;
    }
    if (attributes->reader != NULL)
    {
        // Java: result.append(reader); - same pointer-identity fallback as dependencyLocator
        char identity[2 * sizeof(void *) + 3];
        snprintf(identity, sizeof(identity), "@%zx", (size_t) attributes->reader);
        const char *label = length > 1 ? ", reader: " : "reader: ";
        size_t addition = strlen(label) + strlen(identity);
        result = realloc(result, length + addition + 1);
        snprintf(result + length, addition + 1, "%s%s", label, identity);
        length += addition;
    }

    result = realloc(result, length + 2);
    strcpy(result + length, "]");
    return result;
}
