// Port of net.minecraft.client.renderer.culling.Frustum (implementation).
// calculateFrustum is matrix.mul(projection) like Java (view * projection in
// column-major terms), the view vector is the transformTranspose of (0,0,1,0)
// and the cube tests convert world coordinates to camera-relative floats
// exactly like Java's cubeInFrustum/pointInFrustum/offset helpers.

#include "libmatti/net/minecraft/client/renderer/culling/Frustum.h"

#include <math.h>

// Java: private void calculateFrustum(Matrix4f projection, Matrix4f view)
//   view.mul(projection, this.matrix) - Java's ctor receives (modelView,
//   projection) and multiplies second·first, i.e. the clip matrix P·V
//   (projection applies last). The C Init mirrors the ctor arg order
//   (projection, view) and computes projection·view directly.
//   viewVector = matrix.transformTranspose(new Vector4f(0, 0, 1, 0));
static void calculate_frustum(LIBMATTI_MC_Frustum *frustum,
                              const LIBMATTI_JOML_Matrix4f *projection,
                              const LIBMATTI_JOML_Matrix4f *view)
{
    LIBMATTI_JOML_Matrix4f_Mul(projection, view, &frustum->matrix);
    LIBMATTI_JOML_FrustumIntersection_Set(&frustum->intersection, &frustum->matrix);
    // JOML: transformTranspose((0,0,1,0)) -> dest = (m20, m21, m22, m23)
    // (JOML names mXY column X, row Y - same convention as the C struct).
    frustum->viewVector.x = frustum->matrix.m20;
    frustum->viewVector.y = frustum->matrix.m21;
    frustum->viewVector.z = frustum->matrix.m22;
    frustum->viewVector.w = frustum->matrix.m23;
}

// Java: public Frustum(Matrix4f projection, Matrix4f view)
void LIBMATTI_MC_Frustum_Init(LIBMATTI_MC_Frustum *frustum,
                              const LIBMATTI_JOML_Matrix4f *projection,
                              const LIBMATTI_JOML_Matrix4f *view)
{
    calculate_frustum(frustum, projection, view);
    frustum->camX = 0.0;
    frustum->camY = 0.0;
    frustum->camZ = 0.0;
}

// Java: public Frustum(Frustum other)
void LIBMATTI_MC_Frustum_Copy(LIBMATTI_MC_Frustum *dst, const LIBMATTI_MC_Frustum *src)
{
    dst->intersection = src->intersection;
    dst->matrix = src->matrix;
    dst->viewVector = src->viewVector;
    dst->camX = src->camX;
    dst->camY = src->camY;
    dst->camZ = src->camZ;
}

void LIBMATTI_MC_Frustum_Free(LIBMATTI_MC_Frustum *frustum)
{
    (void) frustum;
}

// Java: public Frustum offset(float amount)
LIBMATTI_MC_Frustum *LIBMATTI_MC_Frustum_Offset(LIBMATTI_MC_Frustum *frustum, float amount)
{
    frustum->camX += (double) (frustum->viewVector.x * amount);
    frustum->camY += (double) (frustum->viewVector.y * amount);
    frustum->camZ += (double) (frustum->viewVector.z * amount);
    return frustum;
}

// Java: public Frustum offsetToFullyIncludeCameraCube(int cubeSize) - walks
// the camera back along the view vector until the cube the camera sits in is
// fully inside the frustum (the loop condition reads intersectAab != INSIDE).
LIBMATTI_MC_Frustum *LIBMATTI_MC_Frustum_OffsetToFullyIncludeCameraCube(LIBMATTI_MC_Frustum *frustum, int cubeSize)
{
    double d0 = floor(frustum->camX / cubeSize) * cubeSize;
    double d1 = floor(frustum->camY / cubeSize) * cubeSize;
    double d2 = floor(frustum->camZ / cubeSize) * cubeSize;
    double d3 = ceil(frustum->camX / cubeSize) * cubeSize;
    double d4 = ceil(frustum->camY / cubeSize) * cubeSize;

    for (double d5 = ceil(frustum->camZ / cubeSize) * cubeSize;
         LIBMATTI_JOML_FrustumIntersection_IntersectAab(&frustum->intersection,
                                                        (float) (d0 - frustum->camX),
                                                        (float) (d1 - frustum->camY),
                                                        (float) (d2 - frustum->camZ),
                                                        (float) (d3 - frustum->camX),
                                                        (float) (d4 - frustum->camY),
                                                        (float) (d5 - frustum->camZ))
         != LIBMATTI_JOML_Ins_INSIDE;
         frustum->camZ -= frustum->viewVector.z * LIBMATTI_MC_Frustum_OFFSET_STEP)
    {
        frustum->camX -= frustum->viewVector.x * LIBMATTI_MC_Frustum_OFFSET_STEP;
        frustum->camY -= frustum->viewVector.y * LIBMATTI_MC_Frustum_OFFSET_STEP;
    }

    return frustum;
}

