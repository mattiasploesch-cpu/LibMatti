#include "libmatti/org/joml/Matrix3f.h"

#include <math.h>
#include <stdlib.h>

LIBMATTI_JOML_Matrix3f *LIBMATTI_JOML_Matrix3f_New(void)
{
    LIBMATTI_JOML_Matrix3f *m = malloc(sizeof(LIBMATTI_JOML_Matrix3f));
    LIBMATTI_JOML_Matrix3f_Identity(m);
    return m;
}

void LIBMATTI_JOML_Matrix3f_Free(LIBMATTI_JOML_Matrix3f *m)
{
    free(m);
}

LIBMATTI_JOML_Matrix3f *LIBMATTI_JOML_Matrix3f_Identity(LIBMATTI_JOML_Matrix3f *m)
{
    m->m00 = 1.0f; m->m01 = 0.0f; m->m02 = 0.0f;
    m->m10 = 0.0f; m->m11 = 1.0f; m->m12 = 0.0f;
    m->m20 = 0.0f; m->m21 = 0.0f; m->m22 = 1.0f;
    return m;
}

// Java: public Matrix3f rotation(float angle, float x, float y, float z)
LIBMATTI_JOML_Matrix3f *LIBMATTI_JOML_Matrix3f_Rotation(LIBMATTI_JOML_Matrix3f *m, float angle, float x, float y,
                                                        float z)
{
    LIBMATTI_JOML_Matrix3f_Identity(m);
    float length = sqrtf(x * x + y * y + z * z);
    if (length != 1.0f)
    {
        x /= length;
        y /= length;
        z /= length;
    }
    float sin = sinf(angle);
    float cos = cosf(angle);
    float oneMinusCos = 1.0f - cos;

    float xx = x * x, yy = y * y, zz = z * z;
    float xy = x * y, xz = x * z, yz = y * z;

    m->m00 = xx * oneMinusCos + cos;
    m->m01 = xy * oneMinusCos + z * sin;
    m->m02 = xz * oneMinusCos - y * sin;
    m->m10 = xy * oneMinusCos - z * sin;
    m->m11 = yy * oneMinusCos + cos;
    m->m12 = yz * oneMinusCos + x * sin;
    m->m20 = xz * oneMinusCos + y * sin;
    m->m21 = yz * oneMinusCos - x * sin;
    m->m22 = zz * oneMinusCos + cos;
    return m;
}

// Java: public Matrix3f mul(Matrix3fc other)
LIBMATTI_JOML_Matrix3f *LIBMATTI_JOML_Matrix3f_Mul(const LIBMATTI_JOML_Matrix3f *m, const LIBMATTI_JOML_Matrix3f *o,
                                                   LIBMATTI_JOML_Matrix3f *dest)
{
    float nm00 = m->m00 * o->m00 + m->m10 * o->m01 + m->m20 * o->m02;
    float nm01 = m->m01 * o->m00 + m->m11 * o->m01 + m->m21 * o->m02;
    float nm02 = m->m02 * o->m00 + m->m12 * o->m01 + m->m22 * o->m02;
    float nm10 = m->m00 * o->m10 + m->m10 * o->m11 + m->m20 * o->m12;
    float nm11 = m->m01 * o->m10 + m->m11 * o->m11 + m->m21 * o->m12;
    float nm12 = m->m02 * o->m10 + m->m12 * o->m11 + m->m22 * o->m12;
    float nm20 = m->m00 * o->m20 + m->m10 * o->m21 + m->m20 * o->m22;
    float nm21 = m->m01 * o->m20 + m->m11 * o->m21 + m->m21 * o->m22;
    float nm22 = m->m02 * o->m20 + m->m12 * o->m21 + m->m22 * o->m22;

    dest->m00 = nm00; dest->m01 = nm01; dest->m02 = nm02;
    dest->m10 = nm10; dest->m11 = nm11; dest->m12 = nm12;
    dest->m20 = nm20; dest->m21 = nm21; dest->m22 = nm22;
    return dest;
}

