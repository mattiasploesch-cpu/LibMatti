// Port of net.minecraft.world.level.block.Block (the registry-data part).

#include "libmatti/net/minecraft/world/level/block/Block.h"

#include "libmatti/net/minecraft/core/MappedRegistry.h"
#include "libmatti/net/minecraft/core/Registry.h"
#include "libmatti/net/minecraft/core/registries/BuiltInRegistries.h"
#include "libmatti/net/minecraft/world/level/block/state/StateDefinition.h"

#include <stdlib.h>

// Java: the StateDefinition is created inside the Block constructor with `this` as
// the owner - exactly one block owns the table
static void attach_state_definition(LIBMATTI_MC_Block *block, LIBMATTI_MC_StateDefinition *stateDefinition)
{
    stateDefinition->owner = block;
    int stateCount = 0;
    LIBMATTI_MC_StateHolder **states =
            LIBMATTI_MC_StateDefinition_GetPossibleStates(stateDefinition, &stateCount);
    for (int i = 0; i < stateCount; i++)
        states[i]->owner = block;
    block->stateDefinition = stateDefinition;
}

// Java: Block(BlockBehaviour.Properties) - the empty state definition when the
// constructor runs without a state supplier
static LIBMATTI_MC_Block *alloc_block(LIBMATTI_MC_BlockBehaviour_Properties *properties)
{
    LIBMATTI_MC_Block *block = calloc(1, sizeof(LIBMATTI_MC_Block));
    block->properties = properties;
    LIBMATTI_MC_StateDefinition_Builder *builder = LIBMATTI_MC_StateDefinition_Builder_New(block);
    block->stateDefinition = LIBMATTI_MC_StateDefinition_Builder_Create(builder);
    return block;
}

LIBMATTI_MC_Block *LIBMATTI_MC_Block_New(void)
{
    // Java: Block(BlockBehaviour.Properties.of())
    return alloc_block(LIBMATTI_MC_BlockBehaviour_Properties_Of());
}

LIBMATTI_MC_Block *LIBMATTI_MC_Block_NewWithProperties(LIBMATTI_MC_BlockBehaviour_Properties *properties)
{
    return alloc_block(properties);
}

LIBMATTI_MC_Block *LIBMATTI_MC_Block_NewWithPropertiesAndStateDefinition(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, LIBMATTI_MC_StateDefinition *stateDefinition)
{
    LIBMATTI_MC_Block *block = calloc(1, sizeof(LIBMATTI_MC_Block));
    block->properties = properties;
    attach_state_definition(block, stateDefinition);
    return block;
}

LIBMATTI_MC_Block *LIBMATTI_MC_Block_NewWithStateDefinition(LIBMATTI_MC_StateDefinition *stateDefinition)
{
    LIBMATTI_MC_Block *block = calloc(1, sizeof(LIBMATTI_MC_Block));
    block->properties = LIBMATTI_MC_BlockBehaviour_Properties_Of();
    attach_state_definition(block, stateDefinition);
    return block;
}

// Java: public BlockState defaultBlockState()
LIBMATTI_MC_BlockState *LIBMATTI_MC_Block_DefaultBlockState(const LIBMATTI_MC_Block *block)
{
    return (LIBMATTI_MC_BlockState *) LIBMATTI_MC_StateDefinition_Any(block->stateDefinition);
}

// Java: public StateDefinition<Block, BlockState> getStateDefinition()
LIBMATTI_MC_StateDefinition *LIBMATTI_MC_Block_GetStateDefinition(const LIBMATTI_MC_Block *block)
{
    return block->stateDefinition;
}

// Java: the registry key set through Block.setId
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Block_GetKey(const LIBMATTI_MC_Block *block)
{
    return block->key;
}

// Java: protected BlockBehaviour.Properties getProperties()
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_Block_GetProperties(const LIBMATTI_MC_Block *block)
{
    return block->properties;
}

// Java: protected ResourceKey<LootTable> getLootTable() - hasLootTable against the id
int LIBMATTI_MC_Block_HasLootTable(const LIBMATTI_MC_Block *block)
{
    return LIBMATTI_MC_BlockBehaviour_Properties_EffectiveDrops(block->properties);
}

// Java: protected String getDescriptionId()
const char *LIBMATTI_MC_Block_GetDescriptionId(const LIBMATTI_MC_Block *block)
{
    return LIBMATTI_MC_BlockBehaviour_Properties_EffectiveDescription(block->properties);
}

// Java: Blocks.register tail - Registry.register(BuiltInRegistries.BLOCK, key, block)
LIBMATTI_MC_Block *LIBMATTI_MC_Block_Register(LIBMATTI_MC_ResourceKey *key, LIBMATTI_MC_Block *block)
{
    block->key = key;
    LIBMATTI_MC_BlockBehaviour_Properties_SetId(block->properties, key);
    LIBMATTI_MC_Registry_Register(LIBMATTI_MC_BuiltInRegistries_BLOCK(), key, block);
    return block;
}
