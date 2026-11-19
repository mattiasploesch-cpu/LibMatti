





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







    LIBMATTI_MC_Frustum frustum;
    {
        LIBMATTI_JOML_Matrix4f proj, view;
        LIBMATTI_JOML_Matrix4f_SetPerspective(&proj, 1.2217f, 854.0f / 480.0f, 0.05f, 1000.0f);
        LIBMATTI_JOML_Matrix4f_Identity(&view);
        LIBMATTI_MC_Frustum_Init(&frustum, &proj, &view);
        LIBMATTI_MC_Frustum_Prepare(&frustum, 0.0, 0.0, 0.0);
    }
    CHECK(fabsf(frustum.intersection.nyY - 0.81916f) < 1e-3f);

    CHECK(fabsf(frustum.intersection.pyY + 0.81916f) < 1e-3f);

    CHECK(fabsf(frustum.intersection.nzZ + 1.0001f) < 1e-3f);

    CHECK(fabsf(frustum.intersection.nzW + 0.05f) < 1e-3f);






    CHECK(LIBMATTI_MC_Frustum_PointInFrustum(&frustum, 0.0, 0.0, -0.5));
    CHECK(!LIBMATTI_MC_Frustum_PointInFrustum(&frustum, 0.0, 0.0, 0.5));







    CHECK(LIBMATTI_MC_Frustum_IsVisible(&frustum, -1.0, -1.0, -5.0, 1.0, 1.0, -3.0));
    CHECK(LIBMATTI_MC_Frustum_IsVisible(&frustum, -1.0, -1.0, 3.0, 1.0, 1.0, 5.0) == 0);
    int behind = LIBMATTI_MC_Frustum_CubeInFrustum(&frustum, -1.0, -1.0, 3.0, 1.0, 1.0, 5.0);



    CHECK(behind == LIBMATTI_JOML_PLANE_NX);



    int surrounding = LIBMATTI_MC_Frustum_CubeInFrustum(&frustum, -1000.0, -1000.0, -1000.0, 1000.0, 1000.0, 1000.0);
    CHECK(surrounding == LIBMATTI_JOML_Ins_INTERSECT);







    build_identity_frustum(&frustum);
    LIBMATTI_MC_Frustum_Prepare(&frustum, 10.0, 0.0, 0.0);



    int rel = LIBMATTI_MC_Frustum_CubeInFrustum(&frustum, 10.5, -1.0, -1.0, 11.5, 1.0, 1.0);
    CHECK(rel == LIBMATTI_JOML_Ins_INTERSECT || rel >= 0);






    {
        LIBMATTI_MC_Camera camera;
        LIBMATTI_MC_Camera_Init(&camera);
        LIBMATTI_MC_Camera_SetRotation(&camera, 0.0f, 0.0f);
        const LIBMATTI_JOML_Vector3f *fwd = LIBMATTI_MC_Camera_ForwardVector(&camera);
        CHECK(fabsf(fwd->z - 1.0f) < 1e-5f);

        CHECK(fabsf(fwd->y) < 1e-5f);

        LIBMATTI_MC_Camera_SetRotation(&camera, 180.0f, 50.0f);
        fwd = LIBMATTI_MC_Camera_ForwardVector(&camera);
        CHECK(fwd->z < 0.0f);

        CHECK(fwd->y < 0.0f);





        double before[3];
        LIBMATTI_MC_Camera_Position(&camera, before);
        LIBMATTI_MC_Camera_Move(&camera, -4.0f, 0.0f, 0.0f);
        double after[3];
        LIBMATTI_MC_Camera_Position(&camera, after);
        CHECK(after[2] > before[2]);

        CHECK(after[1] > before[1]);


        LIBMATTI_MC_Camera_Reset(&camera);
        CHECK(!LIBMATTI_MC_Camera_IsInitialized(&camera));
    }

    printf("frustum: %d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
