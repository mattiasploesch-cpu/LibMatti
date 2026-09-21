// Port of net.minecraft.world.level.block.Block (the registry-data part).
// Java's Block carries BlockBehaviour.Properties, the StateDefinition and the
// defaultBlockState; the port keeps the id mapping plus the state machinery and
// the properties surface (getProperties, getLootTable, getDescriptionId).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_BLOCK_BLOCK_H
#define MATTICRAFT_MC_WORLD_LEVEL_BLOCK_BLOCK_H

#include "libmatti/net/minecraft/core/Direction.h"
#include "libmatti/net/minecraft/resources/ResourceKey.h"
#include "libmatti/net/minecraft/world/level/block/state/BlockBehaviour.h"
#include "libmatti/net/minecraft/world/level/block/state/StateDefinition.h"
#include "libmatti/net/minecraft/world/level/block/state/BlockState.h"

#include <stddef.h>

// Java: net.minecraft.world.level.block.Block
typedef struct LIBMATTI_MC_Block
{
    // Java: the registered key (setId in the Properties)
    LIBMATTI_MC_ResourceKey *key;
    // Java: protected final BlockBehaviour.Properties properties
    LIBMATTI_MC_BlockBehaviour_Properties *properties;
    // Java: private final StateDefinition<Block, BlockState> stateDefinition
    LIBMATTI_MC_StateDefinition *stateDefinition;
} LIBMATTI_MC_Block;

// Java: Block(Properties) - the empty state definition (register(id, properties))
LIBMATTI_MC_Block *LIBMATTI_MC_Block_New(void);
// Java: the constructor path with an externally built state definition
LIBMATTI_MC_Block *LIBMATTI_MC_Block_NewWithStateDefinition(LIBMATTI_MC_StateDefinition *stateDefinition);
// Java: Block(Properties) with the properties attached (register(id, factory, properties))
LIBMATTI_MC_Block *LIBMATTI_MC_Block_NewWithProperties(LIBMATTI_MC_BlockBehaviour_Properties *properties);
// Java: Block(Properties) + a state definition built with the block as owner
LIBMATTI_MC_Block *LIBMATTI_MC_Block_NewWithPropertiesAndStateDefinition(
        LIBMATTI_MC_BlockBehaviour_Properties *properties, LIBMATTI_MC_StateDefinition *stateDefinition);

// Java: public BlockState defaultBlockState()
LIBMATTI_MC_BlockState *LIBMATTI_MC_Block_DefaultBlockState(const LIBMATTI_MC_Block *block);
// Java: public StateDefinition<Block, BlockState> getStateDefinition()
LIBMATTI_MC_StateDefinition *LIBMATTI_MC_Block_GetStateDefinition(const LIBMATTI_MC_Block *block);
// Java: BlockStateBase.getBlock() side - the registry key of this block
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Block_GetKey(const LIBMATTI_MC_Block *block);
// Java: protected BlockBehaviour.Properties getProperties() - NeoForge accessor
LIBMATTI_MC_BlockBehaviour_Properties *LIBMATTI_MC_Block_GetProperties(const LIBMATTI_MC_Block *block);
// Java: protected ResourceKey<LootTable> getLootTable() - effectiveDrops() against the id
int LIBMATTI_MC_Block_HasLootTable(const LIBMATTI_MC_Block *block);
// Java: protected String getDescriptionId()
const char *LIBMATTI_MC_Block_GetDescriptionId(const LIBMATTI_MC_Block *block);
// Java: Registry.register(BuiltInRegistries.BLOCK, key, block) - the Blocks.register tail
LIBMATTI_MC_Block *LIBMATTI_MC_Block_Register(LIBMATTI_MC_ResourceKey *key, LIBMATTI_MC_Block *block);

#endif
