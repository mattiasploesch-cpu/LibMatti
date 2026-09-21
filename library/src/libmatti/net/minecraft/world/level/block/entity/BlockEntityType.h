// Port of net.minecraft.world.level.block.entity.BlockEntityType.

#ifndef MATTICRAFT_MC_WORLD_LEVEL_BLOCKENTITY_BLOCKENTITYTYPE_H
#define MATTICRAFT_MC_WORLD_LEVEL_BLOCKENTITY_BLOCKENTITYTYPE_H

#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/world/level/block/state/BlockState.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class BlockEntityType<T extends BlockEntity> - the port's type
// carries the registry id, the valid-block set and the factory the concrete
// BlockEntity subclass installs (the typed data subclasses carry is game-port
// content)
typedef struct LIBMATTI_MC_BlockEntityType
{
    // Java: the registered id (registry key path)
    const char *id;
    // Java: private final Set<Block> validBlocks - resolved VanillaBlocks constants
    const void **validBlocks;
    size_t validBlockCount;
    // Java: BlockEntityType.BlockEntitySupplier<T> - creates the instance
    void *(*factory)(const struct LIBMATTI_MC_BlockEntityType *type, const LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_BlockState *state);
} LIBMATTI_MC_BlockEntityType;

// Java: public boolean isValid(BlockState) - the state's block is in validBlocks
bool LIBMATTI_MC_BlockEntityType_IsValid(const LIBMATTI_MC_BlockEntityType *type, LIBMATTI_MC_BlockState *state);
// Java: public T create(BlockPos, BlockState) - through the supplier
void *LIBMATTI_MC_BlockEntityType_Create(const LIBMATTI_MC_BlockEntityType *type, const LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_BlockState *state);
// Java: the registered id ("minecraft:<id>")
const char *LIBMATTI_MC_BlockEntityType_GetId(const LIBMATTI_MC_BlockEntityType *type);
// the by-index access over all 49 vanilla types for the hasBlockEntity/create scan
// (Java: getBlock() instanceof EntityBlock); NULL outside 0..48
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_AccessIndex(int index);

// Java: the 49 static constants - resolved lazily (BuiltInRegistries registration
// order), one accessor per constant
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_FURNACE(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_CHEST(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_TRAPPED_CHEST(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_ENDER_CHEST(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_JUKEBOX(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_DISPENSER(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_DROPPER(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_SIGN(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_HANGING_SIGN(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_MOB_SPAWNER(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_CREAKING_HEART(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_PISTON(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_BREWING_STAND(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_ENCHANTING_TABLE(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_END_PORTAL(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_BEACON(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_SKULL(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_DAYLIGHT_DETECTOR(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_HOPPER(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_COMPARATOR(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_BANNER(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_STRUCTURE_BLOCK(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_END_GATEWAY(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_COMMAND_BLOCK(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_SHULKER_BOX(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_BED(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_CONDUIT(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_BARREL(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_SMOKER(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_BLAST_FURNACE(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_LECTERN(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_BELL(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_JIGSAW(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_CAMPFIRE(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_BEEHIVE(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_SCULK_SENSOR(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_CALIBRATED_SCULK_SENSOR(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_SCULK_CATALYST(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_SCULK_SHRIEKER(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_CHISELED_BOOKSHELF(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_SHELF(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_BRUSHABLE_BLOCK(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_DECORATED_POT(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_CRAFTER(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_TRIAL_SPAWNER(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_VAULT(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_TEST_BLOCK(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_TEST_INSTANCE_BLOCK(void);
const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntityType_COPPER_GOLEM_STATUE(void);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_BLOCKENTITY_BLOCKENTITYTYPE_H
