// Port of org.joml.Matrix3f.

#ifndef MATTICRAFT_JOML_MATRIX3F_H
#define MATTICRAFT_JOML_MATRIX3F_H

// Java: public class Matrix3f - JOML field order
typedef struct
{
    float m00, m01, m02;
    float m10, m11, m12;
    float m20, m21, m22;
} LIBMATTI_JOML_Matrix3f;

LIBMATTI_JOML_Matrix3f *LIBMATTI_JOML_Matrix3f_New(void);
void LIBMATTI_JOML_Matrix3f_Free(LIBMATTI_JOML_Matrix3f *m);

LIBMATTI_JOML_Matrix3f *LIBMATTI_JOML_Matrix3f_Identity(LIBMATTI_JOML_Matrix3f *m);
// Java: public Matrix3f rotation(float angle, float x, float y, float z)
LIBMATTI_JOML_Matrix3f *LIBMATTI_JOML_Matrix3f_Rotation(LIBMATTI_JOML_Matrix3f *m, float angle, float x, float y,
                                                        float z);
// Java: public Matrix3f mul(Matrix3fc other)
LIBMATTI_JOML_Matrix3f *LIBMATTI_JOML_Matrix3f_Mul(const LIBMATTI_JOML_Matrix3f *m, const LIBMATTI_JOML_Matrix3f *o,
                                                   LIBMATTI_JOML_Matrix3f *dest);
// Java: public Vector3f transform(Vector3f v)
void LIBMATTI_JOML_Matrix3f_Transform(const LIBMATTI_JOML_Matrix3f *m, float *x, float *y, float *z);

#endif //MATTICRAFT_JOML_MATRIX3F_H
