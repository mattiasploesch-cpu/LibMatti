// Port of org.joml.Vector2f.

#ifndef MATTICRAFT_JOML_VECTOR2F_H
#define MATTICRAFT_JOML_VECTOR2F_H

// Java: public class Vector2f
typedef struct
{
    float x, y;
} LIBMATTI_JOML_Vector2f;

LIBMATTI_JOML_Vector2f *LIBMATTI_JOML_Vector2f_New(void);
LIBMATTI_JOML_Vector2f *LIBMATTI_JOML_Vector2f_Of(float x, float y);
void LIBMATTI_JOML_Vector2f_Free(LIBMATTI_JOML_Vector2f *v);

// Java: public Vector2f set(float x, float y)
LIBMATTI_JOML_Vector2f *LIBMATTI_JOML_Vector2f_Set(LIBMATTI_JOML_Vector2f *v, float x, float y);
// Java: public Vector2f add(Vector2fc v)
LIBMATTI_JOML_Vector2f *LIBMATTI_JOML_Vector2f_Add(LIBMATTI_JOML_Vector2f *v, const LIBMATTI_JOML_Vector2f *o);
// Java: public Vector2f sub(Vector2fc v)
LIBMATTI_JOML_Vector2f *LIBMATTI_JOML_Vector2f_Sub(LIBMATTI_JOML_Vector2f *v, const LIBMATTI_JOML_Vector2f *o);
// Java: public Vector2f mul(float scalar)
LIBMATTI_JOML_Vector2f *LIBMATTI_JOML_Vector2f_MulScalar(LIBMATTI_JOML_Vector2f *v, float scalar);
// Java: public float length()
float LIBMATTI_JOML_Vector2f_Length(const LIBMATTI_JOML_Vector2f *v);
// Java: public float lengthSquared()
float LIBMATTI_JOML_Vector2f_LengthSquared(const LIBMATTI_JOML_Vector2f *v);
// Java: public Vector2f normalize()
LIBMATTI_JOML_Vector2f *LIBMATTI_JOML_Vector2f_Normalize(LIBMATTI_JOML_Vector2f *v);
// Java: public float distance(Vector2fc v)
float LIBMATTI_JOML_Vector2f_Distance(const LIBMATTI_JOML_Vector2f *a, const LIBMATTI_JOML_Vector2f *b);
// Java: public float dot(Vector2fc v)
float LIBMATTI_JOML_Vector2f_Dot(const LIBMATTI_JOML_Vector2f *a, const LIBMATTI_JOML_Vector2f *b);

#endif //MATTICRAFT_JOML_VECTOR2F_H
