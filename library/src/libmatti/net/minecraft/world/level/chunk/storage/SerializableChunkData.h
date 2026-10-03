// Port of net.minecraft.world.level.chunk.storage.SerializableChunkData - the
// chunk <-> NBT codec the region files carry. Java splits the work into the
// SerializableChunkData record (parse: tag -> data record, read: record ->
// ProtoChunk/LevelChunk, write: record -> tag) and feeds it through the
// PalettedContainer codecs; the C port fuses the paths into two calls that
// produce the same wire NBT (xPos/yPos/zPos, Status, sections with the
// block_states palette + data longs, Heightmaps, PostProcessing,
// block_entities, InhabitedTime, isLightOn, DataVersion).
//
// The port keeps two format notes:
//   - biomes serialize as the single-entry "minecraft:plains" palette (the
//     biome model lands with the worldgen port) and are ignored on read.
//   - block entities save as the pending-NBT entries (the live BlockEntity
//     NBT writers are the game port's content).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_CHUNK_STORAGE_SERIALIZABLECHUNKDATA_H
#define MATTICRAFT_MC_WORLD_LEVEL_CHUNK_STORAGE_SERIALIZABLECHUNKDATA_H

#include "libmatti/net/minecraft/nbt/CompoundTag.h"
#include "libmatti/net/minecraft/world/level/chunk/LevelChunk.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public static final String X_POS_TAG / Z_POS_TAG / ...
#define LIBMATTI_MC_SerializableChunkData_X_POS_TAG "xPos"
#define LIBMATTI_MC_SerializableChunkData_Z_POS_TAG "zPos"
#define LIBMATTI_MC_SerializableChunkData_HEIGHTMAPS_TAG "Heightmaps"
#define LIBMATTI_MC_SerializableChunkData_IS_LIGHT_ON_TAG "isLightOn"
#define LIBMATTI_MC_SerializableChunkData_SECTIONS_TAG "sections"
#define LIBMATTI_MC_SerializableChunkData_BLOCK_LIGHT_TAG "BlockLight"
#define LIBMATTI_MC_SerializableChunkData_SKY_LIGHT_TAG "SkyLight"

// Java: copyOf(ServerLevel, ChunkAccess).write() - the save NBT of the chunk;
// the caller owns the compound
LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_SerializableChunkData_Write(LIBMATTI_MC_LevelChunk *chunk);

// Java: parse(...).read(...) - a fresh LevelChunk built from the save NBT (not
// stored into the level yet); NULL on a malformed or foreign chunk tag
LIBMATTI_MC_LevelChunk *LIBMATTI_MC_SerializableChunkData_Read(struct LIBMATTI_MC_Level *level,
                                                               const LIBMATTI_MC_Nbt_CompoundTag *tag);

// Java: getChunkStatusFromTag(tag) == ChunkStatus.FULL - the loader only spins
// up chunks whose status is full (the ProtoChunk pipeline is worldgen content)
bool LIBMATTI_MC_SerializableChunkData_IsFullChunk(const LIBMATTI_MC_Nbt_CompoundTag *tag);

// Java: BlockStateCodec - the palette entries ride the
// "minecraft:block[prop=value,...]" string form (Block.toString in vanilla);
// the caller frees the string / NULL on an unknown block or property
char *LIBMATTI_MC_SerializableChunkData_StateToString(const LIBMATTI_MC_BlockState *state);
LIBMATTI_MC_BlockState *LIBMATTI_MC_SerializableChunkData_StateFromString(const char *string);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_CHUNK_STORAGE_SERIALIZABLECHUNKDATA_H
