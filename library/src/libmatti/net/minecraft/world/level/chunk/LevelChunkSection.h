// Port of net.minecraft.world.level.chunk.LevelChunkSection.
//
// Java stores the 4096 states in a PalettedContainer; the in-memory port keeps a
// plain BlockState* array (the palette compression belongs to the save-load
// port). The block-count bookkeeping (nonEmpty/ticking/fluid) is 1:1.

#ifndef MATTICRAFT_MC_WORLD_LEVEL_CHUNK_LEVELCHUNKSECTION_H
#define MATTICRAFT_MC_WORLD_LEVEL_CHUNK_LEVELCHUNKSECTION_H

#include "libmatti/net/minecraft/world/level/block/state/BlockState.h"
#include "libmatti/net/minecraft/world/level/chunk/PalettedContainer.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public static final int SECTION_WIDTH = 16, SECTION_HEIGHT = 16, SECTION_SIZE = 4096
#define LIBMATTI_MC_LevelChunkSection_SECTION_WIDTH 16
#define LIBMATTI_MC_LevelChunkSection_SECTION_HEIGHT 16
#define LIBMATTI_MC_LevelChunkSection_SECTION_SIZE 4096

// Java: public class LevelChunkSection
typedef struct LIBMATTI_MC_LevelChunkSection
{
    // Java: private short nonEmptyBlockCount / tickingBlockCount / tickingFluidCount
    int16_t nonEmptyBlockCount;
    int16_t tickingBlockCount;
    int16_t tickingFluidCount;
    // Java: private final PalettedContainer<BlockState> states - the palette +
    // BitStorage port (SINGLE_VALUE/LINEAR/GLOBAL like Java's strategies)
    LIBMATTI_MC_PalettedContainer *states;
} LIBMATTI_MC_LevelChunkSection;

// Java: public LevelChunkSection() through PalettedContainerFactory - all air
LIBMATTI_MC_LevelChunkSection *LIBMATTI_MC_LevelChunkSection_New(void);
// Java: private static int index = x + z * 16 + y * 256
int LIBMATTI_MC_LevelChunkSection_Index(int x, int y, int z);
// Java: public BlockState getBlockState(int x, int y, int z)
LIBMATTI_MC_BlockState *LIBMATTI_MC_LevelChunkSection_GetBlockState(const LIBMATTI_MC_LevelChunkSection *section, int x, int y, int z);
// Java: public BlockState setBlockState(int x, int y, int z, BlockState) - returns the old state
LIBMATTI_MC_BlockState *LIBMATTI_MC_LevelChunkSection_SetBlockState(LIBMATTI_MC_LevelChunkSection *section, int x, int y, int z, LIBMATTI_MC_BlockState *state);
// the PalettedContainer access (Java: the field read through the section)
LIBMATTI_MC_PalettedContainer *LIBMATTI_MC_LevelChunkSection_GetStates(const LIBMATTI_MC_LevelChunkSection *section);
// Java: public boolean hasOnlyAir()
bool LIBMATTI_MC_LevelChunkSection_HasOnlyAir(const LIBMATTI_MC_LevelChunkSection *section);
// Java: public boolean isRandomlyTicking() / isRandomlyTickingBlocks() / isRandomlyTickingFluids()
bool LIBMATTI_MC_LevelChunkSection_IsRandomlyTicking(const LIBMATTI_MC_LevelChunkSection *section);
bool LIBMATTI_MC_LevelChunkSection_IsRandomlyTickingBlocks(const LIBMATTI_MC_LevelChunkSection *section);
bool LIBMATTI_MC_LevelChunkSection_IsRandomlyTickingFluids(const LIBMATTI_MC_LevelChunkSection *section);
// Java: public void recalcBlockCounts() - recounts from the state array
void LIBMATTI_MC_LevelChunkSection_RecalcBlockCounts(LIBMATTI_MC_LevelChunkSection *section);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_CHUNK_LEVELCHUNKSECTION_H
