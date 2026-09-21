// Port of net.minecraft.world.level.levelgen.Heightmap.

#ifndef MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_HEIGHTMAP_H
#define MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_HEIGHTMAP_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct LIBMATTI_MC_ChunkAccess;

// Java: public static enum Types - the ordinals index the chunk's heightmap array
typedef enum LIBMATTI_MC_HeightmapTypes
{
    LIBMATTI_MC_Heightmap_WORLD_SURFACE_WG = 0,
    LIBMATTI_MC_Heightmap_WORLD_SURFACE = 1,
    LIBMATTI_MC_Heightmap_OCEAN_FLOOR_WG = 2,
    LIBMATTI_MC_Heightmap_OCEAN_FLOOR = 3,
    LIBMATTI_MC_Heightmap_MOTION_BLOCKING = 4,
    LIBMATTI_MC_Heightmap_MOTION_BLOCKING_NO_LEAVES = 5,
} LIBMATTI_MC_HeightmapTypes;

#define LIBMATTI_MC_Heightmap_TYPES_COUNT 6

// Java: public class Heightmap - one 16x16 column of "first available" heights,
// stored relative to the chunk's minY (Java packs it into a SimpleBitStorage; the
// port keeps one entry per column - uint16, because heights up to 320 minus a
// negative min exceed a byte)
typedef struct LIBMATTI_MC_Heightmap
{
    // Java: private final BitStorage data - heights relative to chunk min
    uint16_t data[256];
    // Java: private final ChunkAccess chunk
    struct LIBMATTI_MC_ChunkAccess *chunk;
    // Java: the Types predicate - NOT_AIR vs MATERIAL_MOTION_BLOCKING
    LIBMATTI_MC_HeightmapTypes type;
} LIBMATTI_MC_Heightmap;

// Java: public Heightmap(ChunkAccess, Types)
void LIBMATTI_MC_Heightmap_Init(LIBMATTI_MC_Heightmap *heightmap, struct LIBMATTI_MC_ChunkAccess *chunk, LIBMATTI_MC_HeightmapTypes type);
// Java: public int getFirstAvailable(int x, int z) - data + chunk min
int LIBMATTI_MC_Heightmap_GetFirstAvailable(const LIBMATTI_MC_Heightmap *heightmap, int x, int z);
// Java: public int getHighestTaken(int x, int z) = getFirstAvailable - 1
int LIBMATTI_MC_Heightmap_GetHighestTaken(const LIBMATTI_MC_Heightmap *heightmap, int x, int z);
// Java: public boolean update(int x, int y, int z, BlockState) - the incremental
// update after a setBlock (1:1 with Java's early-outs)
bool LIBMATTI_MC_Heightmap_Update(LIBMATTI_MC_Heightmap *heightmap, int x, int y, int z, const void *state);
// Java: public void setRawData(ChunkAccess, Types, long[]) - the port takes the
// 256 heights directly (Java's packed long[] unpacking lands with the save port)
bool LIBMATTI_MC_Heightmap_SetRawData(LIBMATTI_MC_Heightmap *heightmap, const int64_t *rawData, size_t count);
// Java: public long[] getRawData() - the port packs one height per entry
void LIBMATTI_MC_Heightmap_GetRawData(const LIBMATTI_MC_Heightmap *heightmap, int64_t *out);
// Java: private static int getIndex(int x, int z) = x + z * 16
int LIBMATTI_MC_Heightmap_GetIndex(int x, int z);
// Java: public static void primeHeightmaps(ChunkAccess, Set<Types>) - full scan
void LIBMATTI_MC_Heightmap_PrimeHeightmaps(struct LIBMATTI_MC_ChunkAccess *chunk, const LIBMATTI_MC_HeightmapTypes *types, size_t count);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_LEVELGEN_HEIGHTMAP_H
