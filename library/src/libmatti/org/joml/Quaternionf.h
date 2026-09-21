// Port of org.joml.Quaternionf.

#ifndef MATTICRAFT_JOML_QUATERNIONF_H
#define MATTICRAFT_JOML_QUATERNIONF_H

#include <stddef.h>

#include "libmatti/org/joml/Matrix4f.h"

// Java: public class Quaternionf
typedef struct
{
    float x, y, z, w;
} LIBMATTI_JOML_Quaternionf;

LIBMATTI_JOML_Quaternionf *LIBMATTI_JOML_Quaternionf_New(void);
void LIBMATTI_JOML_Quaternionf_Free(LIBMATTI_JOML_Quaternionf *q);

// Java: public Quaternionf identity() - (0, 0, 0, 1)
LIBMATTI_JOML_Quaternionf *LIBMATTI_JOML_Quaternionf_Identity(LIBMATTI_JOML_Quaternionf *q);
// Java: public Quaternionf rotateAxis(float angle, float axisX, float axisY, float axisZ)
LIBMATTI_JOML_Quaternionf *LIBMATTI_JOML_Quaternionf_RotateAxis(LIBMATTI_JOML_Quaternionf *q, float angle, float ax,
                                                                float ay, float az);
// Java: public Quaterniond rotationAxis / the axis-angle constructor value
void LIBMATTI_JOML_Quaternionf_FromAxisAngle(LIBMATTI_JOML_Quaternionf *q, float angle, float ax, float ay, float az);
// The stack variant Matrix4f.rotate uses (no allocation)
void LIBMATTI_JOML_Quaternionf_FromAxisAngleStack(LIBMATTI_JOML_Quaternionf *q, float angle, float ax, float ay,
                                                  float az);

// Java: public Quaternionf mul(Quaternionfc q) - this = this * other
LIBMATTI_JOML_Quaternionf *LIBMATTI_JOML_Quaternionf_Mul(LIBMATTI_JOML_Quaternionf *q,
                                                         const LIBMATTI_JOML_Quaternionf *o);
// Java: public float length()
float LIBMATTI_JOML_Quaternionf_Length(const LIBMATTI_JOML_Quaternionf *q);
// Java: public Quaternionf normalize()
LIBMATTI_JOML_Quaternionf *LIBMATTI_JOML_Quaternionf_Normalize(LIBMATTI_JOML_Quaternionf *q);
// Java: public Quaternionf conjugate()
LIBMATTI_JOML_Quaternionf *LIBMATTI_JOML_Quaternionf_Conjugate(LIBMATTI_JOML_Quaternionf *q);
// Java: public Quaternionf slerp(Quaternionfc target, float alpha)
LIBMATTI_JOML_Quaternionf *LIBMATTI_JOML_Quaternionf_Slerp(LIBMATTI_JOML_Quaternionf *q,
                                                           const LIBMATTI_JOML_Quaternionf *target, float alpha);

// Java: public Matrix4f get(Matrix4f dest) - the rotation matrix
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Quaternionf_ToMatrix4f(const LIBMATTI_JOML_Quaternionf *q,
                                                             LIBMATTI_JOML_Matrix4f *dest);
// Stack variant without allocation
LIBMATTI_JOML_Matrix4f *LIBMATTI_JOML_Quaternionf_ToMatrix4fStack(const LIBMATTI_JOML_Quaternionf *q,
                                                                  LIBMATTI_JOML_Matrix4f *dest);

#endif //MATTICRAFT_JOML_QUATERNIONF_H
