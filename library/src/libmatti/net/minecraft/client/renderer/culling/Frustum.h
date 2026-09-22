// Port of net.minecraft.client.renderer.culling.Frustum - the camera-
// relative frustum wrapper JOML's FrustumIntersection feeds. The offset
// stepping (OFFSET_STEP) and the cubeInFrustum result contract match Java.

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_CULLING_FRUSTUM_H
#define MATTICRAFT_MC_CLIENT_RENDERER_CULLING_FRUSTUM_H

#include "libmatti/org/joml/FrustumIntersection.h"
#include "libmatti/org/joml/Matrix4f.h"
#include "libmatti/org/joml/Vector4f.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public static final int OFFSET_STEP = 4
#define LIBMATTI_MC_Frustum_OFFSET_STEP 4.0f

// Java: public class Frustum
typedef struct LIBMATTI_MC_Frustum
{
    // Java: private final FrustumIntersection intersection
    LIBMATTI_JOML_FrustumIntersection intersection;
    // Java: private final Matrix4f matrix - the projection*view matrix
    LIBMATTI_JOML_Matrix4f matrix;
    // Java: private Vector4f viewVector - the camera-forward row transform
    LIBMATTI_JOML_Vector4f viewVector;
    // Java: private double camX / camY / camZ
    double camX;
    double camY;
    double camZ;
} LIBMATTI_MC_Frustum;

// Java: public Frustum(Matrix4f projection, Matrix4f view) - calculateFrustum.
void LIBMATTI_MC_Frustum_Init(LIBMATTI_MC_Frustum *frustum,
                              const LIBMATTI_JOML_Matrix4f *projection,
                              const LIBMATTI_JOML_Matrix4f *view);
// Java: public Frustum(Frustum other) - the copy constructor.
void LIBMATTI_MC_Frustum_Copy(LIBMATTI_MC_Frustum *dst, const LIBMATTI_MC_Frustum *src);
void LIBMATTI_MC_Frustum_Free(LIBMATTI_MC_Frustum *frustum);

// Java: public Frustum offset(float amount) - walks the frustum along the
// view vector (the culling bias the level renderer uses).
LIBMATTI_MC_Frustum *LIBMATTI_MC_Frustum_Offset(LIBMATTI_MC_Frustum *frustum, float amount);

// Java: public Frustum offsetToFullyIncludeCameraCube(int cubeSize)
LIBMATTI_MC_Frustum *LIBMATTI_MC_Frustum_OffsetToFullyIncludeCameraCube(LIBMATTI_MC_Frustum *frustum, int cubeSize);

// Java: public void prepare(double camX, double camY, double camZ)
void LIBMATTI_MC_Frustum_Prepare(LIBMATTI_MC_Frustum *frustum, double camX, double camY, double camZ);

// Java: public boolean isVisible(AABB box)
int LIBMATTI_MC_Frustum_IsVisible(const LIBMATTI_MC_Frustum *frustum, double minX, double minY, double minZ,
                                  double maxX, double maxY, double maxZ);

// Java: public int cubeInFrustum(BoundingBox) / the private double overload
// Returns the raw intersectAab contract (INSIDE/INTERSECT/plane index).
int LIBMATTI_MC_Frustum_CubeInFrustum(const LIBMATTI_MC_Frustum *frustum, double minX, double minY, double minZ,
                                      double maxX, double maxY, double maxZ);

// Java: public boolean pointInFrustum(double x, double y, double z)
int LIBMATTI_MC_Frustum_PointInFrustum(const LIBMATTI_MC_Frustum *frustum, double x, double y, double z);

// Java: public Vector4f[] getFrustumPoints() - the eight world-space corners.
void LIBMATTI_MC_Frustum_GetFrustumPoints(const LIBMATTI_MC_Frustum *frustum, LIBMATTI_JOML_Vector4f out[8]);

// Java: public double getCamX() / getCamY() / getCamZ()
double LIBMATTI_MC_Frustum_GetCamX(const LIBMATTI_MC_Frustum *frustum);
double LIBMATTI_MC_Frustum_GetCamY(const LIBMATTI_MC_Frustum *frustum);
double LIBMATTI_MC_Frustum_GetCamZ(const LIBMATTI_MC_Frustum *frustum);

#ifdef __cplusplus
}
#endif

#endif
