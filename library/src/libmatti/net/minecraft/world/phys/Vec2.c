// Port of net.minecraft.world.phys.Vec2 (implementation).

#include "libmatti/net/minecraft/world/phys/Vec2.h"

#include "libmatti/net/minecraft/util/Mth.h"

#include <math.h>

LIBMATTI_MC_Vec2 LIBMATTI_MC_Vec2_Zero(void)
{
    LIBMATTI_MC_Vec2 v = {0.0f, 0.0f};
    return v;
}

LIBMATTI_MC_Vec2 LIBMATTI_MC_Vec2_One(void)
{
    LIBMATTI_MC_Vec2 v = {1.0f, 1.0f};
    return v;
}

LIBMATTI_MC_Vec2 LIBMATTI_MC_Vec2_UnitX(void)
{
    LIBMATTI_MC_Vec2 v = {1.0f, 0.0f};
    return v;
}

LIBMATTI_MC_Vec2 LIBMATTI_MC_Vec2_UnitY(void)
{
    LIBMATTI_MC_Vec2 v = {0.0f, 1.0f};
    return v;
}

// Java: public Vec2(float p_82474_, float p_82475_)
LIBMATTI_MC_Vec2 LIBMATTI_MC_Vec2_Of(float x, float y)
{
    LIBMATTI_MC_Vec2 v = {x, y};
    return v;
}

// Java: public Vec2 scale(float p_165904_)
LIBMATTI_MC_Vec2 LIBMATTI_MC_Vec2_Scale(LIBMATTI_MC_Vec2 v, float factor)
{
    return LIBMATTI_MC_Vec2_Of(v.x * factor, v.y * factor);
}

// Java: public float dot(Vec2 p_165906_)
float LIBMATTI_MC_Vec2_Dot(LIBMATTI_MC_Vec2 a, LIBMATTI_MC_Vec2 b)
{
    return a.x * b.x + a.y * b.y;
}

// Java: public Vec2 add(Vec2 p_165911_)
LIBMATTI_MC_Vec2 LIBMATTI_MC_Vec2_Add(LIBMATTI_MC_Vec2 a, LIBMATTI_MC_Vec2 b)
{
    return LIBMATTI_MC_Vec2_Of(a.x + b.x, a.y + b.y);
}

// Java: public Vec2 normalized() - f < 1.0E-4F returns ZERO
LIBMATTI_MC_Vec2 LIBMATTI_MC_Vec2_Normalized(LIBMATTI_MC_Vec2 v)
{
    // Java: float f = Mth.sqrt(this.x * this.x + this.y * this.y)
    float f = LIBMATTI_MC_Mth_Sqrt(v.x * v.x + v.y * v.y);
    if (f < 1.0e-4f)
        return LIBMATTI_MC_Vec2_Zero();
    return LIBMATTI_MC_Vec2_Of(v.x / f, v.y / f);
}

// Java: public float length()
float LIBMATTI_MC_Vec2_Length(LIBMATTI_MC_Vec2 v)
{
    return LIBMATTI_MC_Mth_Sqrt(v.x * v.x + v.y * v.y);
}

// Java: public float lengthSquared()
float LIBMATTI_MC_Vec2_LengthSquared(LIBMATTI_MC_Vec2 v)
{
    return v.x * v.x + v.y * v.y;
}

// Java: public float distanceToSqr(Vec2 p_165915_)
float LIBMATTI_MC_Vec2_DistanceToSqr(LIBMATTI_MC_Vec2 a, LIBMATTI_MC_Vec2 b)
{
    float f = b.x - a.x;
    float f1 = b.y - a.y;
    return f * f + f1 * f1;
}

// Java: public Vec2 negated()
LIBMATTI_MC_Vec2 LIBMATTI_MC_Vec2_Negated(LIBMATTI_MC_Vec2 v)
{
    return LIBMATTI_MC_Vec2_Of(-v.x, -v.y);
}
