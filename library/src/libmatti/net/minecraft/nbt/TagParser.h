// Port of net.minecraft.nbt.TagParser (the SNBT text parser). Java runs the
// SnbtGrammar packrat parser; the C port is a direct recursive-descent of the
// same grammar: literals (integers with suffixes/0x/0b/underscores, floats with
// f/d suffixes), quoted strings with the escape table, unquoted strings, bare
// true/false, lists [..], typed arrays [B;..]/[I;..]/[L;..] and compounds {..}.

#ifndef MATTICRAFT_NET_MINECRAFT_NBT_TAGPARSER_H
#define MATTICRAFT_NET_MINECRAFT_NBT_TAGPARSER_H

#include "libmatti/net/minecraft/nbt/CompoundTag.h"

// Java: public static CompoundTag parseCompoundFully(String) - aborts with the parser
// error (Java throws CommandSyntaxException); trailing data is an error
LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_Nbt_TagParser_ParseCompoundFully(const char *input);

// Java: public static Tag parseTagFully(String) (the grammar root without the compound cast)
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_TagParser_ParseTagFully(const char *input);

// the last parser error, NULL when the last parse succeeded (read-only)
const char *LIBMATTI_MC_Nbt_TagParser_LastError(void);

#endif //MATTICRAFT_NET_MINECRAFT_NBT_TAGPARSER_H