// Java: public void prepare(double camX, double camY, double camZ)
void LIBMATTI_MC_Frustum_Prepare(LIBMATTI_MC_Frustum *frustum, double camX, double camY, double camZ)
{
    frustum->camX = camX;
    frustum->camY = camY;
    frustum->camZ = camZ;
}

// Java: private int cubeInFrustum(double minX..maxZ) - the world corner
// coordinates become camera-relative floats, then intersectAab.
int LIBMATTI_MC_Frustum_CubeInFrustum(const LIBMATTI_MC_Frustum *frustum, double minX, double minY, double minZ,
                                      double maxX, double maxY, double maxZ)
{
    float f = (float) (minX - frustum->camX);
    float f1 = (float) (minY - frustum->camY);
    float f2 = (float) (minZ - frustum->camZ);
    float f3 = (float) (maxX - frustum->camX);
    float f4 = (float) (maxY - frustum->camY);
    float f5 = (float) (maxZ - frustum->camZ);
    return LIBMATTI_JOML_FrustumIntersection_IntersectAab(&frustum->intersection, f, f1, f2, f3, f4, f5);
}

// Java: public boolean isVisible(AABB box) - INSIDE (-2) or INTERSECT (-1)
// count as visible, a culling plane index does not.
int LIBMATTI_MC_Frustum_IsVisible(const LIBMATTI_MC_Frustum *frustum, double minX, double minY, double minZ,
                                  double maxX, double maxY, double maxZ)
{
    int i = LIBMATTI_MC_Frustum_CubeInFrustum(frustum, minX, minY, minZ, maxX, maxY, maxZ);
    return i == LIBMATTI_JOML_Ins_INSIDE || i == LIBMATTI_JOML_Ins_INTERSECT;
}

// Java: public boolean pointInFrustum(double x, double y, double z)
int LIBMATTI_MC_Frustum_PointInFrustum(const LIBMATTI_MC_Frustum *frustum, double x, double y, double z)
{
    return LIBMATTI_JOML_FrustumIntersection_TestPoint(&frustum->intersection,
                                                       (float) (x - frustum->camX),
                                                       (float) (y - frustum->camY),
                                                       (float) (z - frustum->camZ));
}

// Java: public Vector4f[] getFrustumPoints() - the eight NDC corners mapped
// back to world space through the inverted frustum matrix (w-divided).
void LIBMATTI_MC_Frustum_GetFrustumPoints(const LIBMATTI_MC_Frustum *frustum, LIBMATTI_JOML_Vector4f out[8])
{
    static const float corners[8][4] = {
        {-1.0f, -1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, -1.0f, 1.0f},
        {1.0f, 1.0f, -1.0f, 1.0f},   {-1.0f, 1.0f, -1.0f, 1.0f},
        {-1.0f, -1.0f, 1.0f, 1.0f},  {1.0f, -1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},    {-1.0f, 1.0f, 1.0f, 1.0f},
    };
    LIBMATTI_JOML_Matrix4f inverse;
    LIBMATTI_JOML_Matrix4f_Invert(&frustum->matrix, &inverse);
    for (int i = 0; i < 8; i++)
    {
        float x = corners[i][0], y = corners[i][1], z = corners[i][2], w = corners[i][3];
        LIBMATTI_JOML_Matrix4f_Transform(&inverse, &x, &y, &z, &w);
        out[i].x = x / w;
        out[i].y = y / w;
        out[i].z = z / w;
        out[i].w = 1.0f;
    }
}

double LIBMATTI_MC_Frustum_GetCamX(const LIBMATTI_MC_Frustum *frustum)
{
    return frustum->camX;
}

double LIBMATTI_MC_Frustum_GetCamY(const LIBMATTI_MC_Frustum *frustum)
{
    return frustum->camY;
}

double LIBMATTI_MC_Frustum_GetCamZ(const LIBMATTI_MC_Frustum *frustum)
{
    return frustum->camZ;
}
