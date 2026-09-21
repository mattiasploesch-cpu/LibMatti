// Port of org.joml.Quaternionf.
// Formulas follow vendor/JOML src/main/java/org/joml/Quaternionf.java.

#include "libmatti/org/joml/Quaternionf.h"

#include "libmatti/org/joml/Matrix4f.h"

#include <math.h>
#include <stdlib.h>

LIBMATTI_JOML_Quaternionf *LIBMATTI_JOML_Quaternionf_New(void)
{
    LIBMATTI_JOML_Quaternionf *q = malloc(sizeof(LIBMATTI_JOML_Quaternionf));
    q->x = 0.0f;
    q->y = 0.0f;
    q->z = 0.0f;
    q->w = 1.0f;
    return q;
}

void LIBMATTI_JOML_Quaternionf_Free(LIBMATTI_JOML_Quaternionf *q)
{
    free(q);
}

LIBMATTI_JOML_Quaternionf *LIBMATTI_JOML_Quaternionf_Identity(LIBMATTI_JOML_Quaternionf *q)
{
    q->x = 0.0f;
    q->y = 0.0f;
    q->z = 0.0f;
    q->w = 1.0f;
    return q;
}

void LIBMATTI_JOML_Quaternionf_FromAxisAngle(LIBMATTI_JOML_Quaternionf *q, float angle, float ax, float ay, float az)
{
    // Java: rotationAxis - the axis must be normalized
    float length = sqrtf(ax * ax + ay * ay + az * az);
    if (length != 1.0f)
    {
        ax /= length;
        ay /= length;
        az /= length;
    }
    float sin = sinf(angle * 0.5f);
    float cos = cosf(angle * 0.5f);
    q->x = ax * sin;
    q->y = ay * sin;
    q->z = az * sin;
    q->w = cos;
}

void LIBMATTI_JOML_Quaternionf_FromAxisAngleStack(LIBMATTI_JOML_Quaternionf *q, float angle, float ax, float ay,
                                                  float az)
{
    LIBMATTI_JOML_Quaternionf_FromAxisAngle(q, angle, ax, ay, az);
}

// Java: public Quaternionf rotateAxis(float angle, float axisX, float axisY, float axisZ)
// Java: h = angle/2; sin = sin(h); rx = axisX*sin; ...; then q = rq * q
LIBMATTI_JOML_Quaternionf *LIBMATTI_JOML_Quaternionf_RotateAxis(LIBMATTI_JOML_Quaternionf *q, float angle, float ax,
                                                                float ay, float az)
{
    float h = angle * 0.5f;
    float sin = sinf(h);
    float rx = ax * sin, ry = ay * sin, rz = az * sin, rw = cosf(h);

    // Java: this.set(rw * x + rx * -w + ry * z - rz * y, ...)
    float nx = rw * q->x + rx * q->w + ry * q->z - rz * q->y;
    float ny = rw * q->y + ry * q->w + rz * q->x - rx * q->z;
    float nz = rw * q->z + rz * q->w + rx * q->y - ry * q->x;
    float nw = rw * q->w - rx * q->x - ry * q->y - rz * q->z;
    q->x = nx; q->y = ny; q->z = nz; q->w = nw;
    return q;
}

// Java: public Quaternionf mul(Quaternionfc q)
LIBMATTI_JOML_Quaternionf *LIBMATTI_JOML_Quaternionf_Mul(LIBMATTI_JOML_Quaternionf *q,
                                                         const LIBMATTI_JOML_Quaternionf *o)
{
    float nx = q->w * o->x + q->x * o->w + q->y * o->z - q->z * o->y;
    float ny = q->w * o->y - q->x * o->z + q->y * o->w + q->z * o->x;
    float nz = q->w * o->z + q->x * o->y - q->y * o->x + q->z * o->w;
    float nw = q->w * o->w - q->x * o->x - q->y * o->y - q->z * o->z;
    q->x = nx; q->y = ny; q->z = nz; q->w = nw;
    return q;
}

