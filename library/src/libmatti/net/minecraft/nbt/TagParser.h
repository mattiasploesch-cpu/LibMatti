






#ifndef MATTICRAFT_NET_MINECRAFT_NBT_TAGPARSER_H
#define MATTICRAFT_NET_MINECRAFT_NBT_TAGPARSER_H

#include "libmatti/net/minecraft/nbt/CompoundTag.h"

// Java: public static CompoundTag parseCompoundFully(String) - aborts with the parser
// error (Java throws CommandSyntaxException); trailing data is an error
LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_Nbt_TagParser_ParseCompoundFully(const char *input);

// Java: public static Tag parseTagFully(String) (the grammar root without the compound cast)
LIBMATTI_MC_Nbt_Tag *LIBMATTI_MC_Nbt_TagParser_ParseTagFully(const char *input);



const char *LIBMATTI_MC_Nbt_TagParser_LastError(void);

#endif
