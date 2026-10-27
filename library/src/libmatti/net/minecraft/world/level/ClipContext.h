// Port of net.minecraft.world.level.ClipContext. Java keeps the clip start/end
// as Vec3, the ShapeGetter lambda and the four enums; the port rides the
// explicit shapes (the VoxelShape port collapses to the full-block cube, so
// the BLOCK mode equals the interaction shape like Java's outline shape).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_CLIPCONTEXT_H
#define MATTICRAFT_MC_WORLD_LEVEL_CLIPCONTEXT_H

#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/world/phys/AABB.h"
#include "libmatti/net/minecraft/world/phys/BlockHitResult.h"

// forward: the clip works on the Level without pulling Level.h in (Level.h
// includes this header for the Clip surface)
struct LIBMATTI_MC_Level;
struct LIBMATTI_MC_Entity;
typedef struct LIBMATTI_MC_BlockState LIBMATTI_MC_BlockState;

#ifdef __cplusplus
extern "C" {
#endif

// Java: public static enum ClipContext.Block
typedef enum
{
    LIBMATTI_MC_ClipContext_Block_COLLIDER,
    LIBMATTI_MC_ClipContext_Block_OUTLINE,
    LIBMATTI_MC_ClipContext_Block_VISUAL,
    LIBMATTI_MC_ClipContext_Block_FALLDAMAGE_RESETTING,
} LIBMATTI_MC_ClipContext_Block;

// Java: public static enum ClipContext.Fluid - NONE keeps the fluid clip empty
typedef enum
{
    LIBMATTI_MC_ClipContext_Fluid_NONE,
    LIBMATTI_MC_ClipContext_Fluid_SOURCE_ONLY,
    LIBMATTI_MC_ClipContext_Fluid_ANY,
    LIBMATTI_MC_ClipContext_Fluid_WATER,
} LIBMATTI_MC_ClipContext_Fluid;

// Java: public static interface ShapeGetter
typedef LIBMATTI_MC_AABB *(*LIBMATTI_MC_ClipContext_ShapeGetter)(const struct LIBMATTI_MC_Level *level,
                                                                 const LIBMATTI_MC_BlockState *state,
                                                                 const LIBMATTI_MC_BlockPos *pos);

// Java: public class ClipContext
typedef struct LIBMATTI_MC_ClipContext
{
    // Java: private final Vec3 from / to
    LIBMATTI_MC_Vec3 from;
    LIBMATTI_MC_Vec3 to;
    // Java: private final ClipContext.Block block (the mode the shape getter switches on)
    LIBMATTI_MC_ClipContext_Block block;
    LIBMATTI_MC_ClipContext_Fluid fluid;
    // Java: private final CollisionShapeGetter collisionShape - the port keeps the
    // shape getter + the entity the render section picks the shapes for (unused in
    // the port's from/to shape calls)
    LIBMATTI_MC_ClipContext_ShapeGetter shapeGetter;
    struct LIBMATTI_MC_Entity *collisionEntity;
} LIBMATTI_MC_ClipContext;

// Java: public ClipContext(Vec3, Vec3, Block, Fluid, CollisionShapeGetter, Entity)
LIBMATTI_MC_ClipContext LIBMATTI_MC_ClipContext_New(const LIBMATTI_MC_Vec3 *from, const LIBMATTI_MC_Vec3 *to,
                                                    LIBMATTI_MC_ClipContext_Block block,
                                                    LIBMATTI_MC_ClipContext_Fluid fluid,
                                                    LIBMATTI_MC_ClipContext_ShapeGetter shapeGetter,
                                                    struct LIBMATTI_MC_Entity *collisionEntity);

// Java: public Vec3 getFrom() / getTo()
const LIBMATTI_MC_Vec3 *LIBMATTI_MC_ClipContext_GetFrom(const LIBMATTI_MC_ClipContext *context);
const LIBMATTI_MC_Vec3 *LIBMATTI_MC_ClipContext_GetTo(const LIBMATTI_MC_ClipContext *context);

// Java: public VoxelShape getBlockShape(BlockState, BlockGetter, BlockPos) - the
// mode switch: COLLIDER/OUTLINE/VISUAL/FALLDAMAGE_RESETTING ride the collision
// gate (air passes NULL, no shape)
LIBMATTI_MC_AABB *LIBMATTI_MC_ClipContext_GetBlockShape(const LIBMATTI_MC_ClipContext *context,
                                                        const struct LIBMATTI_MC_Level *level,
                                                        const LIBMATTI_MC_BlockPos *pos);
// Java: public VoxelShape getFluidShape(FluidState, BlockGetter, BlockPos) - NONE
// keeps the fluid clip empty
LIBMATTI_MC_AABB *LIBMATTI_MC_ClipContext_GetFluidShape(const LIBMATTI_MC_ClipContext *context,
                                                        const struct LIBMATTI_MC_Level *level,
                                                        const LIBMATTI_MC_BlockPos *pos);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_CLIPCONTEXT_H
