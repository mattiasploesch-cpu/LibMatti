// Port of net.minecraft.world.level.block.EntityBlock. Java models it as an
// interface the concrete Block implements (newBlockEntity + getTicker); the C
// port hangs a small vtable off the Block so the BlockEntity paths ask the
// block directly instead of scanning the 49 BlockEntityType validBlocks sets.

#ifndef MATTICRAFT_MC_WORLD_LEVEL_BLOCK_ENTITYBLOCK_H
#define MATTICRAFT_MC_WORLD_LEVEL_BLOCK_ENTITYBLOCK_H

#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/world/level/block/state/BlockState.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: the Block type (the vtable hangs off it)
struct LIBMATTI_MC_Block;
typedef struct LIBMATTI_MC_Block LIBMATTI_MC_Block;

#ifdef __cplusplus
extern "C" {
#endif

struct LIBMATTI_MC_Level;
struct LIBMATTI_MC_BlockEntityType;
struct LIBMATTI_MC_BlockEntity;
struct LIBMATTI_MC_Block;

// Java: the EntityBlock interface methods the port carries. newBlockEntity is
// the required member; getTicker arrives with the P5 block-tick port.
typedef struct LIBMATTI_MC_EntityBlock
{
    // Java: @Nullable BlockEntity newBlockEntity(BlockPos, BlockState) -
    // the concrete block creates its block entity (NULL when this block has
    // no entity behaviour)
    struct LIBMATTI_MC_BlockEntity *(*newBlockEntity)(struct LIBMATTI_MC_Block *block,
                                                      const LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_BlockState *state);
    // Java: default BlockEntityType<?> getEntityType() - the port keeps the
    // type pointer next to the factory so the save path can name it
    const struct LIBMATTI_MC_BlockEntityType *entityType;
} LIBMATTI_MC_EntityBlock;

// Java: state.getBlock() instanceof EntityBlock - the vtable presence check
bool LIBMATTI_MC_EntityBlock_HasBlockEntity(const LIBMATTI_MC_Block *block);
// Java: ((EntityBlock) block).newBlockEntity(pos, state) - NULL when the block
// carries no EntityBlock vtable or the factory is absent
struct LIBMATTI_MC_BlockEntity *LIBMATTI_MC_EntityBlock_NewBlockEntity(LIBMATTI_MC_Block *block,
                                                                       const LIBMATTI_MC_BlockPos *pos,
                                                                       LIBMATTI_MC_BlockState *state);
// The vtable the vanilla entity blocks install (the concrete factories live
// with the game content; the helper builds the vtable over a type + factory)
LIBMATTI_MC_EntityBlock *LIBMATTI_MC_EntityBlock_New(
        const struct LIBMATTI_MC_BlockEntityType *entityType,
        struct LIBMATTI_MC_BlockEntity *(*newBlockEntity)(struct LIBMATTI_MC_Block *block,
                                                          const LIBMATTI_MC_BlockPos *pos,
                                                          LIBMATTI_MC_BlockState *state));

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_BLOCK_ENTITYBLOCK_H
