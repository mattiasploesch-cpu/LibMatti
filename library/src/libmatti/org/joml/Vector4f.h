// Port of org.joml.Vector4f.

#ifndef MATTICRAFT_JOML_VECTOR4F_H
#define MATTICRAFT_JOML_VECTOR4F_H

// Java: public class Vector4f
typedef struct
{
    float x, y, z, w;
} LIBMATTI_JOML_Vector4f;

LIBMATTI_JOML_Vector4f *LIBMATTI_JOML_Vector4f_New(void);
LIBMATTI_JOML_Vector4f *LIBMATTI_JOML_Vector4f_Of(float x, float y, float z, float w);
void LIBMATTI_JOML_Vector4f_Free(LIBMATTI_JOML_Vector4f *v);

// Java: public Vector4f set(float x, float y, float z, float w)
LIBMATTI_JOML_Vector4f *LIBMATTI_JOML_Vector4f_Set(LIBMATTI_JOML_Vector4f *v, float x, float y, float z, float w);
// Java: public Vector4f add(Vector4fc v)
LIBMATTI_JOML_Vector4f *LIBMATTI_JOML_Vector4f_Add(LIBMATTI_JOML_Vector4f *v, const LIBMATTI_JOML_Vector4f *o);
// Java: public Vector4f mul(float scalar)
LIBMATTI_JOML_Vector4f *LIBMATTI_JOML_Vector4f_MulScalar(LIBMATTI_JOML_Vector4f *v, float scalar);
// Java: public float length() / lengthSquared()
float LIBMATTI_JOML_Vector4f_Length(const LIBMATTI_JOML_Vector4f *v);
float LIBMATTI_JOML_Vector4f_LengthSquared(const LIBMATTI_JOML_Vector4f *v);
// Java: public Vector4f normalize()
LIBMATTI_JOML_Vector4f *LIBMATTI_JOML_Vector4f_Normalize(LIBMATTI_JOML_Vector4f *v);
// Java: public float dot(Vector4fc v)
float LIBMATTI_JOML_Vector4f_Dot(const LIBMATTI_JOML_Vector4f *a, const LIBMATTI_JOML_Vector4f *b);

#endif //MATTICRAFT_JOML_VECTOR4F_H
