// Port of org.joml.Matrix4f (the surface the game needs).
// Formulas follow vendor/JOML src/main/java/org/joml/Matrix4f.java.

#include "libmatti/org/joml/Matrix4f.h"

#include "libmatti/org/joml/Quaternionf.h"

#include <math.h>
#include <stdlib.h>

LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_New(void)
{
    LIBMATTI_JOML_Matrix4f *m = malloc(sizeof(LIBMATTI_JOML_Matrix4f));
    LIBMATTI_JOML_Matrix4f_Identity(m);
    return m;
}

void LIBMATTI_JOML_Matrix4f_Free(LIBMATTI_JOML_Matrix4f *m)
{
    free(m);
}

// Java: public Matrix4f identity()
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Identity(LIBMATTI_JOML_Matrix4f *m)
{
    m->m00 = 1.0f; m->m01 = 0.0f; m->m02 = 0.0f; m->m03 = 0.0f;
    m->m10 = 0.0f; m->m11 = 1.0f; m->m12 = 0.0f; m->m13 = 0.0f;
    m->m20 = 0.0f; m->m21 = 0.0f; m->m22 = 1.0f; m->m23 = 0.0f;
    m->m30 = 0.0f; m->m31 = 0.0f; m->m32 = 0.0f; m->m33 = 1.0f;
    return m;
}

// Java: public Matrix4f zero()
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Zero(LIBMATTI_JOML_Matrix4f *m)
{
    for (int i = 0; i < 16; i++)
        ((float *) m)[i] = 0.0f;
    return m;
}

LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Set(LIBMATTI_JOML_Matrix4f *m,
                                                   float m00, float m01, float m02, float m03,
                                                   float m10, float m11, float m12, float m13,
                                                   float m20, float m21, float m22, float m23,
                                                   float m30, float m31, float m32, float m33)
{
    m->m00 = m00; m->m01 = m01; m->m02 = m02; m->m03 = m03;
    m->m10 = m10; m->m11 = m11; m->m12 = m12; m->m13 = m13;
    m->m20 = m20; m->m21 = m21; m->m22 = m22; m->m23 = m23;
    m->m30 = m30; m->m31 = m31; m->m32 = m32; m->m33 = m33;
    return m;
}

// Java: public Matrix4f setPerspective(float fovy, float aspect, float zNear, float zFar, boolean zZeroToOne)
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_SetPerspective(LIBMATTI_JOML_Matrix4f *m, float fovy, float aspect,
                                                              float zNear, float zFar)
{
    // Java: MemUtil.INSTANCE.zero(this); float h = Math.tan(fovy * 0.5f)
    LIBMATTI_JOML_Matrix4f_Zero(m);
    float h = tanf(fovy * 0.5f);
    m->m00 = 1.0f / (h * aspect);
    m->m11 = 1.0f / h;
    // zZeroToOne = false: m22 = (zFar + zNear) / (zNear - zFar), m32 = 2 * zFar * zNear / (zNear - zFar)
    m->m22 = (zFar + zNear) / (zNear - zFar);
    m->m32 = (zFar + zFar) * zNear / (zNear - zFar);
    m->m23 = -1.0f;
    return m;
}

// Java: public Matrix4f setOrtho(float left, float right, float bottom, float top, float zNear, float zFar)
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_SetOrtho(LIBMATTI_JOML_Matrix4f *m, float left, float right,
                                                        float bottom, float top, float zNear, float zFar)
{
    LIBMATTI_JOML_Matrix4f_Zero(m);
    m->m00 = 2.0f / (right - left);
    m->m11 = 2.0f / (top - bottom);
    m->m22 = 2.0f / (zNear - zFar);
    m->m30 = -(right + left) / (right - left);
    m->m31 = -(top + bottom) / (top - bottom);
    m->m32 = -(zFar + zNear) / (zFar - zNear);
    m->m33 = 1.0f;
    return m;
}

// Java: public Matrix4f translation(float x, float y, float z)
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Translation(LIBMATTI_JOML_Matrix4f *m, float x, float y, float z)
{
    LIBMATTI_JOML_Matrix4f_Identity(m);
    m->m30 = x;
    m->m31 = y;
    m->m32 = z;
    return m;
}

