// Port of net.neoforged.fml.loading.moddiscovery.ModFileParser.

#ifndef MATTICRAFT_FML_LOADING_MODDISCOVERY_MODFILEPARSER_H
#define MATTICRAFT_FML_LOADING_MODDISCOVERY_MODFILEPARSER_H

#include "libmatti/net/neoforged/neoforgespi/language/IModFileInfo.h"
#include "libmatti/net/neoforged/neoforgespi/language/MavenVersionAdapter.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFile.h"
#include "libmatti/net/neoforged/neoforgespi/locating/ModFileInfoParser.h"

#include <stddef.h>

typedef struct LIBMATTI_FML_ModFile LIBMATTI_FML_ModFile;

// Java: public static IModFileInfo readModList(ModFile modFile, ModFileInfoParser parser)
LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_FML_ModFileParser_ReadModList(
    LIBMATTI_FML_ModFile *modFile, LIBMATTI_NEOFORGESPI_ModFileInfoParser parser, void *parserUserdata);

// Java: public static IModFileInfo modsTomlParser(IModFile imodFile)
LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_FML_ModFileParser_ModsTomlParser(
    LIBMATTI_NEOFORGESPI_IModFile *imodFile);
// Java: the method reference ModFileParser::modsTomlParser as a ModFileInfoParser
LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_FML_ModFileParser_ModsTomlParserAdapter(void *file, void *userdata);

// Java: public record MixinConfig(String config, List<String> requiredMods, @Nullable ArtifactVersion behaviorVersion)
typedef struct
{
    char *config;
    char **requiredMods;
    size_t requiredModCount;
    LIBMATTI_NEOFORGESPI_ArtifactVersion *behaviorVersion;
} LIBMATTI_FML_ModFileParser_MixinConfig;

// Java: protected static List<MixinConfig> getMixinConfigs(IModFileInfo modFileInfo)
LIBMATTI_FML_ModFileParser_MixinConfig *LIBMATTI_FML_ModFileParser_GetMixinConfigs(
    LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo, size_t *count);

// Java: protected static Optional<List<String>> getAccessTransformers(IModFileInfo modFileInfo) - 1 if present
int LIBMATTI_FML_ModFileParser_GetAccessTransformers(LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo, char ***out,
                                                     size_t *count);

#endif //MATTICRAFT_FML_LOADING_MODDISCOVERY_MODFILEPARSER_H
