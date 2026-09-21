#include "libmatti/net/neoforged/neoforgespi/locating/IModFileReader.h"

// Java: public @Nullable IModFile read(JarContents jar, ModFileDiscoveryAttributes attributes)
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_NEOFORGESPI_IModFileReader_Read(
    const LIBMATTI_NEOFORGESPI_IModFileReader *reader, LIBMATTI_FML_JarContents *jar,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes)
{
    return reader->read(reader->ordered.self, jar, attributes);
}
