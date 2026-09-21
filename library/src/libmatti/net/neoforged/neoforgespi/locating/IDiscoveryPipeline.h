// Port of net.neoforged.neoforgespi.locating.IDiscoveryPipeline.

#ifndef MATTICRAFT_NEOFORGESPI_LOCATING_IDISCOVERYPIPELINE_H
#define MATTICRAFT_NEOFORGESPI_LOCATING_IDISCOVERYPIPELINE_H

#include "libmatti/net/neoforged/fml/jarcontents/JarContents.h"
#include "libmatti/net/neoforged/neoforgespi/IIssueReporting.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IncompatibleFileReporting.h"
#include "libmatti/net/neoforged/neoforgespi/locating/ModFileDiscoveryAttributes.h"

#include <stddef.h>

// Java: public interface IDiscoveryPipeline extends IIssueReporting
typedef struct LIBMATTI_NEOFORGESPI_IDiscoveryPipeline
{
    // Java: extends IIssueReporting
    LIBMATTI_NEOFORGESPI_IIssueReporting issueReporting;

    // Java: Optional<IModFile> addPath(List<Path> groupedPaths, ModFileDiscoveryAttributes attributes, IncompatibleFileReporting incompatibleFileReporting)
    int (*addPath)(void *self, const char **groupedPaths, size_t groupedPathCount,
                   const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes,
                   LIBMATTI_NEOFORGESPI_IncompatibleFileReporting incompatibleFileReporting,
                   LIBMATTI_NEOFORGESPI_IModFile **out);
    // Java: Optional<IModFile> addJarContent(JarContents contents, ModFileDiscoveryAttributes attributes, IncompatibleFileReporting incompatibleFileReporting)
    // The pipeline takes ownership of contents and closes it when it is no longer used.
    int (*addJarContent)(void *self, LIBMATTI_FML_JarContents *contents,
                         const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes,
                         LIBMATTI_NEOFORGESPI_IncompatibleFileReporting incompatibleFileReporting,
                         LIBMATTI_NEOFORGESPI_IModFile **out);
    // Java: boolean addModFile(IModFile modFile)
    int (*addModFile)(void *self, LIBMATTI_NEOFORGESPI_IModFile *modFile);
    // Java: @Nullable IModFile readModFile(JarContents jarContents, ModFileDiscoveryAttributes attributes)
    LIBMATTI_NEOFORGESPI_IModFile *(*readModFile)(void *self, LIBMATTI_FML_JarContents *jarContents,
                                                  const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes);
} LIBMATTI_NEOFORGESPI_IDiscoveryPipeline;

// Java: public default Optional<IModFile> addPath(Path path, ModFileDiscoveryAttributes attributes, IncompatibleFileReporting incompatibleFileReporting)
int LIBMATTI_NEOFORGESPI_IDiscoveryPipeline_AddPath(
    const LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline, const char *path,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes,
    LIBMATTI_NEOFORGESPI_IncompatibleFileReporting incompatibleFileReporting, LIBMATTI_NEOFORGESPI_IModFile **out);
// Java: public Optional<IModFile> addPath(List<Path> groupedPaths, ModFileDiscoveryAttributes attributes, IncompatibleFileReporting incompatibleFileReporting)
int LIBMATTI_NEOFORGESPI_IDiscoveryPipeline_AddGroupedPath(
    const LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline, const char **groupedPaths, size_t groupedPathCount,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes,
    LIBMATTI_NEOFORGESPI_IncompatibleFileReporting incompatibleFileReporting, LIBMATTI_NEOFORGESPI_IModFile **out);
// Java: public Optional<IModFile> addJarContent(JarContents contents, ModFileDiscoveryAttributes attributes, IncompatibleFileReporting incompatibleFileReporting)
int LIBMATTI_NEOFORGESPI_IDiscoveryPipeline_AddJarContent(
    const LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline, LIBMATTI_FML_JarContents *contents,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes,
    LIBMATTI_NEOFORGESPI_IncompatibleFileReporting incompatibleFileReporting, LIBMATTI_NEOFORGESPI_IModFile **out);
// Java: public boolean addModFile(IModFile modFile)
int LIBMATTI_NEOFORGESPI_IDiscoveryPipeline_AddModFile(const LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline,
                                                      LIBMATTI_NEOFORGESPI_IModFile *modFile);
// Java: public @Nullable IModFile readModFile(JarContents jarContents, ModFileDiscoveryAttributes attributes)
LIBMATTI_NEOFORGESPI_IModFile *LIBMATTI_NEOFORGESPI_IDiscoveryPipeline_ReadModFile(
    const LIBMATTI_NEOFORGESPI_IDiscoveryPipeline *pipeline, LIBMATTI_FML_JarContents *jarContents,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes);

#endif //MATTICRAFT_NEOFORGESPI_LOCATING_IDISCOVERYPIPELINE_H
