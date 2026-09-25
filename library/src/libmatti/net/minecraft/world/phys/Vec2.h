// Port of net.minecraft.world.phys.Vec2 (the 2-float vector KeyboardInput's
// moveVector and the HUD math use). Java's immutability becomes value
// semantics - the constructors return the struct by value.

#ifndef MATTICRAFT_MC_WORLD_PHYS_VEC2_H
#define MATTICRAFT_MC_WORLD_PHYS_VEC2_H

#ifdef __cplusplus
extern "C" {
#endif

// Java: public final float x / public final float y
typedef struct LIBMATTI_MC_Vec2
{
    float x;
    float y;
} LIBMATTI_MC_Vec2;

// Java: the constants - the port folds them into constructors the callers take
LIBMATTI_MC_Vec2 LIBMATTI_MC_Vec2_Zero(void);
LIBMATTI_MC_Vec2 LIBMATTI_MC_Vec2_One(void);
LIBMATTI_MC_Vec2 LIBMATTI_MC_Vec2_UnitX(void);
LIBMATTI_MC_Vec2 LIBMATTI_MC_Vec2_UnitY(void);

// Java: public Vec2(float, float)
LIBMATTI_MC_Vec2 LIBMATTI_MC_Vec2_Of(float x, float y);
// Java: public Vec2 scale(float)
LIBMATTI_MC_Vec2 LIBMATTI_MC_Vec2_Scale(LIBMATTI_MC_Vec2 v, float factor);
// Java: public float dot(Vec2)
float LIBMATTI_MC_Vec2_Dot(LIBMATTI_MC_Vec2 a, LIBMATTI_MC_Vec2 b);
// Java: public Vec2 add(Vec2)
LIBMATTI_MC_Vec2 LIBMATTI_MC_Vec2_Add(LIBMATTI_MC_Vec2 a, LIBMATTI_MC_Vec2 b);
// Java: public Vec2 normalized() - the 1.0E-4F floor returns ZERO
LIBMATTI_MC_Vec2 LIBMATTI_MC_Vec2_Normalized(LIBMATTI_MC_Vec2 v);
// Java: public float length() / lengthSquared()
float LIBMATTI_MC_Vec2_Length(LIBMATTI_MC_Vec2 v);
float LIBMATTI_MC_Vec2_LengthSquared(LIBMATTI_MC_Vec2 v);
// Java: public float distanceToSqr(Vec2)
float LIBMATTI_MC_Vec2_DistanceToSqr(LIBMATTI_MC_Vec2 a, LIBMATTI_MC_Vec2 b);
// Java: public Vec2 negated()
LIBMATTI_MC_Vec2 LIBMATTI_MC_Vec2_Negated(LIBMATTI_MC_Vec2 v);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_PHYS_VEC2_H
