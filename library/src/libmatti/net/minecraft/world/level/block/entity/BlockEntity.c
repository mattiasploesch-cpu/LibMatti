// Port of net.minecraft.world.level.block.entity.BlockEntity.

#include "libmatti/net/minecraft/world/level/block/entity/BlockEntity.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_MC_BlockEntity *LIBMATTI_MC_BlockEntity_New(const LIBMATTI_MC_BlockEntityType *type, const LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_BlockState *state)
{
    LIBMATTI_MC_BlockEntity *blockEntity = calloc(1, sizeof(LIBMATTI_MC_BlockEntity));
    blockEntity->type = type;
    blockEntity->worldPosition = *pos;
    blockEntity->blockState = state;
    return blockEntity;
}

const LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockEntity_GetBlockPos(const LIBMATTI_MC_BlockEntity *blockEntity)
{
    return &blockEntity->worldPosition;
}

LIBMATTI_MC_BlockState *LIBMATTI_MC_BlockEntity_GetBlockState(const LIBMATTI_MC_BlockEntity *blockEntity)
{
    return blockEntity->blockState;
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntity_GetType(const LIBMATTI_MC_BlockEntity *blockEntity)
{
    return blockEntity->type;
}

bool LIBMATTI_MC_BlockEntity_IsValidBlockState(const LIBMATTI_MC_BlockEntity *blockEntity, LIBMATTI_MC_BlockState *state)
{
    return LIBMATTI_MC_BlockEntityType_IsValid(blockEntity->type, state);
}

struct LIBMATTI_MC_Level *LIBMATTI_MC_BlockEntity_GetLevel(const LIBMATTI_MC_BlockEntity *blockEntity)
{
    return blockEntity->level;
}

void LIBMATTI_MC_BlockEntity_SetLevel(LIBMATTI_MC_BlockEntity *blockEntity, struct LIBMATTI_MC_Level *level)
{
    blockEntity->level = level;
}

bool LIBMATTI_MC_BlockEntity_HasLevel(const LIBMATTI_MC_BlockEntity *blockEntity)
{
    return blockEntity->level != NULL;
}

bool LIBMATTI_MC_BlockEntity_IsRemoved(const LIBMATTI_MC_BlockEntity *blockEntity)
{
    return blockEntity->remove;
}

void LIBMATTI_MC_BlockEntity_SetRemoved(LIBMATTI_MC_BlockEntity *blockEntity)
{
    blockEntity->remove = true;
}

void LIBMATTI_MC_BlockEntity_ClearRemoved(LIBMATTI_MC_BlockEntity *blockEntity)
{
    blockEntity->remove = false;
}

void LIBMATTI_MC_BlockEntity_SetBlockState(LIBMATTI_MC_BlockEntity *blockEntity, LIBMATTI_MC_BlockState *state)
{
    blockEntity->blockState = state;
}
