// Port of net.minecraft.world.level.block.EntityBlock (implementation).

#include "libmatti/net/minecraft/world/level/block/EntityBlock.h"

#include "libmatti/net/minecraft/world/level/block/Block.h"
#include "libmatti/net/minecraft/world/level/block/entity/BlockEntity.h"
#include "libmatti/net/minecraft/world/level/block/entity/BlockEntityType.h"
#include "libmatti/net/minecraft/world/level/block/state/BlockBehaviour.h"

#include <stdlib.h>

LIBMATTI_MC_EntityBlock *LIBMATTI_MC_EntityBlock_New(
        const LIBMATTI_MC_BlockEntityType *entityType,
        LIBMATTI_MC_BlockEntity *(*newBlockEntity)(LIBMATTI_MC_Block *block,
                                                   const LIBMATTI_MC_BlockPos *pos,
                                                   LIBMATTI_MC_BlockState *state))
{
    LIBMATTI_MC_EntityBlock *entityBlock = calloc(1, sizeof(LIBMATTI_MC_EntityBlock));
    if (entityBlock == NULL)
        return NULL;
    entityBlock->entityType = entityType;
    entityBlock->newBlockEntity = newBlockEntity;
    return entityBlock;
}

// Java: block instanceof EntityBlock - the port checks the Block's vtable slot
static LIBMATTI_MC_EntityBlock *entity_block_of(const LIBMATTI_MC_Block *block)
{
    if (block == NULL)
        return NULL;
    // the Block keeps the EntityBlock vtable on its behaviour properties (the
    // optional-interface slot); NULL when the block has no entity behaviour.
    LIBMATTI_MC_BlockBehaviour_Properties *properties = LIBMATTI_MC_Block_GetProperties(block);
    if (properties == NULL)
        return NULL;
    return (LIBMATTI_MC_EntityBlock *) properties->entityBlock;
}

bool LIBMATTI_MC_EntityBlock_HasBlockEntity(const LIBMATTI_MC_Block *block)
{
    return entity_block_of(block) != NULL;
}

LIBMATTI_MC_BlockEntity *LIBMATTI_MC_EntityBlock_NewBlockEntity(LIBMATTI_MC_Block *block,
                                                                const LIBMATTI_MC_BlockPos *pos,
                                                                LIBMATTI_MC_BlockState *state)
{
    LIBMATTI_MC_EntityBlock *entityBlock = entity_block_of(block);
    if (entityBlock == NULL || entityBlock->newBlockEntity == NULL)
        return NULL;
    return entityBlock->newBlockEntity(block, pos, state);
}
