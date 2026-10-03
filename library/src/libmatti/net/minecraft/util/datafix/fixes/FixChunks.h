// The chunk fixers' optic-path helper (P7.3).
//
// Java addresses the fix targets through the per-schema input type: the
// pre-1.18 chunk type nests the payload under a "Level" compound
// (DSL.fieldFinder("Level", type.findFieldType("Level"))), while the 1.18
// chunk type is flat and the rules use type.findField("sections") straight on
// the root. The port resolves the same distinction structurally: the rule
// picks whichever shape the tag actually has.

#ifndef MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKS_H
#define MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKS_H

#include "libmatti/net/minecraft/nbt/CompoundTag.h"
#include "libmatti/net/minecraft/nbt/ListTag.h"
#include "libmatti/net/minecraft/nbt/Tag.h"

// Java: DSL.fieldFinder("Level", type.findFieldType("Level")) - the chunk
// payload compound, or the root itself for the flat (1.18+) chunk shape
LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_FixChunks_LevelOf(LIBMATTI_MC_Nbt_CompoundTag *root);

// Java: type.findField("Sections") / type.findField("sections") - the section
// list of the current chunk shape (1.13 wrote "Sections", 1.18 "sections")
LIBMATTI_MC_Nbt_ListTag *LIBMATTI_MC_FixChunks_SectionsOf(LIBMATTI_MC_Nbt_CompoundTag *root);

#endif // MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXCHUNKS_H