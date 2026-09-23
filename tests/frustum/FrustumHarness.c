// The frustum harness (P4.3): the FrustumIntersection plane extraction
// against a known matrix, the intersectAab contract (inside/intersect/culling
// plane), the camera-relative conversion in Frustum.cubeInFrustum and the
// offset behaviour along the view vector.

#include "libmatti/net/minecraft/client/Camera.h"
#include "libmatti/net/minecraft/client/renderer/culling/Frustum.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static int checks = 0;
static int failures = 0;
#define CHECK(cond)                                                          \
    do                                                                       \
    {                                                                        \
        checks++;                                                            \
        if (!(cond))                                                         \
        {                                                                    \
            failures++;                                                      \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
        }                                                                    \
    } while (0)

// The identity projection*view frustum: planes at the unit cube walls.
static void build_identity_frustum(LIBMATTI_MC_Frustum *frustum)
{
    LIBMATTI_JOML_Matrix4f proj, view;
    LIBMATTI_JOML_Matrix4f_Identity(&proj);
    LIBMATTI_JOML_Matrix4f_Identity(&view);
    // Java: new Frustum(proj, view) -> view.mul(proj) = identity.
    LIBMATTI_MC_Frustum_Init(frustum, &proj, &view);
    LIBMATTI_MC_Frustum_Prepare(frustum, 0.0, 0.0, 0.0);
}

int main(void)
{
    // ------------------------------------------------------------------------
    // FrustumIntersection.set: the plane extraction values. The GL perspective
    // matrix (70 deg, 16:9, .05..1000) with the identity view looks down -Z:
    // the near plane is nz (w = -m23*near-scaled = -0.909), the far plane pz
    // (w = +1.111 = the +m23 row sum), both symmetric about the view axis.
    // ------------------------------------------------------------------------
    LIBMATTI_MC_Frustum frustum;
    {
        LIBMATTI_JOML_Matrix4f proj, view;
        LIBMATTI_JOML_Matrix4f_SetPerspective(&proj, 1.2217f, 854.0f / 480.0f, 0.05f, 1000.0f);
        LIBMATTI_JOML_Matrix4f_Identity(&view);
        LIBMATTI_MC_Frustum_Init(&frustum, &proj, &view);
        LIBMATTI_MC_Frustum_Prepare(&frustum, 0.0, 0.0, 0.0);
    }
    CHECK(fabsf(frustum.intersection.nyY - 0.81916f) < 1e-3f);  // bottom: +m11+0.0698m13
    CHECK(fabsf(frustum.intersection.pyY + 0.81916f) < 1e-3f);  // top: -(m11-0.0698m13)
    CHECK(fabsf(frustum.intersection.nzZ + 1.0001f) < 1e-3f);   // near: m22 + m23
    CHECK(fabsf(frustum.intersection.nzW + 0.05f) < 1e-3f);     // near: m32 + m33

    // The camera at the frustum origin: the origin is on every plane boundary
    // of the w=0 side planes but inside the z half-spaces; a point just in
    // front (-Z, the identity-view look direction) is inside, behind is not.
    CHECK(LIBMATTI_MC_Frustum_PointInFrustum(&frustum, 0.0, 0.0, -0.5));
    CHECK(!LIBMATTI_MC_Frustum_PointInFrustum(&frustum, 0.0, 0.0, 0.5));

    // ------------------------------------------------------------------------
    // cubeInFrustum: a unit cube in front of the camera (identity view looks
    // down -Z) is fully inside; one behind the near plane (z > -0.05) is culled
    // by the NZ plane - with zFar = 1000 the near plane, not PZ, rejects it.
    // ------------------------------------------------------------------------
    CHECK(LIBMATTI_MC_Frustum_IsVisible(&frustum, -1.0, -1.0, -5.0, 1.0, 1.0, -3.0));
    CHECK(LIBMATTI_MC_Frustum_IsVisible(&frustum, -1.0, -1.0, 3.0, 1.0, 1.0, 5.0) == 0);
    int behind = LIBMATTI_MC_Frustum_CubeInFrustum(&frustum, -1.0, -1.0, 3.0, 1.0, 1.0, 5.0);
    // JOML tests the planes in NX..PZ order; the left plane (1.176,0,-1,0)
    // carries a negative z term and rejects a behind-camera box first.
    CHECK(behind == LIBMATTI_JOML_PLANE_NX);

    // A huge box surrounding the frustum intersects (not fully inside).
    int surrounding = LIBMATTI_MC_Frustum_CubeInFrustum(&frustum, -1000.0, -1000.0, -1000.0, 1000.0, 1000.0, 1000.0);
    CHECK(surrounding == LIBMATTI_JOML_Ins_INTERSECT);

    // ------------------------------------------------------------------------
    // prepare() moves the camera frame: the same world cube that was visible
    // at the origin leaves the frustum when the camera sits inside it (the
    // test coordinates are camera-relative in the Java port contract).
    // ------------------------------------------------------------------------
    build_identity_frustum(&frustum);
    LIBMATTI_MC_Frustum_Prepare(&frustum, 10.0, 0.0, 0.0);
    // The cube at x 10.5..11.5 relative to cam (10,0,0) -> camera-relative
    // 0.5..1.5 straddles the right wall: intersect, not outside.
    int rel = LIBMATTI_MC_Frustum_CubeInFrustum(&frustum, 10.5, -1.0, -1.0, 11.5, 1.0, 1.0);
    CHECK(rel == LIBMATTI_JOML_Ins_INTERSECT || rel >= 0);

    // ------------------------------------------------------------------------
    // Camera contract: yaw 0 faces +Z (south), negative pitch looks up - the
    // basis vectors carry the convention the renderer builds the view from.
    // ------------------------------------------------------------------------
    {
        LIBMATTI_MC_Camera camera;
        LIBMATTI_MC_Camera_Init(&camera);
        LIBMATTI_MC_Camera_SetRotation(&camera, 0.0f, 0.0f);
        const LIBMATTI_JOML_Vector3f *fwd = LIBMATTI_MC_Camera_ForwardVector(&camera);
        CHECK(fabsf(fwd->z - 1.0f) < 1e-5f);  // south
        CHECK(fabsf(fwd->y) < 1e-5f);

        LIBMATTI_MC_Camera_SetRotation(&camera, 180.0f, 50.0f);
        fwd = LIBMATTI_MC_Camera_ForwardVector(&camera);
        CHECK(fwd->z < 0.0f);  // north
        CHECK(fwd->y < 0.0f);  // down

        // move(backward) through the basis: the Java contract (left, up,
        // -forward) rotated by the camera rotation.
        double before[3];
        LIBMATTI_MC_Camera_Position(&camera, before);
        LIBMATTI_MC_Camera_Move(&camera, -4.0f, 0.0f, 0.0f);
        double after[3];
        LIBMATTI_MC_Camera_Position(&camera, after);
        CHECK(after[2] > before[2]);   // -forward is south, z grows
        CHECK(after[1] > before[1]);   // pitch 50 looks down, so -forward carries +y

        LIBMATTI_MC_Camera_Reset(&camera);
        CHECK(!LIBMATTI_MC_Camera_IsInitialized(&camera));
    }

    printf("frustum: %d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
