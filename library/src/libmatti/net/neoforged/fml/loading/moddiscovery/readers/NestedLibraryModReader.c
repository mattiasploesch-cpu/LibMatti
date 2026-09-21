#include "libmatti/net/neoforged/fml/loading/moddiscovery/readers/NestedLibraryModReader.h"

#include "libmatti/net/neoforged/fml/loading/moddiscovery/readers/JarModsDotTomlModFileReader.h"

// Java: public @Nullable IModFile read(JarContents jar, ModFileDiscoveryAttributes discoveryAttributes)
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_FML_NestedLibraryModReader_Read(
    void *self, LIBMATTI_FML_JarContents *jar,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *discoveryAttributes)
{
    (void) self;
    // Java: if (discoveryAttributes.parent() != null) return IModFile.create(jar, JarModsDotTomlModFileReader::manifestParser, IModFile.Type.LIBRARY, discoveryAttributes);
    if (discoveryAttributes->parent == NULL)
        return NULL;

    LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes attributes =
        LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_WithReader(discoveryAttributes,
                                                                   LIBMATTI_FML_NestedLibraryModReader_AsReader());
    return LIBMATTI_NEOFORGESPI_IModFile_CreateWithType(
        jar, LIBMATTI_FML_JarModsDotTomlModFileReader_ManifestParserAdapter, NULL,
        LIBMATTI_NEOFORGESPI_IModFile_Type_LIBRARY, &attributes);
}

// Java: public int getPriority() { return LOWEST_SYSTEM_PRIORITY; }
int LIBMATTI_FML_NestedLibraryModReader_GetPriority(void *self)
{
    (void) self;
    return LIBMATTI_NEOFORGESPI_IOrderedProvider_LOWEST_SYSTEM_PRIORITY;
}

// Java: the reader singleton
LIBMATTI_NEOFORGESPI_IModFileReader *LIBMATTI_FML_NestedLibraryModReader_AsReader(void)
{
    static LIBMATTI_NEOFORGESPI_IModFileReader reader;
    static int initialised = 0;
    if (!initialised)
    {
        initialised = 1;
        reader.ordered.self = &reader;
        reader.ordered.getPriority = LIBMATTI_FML_NestedLibraryModReader_GetPriority;
        reader.read = LIBMATTI_FML_NestedLibraryModReader_Read;
    }
    return &reader;
}

// Java: @Override public String toString() { return "nested library mod provider"; }
const char *LIBMATTI_FML_NestedLibraryModReader_ToString(void)
{
    return "nested library mod provider";
}
