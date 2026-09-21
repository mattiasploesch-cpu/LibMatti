#include "libmatti/org/joml/Vector3f.h"

#include <math.h>
#include <stdlib.h>

LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_New(void)
{
    return calloc(1, sizeof(LIBMATTI_JOML_Vector3f));
}

LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Of(float x, float y, float z)
{
    LIBMATTI_JOML_Vector3f *v = LIBMATTI_JOML_Vector3f_New();
    v->x = x;
    v->y = y;
    v->z = z;
    return v;
}

void LIBMATTI_JOML_Vector3f_Free(LIBMATTI_JOML_Vector3f *v)
{
    free(v);
}

LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Set(LIBMATTI_JOML_Vector3f *v, float x, float y, float z)
{
    v->x = x;
    v->y = y;
    v->z = z;
    return v;
}

LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_SetV(const LIBMATTI_JOML_Vector3f *o, LIBMATTI_JOML_Vector3f *v)
{
    v->x = o->x;
    v->y = o->y;
    v->z = o->z;
    return v;
}

LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Add(LIBMATTI_JOML_Vector3f *v, const LIBMATTI_JOML_Vector3f *o)
{
    v->x += o->x;
    v->y += o->y;
    v->z += o->z;
    return v;
}

LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Sub(LIBMATTI_JOML_Vector3f *v, const LIBMATTI_JOML_Vector3f *o)
{
    v->x -= o->x;
    v->y -= o->y;
    v->z -= o->z;
    return v;
}

LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_MulScalar(LIBMATTI_JOML_Vector3f *v, float scalar)
{
    v->x *= scalar;
    v->y *= scalar;
    v->z *= scalar;
    return v;
}

LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Mul(LIBMATTI_JOML_Vector3f *v, float x, float y, float z)
{
    v->x *= x;
    v->y *= y;
    v->z *= z;
    return v;
}

LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_MulComponentwise(LIBMATTI_JOML_Vector3f *v,
                                                                const LIBMATTI_JOML_Vector3f *o)
{
    v->x *= o->x;
    v->y *= o->y;
    v->z *= o->z;
    return v;
}

float LIBMATTI_JOML_Vector3f_Length(const LIBMATTI_JOML_Vector3f *v)
{
    return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
}

float LIBMATTI_JOML_Vector3f_LengthSquared(const LIBMATTI_JOML_Vector3f *v)
{
    return v->x * v->x + v->y * v->y + v->z * v->z;
}

LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Normalize(LIBMATTI_JOML_Vector3f *v)
{
    float length = LIBMATTI_JOML_Vector3f_Length(v);
    if (length != 0.0f)
    {
        v->x /= length;
        v->y /= length;
        v->z /= length;
    }
    return v;
}

float LIBMATTI_JOML_Vector3f_Distance(const LIBMATTI_JOML_Vector3f *a, const LIBMATTI_JOML_Vector3f *b)
{
    return sqrtf(LIBMATTI_JOML_Vector3f_DistanceSquared(a, b));
}

float LIBMATTI_JOML_Vector3f_DistanceSquared(const LIBMATTI_JOML_Vector3f *a, const LIBMATTI_JOML_Vector3f *b)
{
    float dx = a->x - b->x;
    float dy = a->y - b->y;
    float dz = a->z - b->z;
    return dx * dx + dy * dy + dz * dz;
}

float LIBMATTI_JOML_Vector3f_Dot(const LIBMATTI_JOML_Vector3f *a, const LIBMATTI_JOML_Vector3f *b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

// Java: cross - (ry * oz - rz * oy, rz * ox - rx * oz, rx * oy - ry * ox)
LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Cross(LIBMATTI_JOML_Vector3f *v, const LIBMATTI_JOML_Vector3f *o)
{
    float x = v->y * o->z - v->z * o->y;
    float y = v->z * o->x - v->x * o->z;
    float z = v->x * o->y - v->y * o->x;
    v->x = x;
    v->y = y;
    v->z = z;
    return v;
}

LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Negate(LIBMATTI_JOML_Vector3f *v)
{
    v->x = -v->x;
    v->y = -v->y;
    v->z = -v->z;
    return v;
}

// Java: lerp - this + (other - this) * t
LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Lerp(LIBMATTI_JOML_Vector3f *v, const LIBMATTI_JOML_Vector3f *o,
                                                    float t)
{
    v->x += (o->x - v->x) * t;
    v->y += (o->y - v->y) * t;
    v->z += (o->z - v->z) * t;
    return v;
}

LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_RotateAxis(LIBMATTI_JOML_Vector3f *v, float angle, float ax, float ay,
                                                          float az)
{
    // Java: rotateAxisInternal - Rodrigues' rotation formula
    // fast paths for the unit axes fall through to rotateX/Y/Z
    if (ay == 0.0f && az == 0.0f && (ax == 1.0f || ax == -1.0f))
        return LIBMATTI_JOML_Vector3f_RotateX(v, ax * angle);
    if (ax == 0.0f && az == 0.0f && (ay == 1.0f || ay == -1.0f))
        return LIBMATTI_JOML_Vector3f_RotateY(v, ay * angle);
    if (ax == 0.0f && ay == 0.0f && (az == 1.0f || az == -1.0f))
        return LIBMATTI_JOML_Vector3f_RotateZ(v, az * angle);

    float sin = sinf(angle);
    float cos = cosf(angle);
    float dot = v->x * ax + v->y * ay + v->z * az;
    if (dot == 1.0f || dot == -1.0f)
        return v;
    float oneMinusCos = 1.0f - cos;

    float x = v->x * cos + dot * ax * oneMinusCos + (-az * v->y + ay * v->z) * sin;
    float y = v->y * cos + dot * ay * oneMinusCos + (az * v->x - ax * v->z) * sin;
    float z = v->z * cos + dot * az * oneMinusCos + (-ay * v->x + ax * v->y) * sin;
    v->x = x;
    v->y = y;
    v->z = z;
    return v;
}

LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_RotateX(LIBMATTI_JOML_Vector3f *v, float angle)
{
    float sin = sinf(angle);
    float cos = cosf(angle);
    float y = v->y * cos - v->z * sin;
    float z = v->y * sin + v->z * cos;
    v->y = y;
    v->z = z;
    return v;
}

LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_RotateY(LIBMATTI_JOML_Vector3f *v, float angle)
{
    float sin = sinf(angle);
    float cos = cosf(angle);
    float x = v->x * cos + v->z * sin;
    float z = -v->x * sin + v->z * cos;
    v->x = x;
    v->z = z;
    return v;
}

LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_RotateZ(LIBMATTI_JOML_Vector3f *v, float angle)
{
    float sin = sinf(angle);
    float cos = cosf(angle);
    float x = v->x * cos - v->y * sin;
    float y = v->x * sin + v->y * cos;
    v->x = x;
    v->y = y;
    return v;
}

LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Floor(LIBMATTI_JOML_Vector3f *v)
{
    v->x = floorf(v->x);
    v->y = floorf(v->y);
    v->z = floorf(v->z);
    return v;
}

LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Ceil(LIBMATTI_JOML_Vector3f *v)
{
    v->x = ceilf(v->x);
    v->y = ceilf(v->y);
    v->z = ceilf(v->z);
    return v;
}

LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Fabs(LIBMATTI_JOML_Vector3f *v)
{
    v->x = fabsf(v->x);
    v->y = fabsf(v->y);
    v->z = fabsf(v->z);
    return v;
}
