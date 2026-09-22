// Port of org.joml.FrustumIntersection (implementation).
// The plane extraction and the tests follow the vendored JOML source line
// for line (allowTestSpheres = false: unnormalized planes, like the AAB
// tests need them).

#include "libmatti/org/joml/FrustumIntersection.h"

#include <math.h>

// Java: public FrustumIntersection() - zeroed planes (no frustum yet).
void LIBMATTI_JOML_FrustumIntersection_Init(LIBMATTI_JOML_FrustumIntersection *fi)
{
    fi->nxX = 0; fi->nxY = 0; fi->nxZ = 0; fi->nxW = 0;
    fi->pxX = 0; fi->pxY = 0; fi->pxZ = 0; fi->pxW = 0;
    fi->nyX = 0; fi->nyY = 0; fi->nyZ = 0; fi->nyW = 0;
    fi->pyX = 0; fi->pyY = 0; fi->pyZ = 0; fi->pyW = 0;
    fi->nzX = 0; fi->nzY = 0; fi->nzZ = 0; fi->nzW = 0;
    fi->pzX = 0; fi->pzY = 0; fi->pzZ = 0; fi->pzW = 0;
}

// Java: public FrustumIntersection set(Matrix4fc m) { return set(m, true); }
LIBMATTI_JOML_FrustumIntersection *LIBMATTI_JOML_FrustumIntersection_Set(
    LIBMATTI_JOML_FrustumIntersection *fi, const LIBMATTI_JOML_Matrix4f *m)
{
    return LIBMATTI_JOML_FrustumIntersection_SetEx(fi, m, 1);
}

// Java: public FrustumIntersection set(Matrix4fc m, boolean allowTestSpheres)
// Reference: "Fast Extraction of Viewing Frustum Planes from the
// World-View-Projection Matrix" (gamedevs.org).
LIBMATTI_JOML_FrustumIntersection *LIBMATTI_JOML_FrustumIntersection_SetEx(
    LIBMATTI_JOML_FrustumIntersection *fi, const LIBMATTI_JOML_Matrix4f *m, int allowTestSpheres)
{
    // JOML names the fields m<column><row> (the translation sits in
    // m30..m32) - exactly the C struct's convention, so Java mXY() == C.mXY.
    fi->nxX = m->m00 + m->m03; fi->nxY = m->m10 + m->m13; fi->nxZ = m->m20 + m->m23; fi->nxW = m->m30 + m->m33;
    if (allowTestSpheres)
    {
        float invl = 1.0f / sqrtf(fi->nxX * fi->nxX + fi->nxY * fi->nxY + fi->nxZ * fi->nxZ);
        fi->nxX *= invl; fi->nxY *= invl; fi->nxZ *= invl; fi->nxW *= invl;
    }
    fi->pxX = m->m03 - m->m00; fi->pxY = m->m13 - m->m10; fi->pxZ = m->m23 - m->m20; fi->pxW = m->m33 - m->m30;
    if (allowTestSpheres)
    {
        float invl = 1.0f / sqrtf(fi->pxX * fi->pxX + fi->pxY * fi->pxY + fi->pxZ * fi->pxZ);
        fi->pxX *= invl; fi->pxY *= invl; fi->pxZ *= invl; fi->pxW *= invl;
    }
    fi->nyX = m->m01 + m->m03; fi->nyY = m->m11 + m->m13; fi->nyZ = m->m21 + m->m23; fi->nyW = m->m31 + m->m33;
    if (allowTestSpheres)
    {
        float invl = 1.0f / sqrtf(fi->nyX * fi->nyX + fi->nyY * fi->nyY + fi->nyZ * fi->nyZ);
        fi->nyX *= invl; fi->nyY *= invl; fi->nyZ *= invl; fi->nyW *= invl;
    }
    fi->pyX = m->m03 - m->m01; fi->pyY = m->m13 - m->m11; fi->pyZ = m->m23 - m->m21; fi->pyW = m->m33 - m->m31;
    if (allowTestSpheres)
    {
        float invl = 1.0f / sqrtf(fi->pyX * fi->pyX + fi->pyY * fi->pyY + fi->pyZ * fi->pyZ);
        fi->pyX *= invl; fi->pyY *= invl; fi->pyZ *= invl; fi->pyW *= invl;
    }
    fi->nzX = m->m02 + m->m03; fi->nzY = m->m12 + m->m13; fi->nzZ = m->m22 + m->m23; fi->nzW = m->m32 + m->m33;
    if (allowTestSpheres)
    {
        float invl = 1.0f / sqrtf(fi->nzX * fi->nzX + fi->nzY * fi->nzY + fi->nzZ * fi->nzZ);
        fi->nzX *= invl; fi->nzY *= invl; fi->nzZ *= invl; fi->nzW *= invl;
    }
    fi->pzX = m->m03 - m->m02; fi->pzY = m->m13 - m->m12; fi->pzZ = m->m23 - m->m22; fi->pzW = m->m33 - m->m32;
    if (allowTestSpheres)
    {
        float invl = 1.0f / sqrtf(fi->pzX * fi->pzX + fi->pzY * fi->pzY + fi->pzZ * fi->pzZ);
        fi->pzX *= invl; fi->pzY *= invl; fi->pzZ *= invl; fi->pzW *= invl;
    }
    return fi;
}

