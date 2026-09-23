// Port of net.minecraft.client.color.block.BlockColor (the functional
// interface) and net.minecraft.client.color.block.BlockColors.

#ifndef MATTICRAFT_MC_CLIENT_COLOR_BLOCK_BLOCKCOLORS_H
#define MATTICRAFT_MC_CLIENT_COLOR_BLOCK_BLOCKCOLORS_H

#include "libmatti/core/IdMapper.h"

#ifdef __cplusplus
extern "C" {
#endif

struct LIBMATTI_MC_BlockState;
struct LIBMATTI_MC_Level;
struct LIBMATTI_MC_BlockAndTintGetter;
struct LIBMATTI_MC_BlockPos;
struct LIBMATTI_MC_Block;

// Java: the BlockColor functional interface - (state, level, pos, tintIndex) -> ARGB.
// The nullable level/pos arrive as NULL (the provider decides what that means).
typedef int (*LIBMATTI_MC_BlockColor)(
    const struct LIBMATTI_MC_BlockState *state,
    struct LIBMATTI_MC_BlockAndTintGetter *level,
    const struct LIBMATTI_MC_BlockPos *pos,
    int tintIndex);

// Java: public class BlockColors
typedef struct LIBMATTI_MC_BlockColors
{
    // Java: private final IdMapper<BlockColor> blockColors
    LIBMATTI_JL_IdMapper *blockColors;
} LIBMATTI_MC_BlockColors;

// Creates a color registry populated with the built-in providers for the
// supported vanilla block types. Block names absent from the registry are skipped.
LIBMATTI_MC_BlockColors *LIBMATTI_MC_BlockColors_CreateDefault(void);

// Creates an empty color registry.
LIBMATTI_MC_BlockColors *LIBMATTI_MC_BlockColors_New(void);

// Frees the registry and its lookup storage. NULL is allowed.
void LIBMATTI_MC_BlockColors_Free(LIBMATTI_MC_BlockColors *colors);

// Returns the registered provider's tint-index-0 color without world context,
// or the block's map color when no provider exists. Returns -1 when neither is
// available; level and pos are currently unused.
int LIBMATTI_MC_BlockColors_GetColor(struct LIBMATTI_MC_BlockColors *colors,
                                     const struct LIBMATTI_MC_BlockState *state,
                                     struct LIBMATTI_MC_Level *level,
                                     const struct LIBMATTI_MC_BlockPos *pos);

// Invokes the state's registered provider with the supplied nullable level,
// position, and tint index. Returns -1 when no provider is registered.
int LIBMATTI_MC_BlockColors_GetColorIndexed(LIBMATTI_MC_BlockColors *colors,
                                            const struct LIBMATTI_MC_BlockState *state,
                                            struct LIBMATTI_MC_BlockAndTintGetter *level,
                                            const struct LIBMATTI_MC_BlockPos *pos,
                                            int tintIndex);

// Appends provider mappings for count registered blocks. If a block already
// has a provider, the earlier mapping remains the one returned by lookups.
void LIBMATTI_MC_BlockColors_Register(LIBMATTI_MC_BlockColors *colors,
                                      LIBMATTI_MC_BlockColor provider,
                                      const struct LIBMATTI_MC_Block *const *blocks, int count);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_COLOR_BLOCK_BLOCKCOLORS_H
