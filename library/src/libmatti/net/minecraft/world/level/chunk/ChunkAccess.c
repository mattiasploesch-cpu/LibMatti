// Port of net.minecraft.world.level.chunk.ChunkAccess.

#include "libmatti/net/minecraft/world/level/chunk/ChunkAccess.h"

#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/core/SectionPos.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/world/level/chunk/LevelChunk.h"

#include <stdlib.h>
#include <string.h>

void LIBMATTI_MC_ChunkAccess_Init(LIBMATTI_MC_ChunkAccess *chunk, const LIBMATTI_MC_ChunkPos *pos,
                                  const LIBMATTI_MC_LevelHeightAccessor *heightAccessor, int64_t inhabitedTime)
{
    memset(chunk, 0, sizeof(*chunk));
    chunk->chunkPos = *pos;
    chunk->levelHeightAccessor = *heightAccessor;
    chunk->inhabitedTime = inhabitedTime;
    chunk->sectionCount = LIBMATTI_MC_LevelHeightAccessor_GetSectionsCount(heightAccessor);

    // Java: this.sections = new LevelChunkSection[getSectionsCount()] +
    // replaceMissingSections (every NULL slot becomes an empty section)
    chunk->sections = calloc((size_t) chunk->sectionCount, sizeof(LIBMATTI_MC_LevelChunkSection *));
    for (int i = 0; i < chunk->sectionCount; i++)
        chunk->sections[i] = LIBMATTI_MC_LevelChunkSection_New();

    // Java: protected final @Nullable ShortList[] postProcessing - one list per section
    chunk->postProcessingCount = chunk->sectionCount;
    chunk->postProcessing = calloc((size_t) chunk->postProcessingCount, sizeof(LIBMATTI_MC_PostProcessingList));
}

LIBMATTI_MC_LevelChunkSection **LIBMATTI_MC_ChunkAccess_GetSections(LIBMATTI_MC_ChunkAccess *chunk)
{
    return chunk->sections;
}

LIBMATTI_MC_LevelChunkSection *LIBMATTI_MC_ChunkAccess_GetSection(LIBMATTI_MC_ChunkAccess *chunk, int index)
{
    return chunk->sections[index];
}

LIBMATTI_MC_BlockState *LIBMATTI_MC_ChunkAccess_GetBlockStateXYZ(const LIBMATTI_MC_ChunkAccess *chunk, int x, int y, int z)
{
    // Java: the try-body of LevelChunk.getBlockState - section lookup, air outside
    int index = LIBMATTI_MC_LevelHeightAccessor_GetSectionIndex(&chunk->levelHeightAccessor, y);
    if (index < 0 || index >= chunk->sectionCount)
        return LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_AIR());

    LIBMATTI_MC_LevelChunkSection *section = chunk->sections[index];
    if (LIBMATTI_MC_LevelChunkSection_HasOnlyAir(section))
        return LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_AIR());

    return LIBMATTI_MC_LevelChunkSection_GetBlockState(section, x & 15, y & 15, z & 15);
}

int LIBMATTI_MC_ChunkAccess_GetHighestFilledSectionIndex(const LIBMATTI_MC_ChunkAccess *chunk)
{
    for (int i = chunk->sectionCount - 1; i >= 0; i--)
    {
        if (!LIBMATTI_MC_LevelChunkSection_HasOnlyAir(chunk->sections[i]))
            return i;
    }
    return LIBMATTI_MC_ChunkAccess_NO_FILLED_SECTION;
}

int LIBMATTI_MC_ChunkAccess_GetHighestSectionPosition(const LIBMATTI_MC_ChunkAccess *chunk)
{
    int index = LIBMATTI_MC_ChunkAccess_GetHighestFilledSectionIndex(chunk);
    if (index == LIBMATTI_MC_ChunkAccess_NO_FILLED_SECTION)
        return chunk->levelHeightAccessor.minY;
    return LIBMATTI_MC_SectionPos_SectionToBlockCoord(
        LIBMATTI_MC_LevelHeightAccessor_GetSectionYFromSectionIndex(&chunk->levelHeightAccessor, index));
}

