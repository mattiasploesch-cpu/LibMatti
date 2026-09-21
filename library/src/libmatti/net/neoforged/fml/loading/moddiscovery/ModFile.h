// Port of net.neoforged.fml.loading.moddiscovery.ModFile.
// Java's startScan runs the scan through CompletableFuture.supplyAsync(scan, executor); the port
// keeps the scan on the calling thread but wraps the result in a CompletableFuture. The executor is
// the port's LIBMATTI_JUC_Executor (direct = immediate scan, queued = stored for a later drain).

#ifndef MATTICRAFT_FML_LOADING_MODDISCOVERY_MODFILE_H
#define MATTICRAFT_FML_LOADING_MODDISCOVERY_MODFILE_H

#include "libmatti/java/lang/module/ModuleDescriptor.h"
#include "libmatti/java/util/HashMap.h"
#include "libmatti/java/util/concurrent/CompletableFuture.h"
#include "libmatti/java/util/concurrent/Executor.h"
#include "libmatti/net/neoforged/fml/jarcontents/JarContents.h"
#include "libmatti/net/neoforged/fml/jarmoduleinfo/JarModuleInfo.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFileParser.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModFileInfo.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModLanguageLoader.h"
#include "libmatti/net/neoforged/neoforgespi/language/ModFileScanData.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFile.h"
#include "libmatti/net/neoforged/neoforgespi/locating/ModFileDiscoveryAttributes.h"
#include "libmatti/net/neoforged/neoforgespi/locating/ModFileInfoParser.h"

#include <stddef.h>

// Java: public static final Attributes.Name TYPE = new Attributes.Name("FMLModType")
#define LIBMATTI_FML_MODFILE_TYPE "FMLModType"
// Java: private static final String DEFAULT_ACCESS_TRANSFORMER = "META-INF/accesstransformer.cfg"
#define LIBMATTI_FML_MODFILE_DEFAULT_ACCESS_TRANSFORMER "META-INF/accesstransformer.cfg"

typedef struct LIBMATTI_FML_ModFileInfo LIBMATTI_FML_ModFileInfo;

// Java: @ApiStatus.Internal public class ModFile implements IModFile
struct LIBMATTI_FML_ModFile
{
    LIBMATTI_NEOFORGESPI_IModFile modFile;

    char *id;
    char *jarVersion;
    LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes discoveryAttributes;
    LIBMATTI_JU_HashMap *fileProperties; // Java: Map<String, Object> fileProperties
    LIBMATTI_NEOFORGESPI_IModLanguageLoader **loaders;
    size_t loaderCount;
    LIBMATTI_FML_JarContents *contents;
    LIBMATTI_FML_JarModuleInfo *jarModuleInfo;
    LIBMATTI_JL_ModuleDescriptor *moduleDescriptor; // Java: @Nullable private volatile ModuleDescriptor
    LIBMATTI_NEOFORGESPI_IModFile_Type modFileType;
    LIBMATTI_NEOFORGESPI_IModFileInfo *modFileInfo;
    LIBMATTI_FML_ModFileParser_MixinConfig *mixinConfigs;
    size_t mixinConfigCount;
    char **accessTransformers;
    size_t accessTransformerCount;
    // Java: CompletableFuture<ModFileScanData> futureScanResult
    LIBMATTI_JUC_CompletableFuture *futureScanResult;
    // the completed value the future carries (owned here, freed with the file)
    LIBMATTI_NEOFORGESPI_ModFileScanData *scanResult;
};

// Java: public ModFile(JarContents contents, ModFileInfoParser parser, ModFileDiscoveryAttributes attributes)
LIBMATTI_FML_ModFile *LIBMATTI_FML_ModFile_New(LIBMATTI_FML_JarContents *contents,
                                               LIBMATTI_NEOFORGESPI_ModFileInfoParser parser, void *parserUserdata,
                                               const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes);
// Java: public ModFile(JarContents contents, @Nullable JarModuleInfo metadata, ModFileInfoParser parser, ModFileDiscoveryAttributes attributes)
LIBMATTI_FML_ModFile *LIBMATTI_FML_ModFile_NewWithMetadata(
    LIBMATTI_FML_JarContents *contents, LIBMATTI_FML_JarModuleInfo *metadata,
    LIBMATTI_NEOFORGESPI_ModFileInfoParser parser, void *parserUserdata,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes);
