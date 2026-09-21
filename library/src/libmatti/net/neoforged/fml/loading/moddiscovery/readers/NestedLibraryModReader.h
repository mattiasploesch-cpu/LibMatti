// Port of net.neoforged.fml.loading.moddiscovery.readers.NestedLibraryModReader.

#ifndef MATTICRAFT_FML_LOADING_MODDISCOVERY_READERS_NESTEDLIBRARYMODREADER_H
#define MATTICRAFT_FML_LOADING_MODDISCOVERY_READERS_NESTEDLIBRARYMODREADER_H

#include "libmatti/net/neoforged/fml/jarcontents/JarContents.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFile.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFileReader.h"
#include "libmatti/net/neoforged/neoforgespi/locating/ModFileDiscoveryAttributes.h"

// Java: the reader singleton, used as an IModFileReader
LIBMATTI_NEOFORGESPI_IModFileReader *LIBMATTI_FML_NestedLibraryModReader_AsReader(void);
// Java: public @Nullable IModFile read(JarContents jar, ModFileDiscoveryAttributes discoveryAttributes)
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_FML_NestedLibraryModReader_Read(
    void *self, LIBMATTI_FML_JarContents *jar,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *discoveryAttributes);
// Java: public int getPriority() { return LOWEST_SYSTEM_PRIORITY; }
int LIBMATTI_FML_NestedLibraryModReader_GetPriority(void *self);
// Java: @Override public String toString() { return "nested library mod provider"; }
const char *LIBMATTI_FML_NestedLibraryModReader_ToString(void);

#endif //MATTICRAFT_FML_LOADING_MODDISCOVERY_READERS_NESTEDLIBRARYMODREADER_H
