// Port of com.mojang.datafixers.DataFixer / DataFixerUpper (P7.3).
//
// Java resolves the rule chain once per (version, newVersion) pair and caches
// it; the port walks the registered fix list per update instead - same order,
// same predicate, no cache (a chunk save loads the fixer twice at most).

#ifndef MATTICRAFT_MOJIANG_DATAFIXERS_DATAFIXER_H
#define MATTICRAFT_MOJIANG_DATAFIXERS_DATAFIXER_H

#include "libmatti/com/mojang/datafixers/DataFix.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LIBMATTI_MC_DataFixer
{
    // Java: DataFixerBuilder.dataVersion - the DataVersion of the running game
    int dataVersion;
    // Java: DataFixerUpper.globalList - the fix list in registration order
    const LIBMATTI_MC_DataFix *fixes;
    size_t fixCount;
    // the rules the last update() applied (Java logs the "fixed" paths; the
    // port hands the caller the names so the storage layer can log them)
    const char *lastApplied[64];
    int lastAppliedCount;
} LIBMATTI_MC_DataFixer;

// Java: new DataFixerBuilder(dataVersion) + addFixer(fix) for each + build()
LIBMATTI_MC_DataFixer *LIBMATTI_MC_DataFixer_New(int dataVersion, const LIBMATTI_MC_DataFix *fixes, size_t fixCount);

void LIBMATTI_MC_DataFixer_Free(LIBMATTI_MC_DataFixer *fixer);

// Java: <T> Dynamic<T> update(DSL.TypeReference type, Dynamic<T> input, int
// version, int newVersion) - the rules with
// (fix.versionKey > makeKey(version) && fix.version <= newVersion) run in
// registration order; an already-current tag returns untouched.
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_DataFixer_UpdateToCurrentVersion(const LIBMATTI_MC_DataFixer *fixer,
                                                                   const char *type, LIBMATTI_MC_Nbt_Tag *root,
                                                                   int version);

// Java: updateToCurrentVersion(fixer, tag, version) - newVersion is the
// fixer's own dataVersion
static inline LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_DataFixer_Update(const LIBMATTI_MC_DataFixer *fixer, const char *type,
                                                                LIBMATTI_MC_Nbt_Tag *root, int version)
{
    return LIBMATTI_MC_DataFixer_UpdateToCurrentVersion(fixer, type, root, version);
}

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_MOJIANG_DATAFIXERS_DATAFIXER_H