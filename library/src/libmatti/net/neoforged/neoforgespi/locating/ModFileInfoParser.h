// Port of net.neoforged.neoforgespi.locating.ModFileInfoParser.

#ifndef MATTICRAFT_NEOFORGESPI_LOCATING_MODFILEINFOPARSER_H
#define MATTICRAFT_NEOFORGESPI_LOCATING_MODFILEINFOPARSER_H

#include "libmatti/net/neoforged/neoforgespi/language/IModFileInfo.h"

// Java: @FunctionalInterface public interface ModFileInfoParser { IModFileInfo build(IModFile file); }
typedef LIBMATTI_NEOFORGESPI_IModFileInfo *(*LIBMATTI_NEOFORGESPI_ModFileInfoParser)(void *file, void *userdata);

#endif //MATTICRAFT_NEOFORGESPI_LOCATING_MODFILEINFOPARSER_H
