// Port of net.minecraft.world.phys.HitResult + BlockHitResult. Java keeps a
// mutable Vec3 location on the base and a nullable BlockPos on the subclass;
// the port folds both into one value struct (passed/returned by value, like
// the immutable Vec3 fields it carries) - the clip traversal returns "no hit"
// as the MISS type, the miss path fills the Java miss fields (location = the
// ray end, direction = the approximate nearest of the reverse ray).

#ifndef MATTICRAFT_NET_MINECRAFT_WORLD_PHYS_BLOCKHITRESULT_H
#define MATTICRAFT_NET_MINECRAFT_WORLD_PHYS_BLOCKHITRESULT_H

#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/core/Direction.h"
#include "libmatti/net/minecraft/world/phys/Vec3.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public static enum HitResult.Type { MISS, BLOCK, ENTITY }
typedef enum LIBMATTI_MC_HitResult_Type
{
    LIBMATTI_MC_HitResult_MISS = 0,
    LIBMATTI_MC_HitResult_BLOCK,
    LIBMATTI_MC_HitResult_ENTITY,
} LIBMATTI_MC_HitResult_Type;

// Java: public class BlockHitResult extends HitResult
typedef struct LIBMATTI_MC_BlockHitResult
{
    // Java: protected final Vec3 location (the HitResult base field)
    LIBMATTI_MC_Vec3 location;
    // Java: private final HitResult.Type type
    LIBMATTI_MC_HitResult_Type type;
    // Java: private final Direction direction - the block face the ray entered
    LIBMATTI_MC_Direction direction;
    // Java: private final BlockPos blockPos (NULL on Java's plain miss; the port
    // keeps the containing block like the miss path builds it)
    LIBMATTI_MC_BlockPos blockPos;
    // Java: private final boolean inside / isWorldBorder
    bool inside;
    bool worldBorder;
} LIBMATTI_MC_BlockHitResult;

// Java: public static BlockHitResult miss(Vec3, Direction, BlockPos)
LIBMATTI_MC_BlockHitResult LIBMATTI_MC_BlockHitResult_Miss(const LIBMATTI_MC_Vec3 *location,
                                                           LIBMATTI_MC_Direction direction,
                                                           const LIBMATTI_MC_BlockPos *blockPos);
// Java: public BlockHitResult(Vec3, Direction, BlockPos, boolean) - the block hit
LIBMATTI_MC_BlockHitResult LIBMATTI_MC_BlockHitResult_Block(const LIBMATTI_MC_Vec3 *location,
                                                            LIBMATTI_MC_Direction direction,
                                                            const LIBMATTI_MC_BlockPos *blockPos, bool inside);
// Java: public BlockHitResult withDirection(Direction) - the copy with a new face
LIBMATTI_MC_BlockHitResult LIBMATTI_MC_BlockHitResult_WithDirection(const LIBMATTI_MC_BlockHitResult *hit,
                                                                    LIBMATTI_MC_Direction direction);
// Java: static BlockHitResult createMiss(Vec3.ZERO, Direction.DOWN, BlockPos.ZERO)
// - the shared miss the GameRenderer carries before the first pick
LIBMATTI_MC_BlockHitResult LIBMATTI_MC_BlockHitResult_DefaultMiss(void);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_NET_MINECRAFT_WORLD_PHYS_BLOCKHITRESULT_H
