// Port of org.joml.Vector3f.

#ifndef MATTICRAFT_JOML_VECTOR3F_H
#define MATTICRAFT_JOML_VECTOR3F_H

// Java: public class Vector3f
typedef struct
{
    float x, y, z;
} LIBMATTI_JOML_Vector3f;

LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_New(void);
LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Of(float x, float y, float z);
void LIBMATTI_JOML_Vector3f_Free(LIBMATTI_JOML_Vector3f *v);

LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Set(LIBMATTI_JOML_Vector3f *v, float x, float y, float z);
LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_SetV(const LIBMATTI_JOML_Vector3f *o, LIBMATTI_JOML_Vector3f *v);

// Java: public Vector3f add(Vector3fc v)
LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Add(LIBMATTI_JOML_Vector3f *v, const LIBMATTI_JOML_Vector3f *o);
// Java: public Vector3f sub(Vector3fc v)
LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Sub(LIBMATTI_JOML_Vector3f *v, const LIBMATTI_JOML_Vector3f *o);
// Java: public Vector3f mul(float scalar)
LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_MulScalar(LIBMATTI_JOML_Vector3f *v, float scalar);
// Java: public Vector3f mul(float x, float y, float z)
LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Mul(LIBMATTI_JOML_Vector3f *v, float x, float y, float z);
// Java: public Vector3f mulComponentwise(Vector3fc v)
LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_MulComponentwise(LIBMATTI_JOML_Vector3f *v,
                                                                const LIBMATTI_JOML_Vector3f *o);

// Java: public float length() / lengthSquared()
float LIBMATTI_JOML_Vector3f_Length(const LIBMATTI_JOML_Vector3f *v);
float LIBMATTI_JOML_Vector3f_LengthSquared(const LIBMATTI_JOML_Vector3f *v);
// Java: public Vector3f normalize()
LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Normalize(LIBMATTI_JOML_Vector3f *v);
// Java: public float distance(Vector3fc v)
float LIBMATTI_JOML_Vector3f_Distance(const LIBMATTI_JOML_Vector3f *a, const LIBMATTI_JOML_Vector3f *b);
// Java: public float distanceSquared(Vector3fc v)
float LIBMATTI_JOML_Vector3f_DistanceSquared(const LIBMATTI_JOML_Vector3f *a, const LIBMATTI_JOML_Vector3f *b);
// Java: public float dot(Vector3fc v)
float LIBMATTI_JOML_Vector3f_Dot(const LIBMATTI_JOML_Vector3f *a, const LIBMATTI_JOML_Vector3f *b);
// Java: public Vector3f cross(Vector3fc v) - this = this x other
LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Cross(LIBMATTI_JOML_Vector3f *v, const LIBMATTI_JOML_Vector3f *o);
// Java: public Vector3f negate()
LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Negate(LIBMATTI_JOML_Vector3f *v);
// Java: public Vector3f lerp(Vector3fc other, float t)
LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Lerp(LIBMATTI_JOML_Vector3f *v, const LIBMATTI_JOML_Vector3f *o,
                                                    float t);
// Java: public Vector3f rotateAxis(float angle, float x, float y, float z)
LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_RotateAxis(LIBMATTI_JOML_Vector3f *v, float angle, float ax, float ay,
                                                          float az);
// Java: public Vector3f rotateX/rotateY/rotateZ(float angle)
LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_RotateX(LIBMATTI_JOML_Vector3f *v, float angle);
LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_RotateY(LIBMATTI_JOML_Vector3f *v, float angle);
LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_RotateZ(LIBMATTI_JOML_Vector3f *v, float angle);

// Java: public Vector3f floor/ceil/fabs
LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Floor(LIBMATTI_JOML_Vector3f *v);
LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Ceil(LIBMATTI_JOML_Vector3f *v);
LIBMATTI_JOML_Vector3f *LIBMATTI_JOML_Vector3f_Fabs(LIBMATTI_JOML_Vector3f *v);

#endif //MATTICRAFT_JOML_VECTOR3F_H
