#include "libmatti/org/joml/Vector4f.h"

#include <math.h>
#include <stdlib.h>

LIBMATTI_JOML_Vector4f *LIBMATTI_JOML_Vector4f_New(void)
{
    return calloc(1, sizeof(LIBMATTI_JOML_Vector4f));
}

LIBMATTI_JOML_Vector4f *LIBMATTI_JOML_Vector4f_Of(float x, float y, float z, float w)
{
    LIBMATTI_JOML_Vector4f *v = LIBMATTI_JOML_Vector4f_New();
    v->x = x;
    v->y = y;
    v->z = z;
    v->w = w;
    return v;
}

void LIBMATTI_JOML_Vector4f_Free(LIBMATTI_JOML_Vector4f *v)
{
    free(v);
}

LIBMATTI_JOML_Vector4f *LIBMATTI_JOML_Vector4f_Set(LIBMATTI_JOML_Vector4f *v, float x, float y, float z, float w)
{
    v->x = x;
    v->y = y;
    v->z = z;
    v->w = w;
    return v;
}

LIBMATTI_JOML_Vector4f *LIBMATTI_JOML_Vector4f_Add(LIBMATTI_JOML_Vector4f *v, const LIBMATTI_JOML_Vector4f *o)
{
    v->x += o->x;
    v->y += o->y;
    v->z += o->z;
    v->w += o->w;
    return v;
}

LIBMATTI_JOML_Vector4f *LIBMATTI_JOML_Vector4f_MulScalar(LIBMATTI_JOML_Vector4f *v, float scalar)
{
    v->x *= scalar;
    v->y *= scalar;
    v->z *= scalar;
    v->w *= scalar;
    return v;
}

float LIBMATTI_JOML_Vector4f_Length(const LIBMATTI_JOML_Vector4f *v)
{
    return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z + v->w * v->w);
}

float LIBMATTI_JOML_Vector4f_LengthSquared(const LIBMATTI_JOML_Vector4f *v)
{
    return v->x * v->x + v->y * v->y + v->z * v->z + v->w * v->w;
}

LIBMATTI_JOML_Vector4f *LIBMATTI_JOML_Vector4f_Normalize(LIBMATTI_JOML_Vector4f *v)
{
    float length = LIBMATTI_JOML_Vector4f_Length(v);
    if (length != 0.0f)
    {
        v->x /= length;
        v->y /= length;
        v->z /= length;
        v->w /= length;
    }
    return v;
}

float LIBMATTI_JOML_Vector4f_Dot(const LIBMATTI_JOML_Vector4f *a, const LIBMATTI_JOML_Vector4f *b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
}
