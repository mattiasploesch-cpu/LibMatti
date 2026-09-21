// Port of net.neoforged.fml.loading.moddiscovery.readers.JarModsDotTomlModFileReader.

#ifndef MATTICRAFT_FML_LOADING_MODDISCOVERY_READERS_JARMODSDOTTOMLMODFILEREADER_H
#define MATTICRAFT_FML_LOADING_MODDISCOVERY_READERS_JARMODSDOTTOMLMODFILEREADER_H

#include "libmatti/net/neoforged/fml/jarcontents/JarContents.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModFileInfo.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFile.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFileReader.h"
#include "libmatti/net/neoforged/neoforgespi/locating/ModFileDiscoveryAttributes.h"

// Java: public static final String MODS_TOML = "META-INF/neoforge.mods.toml"
#define LIBMATTI_FML_MODS_TOML "META-INF/neoforge.mods.toml"
// Java: public static final String MANIFEST = "META-INF/MANIFEST.MF"
#define LIBMATTI_FML_MODS_TOML_MANIFEST "META-INF/MANIFEST.MF"

// Java: public static IModFile createModFile(JarContents contents, ModFileDiscoveryAttributes discoveryAttributes)
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_FML_JarModsDotTomlModFileReader_CreateModFile(
    LIBMATTI_FML_JarContents *contents, const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *discoveryAttributes);

// Java: public static IModFileInfo manifestParser(IModFile mod)
LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_FML_JarModsDotTomlModFileReader_ManifestParser(
    LIBMATTI_NEOFORGESPI_IModFile *mod);
// Java: the method reference JarModsDotTomlModFileReader::manifestParser as a ModFileInfoParser
LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_FML_JarModsDotTomlModFileReader_ManifestParserAdapter(void *file,
                                                                                                    void *userdata);

// Java: public static IModFile.Type getModType(JarContents jar) - 1 if present
int LIBMATTI_FML_JarModsDotTomlModFileReader_GetModType(LIBMATTI_FML_JarContents *jar,
                                                       LIBMATTI_NEOFORGESPI_IModFile_Type *out);

// Java: the reader singleton, used as an IModFileReader
LIBMATTI_NEOFORGESPI_IModFileReader *LIBMATTI_FML_JarModsDotTomlModFileReader_AsReader(void);
// Java: public @Nullable IModFile read(JarContents jar, ModFileDiscoveryAttributes discoveryAttributes)
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_FML_JarModsDotTomlModFileReader_Read(
    void *self, LIBMATTI_FML_JarContents *jar,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *discoveryAttributes);
// Java: @Override public String toString() { return "mod manifest"; }
const char *LIBMATTI_FML_JarModsDotTomlModFileReader_ToString(void);

#endif //MATTICRAFT_FML_LOADING_MODDISCOVERY_READERS_JARMODSDOTTOMLMODFILEREADER_H
