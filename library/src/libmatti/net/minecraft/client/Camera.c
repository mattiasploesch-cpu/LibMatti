// Port of net.minecraft.client.Camera (implementation).
// setRotation builds JOML's rotationYXZ(PI - yRot, -xRot, 0) through the
// port's identity + rotateAxis pair. RotateAxis PRE-multiplies (q = axisQ*q),
// so the X pitch term must be applied FIRST and the Y yaw term LAST to yield
// the rotationYXZ product qY·qX (the zero Z term drops out), and
// the basis vectors rotate through the standard q*v*q*-1 sandwich - the
// same math Vector3f.rotate(Quaternionfc, Vector3f) performs.

#include "libmatti/net/minecraft/client/Camera.h"

#include "libmatti/net/minecraft/util/Mth.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Java: private static final Vector3f FORWARDS = new Vector3f(0, 0, -1)
//      private static final Vector3f UP = new Vector3f(0, 1, 0)
//      private static final Vector3f LEFT = new Vector3f(-1, 0, 0)
static void set_vec3(LIBMATTI_JOML_Vector3f *v, float x, float y, float z)
{
    v->x = x;
    v->y = y;
    v->z = z;
}

// Vector3f.rotate(Quaternionfc, Vector3f): v' = q * (0, v) * q^-1.
static void rotate_by_quaternion(const LIBMATTI_JOML_Quaternionf *q,
                                 const LIBMATTI_JOML_Vector3f *v,
                                 LIBMATTI_JOML_Vector3f *out)
{
    // t = 2 * cross(q.xyz, v)
    float tx = 2.0f * (q->y * v->z - q->z * v->y);
    float ty = 2.0f * (q->z * v->x - q->x * v->z);
    float tz = 2.0f * (q->x * v->y - q->y * v->x);
    // out = v + q.w * t + cross(q.xyz, t)
    out->x = v->x + q->w * tx + (q->y * tz - q->z * ty);
    out->y = v->y + q->w * ty + (q->z * tx - q->x * tz);
    out->z = v->z + q->w * tz + (q->x * ty - q->y * tx);
}

// Java: public Camera() - the field initializers.
void LIBMATTI_MC_Camera_Init(LIBMATTI_MC_Camera *camera)
{
    camera->initialized = 0;
    camera->x = 0.0;
    camera->y = 0.0;
    camera->z = 0.0;
    camera->blockX = 0;
    camera->blockY = 0;
    camera->blockZ = 0;
    set_vec3(&camera->forwards, 0.0f, 0.0f, -1.0f);
    set_vec3(&camera->up, 0.0f, 1.0f, 0.0f);
    set_vec3(&camera->left, -1.0f, 0.0f, 0.0f);
    camera->xRot = 0.0f;
    camera->yRot = 0.0f;
    LIBMATTI_JOML_Quaternionf_Identity(&camera->rotation);
    camera->detached = 0;
    camera->eyeHeight = 0.0f;
    camera->eyeHeightOld = 0.0f;
    camera->partialTickTime = 0.0f;
}

void LIBMATTI_MC_Camera_Free(LIBMATTI_MC_Camera *camera)
{
    (void) camera;
}

// Java: protected void setRotation(float yRot, float xRot)
void LIBMATTI_MC_Camera_SetRotation(LIBMATTI_MC_Camera *camera, float yRot, float xRot)
{
    camera->yRot = yRot;
    camera->xRot = xRot;
    // Java: this.rotation.rotationYXZ((float) Math.PI - yRot * rad, -xRot * rad, 0)
    // = RY(PI - yRot) * RX(-xRot). RotateAxis PRE-multiplies (q = axisQ * q),
    // so the pitch about X goes in FIRST and the yaw about Y LAST - the yaw
    // must sit on the LEFT of the product (the outermost world-space turn).
    // The previous order (yaw first, pitch second) built RX*RY instead, which
    // pitches around the WORLD X axis: at yaw 90/270 the pitch vanished and
    // the horizon rolled to the side when looking down.
    LIBMATTI_JOML_Quaternionf_Identity(&camera->rotation);
    LIBMATTI_JOML_Quaternionf_RotateAxis(&camera->rotation,
                                         -xRot * ((float) M_PI / 180.0f), 1.0f, 0.0f, 0.0f);
    LIBMATTI_JOML_Quaternionf_RotateAxis(&camera->rotation,
                                         (float) M_PI - yRot * ((float) M_PI / 180.0f), 0.0f, 1.0f, 0.0f);
    // Java: FORWARDS.rotate(this.rotation, this.forwards) - the CONSTANTS
    // rotate INTO the fields, so every call starts from the unit axes again
    // (rotating the fields in place would compose with the previous frame).
    set_vec3(&camera->forwards, 0.0f, 0.0f, -1.0f);
    set_vec3(&camera->up, 0.0f, 1.0f, 0.0f);
    set_vec3(&camera->left, -1.0f, 0.0f, 0.0f);
    rotate_by_quaternion(&camera->rotation, &camera->forwards, &camera->forwards);
    rotate_by_quaternion(&camera->rotation, &camera->up, &camera->up);
    rotate_by_quaternion(&camera->rotation, &camera->left, &camera->left);
}

