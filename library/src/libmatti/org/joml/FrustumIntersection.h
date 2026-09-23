// Port of org.joml.FrustumIntersection (vendor/JOML).
// The six frustum planes extracted from the projection*view matrix; the
// intersection tests are the JOML ones line for line.

#ifndef MATTICRAFT_JOML_FRUSTUMINTERSECTION_H
#define MATTICRAFT_JOML_FRUSTUMINTERSECTION_H

#include "libmatti/org/joml/Matrix4f.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public static final int INSIDE = -2, INTERSECT = -1
// (the two results the Minecraft Frustum checks for)
#define LIBMATTI_JOML_Ins_OUTSIDE 0
#define LIBMATTI_JOML_Ins_INSIDE -2
#define LIBMATTI_JOML_Ins_INTERSECT -1

// Java: public static final int PLANE_NX = 0 .. PLANE_PZ = 5
#define LIBMATTI_JOML_PLANE_NX 0
#define LIBMATTI_JOML_PLANE_PX 1
#define LIBMATTI_JOML_PLANE_NY 2
#define LIBMATTI_JOML_PLANE_PY 3
#define LIBMATTI_JOML_PLANE_NZ 4
#define LIBMATTI_JOML_PLANE_PZ 5

// Java: public class FrustumIntersection
// The fields are the six planes; allowTestSpheres = false keeps them
// unnormalized exactly like JOML (the AAB tests do not need unit normals).
typedef struct LIBMATTI_JOML_FrustumIntersection
{
    // Java: nxX .. pzW (plane x/y/z/w six times)
    float nxX, nxY, nxZ, nxW;
    float pxX, pxY, pxZ, pxW;
    float nyX, nyY, nyZ, nyW;
    float pyX, pyY, pyZ, pyW;
    float nzX, nzY, nzZ, nzW;
    float pzX, pzY, pzZ, pzW;
} LIBMATTI_JOML_FrustumIntersection;

// Java: public FrustumIntersection()
void LIBMATTI_JOML_FrustumIntersection_Init(LIBMATTI_JOML_FrustumIntersection *fi);

// Java: public FrustumIntersection set(Matrix4fc m) { return set(m, true); }
LIBMATTI_JOML_FrustumIntersection *LIBMATTI_JOML_FrustumIntersection_Set(
    LIBMATTI_JOML_FrustumIntersection *fi, const LIBMATTI_JOML_Matrix4f *m);
// Java: public FrustumIntersection set(Matrix4fc m, boolean allowTestSpheres)
LIBMATTI_JOML_FrustumIntersection *LIBMATTI_JOML_FrustumIntersection_SetEx(
    LIBMATTI_JOML_FrustumIntersection *fi, const LIBMATTI_JOML_Matrix4f *m, int allowTestSpheres);

// Java: public int intersectAab(float minX, float minY, float minZ,
//                               float maxX, float maxY, float maxZ)
// Returns LIBMATTI_JOML_Ins_INSIDE / _INTERSECT, or the first plane index
// (LIBMATTI_JOML_PLANE_*) that culled the box.
int LIBMATTI_JOML_FrustumIntersection_IntersectAab(
    const LIBMATTI_JOML_FrustumIntersection *fi,
    float minX, float minY, float minZ, float maxX, float maxY, float maxZ);

// Java: public boolean testPoint(float x, float y, float z)
int LIBMATTI_JOML_FrustumIntersection_TestPoint(
    const LIBMATTI_JOML_FrustumIntersection *fi, float x, float y, float z);

// Java: public float distanceToPlane(minX..maxZ, int plane)
float LIBMATTI_JOML_FrustumIntersection_DistanceToPlane(
    const LIBMATTI_JOML_FrustumIntersection *fi,
    float minX, float minY, float minZ, float maxX, float maxY, float maxZ, int plane);

#ifdef __cplusplus
}
#endif

#endif
