#include "libmatti/net/neoforged/neoforgespi/locating/IModFile.h"

#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"

// Java: static IModFile create(JarContents contents, ModFileInfoParser parser)
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_NEOFORGESPI_IModFile_Create(
    LIBMATTI_FML_JarContents *contents, LIBMATTI_NEOFORGESPI_ModFileInfoParser parser, void *parserUserdata)
{
    // Java: return new ModFile(contents, parser, ModFileDiscoveryAttributes.DEFAULT);
    LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes attributes =
        LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_Default();
    return LIBMATTI_NEOFORGESPI_IModFile_CreateWithAttributes(contents, parser, parserUserdata, &attributes);
}

// Java: static IModFile create(JarContents contents, JarModuleInfo metadata, ModFileInfoParser parser)
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_NEOFORGESPI_IModFile_CreateWithMetadata(
    LIBMATTI_FML_JarContents *contents, LIBMATTI_FML_JarModuleInfo *metadata,
    LIBMATTI_NEOFORGESPI_ModFileInfoParser parser, void *parserUserdata)
{
    // Java: return new ModFile(contents, metadata, parser, ModFileDiscoveryAttributes.DEFAULT);
    LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes attributes =
        LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_Default();
    return LIBMATTI_FML_ModFile_AsModFile(
        LIBMATTI_FML_ModFile_NewWithMetadata(contents, metadata, parser, parserUserdata, &attributes));
}

// Java: static IModFile create(JarContents contents, ModFileInfoParser parser, ModFileDiscoveryAttributes attributes)
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_NEOFORGESPI_IModFile_CreateWithAttributes(
    LIBMATTI_FML_JarContents *contents, LIBMATTI_NEOFORGESPI_ModFileInfoParser parser, void *parserUserdata,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes)
{
    // Java: return new ModFile(contents, parser, attributes);
    return LIBMATTI_FML_ModFile_AsModFile(
        LIBMATTI_FML_ModFile_New(contents, parser, parserUserdata, attributes));
}

// Java: static IModFile create(JarContents contents, JarModuleInfo metadata, ModFileInfoParser parser, Type type, ModFileDiscoveryAttributes attributes)
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_NEOFORGESPI_IModFile_CreateWithMetadataAndType(
    LIBMATTI_FML_JarContents *contents, LIBMATTI_FML_JarModuleInfo *metadata,
    LIBMATTI_NEOFORGESPI_ModFileInfoParser parser, void *parserUserdata, LIBMATTI_NEOFORGESPI_IModFile_Type type,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes)
{
    // Java: return new ModFile(contents, metadata, parser, type, attributes);
    return LIBMATTI_FML_ModFile_AsModFile(LIBMATTI_FML_ModFile_NewWithMetadataAndType(
        contents, metadata, parser, parserUserdata, type, attributes));
}

// Java: static IModFile create(JarContents contents, ModFileInfoParser parser, Type type, ModFileDiscoveryAttributes attributes)
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_NEOFORGESPI_IModFile_CreateWithType(
    LIBMATTI_FML_JarContents *contents, LIBMATTI_NEOFORGESPI_ModFileInfoParser parser, void *parserUserdata,
    LIBMATTI_NEOFORGESPI_IModFile_Type type, const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes)
{
    // Java: return new ModFile(contents, null, parser, type, attributes);
    return LIBMATTI_FML_ModFile_AsModFile(LIBMATTI_FML_ModFile_NewWithMetadataAndType(
        contents, NULL, parser, parserUserdata, type, attributes));
}

// Java: public String getId()
const char *LIBMATTI_NEOFORGESPI_IModFile_GetId(const LIBMATTI_NEOFORGESPI_IModFile *modFile)
{
    return modFile->getId(modFile->self);
}

// Java: public JarContents getContents()
LIBMATTI_FML_JarContents *LIBMATTI_NEOFORGESPI_IModFile_GetContents(const LIBMATTI_NEOFORGESPI_IModFile *modFile)
{
    return modFile->getContents(modFile->self);
}

// Java: public Supplier<Map<String, Object>> getSubstitutionMap()
void *LIBMATTI_NEOFORGESPI_IModFile_GetSubstitutionMap(const LIBMATTI_NEOFORGESPI_IModFile *modFile)
{
    return modFile->getSubstitutionMap(modFile->self);
}

// Java: public Type getType()
LIBMATTI_NEOFORGESPI_IModFile_Type LIBMATTI_NEOFORGESPI_IModFile_GetType(const LIBMATTI_NEOFORGESPI_IModFile *modFile)
{
    return modFile->getType(modFile->self);
}

// Java: public Path getFilePath()
const char *LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(const LIBMATTI_NEOFORGESPI_IModFile *modFile)
{
    return modFile->getFilePath(modFile->self);
}

// Java: public List<IModInfo> getModInfos()
LIBMATTI_NEOFORGESPI_IModInfo **LIBMATTI_NEOFORGESPI_IModFile_GetModInfos(const LIBMATTI_NEOFORGESPI_IModFile *modFile,
                                                                         size_t *count)
{
    return modFile->getModInfos(modFile->self, count);
}

// Java: public ModFileScanData getScanResult()
LIBMATTI_NEOFORGESPI_ModFileScanData *LIBMATTI_NEOFORGESPI_IModFile_GetScanResult(
    const LIBMATTI_NEOFORGESPI_IModFile *modFile)
{
    return modFile->getScanResult(modFile->self);
}

// Java: public String getFileName()
const char *LIBMATTI_NEOFORGESPI_IModFile_GetFileName(const LIBMATTI_NEOFORGESPI_IModFile *modFile)
{
    return modFile->getFileName(modFile->self);
}

// Java: public ModFileDiscoveryAttributes getDiscoveryAttributes()
LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *LIBMATTI_NEOFORGESPI_IModFile_GetDiscoveryAttributes(
    const LIBMATTI_NEOFORGESPI_IModFile *modFile)
{
    return modFile->getDiscoveryAttributes(modFile->self);
}

// Java: public IModFileInfo getModFileInfo()
LIBMATTI_NEOFORGESPI_IModFileInfo *LIBMATTI_NEOFORGESPI_IModFile_GetModFileInfo(
    const LIBMATTI_NEOFORGESPI_IModFile *modFile)
{
    return modFile->getModFileInfo(modFile->self);
}