// Java: protected void setPosition(double, double, double) / setPosition(Vec3)
// Vec3 position + the block position containing it.
void LIBMATTI_MC_Camera_SetPosition(LIBMATTI_MC_Camera *camera, double x, double y, double z)
{
    camera->x = x;
    camera->y = y;
    camera->z = z;
    // Java: this.blockPosition.set(x, y, z) - BlockPos.containing (floor).
    camera->blockX = (int) floor(x);
    camera->blockY = (int) floor(y);
    camera->blockZ = (int) floor(z);
}

// Java: protected void move(float forward, float up, float left)
// The input vector (left, up, -forward) is rotated by the camera rotation.
void LIBMATTI_MC_Camera_Move(LIBMATTI_MC_Camera *camera, float forward, float up, float left)
{
    LIBMATTI_JOML_Vector3f input, rotated;
    set_vec3(&input, left, up, -forward);
    rotate_by_quaternion(&camera->rotation, &input, &rotated);
    LIBMATTI_MC_Camera_SetPosition(camera,
                                   camera->x + rotated.x,
                                   camera->y + rotated.y,
                                   camera->z + rotated.z);
}

// Java: public void tick() - the eye-height easing halves the delta per tick.
void LIBMATTI_MC_Camera_Tick(LIBMATTI_MC_Camera *camera)
{
    camera->eyeHeightOld = camera->eyeHeight;
    // Java: eyeHeight + (entity.getEyeHeight() - eyeHeight) * 0.5F - the
    // entity eye height arrives with P5, so the easing is a no-op there.
    camera->eyeHeight += (0.0f - camera->eyeHeight) * 0.5f;
}

// Java: public Vec3 position()
void LIBMATTI_MC_Camera_Position(const LIBMATTI_MC_Camera *camera, double out[3])
{
    out[0] = camera->x;
    out[1] = camera->y;
    out[2] = camera->z;
}

// Java: public BlockPos blockPosition()
void LIBMATTI_MC_Camera_BlockPosition(const LIBMATTI_MC_Camera *camera, int out[3])
{
    out[0] = camera->blockX;
    out[1] = camera->blockY;
    out[2] = camera->blockZ;
}

float LIBMATTI_MC_Camera_XRot(const LIBMATTI_MC_Camera *camera)
{
    return camera->xRot;
}

float LIBMATTI_MC_Camera_YRot(const LIBMATTI_MC_Camera *camera)
{
    return camera->yRot;
}

// Java: public float yaw() { return Mth.wrapDegrees(this.yRot()); }
float LIBMATTI_MC_Camera_Yaw(const LIBMATTI_MC_Camera *camera)
{
    return LIBMATTI_MC_Mth_WrapDegrees(LIBMATTI_MC_Camera_YRot(camera));
}

const LIBMATTI_JOML_Quaternionf *LIBMATTI_MC_Camera_Rotation(const LIBMATTI_MC_Camera *camera)
{
    return &camera->rotation;
}

const LIBMATTI_JOML_Vector3f *LIBMATTI_MC_Camera_ForwardVector(const LIBMATTI_MC_Camera *camera)
{
    return &camera->forwards;
}

const LIBMATTI_JOML_Vector3f *LIBMATTI_MC_Camera_UpVector(const LIBMATTI_MC_Camera *camera)
{
    return &camera->up;
}

const LIBMATTI_JOML_Vector3f *LIBMATTI_MC_Camera_LeftVector(const LIBMATTI_MC_Camera *camera)
{
    return &camera->left;
}

int LIBMATTI_MC_Camera_IsInitialized(const LIBMATTI_MC_Camera *camera)
{
    return camera->initialized;
}

int LIBMATTI_MC_Camera_IsDetached(const LIBMATTI_MC_Camera *camera)
{
    return camera->detached;
}

float LIBMATTI_MC_Camera_GetPartialTickTime(const LIBMATTI_MC_Camera *camera)
{
    return camera->partialTickTime;
}

// Java: public void reset()
void LIBMATTI_MC_Camera_Reset(LIBMATTI_MC_Camera *camera)
{
    camera->initialized = 0;
}