// Java: public Vector3f transform(Vector3f v)
void LIBMATTI_JOML_Matrix3f_Transform(const LIBMATTI_JOML_Matrix3f *m, float *x, float *y, float *z)
{
    float nx = m->m00 * *x + m->m10 * *y + m->m20 * *z;
    float ny = m->m01 * *x + m->m11 * *y + m->m21 * *z;
    float nz = m->m02 * *x + m->m12 * *y + m->m22 * *z;
    *x = nx; *y = ny; *z = nz;
}

// Java: public Matrix3f rotateTowards(float dirX, float dirY, float dirZ,
//                                     float upX, float upY, float upZ, Matrix3f dest)
LIBMATTI_JOML_Matrix3f *LIBMATTI_JOML_Matrix3f_RotateTowards(LIBMATTI_JOML_Matrix3f *m, float dirX, float dirY,
                                                             float dirZ, float upX, float upY, float upZ)
{
    // Normalize direction
    float invDirLength = 1.0f / sqrtf(dirX * dirX + dirY * dirY + dirZ * dirZ);
    float ndirX = dirX * invDirLength;
    float ndirY = dirY * invDirLength;
    float ndirZ = dirZ * invDirLength;
    // left = up x direction
    float leftX = upY * ndirZ - upZ * ndirY;
    float leftY = upZ * ndirX - upX * ndirZ;
    float leftZ = upX * ndirY - upY * ndirX;
    // normalize left
    float invLeftLength = 1.0f / sqrtf(leftX * leftX + leftY * leftY + leftZ * leftZ);
    leftX *= invLeftLength;
    leftY *= invLeftLength;
    leftZ *= invLeftLength;
    // up = direction x left
    float upnX = ndirY * leftZ - ndirZ * leftY;
    float upnY = ndirZ * leftX - ndirX * leftZ;
    float upnZ = ndirX * leftY - ndirY * leftX;
    float rm00 = leftX;
    float rm01 = leftY;
    float rm02 = leftZ;
    float rm10 = upnX;
    float rm11 = upnY;
    float rm12 = upnZ;
    float rm20 = ndirX;
    float rm21 = ndirY;
    float rm22 = ndirZ;
    float nm00 = m->m00 * rm00 + m->m10 * rm01 + m->m20 * rm02;
    float nm01 = m->m01 * rm00 + m->m11 * rm01 + m->m21 * rm02;
    float nm02 = m->m02 * rm00 + m->m12 * rm01 + m->m22 * rm02;
    float nm10 = m->m00 * rm10 + m->m10 * rm11 + m->m20 * rm12;
    float nm11 = m->m01 * rm10 + m->m11 * rm11 + m->m21 * rm12;
    float nm12 = m->m02 * rm10 + m->m12 * rm11 + m->m22 * rm12;
    float nm20 = m->m00 * rm20 + m->m10 * rm21 + m->m20 * rm22;
    float nm21 = m->m01 * rm20 + m->m11 * rm21 + m->m21 * rm22;
    float nm22 = m->m02 * rm20 + m->m12 * rm21 + m->m22 * rm22;
    m->m00 = nm00; m->m01 = nm01; m->m02 = nm02;
    m->m10 = nm10; m->m11 = nm11; m->m12 = nm12;
    m->m20 = nm20; m->m21 = nm21; m->m22 = nm22;
    return m;
}

// Java: public Matrix3f rotateZ(float ang, Matrix3f dest)
LIBMATTI_JOML_Matrix3f *LIBMATTI_JOML_Matrix3f_RotateZ(LIBMATTI_JOML_Matrix3f *m, float ang)
{
    float sin = sinf(ang);
    float cos = cosf(ang);
    float rm00 = cos;
    float rm10 = -sin;
    float rm01 = sin;
    float rm11 = cos;

    float nm00 = m->m00 * rm00 + m->m10 * rm01;
    float nm01 = m->m01 * rm00 + m->m11 * rm01;
    float nm02 = m->m02 * rm00 + m->m12 * rm01;
    float nm10 = m->m00 * rm10 + m->m10 * rm11;
    float nm11 = m->m01 * rm10 + m->m11 * rm11;
    float nm12 = m->m02 * rm10 + m->m12 * rm11;
    m->m00 = nm00; m->m01 = nm01; m->m02 = nm02;
    m->m10 = nm10; m->m11 = nm11; m->m12 = nm12;
    return m;
}
