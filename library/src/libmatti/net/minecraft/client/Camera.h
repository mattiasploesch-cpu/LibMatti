// Port of net.minecraft.client.Camera - the camera position/rotation state
// and the basis vectors derived from it. The Java entity/minecart/fluid
// branches arrive with P5 (entities); the port exposes the transforms the
// game renderer needs: setup from yaw/pitch, move along the basis and the
// accessors the frustum/rendering paths read.

#ifndef MATTICRAFT_MC_CLIENT_CAMERA_H
#define MATTICRAFT_MC_CLIENT_CAMERA_H

#include "libmatti/org/joml/Quaternionf.h"
#include "libmatti/org/joml/Vector3f.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: private static final float DEFAULT_CAMERA_DISTANCE = 4.0F
#define LIBMATTI_MC_Camera_DEFAULT_CAMERA_DISTANCE 4.0f

// Java: public class Camera
typedef struct LIBMATTI_MC_Camera
{
    // Java: private boolean initialized
    int initialized;
    // Java: private Vec3 position - the eye point in world space
    double x, y, z;
    // Java: private final BlockPos.MutableBlockPos blockPosition
    int blockX, blockY, blockZ;
    // Java: private final Vector3f forwards / up / left
    LIBMATTI_JOML_Vector3f forwards;
    LIBMATTI_JOML_Vector3f up;
    LIBMATTI_JOML_Vector3f left;
    // Java: private float xRot / yRot - pitch / yaw in degrees
    float xRot;
    float yRot;
    // Java: private final Quaternionf rotation
    LIBMATTI_JOML_Quaternionf rotation;
    // Java: private boolean detached / eyeHeight / eyeHeightOld / partialTickTime
    int detached;
    float eyeHeight;
    float eyeHeightOld;
    float partialTickTime;
} LIBMATTI_MC_Camera;

// Java: public Camera() (field initializers)
void LIBMATTI_MC_Camera_Init(LIBMATTI_MC_Camera *camera);
void LIBMATTI_MC_Camera_Free(LIBMATTI_MC_Camera *camera);

// Java: protected void setRotation(float yRot, float xRot) - the rotation
// quaternion is rotationYXZ(PI - yRot*rad, -xRot*rad, 0); the basis vectors
// are the constants rotated by it.
void LIBMATTI_MC_Camera_SetRotation(LIBMATTI_MC_Camera *camera, float yRot, float xRot);

// Java: protected void setPosition(double, double, double) / setPosition(Vec3)
void LIBMATTI_MC_Camera_SetPosition(LIBMATTI_MC_Camera *camera, double x, double y, double z);

// Java: protected void move(float forward, float up, float left) - the input
// vector (left, up, -forward) is rotated by the camera rotation.
void LIBMATTI_MC_Camera_Move(LIBMATTI_MC_Camera *camera, float forward, float up, float left);

// Java: public void tick() - the eye-height easing (the port keeps the lerp
// shape; the entity's eye height arrives with P5).
void LIBMATTI_MC_Camera_Tick(LIBMATTI_MC_Camera *camera);

// Java: public Vec3 position() - the position as an out-parameter triple.
void LIBMATTI_MC_Camera_Position(const LIBMATTI_MC_Camera *camera, double out[3]);

// Java: public BlockPos blockPosition()
void LIBMATTI_MC_Camera_BlockPosition(const LIBMATTI_MC_Camera *camera, int out[3]);

// Java: public float xRot() / yRot() / yaw() (wrapDegrees)
float LIBMATTI_MC_Camera_XRot(const LIBMATTI_MC_Camera *camera);
float LIBMATTI_MC_Camera_YRot(const LIBMATTI_MC_Camera *camera);
float LIBMATTI_MC_Camera_Yaw(const LIBMATTI_MC_Camera *camera);

// Java: public Quaternionf rotation()
const LIBMATTI_JOML_Quaternionf *LIBMATTI_MC_Camera_Rotation(const LIBMATTI_MC_Camera *camera);

// Java: public Vector3fc forwardVector() / upVector() / leftVector()
const LIBMATTI_JOML_Vector3f *LIBMATTI_MC_Camera_ForwardVector(const LIBMATTI_MC_Camera *camera);
const LIBMATTI_JOML_Vector3f *LIBMATTI_MC_Camera_UpVector(const LIBMATTI_MC_Camera *camera);
const LIBMATTI_JOML_Vector3f *LIBMATTI_MC_Camera_LeftVector(const LIBMATTI_MC_Camera *camera);

// Java: public boolean isInitialized() / isDetached() / getPartialTickTime()
int LIBMATTI_MC_Camera_IsInitialized(const LIBMATTI_MC_Camera *camera);
int LIBMATTI_MC_Camera_IsDetached(const LIBMATTI_MC_Camera *camera);
float LIBMATTI_MC_Camera_GetPartialTickTime(const LIBMATTI_MC_Camera *camera);

// Java: public void reset()
void LIBMATTI_MC_Camera_Reset(LIBMATTI_MC_Camera *camera);

#ifdef __cplusplus
}
#endif

#endif
