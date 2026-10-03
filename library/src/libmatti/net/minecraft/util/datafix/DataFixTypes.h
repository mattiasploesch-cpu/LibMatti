// Port of net.minecraft.util.datafix.DataFixTypes (P7.3) - the DSL.TypeReference
// names the fix rules are registered for. Java models them as an enum whose
// constant carries the References name; the port needs the names only.

#ifndef MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_DATAFIXTYPES_H
#define MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_DATAFIXTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

// Java: DataFixTypes.CHUNK -> References.CHUNK
#define LIBMATTI_MC_DataFixTypes_CHUNK "chunk"
// Java: DataFixTypes.LEVEL
#define LIBMATTI_MC_DataFixTypes_LEVEL "level"
// Java: DataFixTypes.LEVEL_SUMMARY
#define LIBMATTI_MC_DataFixTypes_LEVEL_SUMMARY "levelsummary"
// Java: DataFixTypes.PLAYER
#define LIBMATTI_MC_DataFixTypes_PLAYER "player"
// Java: DataFixTypes.WORLD_GEN_SETTINGS
#define LIBMATTI_MC_DataFixTypes_WORLD_GEN_SETTINGS "worldgensettings"
// Java: References.ITEM_STACK - the block-entity name hook the 1.13 fix rides
#define LIBMATTI_MC_DataFixTypes_ITEM_STACK "itemstack"
// Java: References.BLOCK_ENTITY - the tagged choice holding the old ids
#define LIBMATTI_MC_DataFixTypes_BLOCK_ENTITY "blockentity"
// Java: References.ENTITY - the entity type choice
#define LIBMATTI_MC_DataFixTypes_ENTITY "entity"
// Java: References.ENTITY_CHUNK - the chunk entity list
#define LIBMATTI_MC_DataFixTypes_ENTITY_CHUNK "entitychunk"

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_DATAFIXTYPES_H