bool LIBMATTI_MC_ChunkAccess_IsYSpaceEmpty(const LIBMATTI_MC_ChunkAccess *chunk, int from, int to)
{
    // Java: clamps + a 16-step section walk
    if (from < chunk->levelHeightAccessor.minY)
        from = chunk->levelHeightAccessor.minY;
    int maxY = LIBMATTI_MC_LevelHeightAccessor_GetMaxY(&chunk->levelHeightAccessor);
    if (to > maxY)
        to = maxY;
    for (int i = from; i <= to; i += 16)
    {
        int sectionIndex = LIBMATTI_MC_LevelHeightAccessor_GetSectionIndex(&chunk->levelHeightAccessor, i);
        if (sectionIndex >= 0 && sectionIndex < chunk->sectionCount
            && !LIBMATTI_MC_LevelChunkSection_HasOnlyAir(chunk->sections[sectionIndex]))
            return false;
    }
    return true;
}

struct LIBMATTI_MC_Heightmap *LIBMATTI_MC_ChunkAccess_GetOrCreateHeightmapUnprimed(LIBMATTI_MC_ChunkAccess *chunk, int type)
{
    // Java: heightmaps.computeIfAbsent(type, t -> new Heightmap(this, t))
    if (type < 0 || type >= LIBMATTI_MC_Heightmap_TYPES_COUNT)
        return NULL;
    if (chunk->heightmaps[type] == NULL)
    {
        LIBMATTI_MC_Heightmap *heightmap = malloc(sizeof(LIBMATTI_MC_Heightmap));
        LIBMATTI_MC_Heightmap_Init(heightmap, chunk, (LIBMATTI_MC_HeightmapTypes) type);
        chunk->heightmaps[type] = heightmap;
    }
    return chunk->heightmaps[type];
}

bool LIBMATTI_MC_ChunkAccess_HasPrimedHeightmap(const LIBMATTI_MC_ChunkAccess *chunk, int type)
{
    // Java: hasPrimedHeightmap - a stored entry means primed
    return type >= 0 && type < LIBMATTI_MC_Heightmap_TYPES_COUNT && chunk->heightmaps[type] != NULL;
}

int LIBMATTI_MC_ChunkAccess_GetHeight(LIBMATTI_MC_ChunkAccess *chunk, int type, int x, int z)
{
    // Java: getHeight - prime on demand when missing, then firstAvailable - 1
    if (!LIBMATTI_MC_ChunkAccess_HasPrimedHeightmap(chunk, type))
    {
        LIBMATTI_MC_HeightmapTypes one = (LIBMATTI_MC_HeightmapTypes) type;
        LIBMATTI_MC_Heightmap_PrimeHeightmaps(chunk, &one, 1);
    }
    return LIBMATTI_MC_Heightmap_GetFirstAvailable(chunk->heightmaps[type], x & 15, z & 15) - 1;
}

void LIBMATTI_MC_ChunkAccess_SetHeightmapRaw(LIBMATTI_MC_ChunkAccess *chunk, int type, const int64_t *rawData, size_t count)
{
    struct LIBMATTI_MC_Heightmap *heightmap = LIBMATTI_MC_ChunkAccess_GetOrCreateHeightmapUnprimed(chunk, type);
    if (heightmap != NULL)
        LIBMATTI_MC_Heightmap_SetRawData(heightmap, rawData, count);
}

size_t LIBMATTI_MC_ChunkAccess_GetBlockEntitiesPos(const LIBMATTI_MC_ChunkAccess *chunk, int64_t *out, size_t capacity)
{
    // Java: the union of pendingBlockEntities.keySet() and blockEntities.keySet()
    size_t count = 0;
    for (int i = 0; i < chunk->blockEntityCount && count < capacity; i++)
        out[count++] = chunk->blockEntities[i].packedPos;
    for (int i = 0; i < chunk->pendingBlockEntityCount && count < capacity; i++)
    {
        int64_t packed = chunk->pendingBlockEntities[i].packedPos;
        bool duplicate = false;
        for (size_t j = 0; j < count; j++)
        {
            if (out[j] == packed)
            {
                duplicate = true;
                break;
            }
        }
        if (!duplicate)
            out[count++] = packed;
    }
    return count;
}

