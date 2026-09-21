// Port of net.minecraft.world.level.chunk.LevelChunkSection.

#include "libmatti/net/minecraft/world/level/chunk/LevelChunkSection.h"

#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"

#include <stdlib.h>

// Java: isRandomlyTicking is block-content driven; the port treats every
// non-air state as randomly ticking until the block behaviour model lands
static bool is_air_state(const LIBMATTI_MC_BlockState *state)
{
    return state == NULL || LIBMATTI_MC_BlockState_GetBlock(state) == (void *) LIBMATTI_MC_VanillaBlocks_AIR()
           || LIBMATTI_MC_BlockState_GetBlock(state) == NULL;
}

int LIBMATTI_MC_LevelChunkSection_Index(int x, int y, int z)
{
    // Java: PalettedContainer index - y * 256 + z * 16 + x
    return y * 256 + z * 16 + x;
}

LIBMATTI_MC_LevelChunkSection *LIBMATTI_MC_LevelChunkSection_New(void)
{
    LIBMATTI_MC_LevelChunkSection *section = calloc(1, sizeof(LIBMATTI_MC_LevelChunkSection));
    // Java: PalettedContainerFactory.createForBlockStates() starts filled with air
    LIBMATTI_MC_BlockState *air = LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_AIR());
    for (int i = 0; i < LIBMATTI_MC_LevelChunkSection_SECTION_SIZE; i++)
        section->states[i] = air;
    return section;
}

LIBMATTI_MC_BlockState *LIBMATTI_MC_LevelChunkSection_GetBlockState(const LIBMATTI_MC_LevelChunkSection *section, int x, int y, int z)
{
    return section->states[LIBMATTI_MC_LevelChunkSection_Index(x, y, z)];
}

LIBMATTI_MC_BlockState *LIBMATTI_MC_LevelChunkSection_SetBlockState(LIBMATTI_MC_LevelChunkSection *section, int x, int y, int z, LIBMATTI_MC_BlockState *state)
{
    // Java: states.getAndSet(x, y, z, state) + the count bookkeeping
    int index = LIBMATTI_MC_LevelChunkSection_Index(x, y, z);
    LIBMATTI_MC_BlockState *oldState = section->states[index];
    section->states[index] = state;

    // Java: if (!blockstate.isAir()) nonEmptyBlockCount-- (+ tickingBlockCount--)
    if (!is_air_state(oldState))
    {
        section->nonEmptyBlockCount--;
        section->tickingBlockCount--;
    }
    // Java: if (!p_62995_.isAir()) nonEmptyBlockCount++ (+ tickingBlockCount++)
    if (!is_air_state(state))
    {
        section->nonEmptyBlockCount++;
        section->tickingBlockCount++;
    }
    return oldState;
}

bool LIBMATTI_MC_LevelChunkSection_HasOnlyAir(const LIBMATTI_MC_LevelChunkSection *section)
{
    return section->nonEmptyBlockCount == 0;
}

bool LIBMATTI_MC_LevelChunkSection_IsRandomlyTicking(const LIBMATTI_MC_LevelChunkSection *section)
{
    return LIBMATTI_MC_LevelChunkSection_IsRandomlyTickingBlocks(section)
           || LIBMATTI_MC_LevelChunkSection_IsRandomlyTickingFluids(section);
}

bool LIBMATTI_MC_LevelChunkSection_IsRandomlyTickingBlocks(const LIBMATTI_MC_LevelChunkSection *section)
{
    return section->tickingBlockCount > 0;
}

bool LIBMATTI_MC_LevelChunkSection_IsRandomlyTickingFluids(const LIBMATTI_MC_LevelChunkSection *section)
{
    // Java: tickingFluidCount - the fluid model is game-port content, no fluids yet
    return section->tickingFluidCount > 0;
}

void LIBMATTI_MC_LevelChunkSection_RecalcBlockCounts(LIBMATTI_MC_LevelChunkSection *section)
{
    // Java: recalcBlockCounts - walk the palette and count
    section->nonEmptyBlockCount = 0;
    section->tickingBlockCount = 0;
    section->tickingFluidCount = 0;
    for (int i = 0; i < LIBMATTI_MC_LevelChunkSection_SECTION_SIZE; i++)
    {
        if (!is_air_state(section->states[i]))
        {
            section->nonEmptyBlockCount++;
            section->tickingBlockCount++;
        }
    }
}
