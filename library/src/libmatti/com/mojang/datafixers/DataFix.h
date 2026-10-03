// Port of com.mojang.datafixers.DataFix (P7.3).
//
// Java declares a DataFix as a Schema plus a makeRule() that builds a
// TypeRewriteRule (an optic path composed with a transformation). The port
// keeps the same three pieces of information - the version the rule fires at,
// the DSL.TypeReference it targets and the compiled rule - but the rule is a
// plain function over the NBT tree: the optic path (where the rule lives in
// the tree) is resolved inside the rule, which is what the per-schema input
// type did in Java.

#ifndef MATTICRAFT_MOJIANG_DATAFIXERS_DATAFIX_H
#define MATTICRAFT_MOJIANG_DATAFIXERS_DATAFIX_H

#include "libmatti/net/minecraft/nbt/Tag.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public abstract class DataFix (Schema, boolean) - one rewrite rule
typedef struct LIBMATTI_MC_DataFix
{
    // Java: DataFixUtils.getVersion(fix.getVersionKey()) - the DataVersion the
    // rule rewrites from (the schema it was built on)
    int version;
    // Java: the schema sub-version, the low eight bits of the version key
    // (ChunkPalettedStorageFix is registered on 1451.1, so a 1451 chunk still
    // needs it - getLowestFixSameVersion() compares the full key)
    int subVersion;
    // Java: the DSL.TypeReference typeName the rule is registered for
    const char *type;
    // Java: the rule name the logger prints ("ChunkDeleteLightFix for ...")
    const char *name;
    // Java: TypeRewriteRule.seq(rule) applied to the chunk - the rule mutates
    // the tag in place and returns false when it could not be applied
    bool (*apply)(LIBMATTI_MC_Nbt_Tag *root);
} LIBMATTI_MC_DataFix;

// Java: DataFixUtils.makeKey(version, subVersion)
#define LIBMATTI_MC_DataFix_VersionKey(fix) ((((int64_t) (fix)->version) << 8) | (fix)->subVersion)

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_MOJIANG_DATAFIXERS_DATAFIX_H