float LIBMATTI_JOML_Quaternionf_Length(const LIBMATTI_JOML_Quaternionf *q)
{
    return sqrtf(q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w);
}

LIBMATTI_JOML_Quaternionf *LIBMATTI_JOML_Quaternionf_Normalize(LIBMATTI_JOML_Quaternionf *q)
{
    float length = LIBMATTI_JOML_Quaternionf_Length(q);
    if (length != 0.0f)
    {
        q->x /= length;
        q->y /= length;
        q->z /= length;
        q->w /= length;
    }
    return q;
}

LIBMATTI_JOML_Quaternionf *LIBMATTI_JOML_Quaternionf_Conjugate(LIBMATTI_JOML_Quaternionf *q)
{
    q->x = -q->x;
    q->y = -q->y;
    q->z = -q->z;
    return q;
}

// Java: public Quaternionf slerp(Quaternionfc target, float alpha)
LIBMATTI_JOML_Quaternionf *LIBMATTI_JOML_Quaternionf_Slerp(LIBMATTI_JOML_Quaternionf *q,
                                                           const LIBMATTI_JOML_Quaternionf *target, float alpha)
{
    // Java: cosom = dot; handle the negative-dot shortest path
    float cosom = q->x * target->x + q->y * target->y + q->z * target->z + q->w * target->w;
    float qx = target->x, qy = target->y, qz = target->z, qw = target->w;
    if (cosom < 0.0f)
    {
        cosom = -cosom;
        qx = -qx; qy = -qy; qz = -qz; qw = -qw;
    }

    float scale0, scale1;
    if (1.0f - cosom > 1.0e-6f)
    {
        // Java: the standard slerp with the sine interpolation
        float omega = acosf(cosom);
        float sinom = sinf(omega);
        scale0 = sinf((1.0f - alpha) * omega) / sinom;
        scale1 = sinf(alpha * omega) / sinom;
    }
    else
    {
        // Java: lerp fallback for nearly identical quaternions
        scale0 = 1.0f - alpha;
        scale1 = alpha;
    }

    q->x = scale0 * q->x + scale1 * qx;
    q->y = scale0 * q->y + scale1 * qy;
    q->z = scale0 * q->z + scale1 * qz;
    q->w = scale0 * q->w + scale1 * qw;
    return q;
}

// Java: public Matrix4f get(Matrix4f dest) - the rotation matrix, JOML layout
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Quaternionf_ToMatrix4f(const LIBMATTI_JOML_Quaternionf *q,
                                                             LIBMATTI_JOML_Matrix4f *dest)
{
    float dx = q->x + q->x;
    float dy = q->y + q->y;
    float dz = q->z + q->z;
    float xx = q->x * dx, xy = q->x * dy, xz = q->x * dz;
    float yy = q->y * dy, yz = q->y * dz, zz = q->z * dz;
    float wx = q->w * dx, wy = q->w * dy, wz = q->w * dz;

    dest->m00 = 1.0f - (yy + zz);
    dest->m01 = xy + wz;
    dest->m02 = xz - wy;
    dest->m03 = 0.0f;
    dest->m10 = xy - wz;
    dest->m11 = 1.0f - (xx + zz);
    dest->m12 = yz + wx;
    dest->m13 = 0.0f;
    dest->m20 = xz + wy;
    dest->m21 = yz - wx;
    dest->m22 = 1.0f - (xx + yy);
    dest->m23 = 0.0f;
    dest->m30 = 0.0f;
    dest->m31 = 0.0f;
    dest->m32 = 0.0f;
    dest->m33 = 1.0f;
    return dest;
}

LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Quaternionf_ToMatrix4fStack(const LIBMATTI_JOML_Quaternionf *q,
                                                                  LIBMATTI_JOML_Matrix4f *dest)
{
    return LIBMATTI_JOML_Quaternionf_ToMatrix4f(q, dest);
}
