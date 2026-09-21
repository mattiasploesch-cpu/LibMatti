#include "libmatti/org/joml/Vector2f.h"

#include <math.h>
#include <stdlib.h>

LIBMATTI_JOML_Vector2f *LIBMATTI_JOML_Vector2f_New(void)
{
    return calloc(1, sizeof(LIBMATTI_JOML_Vector2f));
}

LIBMATTI_JOML_Vector2f *LIBMATTI_JOML_Vector2f_Of(float x, float y)
{
    LIBMATTI_JOML_Vector2f *v = LIBMATTI_JOML_Vector2f_New();
    v->x = x;
    v->y = y;
    return v;
}

void LIBMATTI_JOML_Vector2f_Free(LIBMATTI_JOML_Vector2f *v)
{
    free(v);
}

LIBMATTI_JOML_Vector2f *LIBMATTI_JOML_Vector2f_Set(LIBMATTI_JOML_Vector2f *v, float x, float y)
{
    v->x = x;
    v->y = y;
    return v;
}

LIBMATTI_JOML_Vector2f *LIBMATTI_JOML_Vector2f_Add(LIBMATTI_JOML_Vector2f *v, const LIBMATTI_JOML_Vector2f *o)
{
    v->x += o->x;
    v->y += o->y;
    return v;
}

LIBMATTI_JOML_Vector2f *LIBMATTI_JOML_Vector2f_Sub(LIBMATTI_JOML_Vector2f *v, const LIBMATTI_JOML_Vector2f *o)
{
    v->x -= o->x;
    v->y -= o->y;
    return v;
}

LIBMATTI_JOML_Vector2f *LIBMATTI_JOML_Vector2f_MulScalar(LIBMATTI_JOML_Vector2f *v, float scalar)
{
    v->x *= scalar;
    v->y *= scalar;
    return v;
}

float LIBMATTI_JOML_Vector2f_Length(const LIBMATTI_JOML_Vector2f *v)
{
    return sqrtf(v->x * v->x + v->y * v->y);
}

float LIBMATTI_JOML_Vector2f_LengthSquared(const LIBMATTI_JOML_Vector2f *v)
{
    return v->x * v->x + v->y * v->y;
}

LIBMATTI_JOML_Vector2f *LIBMATTI_JOML_Vector2f_Normalize(LIBMATTI_JOML_Vector2f *v)
{
    // Java: float invLength = Math.invsqrt(lengthSquared())
    float length = LIBMATTI_JOML_Vector2f_Length(v);
    if (length != 0.0f)
    {
        v->x /= length;
        v->y /= length;
    }
    return v;
}

float LIBMATTI_JOML_Vector2f_Distance(const LIBMATTI_JOML_Vector2f *a, const LIBMATTI_JOML_Vector2f *b)
{
    float dx = a->x - b->x;
    float dy = a->y - b->y;
    return sqrtf(dx * dx + dy * dy);
}

float LIBMATTI_JOML_Vector2f_Dot(const LIBMATTI_JOML_Vector2f *a, const LIBMATTI_JOML_Vector2f *b)
{
    return a->x * b->x + a->y * b->y;
}