// Java: public int intersectAab(float minX, float minY, float minZ,
//                               float maxX, float maxY, float maxZ)
// The "2.4 Basic intersection test" nesting: the first plane whose
// negative-corner test fails culls the box, the positive-corner tests
// accumulate the INSIDE result.
int LIBMATTI_JOML_FrustumIntersection_IntersectAab(
    const LIBMATTI_JOML_FrustumIntersection *fi,
    float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
{
    int plane = LIBMATTI_JOML_PLANE_NX;
    int inside = 1;
    if (fi->nxX * (fi->nxX < 0 ? minX : maxX) + fi->nxY * (fi->nxY < 0 ? minY : maxY)
            + fi->nxZ * (fi->nxZ < 0 ? minZ : maxZ) >= -fi->nxW)
    {
        plane = LIBMATTI_JOML_PLANE_PX;
        inside &= fi->nxX * (fi->nxX < 0 ? maxX : minX) + fi->nxY * (fi->nxY < 0 ? maxY : minY)
                      + fi->nxZ * (fi->nxZ < 0 ? maxZ : minZ) >= -fi->nxW;
        if (fi->pxX * (fi->pxX < 0 ? minX : maxX) + fi->pxY * (fi->pxY < 0 ? minY : maxY)
                + fi->pxZ * (fi->pxZ < 0 ? minZ : maxZ) >= -fi->pxW)
        {
            plane = LIBMATTI_JOML_PLANE_NY;
            inside &= fi->pxX * (fi->pxX < 0 ? maxX : minX) + fi->pxY * (fi->pxY < 0 ? maxY : minY)
                          + fi->pxZ * (fi->pxZ < 0 ? maxZ : minZ) >= -fi->pxW;
            if (fi->nyX * (fi->nyX < 0 ? minX : maxX) + fi->nyY * (fi->nyY < 0 ? minY : maxY)
                    + fi->nyZ * (fi->nyZ < 0 ? minZ : maxZ) >= -fi->nyW)
            {
                plane = LIBMATTI_JOML_PLANE_PY;
                inside &= fi->nyX * (fi->nyX < 0 ? maxX : minX) + fi->nyY * (fi->nyY < 0 ? maxY : minY)
                              + fi->nyZ * (fi->nyZ < 0 ? maxZ : minZ) >= -fi->nyW;
                if (fi->pyX * (fi->pyX < 0 ? minX : maxX) + fi->pyY * (fi->pyY < 0 ? minY : maxY)
                        + fi->pyZ * (fi->pyZ < 0 ? minZ : maxZ) >= -fi->pyW)
                {
                    plane = LIBMATTI_JOML_PLANE_NZ;
                    inside &= fi->pyX * (fi->pyX < 0 ? maxX : minX) + fi->pyY * (fi->pyY < 0 ? maxY : minY)
                                  + fi->pyZ * (fi->pyZ < 0 ? maxZ : minZ) >= -fi->pyW;
                    if (fi->nzX * (fi->nzX < 0 ? minX : maxX) + fi->nzY * (fi->nzY < 0 ? minY : maxY)
                            + fi->nzZ * (fi->nzZ < 0 ? minZ : maxZ) >= -fi->nzW)
                    {
                        plane = LIBMATTI_JOML_PLANE_PZ;
                        inside &= fi->nzX * (fi->nzX < 0 ? maxX : minX) + fi->nzY * (fi->nzY < 0 ? maxY : minY)
                                      + fi->nzZ * (fi->nzZ < 0 ? maxZ : minZ) >= -fi->nzW;
                        if (fi->pzX * (fi->pzX < 0 ? minX : maxX) + fi->pzY * (fi->pzY < 0 ? minY : maxY)
                                + fi->pzZ * (fi->pzZ < 0 ? minZ : maxZ) >= -fi->pzW)
                        {
                            inside &= fi->pzX * (fi->pzX < 0 ? maxX : minX) + fi->pzY * (fi->pzY < 0 ? maxY : minY)
                                          + fi->pzZ * (fi->pzZ < 0 ? maxZ : minZ) >= -fi->pzW;
                            return inside ? LIBMATTI_JOML_Ins_INSIDE : LIBMATTI_JOML_Ins_INTERSECT;
                        }
                    }
                }
            }
        }
    }
    return plane;
}

// Java: public boolean testPoint(float x, float y, float z)
int LIBMATTI_JOML_FrustumIntersection_TestPoint(
    const LIBMATTI_JOML_FrustumIntersection *fi, float x, float y, float z)
{
    return fi->nxX * x + fi->nxY * y + fi->nxZ * z + fi->nxW >= 0 &&
           fi->pxX * x + fi->pxY * y + fi->pxZ * z + fi->pxW >= 0 &&
           fi->nyX * x + fi->nyY * y + fi->nyZ * z + fi->nyW >= 0 &&
           fi->pyX * x + fi->pyY * y + fi->pyZ * z + fi->pyW >= 0 &&
           fi->nzX * x + fi->nzY * y + fi->nzZ * z + fi->nzW >= 0 &&
           fi->pzX * x + fi->pzY * y + fi->pzZ * z + fi->pzW >= 0;
}

// Java: public float distanceToPlane(float minX, float minY, float minZ,
//                                    float maxX, float maxY, float maxZ, int plane)
float LIBMATTI_JOML_FrustumIntersection_DistanceToPlane(
    const LIBMATTI_JOML_FrustumIntersection *fi,
    float minX, float minY, float minZ, float maxX, float maxY, float maxZ, int plane)
{
    switch (plane)
    {
        case LIBMATTI_JOML_PLANE_NX: return fi->nxX * (fi->nxX < 0 ? maxX : minX) + fi->nxY * (fi->nxY < 0 ? maxY : minY) + fi->nxZ * (fi->nxZ < 0 ? maxZ : minZ) + fi->nxW;
        case LIBMATTI_JOML_PLANE_PX: return fi->pxX * (fi->pxX < 0 ? maxX : minX) + fi->pxY * (fi->pxY < 0 ? maxY : minY) + fi->pxZ * (fi->pxZ < 0 ? maxZ : minZ) + fi->pxW;
        case LIBMATTI_JOML_PLANE_NY: return fi->nyX * (fi->nyX < 0 ? maxX : minX) + fi->nyY * (fi->nyY < 0 ? maxY : minY) + fi->nyZ * (fi->nyZ < 0 ? maxZ : minZ) + fi->nyW;
        case LIBMATTI_JOML_PLANE_PY: return fi->pyX * (fi->pyX < 0 ? maxX : minX) + fi->pyY * (fi->pyY < 0 ? maxY : minY) + fi->pyZ * (fi->pyZ < 0 ? maxZ : minZ) + fi->pyW;
        case LIBMATTI_JOML_PLANE_NZ: return fi->nzX * (fi->nzX < 0 ? maxX : minX) + fi->nzY * (fi->nzY < 0 ? maxY : minY) + fi->nzZ * (fi->nzZ < 0 ? maxZ : minZ) + fi->nzW;
        default: return fi->pzX * (fi->pzX < 0 ? maxX : minX) + fi->pzY * (fi->pzY < 0 ? maxY : minY) + fi->pzZ * (fi->pzZ < 0 ? maxZ : minZ) + fi->pzW;
    }
}
