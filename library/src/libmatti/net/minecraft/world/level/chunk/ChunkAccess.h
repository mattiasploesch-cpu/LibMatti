// Port of net.minecraft.world.level.chunk.ChunkAccess (the data basis).
//
// Java: public abstract class ChunkAccess - the port makes it the embedded base
// of LevelChunk with the full data layout: sections, heightmaps, block entity
// containers, post-processing, the unsaved flag and the upgrade/blending slots.

#ifndef MATTICRAFT_MC_WORLD_LEVEL_CHUNK_CHUNKACCESS_H
#define MATTICRAFT_MC_WORLD_LEVEL_CHUNK_CHUNKACCESS_H

#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/world/level/ChunkPos.h"
#include "libmatti/net/minecraft/world/level/LevelHeightAccessor.h"
#include "libmatti/net/minecraft/world/level/block/entity/BlockEntity.h"
#include "libmatti/net/minecraft/world/level/block/state/BlockState.h"
#include "libmatti/net/minecraft/world/level/chunk/LevelChunkSection.h"
#include "libmatti/net/minecraft/world/level/levelgen/Heightmap.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public static final int NO_FILLED_SECTION = -1
#define LIBMATTI_MC_ChunkAccess_NO_FILLED_SECTION (-1)

// Java: protected final Map<BlockPos, CompoundTag> pendingBlockEntities - the port
// keeps the raw NBT handle per position (the CompoundTag model lives in nbt/)
typedef struct PendingBlockEntityEntry
{
    int64_t packedPos;
    void *tag;
} LIBMATTI_MC_PendingBlockEntityEntry;

// Java: protected final Map<BlockPos, BlockEntity> blockEntities
typedef struct BlockEntityEntry
{
    int64_t packedPos;
    LIBMATTI_MC_BlockEntity *blockEntity;
} LIBMATTI_MC_ChunkBlockEntityEntry;

// Java: protected final ShortList[] postProcessing - one packed-position list per section
typedef struct PostProcessingList
{
    uint16_t *packed;
    int count;
    int capacity;
} LIBMATTI_MC_PostProcessingList;

// Java: public abstract class ChunkAccess
typedef struct LIBMATTI_MC_ChunkAccess
{
    // Java: protected final ChunkPos chunkPos
    LIBMATTI_MC_ChunkPos chunkPos;
    // Java: protected final LevelHeightAccessor levelHeightAccessor
    LIBMATTI_MC_LevelHeightAccessor levelHeightAccessor;
    // Java: protected final LevelChunkSection[] sections
    LIBMATTI_MC_LevelChunkSection **sections;
    int sectionCount;
    // Java: protected final Map<Heightmap.Types, Heightmap> heightmaps - indexed by
    // the Types ordinal (Java's EnumMap)
    struct LIBMATTI_MC_Heightmap *heightmaps[LIBMATTI_MC_Heightmap_TYPES_COUNT];
    // Java: protected final Map<BlockPos, BlockEntity> blockEntities
    LIBMATTI_MC_ChunkBlockEntityEntry *blockEntities;
    int blockEntityCount;
    int blockEntityCapacity;
    // Java: protected final Map<BlockPos, CompoundTag> pendingBlockEntities
    LIBMATTI_MC_PendingBlockEntityEntry *pendingBlockEntities;
    int pendingBlockEntityCount;
    int pendingBlockEntityCapacity;
    // Java: protected final @Nullable ShortList[] postProcessing
    LIBMATTI_MC_PostProcessingList *postProcessing;
    int postProcessingCount;
    // Java: private volatile boolean unsaved / isLightCorrect, private long inhabitedTime
    bool unsaved;
    bool isLightCorrect;
    int64_t inhabitedTime;
    // Java: protected @Nullable NoiseChunk noiseChunk / UpgradeData / BlendingData -
    // game-port content, kept as opaque slots
    void *noiseChunk;
    void *upgradeData;
    void *blendingData;
} LIBMATTI_MC_ChunkAccess;

// Java: the ChunkAccess constructor - empty sections where none were provided
// (replaceMissingSections), the height accessor and the pos stored
void LIBMATTI_MC_ChunkAccess_Init(LIBMATTI_MC_ChunkAccess *chunk, const LIBMATTI_MC_ChunkPos *pos,
                                  const LIBMATTI_MC_LevelHeightAccessor *heightAccessor, int64_t inhabitedTime);
