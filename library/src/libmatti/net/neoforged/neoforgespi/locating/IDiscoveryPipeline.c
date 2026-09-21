#include "libmatti/net/neoforged/neoforgespi/locating/IDiscoveryPipeline.h"

#include "libmatti/net/neoforged/neoforgespi/locating/IModFile.h"

// Java: public default Optional<IModFile> addPath(Path path, ModFileDiscoveryAttributes attributes, IncompatibleFileReporting incompatibleFileReporting)
int LIBMATTI_NEOFORGESPI_IDiscoveryPipeline_AddPath(
    const LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline, const char *path,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes,
    LIBMATTI_NEOFORGESPI_IncompatibleFileReporting incompatibleFileReporting, LIBMATTI_NEOFORGESPI_IModFile **out)
{
    // Java: return addPath(List.of(path), attributes, incompatibleFileReporting);
    return pipeline->addPath(pipeline->issueReporting.self, &path, 1, attributes, incompatibleFileReporting, out);
}

// Java: public Optional<IModFile> addPath(List<Path> groupedPaths, ModFileDiscoveryAttributes attributes, IncompatibleFileReporting incompatibleFileReporting)
int LIBMATTI_NEOFORGESPI_IDiscoveryPipeline_AddGroupedPath(
    const LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline, const char **groupedPaths, size_t groupedPathCount,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes,
    LIBMATTI_NEOFORGESPI_IncompatibleFileReporting incompatibleFileReporting, LIBMATTI_NEOFORGESPI_IModFile **out)
{
    return pipeline->addPath(pipeline->issueReporting.self, groupedPaths, groupedPathCount, attributes,
                             incompatibleFileReporting, out);
}

// Java: public Optional<IModFile> addJarContent(JarContents contents, ModFileDiscoveryAttributes attributes, IncompatibleFileReporting incompatibleFileReporting)
int LIBMATTI_NEOFORGESPI_IDiscoveryPipeline_AddJarContent(
    const LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline, LIBMATTI_FML_JarContents *contents,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes,
    LIBMATTI_NEOFORGESPI_IncompatibleFileReporting incompatibleFileReporting, LIBMATTI_NEOFORGESPI_IModFile **out)
{
    return pipeline->addJarContent(pipeline->issueReporting.self, contents, attributes, incompatibleFileReporting, out);
}

// Java: public boolean addModFile(IModFile modFile)
int LIBMATTI_NEOFORGESPI_IDiscoveryPipeline_AddModFile(const LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline,
                                                      LIBMATTI_NEOFORGESPI_IModFile *modFile)
{
    return pipeline->addModFile(pipeline->issueReporting.self, modFile);
}

// Java: public @Nullable IModFile readModFile(JarContents jarContents, ModFileDiscoveryAttributes attributes)
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_NEOFORGESPI_IDiscoveryPipeline_ReadModFile(
    const LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline, LIBMATTI_FML_JarContents *jarContents,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes)
{
    return pipeline->readModFile(pipeline->issueReporting.self, jarContents, attributes);
}
