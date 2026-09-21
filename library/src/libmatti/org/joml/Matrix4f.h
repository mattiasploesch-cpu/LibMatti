// Port of org.joml.Matrix4f.
// The field order matches JOML exactly (m00..m32), so the struct can be
// uploaded to OpenGL with glUniformMatrix4fv without transposing.

#ifndef MATTICRAFT_JOML_MATRIX4F_H
#define MATTICRAFT_JOML_MATRIX4F_H

// Java: public class Matrix4f
typedef struct LIBMATTI_JOML_Matrix4f
{
    float m00, m01, m02, m03;
    float m10, m11, m12, m13;
    float m20, m21, m22, m23;
    float m30, m31, m32, m33;
} LIBMATTI_JOML_Matrix4f;

LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_New(void);
void LIBMATTI_JOML_Matrix4f_Free(LIBMATTI_JOML_Matrix4f *m);

// Java: public Matrix4f identity()
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Identity(LIBMATTI_JOML_Matrix4f *m);
// Java: public Matrix4f zero()
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Zero(LIBMATTI_JOML_Matrix4f *m);

// Java: public Matrix4f set(float m00, ..., float m33)
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Set(LIBMATTI_JOML_Matrix4f *m,
                                                   float m00, float m01, float m02, float m03,
                                                   float m10, float m11, float m12, float m13,
                                                   float m20, float m21, float m22, float m23,
                                                   float m30, float m31, float m32, float m33);

// Java: public Matrix4f setPerspective(float fovy, float aspect, float zNear, float zFar)
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_SetPerspective(LIBMATTI_JOML_Matrix4f *m, float fovy, float aspect,
                                                              float zNear, float zFar);
// Java: public Matrix4f setOrtho(float left, float right, float bottom, float top, float zNear, float zFar)
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_SetOrtho(LIBMATTI_JOML_Matrix4f *m, float left, float right,
                                                        float bottom, float top, float zNear, float zFar);

// Java: public Matrix4f translation(float x, float y, float z)
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Translation(LIBMATTI_JOML_Matrix4f *m, float x, float y, float z);
// Java: public Matrix4f rotation(float angle, float x, float y, float z)
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Rotation(LIBMATTI_JOML_Matrix4f *m, float angle, float x, float y,
                                                        float z);
// Java: public Matrix4f rotationX/Y/Z(float angle)
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_RotationX(LIBMATTI_JOML_Matrix4f *m, float angle);
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_RotationY(LIBMATTI_JOML_Matrix4f *m, float angle);
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_RotationZ(LIBMATTI_JOML_Matrix4f *m, float angle);
// Java: public Matrix4f scaling(float x, float y, float z)
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Scaling(LIBMATTI_JOML_Matrix4f *m, float x, float y, float z);

// Java: public Matrix4f translate(float x, float y, float z) - this = this * T
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Translate(LIBMATTI_JOML_Matrix4f *m, float x, float y, float z);
// Java: public Matrix4f rotate(float ang, float x, float y, float z) - this = this * R
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Rotate(LIBMATTI_JOML_Matrix4f *m, float ang, float x, float y, float z);
// Java: public Matrix4f scale(float x, float y, float z)
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Scale(LIBMATTI_JOML_Matrix4f *m, float x, float y, float z);

// Java: public Matrix4f mul(Matrix4fc other) - this = this * other
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Mul(const LIBMATTI_JOML_Matrix4f *m,
                                                   const LIBMATTI_JOML_Matrix4f *o, LIBMATTI_JOML_Matrix4f *dest);
// Java: public Matrix4f invert() - NULL when singular (Java throws)
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Invert(const LIBMATTI_JOML_Matrix4f *m, LIBMATTI_JOML_Matrix4f *dest);
// Java: public Matrix4f transpose()
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Matrix4f_Transpose(const LIBMATTI_JOML_Matrix4f *m,
                                                         LIBMATTI_JOML_Matrix4f *dest);

// Java: public Vector4f transform(Vector4f v) / Vector3f transformPosition(Vector3f v)
void LIBMATTI_JOML_Matrix4f_Transform(const LIBMATTI_JOML_Matrix4f *m, float *x, float *y, float *z, float *w);
void LIBMATTI_JOML_Matrix4f_TransformPosition(const LIBMATTI_JOML_Matrix4f *m, float *x, float *y, float *z);
// Java: public Vector3f transformDirection(Vector3f v) - the upper 3x3 only, not normalized
void LIBMATTI_JOML_Matrix4f_TransformDirection(const LIBMATTI_JOML_Matrix4f *m, float *x, float *y, float *z);

#endif //MATTICRAFT_JOML_MATRIX4F_H
