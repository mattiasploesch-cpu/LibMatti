// Port of net.minecraft.world.level.block.entity.BlockEntity.

#ifndef MATTICRAFT_MC_WORLD_LEVEL_BLOCKENTITY_BLOCKENTITY_H
#define MATTICRAFT_MC_WORLD_LEVEL_BLOCKENTITY_BLOCKENTITY_H

#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/world/level/block/entity/BlockEntityType.h"
#include "libmatti/net/minecraft/world/level/block/state/BlockState.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct LIBMATTI_MC_Level;

// Java: public abstract class BlockEntity
typedef struct LIBMATTI_MC_BlockEntity
{
    // Java: private final BlockEntityType<?> type
    const LIBMATTI_MC_BlockEntityType *type;
    // Java: protected @Nullable Level level
    struct LIBMATTI_MC_Level *level;
    // Java: protected final BlockPos worldPosition
    LIBMATTI_MC_BlockPos worldPosition;
    // Java: protected boolean remove
    bool remove;
    // Java: private BlockState blockState
    LIBMATTI_MC_BlockState *blockState;
    // Java: private DataComponentMap components (EMPTY until the data-component
    // model lands on block entities)
    void *components;
} LIBMATTI_MC_BlockEntity;

// Java: public BlockEntity(BlockEntityType<?>, BlockPos, BlockState) - validates
// the state against the type
LIBMATTI_MC_BlockEntity *LIBMATTI_MC_BlockEntity_New(const LIBMATTI_MC_BlockEntityType *type, const LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_BlockState *state);

// Java: public BlockPos getBlockPos()
const LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockEntity_GetBlockPos(const LIBMATTI_MC_BlockEntity *blockEntity);
// Java: public BlockState getBlockState()
LIBMATTI_MC_BlockState *LIBMATTI_MC_BlockEntity_GetBlockState(const LIBMATTI_MC_BlockEntity *blockEntity);
// Java: public BlockEntityType<?> getType()
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntity_GetType(const LIBMATTI_MC_BlockEntity *blockEntity);
// Java: public boolean isValidBlockState(BlockState)
bool LIBMATTI_MC_BlockEntity_IsValidBlockState(const LIBMATTI_MC_BlockEntity *blockEntity, LIBMATTI_MC_BlockState *state);

// Java: public @Nullable Level getLevel() / setLevel(Level) / hasLevel()
struct LIBMATTI_MC_Level *LIBMATTI_MC_BlockEntity_GetLevel(const LIBMATTI_MC_BlockEntity *blockEntity);
void LIBMATTI_MC_BlockEntity_SetLevel(LIBMATTI_MC_BlockEntity *blockEntity, struct LIBMATTI_MC_Level *level);
bool LIBMATTI_MC_BlockEntity_HasLevel(const LIBMATTI_MC_BlockEntity *blockEntity);

// Java: public boolean isRemoved() / setRemoved() / clearRemoved()
bool LIBMATTI_MC_BlockEntity_IsRemoved(const LIBMATTI_MC_BlockEntity *blockEntity);
void LIBMATTI_MC_BlockEntity_SetRemoved(LIBMATTI_MC_BlockEntity *blockEntity);
void LIBMATTI_MC_BlockEntity_ClearRemoved(LIBMATTI_MC_BlockEntity *blockEntity);

// Java: public void setBlockState(BlockState) - the state refresh (NeoForge calls
// it when the backing block changes)
void LIBMATTI_MC_BlockEntity_SetBlockState(LIBMATTI_MC_BlockEntity *blockEntity, LIBMATTI_MC_BlockState *state);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_BLOCKENTITY_BLOCKENTITY_H
