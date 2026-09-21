// Port of net.minecraft.world.level.levelgen.Heightmap.

#include "libmatti/net/minecraft/world/level/levelgen/Heightmap.h"

#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/world/level/chunk/ChunkAccess.h"

#include <string.h>

// Java: NOT_AIR = state -> !state.isAir(); MATERIAL_MOTION_BLOCKING - the port
// treats every non-air state as motion blocking until the block model carries it
static bool is_opaque(const LIBMATTI_MC_Heightmap *heightmap, const LIBMATTI_MC_BlockState *state)
{
    if (state == NULL)
        return false;
    const void *block = LIBMATTI_MC_BlockState_GetBlock(state);
    bool notAir = block != (const void *) LIBMATTI_MC_VanillaBlocks_AIR();
    switch (heightmap->type)
    {
        case LIBMATTI_MC_Heightmap_WORLD_SURFACE_WG:
        case LIBMATTI_MC_Heightmap_WORLD_SURFACE:
            return notAir;
        default:
            // Java: blocksMotion() || fluid - non-air approximates it
            return notAir;
    }
}

int LIBMATTI_MC_Heightmap_GetIndex(int x, int z)
{
    // Java: x + z * 16
    return x + z * 16;
}

void LIBMATTI_MC_Heightmap_Init(LIBMATTI_MC_Heightmap *heightmap, struct LIBMATTI_MC_ChunkAccess *chunk, LIBMATTI_MC_HeightmapTypes type)
{
    memset(heightmap->data, 0, sizeof(heightmap->data));
    heightmap->chunk = chunk;
    heightmap->type = type;
}

static int chunk_min_y(const LIBMATTI_MC_ChunkAccess *chunk)
{
    return chunk->levelHeightAccessor.minY;
}

int LIBMATTI_MC_Heightmap_GetFirstAvailable(const LIBMATTI_MC_Heightmap *heightmap, int x, int z)
{
    // Java: data.get(index) + chunk.getMinY()
    return heightmap->data[LIBMATTI_MC_Heightmap_GetIndex(x, z)] + chunk_min_y(heightmap->chunk);
}

int LIBMATTI_MC_Heightmap_GetHighestTaken(const LIBMATTI_MC_Heightmap *heightmap, int x, int z)
{
    return LIBMATTI_MC_Heightmap_GetFirstAvailable(heightmap, x, z) - 1;
}

static void set_height(LIBMATTI_MC_Heightmap *heightmap, int x, int z, int y)
{
    heightmap->data[LIBMATTI_MC_Heightmap_GetIndex(x, z)] = (uint16_t) (y - chunk_min_y(heightmap->chunk));
}

bool LIBMATTI_MC_Heightmap_Update(LIBMATTI_MC_Heightmap *heightmap, int x, int y, int z, const void *state)
{
    // Java: 1:1 - the early-out ladder
    int firstAvailable = LIBMATTI_MC_Heightmap_GetFirstAvailable(heightmap, x, z);
    if (y <= firstAvailable - 2)
        return false;

    if (is_opaque(heightmap, (const LIBMATTI_MC_BlockState *) state))
    {
        if (y >= firstAvailable)
        {
            set_height(heightmap, x, z, y + 1);
            return true;
        }
    }
    else if (firstAvailable - 1 == y)
    {
        // the removed block was the top - rescan down for the new top
        const LIBMATTI_MC_ChunkAccess *chunk = heightmap->chunk;
        for (int j = y - 1; j >= chunk->levelHeightAccessor.minY; j--)
        {
            LIBMATTI_MC_BlockState *below = LIBMATTI_MC_LevelChunkSection_GetBlockState(
                chunk->sections[LIBMATTI_MC_LevelHeightAccessor_GetSectionIndex(&chunk->levelHeightAccessor, j)],
                x & 15, j & 15, z & 15);
            if (is_opaque(heightmap, below))
            {
                set_height(heightmap, x, z, j + 1);
                return true;
            }
        }
        set_height(heightmap, x, z, chunk->levelHeightAccessor.minY);
        return true;
    }
    return false;
}

bool LIBMATTI_MC_Heightmap_SetRawData(LIBMATTI_MC_Heightmap *heightmap, const int64_t *rawData, size_t count)
{
    // Java: the packed long[] must match the storage length (256 entries packed);
    // the port takes the unpacked heights
    if (count != 256)
        return false;
    for (size_t i = 0; i < count; i++)
        heightmap->data[i] = (uint16_t) (rawData[i] - chunk_min_y(heightmap->chunk));
    return true;
}

void LIBMATTI_MC_Heightmap_GetRawData(const LIBMATTI_MC_Heightmap *heightmap, int64_t *out)
{
    for (int i = 0; i < 256; i++)
        out[i] = heightmap->data[i] + chunk_min_y(heightmap->chunk);
}

void LIBMATTI_MC_Heightmap_PrimeHeightmaps(struct LIBMATTI_MC_ChunkAccess *chunk, const LIBMATTI_MC_HeightmapTypes *types, size_t count)
{
    if (count == 0)
        return;

    // Java: primeHeightmaps - scan every column top-down, set each heightmap once
    LIBMATTI_MC_Heightmap *maps[LIBMATTI_MC_Heightmap_TYPES_COUNT];
    for (size_t i = 0; i < count; i++)
        maps[i] = LIBMATTI_MC_ChunkAccess_GetOrCreateHeightmapUnprimed((LIBMATTI_MC_ChunkAccess *) chunk, (int) types[i]);

    int minY = chunk->levelHeightAccessor.minY;
    int maxY = LIBMATTI_MC_LevelHeightAccessor_GetMaxY(&chunk->levelHeightAccessor);
    for (int x = 0; x < 16; x++)
    {
        for (int z = 0; z < 16; z++)
        {
            for (int y = maxY; y >= minY; y--)
            {
                bool anyLeft = false;
                for (size_t i = 0; i < count; i++)
                {
                    if (maps[i] == NULL)
                        continue;
                    anyLeft = true;
                    LIBMATTI_MC_BlockState *state = LIBMATTI_MC_ChunkAccess_GetBlockStateXYZ(chunk, x, y, z);
                    if (is_opaque(maps[i], state))
                    {
                        set_height(maps[i], x, z, y + 1);
                        maps[i] = NULL;
                    }
                }
                if (!anyLeft)
                    return;
            }
        }
    }
}