// Java: public Matrix4f rotation(float angle, float x, float y, float z) -> rotationInternal
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Rotation(LIBMATTI_JOML_Matrix4f *m, float angle, float x, float y,
                                                        float z)
{
    // Java: the fast paths for the unit axes
    if (y == 0.0f && z == 0.0f && (x == 1.0f || x == -1.0f))
        return LIBMATTI_JOML_Matrix4f_RotationX(m, x * angle);
    if (x == 0.0f && z == 0.0f && (y == 1.0f || y == -1.0f))
        return LIBMATTI_JOML_Matrix4f_RotationY(m, y * angle);
    if (x == 0.0f && y == 0.0f && (z == 1.0f || z == -1.0f))
        return LIBMATTI_JOML_Matrix4f_RotationZ(m, z * angle);

    LIBMATTI_JOML_Matrix4f_Identity(m);
    float sin = sinf(angle);
    float cos = cosf(angle);
    float oneMinusCos = 1.0f - cos;

    // Java: normalization of the axis (rotationInternal assumes a unit axis but
    // JOML divides by the length when it is not 1)
    float length = sqrtf(x * x + y * y + z * z);
    if (length != 1.0f)
    {
        x /= length;
        y /= length;
        z /= length;
    }

    float xx = x * x, yy = y * y, zz = z * z;
    float xy = x * y, xz = x * z, yz = y * z;

    // Java: the standard axis-angle matrix (column-major layout in m00..m22)
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

// Java: public Matrix4f rotationX(float angle) - m11/m12/m21/m22 filled
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_RotationX(LIBMATTI_JOML_Matrix4f *m, float angle)
{
    LIBMATTI_JOML_Matrix4f_Identity(m);
    float sin = sinf(angle);
    float cos = cosf(angle);
    m->m11 = cos;
    m->m12 = sin;
    m->m21 = -sin;
    m->m22 = cos;
    return m;
}

LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_RotationY(LIBMATTI_JOML_Matrix4f *m, float angle)
{
    LIBMATTI_JOML_Matrix4f_Identity(m);
    float sin = sinf(angle);
    float cos = cosf(angle);
    m->m00 = cos;
    m->m02 = -sin;
    m->m20 = sin;
    m->m22 = cos;
    return m;
}

LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_RotationZ(LIBMATTI_JOML_Matrix4f *m, float angle)
{
    LIBMATTI_JOML_Matrix4f_Identity(m);
    float sin = sinf(angle);
    float cos = cosf(angle);
    m->m00 = cos;
    m->m01 = sin;
    m->m10 = -sin;
    m->m11 = cos;
    return m;
}

// Java: public Matrix4f scaling(float x, float y, float z)
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Scaling(LIBMATTI_JOML_Matrix4f *m, float x, float y, float z)
{
    LIBMATTI_JOML_Matrix4f_Identity(m);
    m->m00 = x;
    m->m11 = y;
    m->m22 = z;
    return m;
}

// Java: public Matrix4f translate(float x, float y, float z) - this = this * T
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Translate(LIBMATTI_JOML_Matrix4f *m, float x, float y, float z)
{
    // Java: translation deltas apply through the rotation part
    m->m30 = m->m00 * x + m->m10 * y + m->m20 * z + m->m30;
    m->m31 = m->m01 * x + m->m11 * y + m->m21 * z + m->m31;
    m->m32 = m->m02 * x + m->m12 * y + m->m22 * z + m->m32;
    m->m33 = m->m03 * x + m->m13 * y + m->m23 * z + m->m33;
    return m;
}

// Java: public Matrix4f rotate(float ang, float x, float y, float z) - this = this * R
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Rotate(LIBMATTI_JOML_Matrix4f *m, float ang, float x, float y, float z)
{
    // Java: rotate uses a quaternion internally
    LIBMATTI_JOML_Quaternionf q;
    LIBMATTI_JOML_Quaternionf_FromAxisAngleStack(&q, ang, x, y, z);
    return LIBMATTI_JOML_Matrix4f_Mul(m, LIBMATTI_JOML_Quaternionf_ToMatrix4fStack(&q, (LIBMATTI_JOML_Matrix4f[1]){{0}}),
                                      m);
}

// Java: public Matrix4f scale(float x, float y, float z)
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Scale(LIBMATTI_JOML_Matrix4f *m, float x, float y, float z)
{
    m->m00 *= x;
    m->m01 *= x;
    m->m02 *= x;
    m->m03 *= x;
    m->m10 *= y;
    m->m11 *= y;
    m->m12 *= y;
    m->m13 *= y;
    m->m20 *= z;
    m->m21 *= z;
    m->m22 *= z;
    m->m23 *= z;
    return m;
}

// Java: public Matrix4f mul(Matrix4fc other) - this = this * other
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Mul(const LIBMATTI_JOML_Matrix4f *m,
                                                   const LIBMATTI_JOML_Matrix4f *o, LIBMATTI_JOML_Matrix4f *dest)
{
    float nm00 = m->m00 * o->m00 + m->m10 * o->m01 + m->m20 * o->m02 + m->m30 * o->m03;
    float nm01 = m->m01 * o->m00 + m->m11 * o->m01 + m->m21 * o->m02 + m->m31 * o->m03;
    float nm02 = m->m02 * o->m00 + m->m12 * o->m01 + m->m22 * o->m02 + m->m32 * o->m03;
    float nm03 = m->m03 * o->m00 + m->m13 * o->m01 + m->m23 * o->m02 + m->m33 * o->m03;
    float nm10 = m->m00 * o->m10 + m->m10 * o->m11 + m->m20 * o->m12 + m->m30 * o->m13;
    float nm11 = m->m01 * o->m10 + m->m11 * o->m11 + m->m21 * o->m12 + m->m31 * o->m13;
    float nm12 = m->m02 * o->m10 + m->m12 * o->m11 + m->m22 * o->m12 + m->m32 * o->m13;
    float nm13 = m->m03 * o->m10 + m->m13 * o->m11 + m->m23 * o->m12 + m->m33 * o->m13;
    float nm20 = m->m00 * o->m20 + m->m10 * o->m21 + m->m20 * o->m22 + m->m30 * o->m23;
    float nm21 = m->m01 * o->m20 + m->m11 * o->m21 + m->m21 * o->m22 + m->m31 * o->m23;
    float nm22 = m->m02 * o->m20 + m->m12 * o->m21 + m->m22 * o->m22 + m->m32 * o->m23;
    float nm23 = m->m03 * o->m20 + m->m13 * o->m21 + m->m23 * o->m22 + m->m33 * o->m23;
    float nm30 = m->m00 * o->m30 + m->m10 * o->m31 + m->m20 * o->m32 + m->m30 * o->m33;
    float nm31 = m->m01 * o->m30 + m->m11 * o->m31 + m->m21 * o->m32 + m->m31 * o->m33;
    float nm32 = m->m02 * o->m30 + m->m12 * o->m31 + m->m22 * o->m32 + m->m32 * o->m33;
    float nm33 = m->m03 * o->m30 + m->m13 * o->m31 + m->m23 * o->m32 + m->m33 * o->m33;

    dest->m00 = nm00; dest->m01 = nm01; dest->m02 = nm02; dest->m03 = nm03;
    dest->m10 = nm10; dest->m11 = nm11; dest->m12 = nm12; dest->m13 = nm13;
    dest->m20 = nm20; dest->m21 = nm21; dest->m22 = nm22; dest->m23 = nm23;
    dest->m30 = nm30; dest->m31 = nm31; dest->m32 = nm32; dest->m33 = nm33;
    return dest;
}

// Java: public Matrix4f invert() - the general adjugate path (JOML's invert is
// the hand-optimized standard 4x4 inverse, here in its plain form)
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Invert(const LIBMATTI_JOML_Matrix4f *m, LIBMATTI_JOML_Matrix4f *dest)
{
    float a00 = m->m00, a01 = m->m01, a02 = m->m02, a03 = m->m03;
    float a10 = m->m10, a11 = m->m11, a12 = m->m12, a13 = m->m13;
    float a20 = m->m20, a21 = m->m21, a22 = m->m22, a23 = m->m23;
    float a30 = m->m30, a31 = m->m31, a32 = m->m32, a33 = m->m33;

    float b00 = a00 * a11 - a01 * a10;
    float b01 = a00 * a12 - a02 * a10;
    float b02 = a00 * a13 - a03 * a10;
    float b03 = a01 * a12 - a02 * a11;
    float b04 = a01 * a13 - a03 * a11;
    float b05 = a02 * a13 - a03 * a12;
    float b06 = a20 * a31 - a21 * a30;
    float b07 = a20 * a32 - a22 * a30;
    float b08 = a20 * a33 - a23 * a30;
    float b09 = a21 * a32 - a22 * a31;
    float b10 = a21 * a33 - a23 * a31;
    float b11 = a22 * a33 - a23 * a32;

    // Java: det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06
    float det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
    if (det == 0.0f)
        return NULL; // Java: throws SingularMatrixException
    float invDet = 1.0f / det;

    dest->m00 = (a11 * b11 - a12 * b10 + a13 * b09) * invDet;
    dest->m01 = (-a01 * b11 + a02 * b10 - a03 * b09) * invDet;
    dest->m02 = (a31 * b05 - a32 * b04 + a33 * b03) * invDet;
    dest->m03 = (-a21 * b05 + a22 * b04 - a23 * b03) * invDet;
    dest->m10 = (-a10 * b11 + a12 * b08 - a13 * b07) * invDet;
    dest->m11 = (a00 * b11 - a02 * b08 + a03 * b07) * invDet;
    dest->m12 = (-a30 * b05 + a32 * b02 - a33 * b01) * invDet;
    dest->m13 = (a20 * b05 - a22 * b02 + a23 * b01) * invDet;
    dest->m20 = (a10 * b10 - a11 * b08 + a13 * b06) * invDet;
    dest->m21 = (-a00 * b10 + a01 * b08 - a03 * b06) * invDet;
    dest->m22 = (a30 * b04 - a31 * b02 + a33 * b00) * invDet;
    dest->m23 = (-a20 * b04 + a21 * b02 - a23 * b00) * invDet;
    dest->m30 = (-a10 * b09 + a11 * b07 - a12 * b06) * invDet;
    dest->m31 = (a00 * b09 - a01 * b07 + a02 * b06) * invDet;
    dest->m32 = (-a30 * b03 + a31 * b01 - a32 * b00) * invDet;
    dest->m33 = (a20 * b03 - a21 * b01 + a22 * b00) * invDet;
    return dest;
}

// Java: public Matrix4f transpose()
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Transpose(const LIBMATTI_JOML_Matrix4f *m,
                                                         LIBMATTI_JOML_Matrix4f *dest)
{
    LIBMATTI_JOML_Matrix4f tmp = *m;
    dest->m00 = tmp.m00; dest->m01 = tmp.m10; dest->m02 = tmp.m20; dest->m03 = tmp.m30;
    dest->m10 = tmp.m01; dest->m11 = tmp.m11; dest->m12 = tmp.m21; dest->m13 = tmp.m31;
    dest->m20 = tmp.m02; dest->m21 = tmp.m12; dest->m22 = tmp.m22; dest->m23 = tmp.m32;
    dest->m30 = tmp.m03; dest->m31 = tmp.m13; dest->m32 = tmp.m23; dest->m33 = tmp.m33;
    return dest;
}

// Java: public Vector4f transform(Vector4f v)
void LIBMATTI_JOML_Matrix4f_Transform(const LIBMATTI_JOML_Matrix4f *m, float *x, float *y, float *z, float *w)
{
    float nx = m->m00 * *x + m->m10 * *y + m->m20 * *z + m->m30 * *w;
    float ny = m->m01 * *x + m->m11 * *y + m->m21 * *z + m->m31 * *w;
    float nz = m->m02 * *x + m->m12 * *y + m->m22 * *z + m->m32 * *w;
    float nw = m->m03 * *x + m->m13 * *y + m->m23 * *z + m->m33 * *w;
    *x = nx; *y = ny; *z = nz; *w = nw;
}

// Java: public Vector3f transformPosition(Vector3f v)
void LIBMATTI_JOML_Matrix4f_TransformPosition(const LIBMATTI_JOML_Matrix4f *m, float *x, float *y, float *z)
{
    float nx = m->m00 * *x + m->m10 * *y + m->m20 * *z + m->m30;
    float ny = m->m01 * *x + m->m11 * *y + m->m21 * *z + m->m31;
    float nz = m->m02 * *x + m->m12 * *y + m->m22 * *z + m->m32;
    *x = nx; *y = ny; *z = nz;
}

// Java: public Vector3f transformDirection(Vector3f v)
void LIBMATTI_JOML_Matrix4f_TransformDirection(const LIBMATTI_JOML_Matrix4f *m, float *x, float *y, float *z)
{
    float nx = m->m00 * *x + m->m10 * *y + m->m20 * *z;
    float ny = m->m01 * *x + m->m11 * *y + m->m21 * *z;
    float nz = m->m02 * *x + m->m12 * *y + m->m22 * *z;
    *x = nx; *y = ny; *z = nz;
}