// Java: public LevelChunkSection[] getSections() / getSection(int)
LIBMATTI_MC_LevelChunkSection **LIBMATTI_MC_ChunkAccess_GetSections(LIBMATTI_MC_ChunkAccess *chunk);
LIBMATTI_MC_LevelChunkSection *LIBMATTI_MC_ChunkAccess_GetSection(LIBMATTI_MC_ChunkAccess *chunk, int index);
// the xyz block read through the section array (chunk-local x/z 0..15) - the shared
// body of ChunkAccess.getBlockState/LevelChunk.getBlockState
LIBMATTI_MC_BlockState *LIBMATTI_MC_ChunkAccess_GetBlockStateXYZ(const LIBMATTI_MC_ChunkAccess *chunk, int x, int y, int z);
// Java: public int getHighestFilledSectionIndex() / getHighestSectionPosition()
int LIBMATTI_MC_ChunkAccess_GetHighestFilledSectionIndex(const LIBMATTI_MC_ChunkAccess *chunk);
int LIBMATTI_MC_ChunkAccess_GetHighestSectionPosition(const LIBMATTI_MC_ChunkAccess *chunk);
// Java: public boolean isYSpaceEmpty(int from, int to)
bool LIBMATTI_MC_ChunkAccess_IsYSpaceEmpty(const LIBMATTI_MC_ChunkAccess *chunk, int from, int to);

// Java: public int getHeight(Heightmap.Types, int x, int z) - primes on demand
int LIBMATTI_MC_ChunkAccess_GetHeight(LIBMATTI_MC_ChunkAccess *chunk, int type, int x, int z);
// Java: public Heightmap getOrCreateHeightmapUnprimed(Heightmap.Types)
struct LIBMATTI_MC_Heightmap *LIBMATTI_MC_ChunkAccess_GetOrCreateHeightmapUnprimed(LIBMATTI_MC_ChunkAccess *chunk, int type);
// Java: public boolean hasPrimedHeightmap(Heightmap.Types)
bool LIBMATTI_MC_ChunkAccess_HasPrimedHeightmap(const LIBMATTI_MC_ChunkAccess *chunk, int type);
// Java: public void setHeightmap(Heightmap.Types, long[] rawData)
void LIBMATTI_MC_ChunkAccess_SetHeightmapRaw(LIBMATTI_MC_ChunkAccess *chunk, int type, const int64_t *rawData, size_t count);

// Java: public Set<BlockPos> getBlockEntitiesPos() - the union of both maps; the
// port fills the caller's array with packed positions
size_t LIBMATTI_MC_ChunkAccess_GetBlockEntitiesPos(const LIBMATTI_MC_ChunkAccess *chunk, int64_t *out, size_t capacity);
// Java: public void setBlockEntityNbt(CompoundTag)
void LIBMATTI_MC_ChunkAccess_SetBlockEntityNbt(LIBMATTI_MC_ChunkAccess *chunk, void *tag, const LIBMATTI_MC_BlockPos *pos);
// Java: public @Nullable CompoundTag getBlockEntityNbtForSaving(BlockPos,...) - the
// pending tag for a position that has no live block entity
void *LIBMATTI_MC_ChunkAccess_GetPendingBlockEntityNbt(const LIBMATTI_MC_ChunkAccess *chunk, const LIBMATTI_MC_BlockPos *pos);

// Java: public void markPosForPostprocessing(BlockPos) / addPackedPostProcess /
// getPostProcessing
void LIBMATTI_MC_ChunkAccess_MarkPosForPostprocessing(LIBMATTI_MC_ChunkAccess *chunk, const LIBMATTI_MC_BlockPos *pos, int index);
void LIBMATTI_MC_ChunkAccess_AddPackedPostProcess(LIBMATTI_MC_ChunkAccess *chunk, int index, uint16_t packed);
const LIBMATTI_MC_PostProcessingList *LIBMATTI_MC_ChunkAccess_GetPostProcessing(const LIBMATTI_MC_ChunkAccess *chunk);

// Java: public void markUnsaved() / tryMarkSaved() / isUnsaved()
void LIBMATTI_MC_ChunkAccess_MarkUnsaved(LIBMATTI_MC_ChunkAccess *chunk);
bool LIBMATTI_MC_ChunkAccess_TryMarkSaved(LIBMATTI_MC_ChunkAccess *chunk);
bool LIBMATTI_MC_ChunkAccess_IsUnsaved(const LIBMATTI_MC_ChunkAccess *chunk);

// Java: public long getInhabitedTime() / setInhabitedTime(long)
int64_t LIBMATTI_MC_ChunkAccess_GetInhabitedTime(const LIBMATTI_MC_ChunkAccess *chunk);
void LIBMATTI_MC_ChunkAccess_SetInhabitedTime(LIBMATTI_MC_ChunkAccess *chunk, int64_t inhabitedTime);

// Java: the abstract setBlockState/setBlockEntity/removeBlockEntity stay virtual on
// LevelChunk; the port exposes the shared base free of the block-entity map only
void LIBMATTI_MC_ChunkAccess_Free(LIBMATTI_MC_ChunkAccess *chunk);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_CHUNK_CHUNKACCESS_H