// Java: public ModFile(JarContents contents, @Nullable JarModuleInfo metadata, ModFileInfoParser parser, Type type, ModFileDiscoveryAttributes discoveryAttributes)
LIBMATTI_FML_ModFile *LIBMATTI_FML_ModFile_NewWithMetadataAndType(
    LIBMATTI_FML_JarContents *contents, LIBMATTI_FML_JarModuleInfo *metadata,
    LIBMATTI_NEOFORGESPI_ModFileInfoParser parser, void *parserUserdata, LIBMATTI_NEOFORGESPI_IModFile_Type type,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *discoveryAttributes);

void LIBMATTI_FML_ModFile_Free(LIBMATTI_FML_ModFile *modFile);

// Java: the instance used as an IModFile
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_FML_ModFile_AsModFile(LIBMATTI_FML_ModFile *modFile);
// Java: the cast used by the parser and the sorter
LIBMATTI_FML_ModFile *LIBMATTI_FML_ModFile_AsConcrete(const LIBMATTI_NEOFORGESPI_IModFile *modFile);

// Java: public JarContents getContents()
LIBMATTI_FML_JarContents *LIBMATTI_FML_ModFile_GetContents(const LIBMATTI_FML_ModFile *modFile);
// Java: public List<IModLanguageLoader> getLoaders()
LIBMATTI_NEOFORGESPI_IModLanguageLoader **LIBMATTI_FML_ModFile_GetLoaders(const LIBMATTI_FML_ModFile *modFile,
                                                                          size_t *count);
// Java: public ModFileScanData getScanResult()
LIBMATTI_NEOFORGESPI_ModFileScanData *LIBMATTI_FML_ModFile_GetScanResult(const LIBMATTI_FML_ModFile *modFile);
// Java: public CompletionStage<ModFileScanData> startScan(Executor executor)
LIBMATTI_JUC_CompletableFuture *LIBMATTI_FML_ModFile_StartScan(LIBMATTI_FML_ModFile *modFile,
                                                               LIBMATTI_JUC_Executor *executor);
// Java: public void setFileProperties(Map<String, Object> fileProperties)
void LIBMATTI_FML_ModFile_SetFileProperties(LIBMATTI_FML_ModFile *modFile, LIBMATTI_JU_HashMap *fileProperties);
// Java: public Supplier<Map<String, Object>> getSubstitutionMap() - the map behind the supplier;
// caller frees (the returned map is a fresh view: jarVersion + fileProperties)
LIBMATTI_JU_HashMap *LIBMATTI_FML_ModFile_GetSubstitutionMap(const LIBMATTI_FML_ModFile *modFile);
// Java: public List<String> getAccessTransformers()
char **LIBMATTI_FML_ModFile_GetAccessTransformers(const LIBMATTI_FML_ModFile *modFile, size_t *count);
// Java: public List<MixinConfig> getMixinConfigs()
LIBMATTI_FML_ModFileParser_MixinConfig *LIBMATTI_FML_ModFile_GetMixinConfigs(const LIBMATTI_FML_ModFile *modFile,
                                                                            size_t *count);
// Java: public void identifyLanguage()
void LIBMATTI_FML_ModFile_IdentifyLanguage(LIBMATTI_FML_ModFile *modFile);
// Java: public ModuleDescriptor getModuleDescriptor()
LIBMATTI_JL_ModuleDescriptor *LIBMATTI_FML_ModFile_GetModuleDescriptor(LIBMATTI_FML_ModFile *modFile);
// Java: public ArtifactVersion getJarVersion()
LIBMATTI_NEOFORGESPI_ArtifactVersion *LIBMATTI_FML_ModFile_GetJarVersion(const LIBMATTI_FML_ModFile *modFile);
// Java: public IModFileInfo getModFileInfo()
LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_FML_ModFile_GetModFileInfo(const LIBMATTI_FML_ModFile *modFile);
// Java: public ModFileDiscoveryAttributes getDiscoveryAttributes() / setDiscoveryAttributes(...)
LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *LIBMATTI_FML_ModFile_GetDiscoveryAttributes(
    const LIBMATTI_FML_ModFile *modFile);
void LIBMATTI_FML_ModFile_SetDiscoveryAttributes(LIBMATTI_FML_ModFile *modFile,
                                                 const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes);
// Java: public Supplier<Map<String, Object>> getSubstitutionMap() - the value behind the key, or NULL
const char *LIBMATTI_FML_ModFile_GetSubstitutionValue(const LIBMATTI_FML_ModFile *modFile, const char *key);
// Java: public void close()
void LIBMATTI_FML_ModFile_Close(LIBMATTI_FML_ModFile *modFile);

#endif //MATTICRAFT_FML_LOADING_MODDISCOVERY_MODFILE_H
