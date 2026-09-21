// Port of net.minecraft.world.level.block.state.BlockState (BlockBehaviour.BlockStateBase).
// Java's BlockStateBase carries the owner block plus the StateHolder machinery; the port
// embeds the StateHolder and adds the block accessors and the default-state link.

#ifndef MATTICRAFT_MC_BLOCKSTATE_BLOCKSTATE_H
#define MATTICRAFT_MC_BLOCKSTATE_BLOCKSTATE_H

#include "libmatti/net/minecraft/world/level/block/state/StateHolder.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class BlockState extends BlockBehaviour.BlockStateBase
typedef struct LIBMATTI_MC_BlockState
{
    // Java: the StateHolder base (owner = the Block, values, neighbours)
    LIBMATTI_MC_StateHolder holder;
} LIBMATTI_MC_BlockState;

// Java: BlockStateBase.getBlock()
void *LIBMATTI_MC_BlockState_GetBlock(const LIBMATTI_MC_BlockState *state);
// Java: BlockBehaviour.BlockStateBase.getBlock().defaultBlockState() == state check
int LIBMATTI_MC_BlockState_IsDefault(const LIBMATTI_MC_BlockState *state);

// Java: the StateHolder surface re-exposed on BlockState
LIBMATTI_MC_Property_Value LIBMATTI_MC_BlockState_GetValue(const LIBMATTI_MC_BlockState *state,
                                                           const LIBMATTI_MC_Property *property);
LIBMATTI_MC_BlockState *LIBMATTI_MC_BlockState_SetValue(LIBMATTI_MC_BlockState *state,
                                                        const LIBMATTI_MC_Property *property,
                                                        LIBMATTI_MC_Property_Value value);
LIBMATTI_MC_BlockState *LIBMATTI_MC_BlockState_TrySetValue(LIBMATTI_MC_BlockState *state,
                                                           const LIBMATTI_MC_Property *property,
                                                           LIBMATTI_MC_Property_Value value);
LIBMATTI_MC_BlockState *LIBMATTI_MC_BlockState_Cycle(LIBMATTI_MC_BlockState *state, LIBMATTI_MC_Property *property);
int LIBMATTI_MC_BlockState_HasProperty(const LIBMATTI_MC_BlockState *state, const LIBMATTI_MC_Property *property);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_BLOCKSTATE_BLOCKSTATE_H
