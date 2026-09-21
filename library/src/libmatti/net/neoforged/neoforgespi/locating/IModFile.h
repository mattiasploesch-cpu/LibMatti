// Port of net.neoforged.neoforgespi.locating.IModFile.
// TODO: net.neoforged.fml.loading.moddiscovery.ModFile - the create() factories
//       build the FML implementation, which is not ported yet.
// TODO: java.util.function.Supplier / java.util.Map - getSubstitutionMap()
// TODO: net.neoforged.neoforgespi.language.IModInfo / IModFileInfo / ModFileScanData

#ifndef MATTICRAFT_NEOFORGESPI_LOCATING_IMODFILE_H
#define MATTICRAFT_NEOFORGESPI_LOCATING_IMODFILE_H

#include "libmatti/net/neoforged/fml/jarcontents/JarContents.h"
#include "libmatti/net/neoforged/fml/jarmoduleinfo/JarModuleInfo.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModFileInfo.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModInfo.h"
#include "libmatti/net/neoforged/neoforgespi/language/ModFileScanData.h"
#include "libmatti/net/neoforged/neoforgespi/locating/ModFileDiscoveryAttributes.h"
#include "libmatti/net/neoforged/neoforgespi/locating/ModFileInfoParser.h"

#include <stddef.h>

// Java: enum Type { MOD, LIBRARY, GAMELIBRARY }
typedef enum
{
    LIBMATTI_NEOFORGESPI_IModFile_Type_MOD,
    LIBMATTI_NEOFORGESPI_IModFile_Type_LIBRARY,
    LIBMATTI_NEOFORGESPI_IModFile_Type_GAMELIBRARY
} LIBMATTI_NEOFORGESPI_IModFile_Type;

// Java: public interface IModFile
typedef struct LIBMATTI_NEOFORGESPI_IModFile LIBMATTI_NEOFORGESPI_IModFile;

struct LIBMATTI_NEOFORGESPI_IModFile
{
    void *self;

    // Java: String getId()
    const char *(*getId)(void *self);
    // Java: JarContents getContents()
    LIBMATTI_FML_JarContents *(*getContents)(void *self);
    // Java: Supplier<Map<String, Object>> getSubstitutionMap() - invoked to build the map
    void *(*getSubstitutionMap)(void *self);
    // Java: Type getType()
    LIBMATTI_NEOFORGESPI_IModFile_Type (*getType)(void *self);
    // Java: Path getFilePath()
    const char *(*getFilePath)(void *self);
    // Java: List<IModInfo> getModInfos()
    LIBMATTI_NEOFORGESPI_IModInfo **(*getModInfos)(void *self, size_t *count);
    // Java: ModFileScanData getScanResult()
    LIBMATTI_NEOFORGESPI_ModFileScanData *(*getScanResult)(void *self);
    // Java: String getFileName()
    const char *(*getFileName)(void *self);
    // Java: ModFileDiscoveryAttributes getDiscoveryAttributes()
    LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *(*getDiscoveryAttributes)(void *self);
    // Java: IModFileInfo getModFileInfo()
    LIBMATTI_NEOFORGESPI_IModFileInfo *(*getModFileInfo)(void *self);
};

// Java: static IModFile create(JarContents contents, ModFileInfoParser parser)
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_NEOFORGESPI_IModFile_Create(
    LIBMATTI_FML_JarContents *contents, LIBMATTI_NEOFORGESPI_ModFileInfoParser parser, void *parserUserdata);
// Java: static IModFile create(JarContents contents, JarModuleInfo metadata, ModFileInfoParser parser)
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_NEOFORGESPI_IModFile_CreateWithMetadata(
    LIBMATTI_FML_JarContents *contents, LIBMATTI_FML_JarModuleInfo *metadata,
    LIBMATTI_NEOFORGESPI_ModFileInfoParser parser, void *parserUserdata);
// Java: static IModFile create(JarContents contents, ModFileInfoParser parser, ModFileDiscoveryAttributes attributes)
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_NEOFORGESPI_IModFile_CreateWithAttributes(
    LIBMATTI_FML_JarContents *contents, LIBMATTI_NEOFORGESPI_ModFileInfoParser parser, void *parserUserdata,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes);
// Java: static IModFile create(JarContents contents, JarModuleInfo metadata, ModFileInfoParser parser, Type type, ModFileDiscoveryAttributes attributes)
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_NEOFORGESPI_IModFile_CreateWithMetadataAndType(
    LIBMATTI_FML_JarContents *contents, LIBMATTI_FML_JarModuleInfo *metadata,
    LIBMATTI_NEOFORGESPI_ModFileInfoParser parser, void *parserUserdata, LIBMATTI_NEOFORGESPI_IModFile_Type type,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes);
// Java: static IModFile create(JarContents contents, ModFileInfoParser parser, Type type, ModFileDiscoveryAttributes attributes)
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_NEOFORGESPI_IModFile_CreateWithType(
    LIBMATTI_FML_JarContents *contents, LIBMATTI_NEOFORGESPI_ModFileInfoParser parser, void *parserUserdata,
    LIBMATTI_NEOFORGESPI_IModFile_Type type, const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes);

// Java: public String getId()
const char *LIBMATTI_NEOFORGESPI_IModFile_GetId(const LIBMATTI_NEOFORGESPI_IModFile *modFile);
// Java: public JarContents getContents()
LIBMATTI_FML_JarContents *LIBMATTI_NEOFORGESPI_IModFile_GetContents(const LIBMATTI_NEOFORGESPI_IModFile *modFile);
// Java: public Supplier<Map<String, Object>> getSubstitutionMap()
void *LIBMATTI_NEOFORGESPI_IModFile_GetSubstitutionMap(const LIBMATTI_NEOFORGESPI_IModFile *modFile);
// Java: public Type getType()
LIBMATTI_NEOFORGESPI_IModFile_Type LIBMATTI_NEOFORGESPI_IModFile_GetType(const LIBMATTI_NEOFORGESPI_IModFile *modFile);
// Java: public Path getFilePath()
const char *LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(const LIBMATTI_NEOFORGESPI_IModFile *modFile);
// Java: public List<IModInfo> getModInfos()
LIBMATTI_NEOFORGESPI_IModInfo **LIBMATTI_NEOFORGESPI_IModFile_GetModInfos(const LIBMATTI_NEOFORGESPI_IModFile *modFile,
                                                                         size_t *count);
// Java: public ModFileScanData getScanResult()
LIBMATTI_NEOFORGESPI_ModFileScanData *LIBMATTI_NEOFORGESPI_IModFile_GetScanResult(
    const LIBMATTI_NEOFORGESPI_IModFile *modFile);
// Java: public String getFileName()
const char *LIBMATTI_NEOFORGESPI_IModFile_GetFileName(const LIBMATTI_NEOFORGESPI_IModFile *modFile);
// Java: public ModFileDiscoveryAttributes getDiscoveryAttributes()
LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *LIBMATTI_NEOFORGESPI_IModFile_GetDiscoveryAttributes(
    const LIBMATTI_NEOFORGESPI_IModFile *modFile);
// Java: public IModFileInfo getModFileInfo()
LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_NEOFORGESPI_IModFile_GetModFileInfo(
    const LIBMATTI_NEOFORGESPI_IModFile *modFile);

#endif //MATTICRAFT_NEOFORGESPI_LOCATING_IMODFILE_H
