// Port of net.neoforged.fml.loading.UniqueModListBuilder.

#ifndef MATTICRAFT_FML_LOADING_UNIQUEMODLISTBUILDER_H
#define MATTICRAFT_FML_LOADING_UNIQUEMODLISTBUILDER_H

#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"

#include <stddef.h>

// Java: public record UniqueModListData(List<ModFile> modFiles, List<ModFile> discardedFiles,
//         Map<String, List<ModFile>> modFilesByFirstId)
typedef struct
{
    LIBMATTI_FML_ModFile **modFiles;
    size_t modFileCount;
    LIBMATTI_FML_ModFile **discardedFiles;
    size_t discardedFileCount;
    // Java: Map<String, List<ModFile>> modFilesByFirstId
    char **modFilesByFirstIdKeys;
    LIBMATTI_FML_ModFile ***modFilesByFirstIdValues;
    size_t *modFilesByFirstIdValueCounts;
    size_t modFilesByFirstIdCount;
} LIBMATTI_FML_UniqueModListData;

// Java: public class UniqueModListBuilder
typedef struct LIBMATTI_FML_UniqueModListBuilder LIBMATTI_FML_UniqueModListBuilder;

// Java: public UniqueModListBuilder(List<ModFile> modFiles)
LIBMATTI_FML_UniqueModListBuilder *LIBMATTI_FML_UniqueModListBuilder_New(LIBMATTI_FML_ModFile **modFiles,
                                                                         size_t modFileCount);

// Java: public UniqueModListData buildUniqueList()
// Returns NULL when duplicates were found (Java: throws ModLoadingException) and fills the issues.
LIBMATTI_FML_UniqueModListData *LIBMATTI_FML_UniqueModListBuilder_BuildUniqueList(
    LIBMATTI_FML_UniqueModListBuilder *builder, LIBMATTI_FML_ModLoadingIssue ***issues, size_t *issueCount);

void LIBMATTI_FML_UniqueModListData_Free(LIBMATTI_FML_UniqueModListData *data);

#endif //MATTICRAFT_FML_LOADING_UNIQUEMODLISTBUILDER_H
