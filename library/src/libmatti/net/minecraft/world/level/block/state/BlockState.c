// Port of net.minecraft.world.level.block.state.BlockState.

#include "libmatti/net/minecraft/world/level/block/state/BlockState.h"

#include "libmatti/net/minecraft/world/level/block/Block.h"

void *LIBMATTI_MC_BlockState_GetBlock(const LIBMATTI_MC_BlockState *state)
{
    return state->holder.owner;
}

// Java: BlockStateBase.isAir-like default check - the port compares the block's
// defaultBlockState pointer with this state (Block.defaultBlockState())
int LIBMATTI_MC_BlockState_IsDefault(const LIBMATTI_MC_BlockState *state)
{
    LIBMATTI_MC_Block *block = state->holder.owner;
    if (block == NULL)
        return 0;
    return LIBMATTI_MC_Block_DefaultBlockState(block) == state;
}

// Java: public <T> T getValue(Property<T>)
LIBMATTI_MC_Property_Value LIBMATTI_MC_BlockState_GetValue(const LIBMATTI_MC_BlockState *state,
                                                           const LIBMATTI_MC_Property *property)
{
    return LIBMATTI_MC_StateHolder_GetValue(&state->holder, property);
}

// Java: public <T, V extends T> S setValue(Property<T>, V)
LIBMATTI_MC_BlockState *LIBMATTI_MC_BlockState_SetValue(LIBMATTI_MC_BlockState *state,
                                                        const LIBMATTI_MC_Property *property,
                                                        LIBMATTI_MC_Property_Value value)
{
    return (LIBMATTI_MC_BlockState *) LIBMATTI_MC_StateHolder_SetValue(&state->holder, property, value);
}

// Java: public <T, V extends T> S trySetValue(Property<T>, V)
LIBMATTI_MC_BlockState *LIBMATTI_MC_BlockState_TrySetValue(LIBMATTI_MC_BlockState *state,
                                                           const LIBMATTI_MC_Property *property,
                                                           LIBMATTI_MC_Property_Value value)
{
    return (LIBMATTI_MC_BlockState *) LIBMATTI_MC_StateHolder_TrySetValue(&state->holder, property, value);
}

// Java: public <T> S cycle(Property<T>)
LIBMATTI_MC_BlockState *LIBMATTI_MC_BlockState_Cycle(LIBMATTI_MC_BlockState *state, LIBMATTI_MC_Property *property)
{
    return (LIBMATTI_MC_BlockState *) LIBMATTI_MC_StateHolder_Cycle(&state->holder, property);
}

// Java: public boolean hasProperty(Property<?>)
int LIBMATTI_MC_BlockState_HasProperty(const LIBMATTI_MC_BlockState *state, const LIBMATTI_MC_Property *property)
{
    return LIBMATTI_MC_StateHolder_HasProperty(&state->holder, property);
}
