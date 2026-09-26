// Port of net.minecraft.world.level.ClipContext (implementation).

#include "libmatti/net/minecraft/world/level/ClipContext.h"

#include "libmatti/net/minecraft/world/level/Level.h"
#include "libmatti/net/minecraft/world/level/block/Block.h"
#include "libmatti/net/minecraft/world/level/block/state/BlockBehaviour.h"

#include <stdlib.h>

// Java: CollisionShapeGetter via the entity - the port resolves the shape over
// the block's hasCollision gate exactly like the collision scan does: the full
// cube for collidable states, NULL otherwise (air/flowers/... clip empty).
static LIBMATTI_MC_AABB *collider_shape(const struct LIBMATTI_MC_Level *level, const LIBMATTI_MC_BlockPos *pos)
{
    LIBMATTI_MC_BlockState *state = LIBMATTI_MC_Level_GetBlockState((struct LIBMATTI_MC_Level *) level, pos);
    if (state == NULL)
        return NULL;
    const LIBMATTI_MC_Block *block = LIBMATTI_MC_BlockState_GetBlock(state);
    if (block == NULL || block->properties == NULL || !block->properties->hasCollision)
        return NULL;
    return LIBMATTI_MC_AABB_FromBlockPos(pos);
}

LIBMATTI_MC_ClipContext LIBMATTI_MC_ClipContext_New(const LIBMATTI_MC_Vec3 *from, const LIBMATTI_MC_Vec3 *to,
                                                    LIBMATTI_MC_ClipContext_Block block,
                                                    LIBMATTI_MC_ClipContext_Fluid fluid,
                                                    LIBMATTI_MC_ClipContext_ShapeGetter shapeGetter,
                                                    struct LIBMATTI_MC_Entity *collisionEntity)
{
    LIBMATTI_MC_ClipContext context;
    context.from = *from;
    context.to = *to;
    context.block = block;
    context.fluid = fluid;
    context.shapeGetter = shapeGetter;
    context.collisionEntity = collisionEntity;
    return context;
}

const LIBMATTI_MC_Vec3 *LIBMATTI_MC_ClipContext_GetFrom(const LIBMATTI_MC_ClipContext *context)
{
    return &context->from;
}

const LIBMATTI_MC_Vec3 *LIBMATTI_MC_ClipContext_GetTo(const LIBMATTI_MC_ClipContext *context)
{
    return &context->to;
}

LIBMATTI_MC_AABB *LIBMATTI_MC_ClipContext_GetBlockShape(const LIBMATTI_MC_ClipContext *context,
                                                        const struct LIBMATTI_MC_Level *level,
                                                        const LIBMATTI_MC_BlockPos *pos)
{
    // Java: switch (this.block) - every mode resolves the state's shape; the
    // port's shape model is the full cube behind the hasCollision gate, so the
    // modes collapse onto one resolution (FALLDAMAGE_RESETTING rides the same).
    (void) context;
    return collider_shape(level, pos);
}

LIBMATTI_MC_AABB *LIBMATTI_MC_ClipContext_GetFluidShape(const LIBMATTI_MC_ClipContext *context,
                                                        const struct LIBMATTI_MC_Level *level,
                                                        const LIBMATTI_MC_BlockPos *pos)
{
    // Java: this.fluid == Fluid.NONE ? Shapes.empty() : the fluid state's shape -
    // the port has no fluids, so only NONE exists for now.
    (void) level;
    (void) pos;
    if (context->fluid == LIBMATTI_MC_ClipContext_Fluid_NONE)
        return NULL;
    return NULL;
}