void LIBMATTI_MC_ChunkAccess_SetBlockEntityNbt(LIBMATTI_MC_ChunkAccess *chunk, void *tag, const LIBMATTI_MC_BlockPos *pos)
{
    // Java: if (!blockEntities.containsKey(pos)) pendingBlockEntities.put(pos, tag)
    int64_t packed = LIBMATTI_MC_BlockPos_AsLong(pos);
    for (int i = 0; i < chunk->blockEntityCount; i++)
    {
        if (chunk->blockEntities[i].packedPos == packed)
            return;
    }
    for (int i = 0; i < chunk->pendingBlockEntityCount; i++)
    {
        if (chunk->pendingBlockEntities[i].packedPos == packed)
        {
            chunk->pendingBlockEntities[i].tag = tag;
            return;
        }
    }
    if (chunk->pendingBlockEntityCount == chunk->pendingBlockEntityCapacity)
    {
        chunk->pendingBlockEntityCapacity = chunk->pendingBlockEntityCapacity > 0 ? chunk->pendingBlockEntityCapacity * 2 : 8;
        chunk->pendingBlockEntities = realloc(chunk->pendingBlockEntities,
                                              sizeof(*chunk->pendingBlockEntities) * (size_t) chunk->pendingBlockEntityCapacity);
    }
    chunk->pendingBlockEntities[chunk->pendingBlockEntityCount].packedPos = packed;
    chunk->pendingBlockEntities[chunk->pendingBlockEntityCount].tag = tag;
    chunk->pendingBlockEntityCount++;
}

void *LIBMATTI_MC_ChunkAccess_GetPendingBlockEntityNbt(const LIBMATTI_MC_ChunkAccess *chunk, const LIBMATTI_MC_BlockPos *pos)
{
    int64_t packed = LIBMATTI_MC_BlockPos_AsLong(pos);
    for (int i = 0; i < chunk->pendingBlockEntityCount; i++)
    {
        if (chunk->pendingBlockEntities[i].packedPos == packed)
            return chunk->pendingBlockEntities[i].tag;
    }
    return NULL;
}

void LIBMATTI_MC_ChunkAccess_MarkPosForPostprocessing(LIBMATTI_MC_ChunkAccess *chunk, const LIBMATTI_MC_BlockPos *pos, int index)
{
    // Java: logs "not supported" on the base; LevelChunk overrides through ProtoChunk
    (void) chunk;
    (void) pos;
    (void) index;
}

static LIBMATTI_MC_PostProcessingList *get_or_create_list(LIBMATTI_MC_ChunkAccess *chunk, int index)
{
    if (index < 0 || index >= chunk->postProcessingCount)
        return NULL;
    return &chunk->postProcessing[index];
}

void LIBMATTI_MC_ChunkAccess_AddPackedPostProcess(LIBMATTI_MC_ChunkAccess *chunk, int index, uint16_t packed)
{
    LIBMATTI_MC_PostProcessingList *list = get_or_create_list(chunk, index);
    if (list == NULL)
        return;
    if (list->count == list->capacity)
    {
        list->capacity = list->capacity > 0 ? list->capacity * 2 : 4;
        list->packed = realloc(list->packed, sizeof(uint16_t) * (size_t) list->capacity);
    }
    list->packed[list->count++] = packed;
}

const LIBMATTI_MC_PostProcessingList *LIBMATTI_MC_ChunkAccess_GetPostProcessing(const LIBMATTI_MC_ChunkAccess *chunk)
{
    return chunk->postProcessing;
}

void LIBMATTI_MC_ChunkAccess_MarkUnsaved(LIBMATTI_MC_ChunkAccess *chunk)
{
    chunk->unsaved = true;
}

bool LIBMATTI_MC_ChunkAccess_TryMarkSaved(LIBMATTI_MC_ChunkAccess *chunk)
{
    if (chunk->unsaved)
    {
        chunk->unsaved = false;
        return true;
    }
    return false;
}

bool LIBMATTI_MC_ChunkAccess_IsUnsaved(const LIBMATTI_MC_ChunkAccess *chunk)
{
    return chunk->unsaved;
}

int64_t LIBMATTI_MC_ChunkAccess_GetInhabitedTime(const LIBMATTI_MC_ChunkAccess *chunk)
{
    return chunk->inhabitedTime;
}

void LIBMATTI_MC_ChunkAccess_SetInhabitedTime(LIBMATTI_MC_ChunkAccess *chunk, int64_t inhabitedTime)
{
    chunk->inhabitedTime = inhabitedTime;
}

void LIBMATTI_MC_ChunkAccess_Free(LIBMATTI_MC_ChunkAccess *chunk)
{
    for (int i = 0; i < chunk->sectionCount; i++)
        free(chunk->sections[i]);
    free(chunk->sections);
    for (int i = 0; i < LIBMATTI_MC_Heightmap_TYPES_COUNT; i++)
        free(chunk->heightmaps[i]);
    free(chunk->blockEntities);
    free(chunk->pendingBlockEntities);
    for (int i = 0; i < chunk->postProcessingCount; i++)
        free(chunk->postProcessing[i].packed);
    free(chunk->postProcessing);
}
