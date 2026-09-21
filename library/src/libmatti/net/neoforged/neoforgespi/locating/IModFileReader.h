// Port of net.neoforged.neoforgespi.locating.IModFileReader.

#ifndef MATTICRAFT_NEOFORGESPI_LOCATING_IMODFILEREADER_H
#define MATTICRAFT_NEOFORGESPI_LOCATING_IMODFILEREADER_H

#include "libmatti/net/neoforged/fml/jarcontents/JarContents.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IOrderedProvider.h"
#include "libmatti/net/neoforged/neoforgespi/locating/ModFileDiscoveryAttributes.h"

// Java: public interface IModFileReader extends IOrderedProvider
typedef struct LIBMATTI_NEOFORGESPI_IModFileReader
{
    // Java: extends IOrderedProvider
    LIBMATTI_NEOFORGESPI_IOrderedProvider ordered;

    // Java: @Nullable IModFile read(JarContents jar, ModFileDiscoveryAttributes attributes)
    // NULL if this provider can't handle the given jar (Java: @Nullable)
    LIBMATTI_NEOFORGESPI_IModFile *(*read)(void *self, LIBMATTI_FML_JarContents *jar,
                                           const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes);
} LIBMATTI_NEOFORGESPI_IModFileReader;

// Java: public @Nullable IModFile read(JarContents jar, ModFileDiscoveryAttributes attributes)
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_NEOFORGESPI_IModFileReader_Read(
    const LIBMATTI_NEOFORGESPI_IModFileReader *reader, LIBMATTI_FML_JarContents *jar,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes);

#endif //MATTICRAFT_NEOFORGESPI_LOCATING_IMODFILEREADER_H
