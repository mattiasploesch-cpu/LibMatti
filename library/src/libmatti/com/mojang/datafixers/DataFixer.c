// Port of com.mojang.datafixers.DataFixerBuilder / DataFixerUpper (P7.3).

#include "libmatti/com/mojang/datafixers/DataFixer.h"
#include "libmatti/com/mojang/datafixers/DataFixUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_APPLIED 64

LIBMATTI_MC_DataFixer *LIBMATTI_MC_DataFixer_New(int dataVersion, const LIBMATTI_MC_DataFix *fixes, size_t fixCount)
{
    // Java: DataFixerBuilder.addFixer drops every fix registered above the
    // running game's DataVersion ("Ignored fix registered for version: ...")
    size_t kept = 0;
    for (size_t i = 0; i < fixCount; i++)
        if (fixes[i].version <= dataVersion)
            kept++;

    LIBMATTI_MC_DataFix *copy = NULL;
    if (kept > 0)
    {
        copy = calloc(kept, sizeof(LIBMATTI_MC_DataFix));
        if (copy == NULL)
            return NULL;
        size_t at = 0;
        for (size_t i = 0; i < fixCount; i++)
            if (fixes[i].version <= dataVersion)
                copy[at++] = fixes[i];
    }

    LIBMATTI_MC_DataFixer *fixer = calloc(1, sizeof(LIBMATTI_MC_DataFixer));
    if (fixer == NULL)
    {
        free(copy);
        return NULL;
    }
    fixer->dataVersion = dataVersion;
    fixer->fixes = copy;
    fixer->fixCount = kept;
    return fixer;
}

void LIBMATTI_MC_DataFixer_Free(LIBMATTI_MC_DataFixer *fixer)
{
    if (fixer == NULL)
        return;
    free((void *) fixer->fixes);
    free(fixer);
}

LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_DataFixer_UpdateToCurrentVersion(const LIBMATTI_MC_DataFixer *fixer,
                                                                   const char *type, LIBMATTI_MC_Nbt_Tag *root,
                                                                   int version)
{
    if (fixer == NULL || root == NULL)
        return root;

    // Java: the mutable fixer the caller holds the applied-rule list on
    LIBMATTI_MC_DataFixer *mutableFixer = (LIBMATTI_MC_DataFixer *) fixer;
    mutableFixer->lastAppliedCount = 0;

    // Java: if (version < newVersion) { ... } - a current or newer tag is
    // handed straight back
    if (version >= fixer->dataVersion)
        return root;

    // Java: getRule(version, newVersion) walks the registered rules with
//     expandedFixVersion > getLowestFixSameVersion(makeKey(version))
// which is schema bookkeeping the C port does not carry: the rule list spans
// schema versions from 704 (1.11) to 4537, and a single monotone comparison
// cannot express it - a 1.12 chunk (1343) needs BOTH the 704 rule (the block
// entity names) and the 2832 rule (the world height), one below and one above
// its own version.
//
// The port therefore runs the whole chain for every tag below the current
// DataVersion, and every rule recognises the shape it produces and leaves a
// converted tag alone. That keeps the effect for a 1343 chunk (all five
// conversions run) while a 2832 chunk passes through without a rule touching
// data that is already current.
    for (size_t i = 0; i < fixer->fixCount; i++)
    {
        const LIBMATTI_MC_DataFix *fix = &fixer->fixes[i];
        if (fix->version > fixer->dataVersion)
            continue;
        if (type != NULL && fix->type != NULL && strcmp(fix->type, type) != 0)
            continue;
        if (fix->apply == NULL)
            continue;
        if (fix->apply(root) && mutableFixer->lastAppliedCount < MAX_APPLIED)
            mutableFixer->lastApplied[mutableFixer->lastAppliedCount++] = fix->name;
    }
    return root;
}