// Port of net.minecraft.util.Mth (the math helpers the game uses everywhere).
// The SIN lookup table, the asin/atan2 fast paths and every integer trick follow the
// Java implementation exactly; the JOML/Quaternion parts stay org.joml's port.

#ifndef MATTICRAFT_NET_MINECRAFT_UTIL_MTH_H
#define MATTICRAFT_NET_MINECRAFT_UTIL_MTH_H

#include <stdint.h>

// Java: public static final float PI = (float) Math.PI
#define LIBMATTI_MC_Mth_PI 3.1415927410125732421875f
// Java: public static final float HALF_PI = (float) (Math.PI / 2)
#define LIBMATTI_MC_Mth_HALF_PI 1.57079637050628662109375f
// Java: public static final float TWO_PI = (float) (Math.PI * 2)
#define LIBMATTI_MC_Mth_TWO_PI 6.283185482025146484375f
// Java: public static final float DEG_TO_RAD = (float) (Math.PI / 180.0)
#define LIBMATTI_MC_Mth_DEG_TO_RAD 0.01745329238474369049072265625f
// Java: public static final float RAD_TO_DEG = 180.0F / (float) Math.PI
#define LIBMATTI_MC_Mth_RAD_TO_DEG 57.2957763671875f
// Java: public static final float EPSILON = 1.0E-5F
#define LIBMATTI_MC_Mth_EPSILON 1.0E-5f
// Java: public static final float SQRT_OF_TWO = sqrt(2.0F)
#define LIBMATTI_MC_Mth_SQRT_OF_TWO 1.41421353816986083984375f

// Java: private static final float[] SIN - the 65536-entry sin table
// Java: public static float sin(double)
float LIBMATTI_MC_Mth_Sin(double value);
// Java: public static float cos(double)
float LIBMATTI_MC_Mth_Cos(double value);
// Java: public static float sqrt(float)
float LIBMATTI_MC_Mth_Sqrt(float value);

// Java: public static int floor(float) / floor(double)
int LIBMATTI_MC_Mth_Floor(float value);
int LIBMATTI_MC_Mth_FloorD(double value);
// Java: public static long lfloor(double)
int64_t LIBMATTI_MC_Mth_LFloor(double value);
// Java: public static float abs(float) / int abs(int)
float LIBMATTI_MC_Mth_Abs(float value);
int LIBMATTI_MC_Mth_AbsI(int value);
// Java: public static int ceil(float) / ceil(double)
int LIBMATTI_MC_Mth_Ceil(float value);
int LIBMATTI_MC_Mth_CeilD(double value);
// Java: public static long ceilLong(double)
int64_t LIBMATTI_MC_Mth_CeilLong(double value);

// Java: public static int clamp(int, int, int)
int LIBMATTI_MC_Mth_ClampI(int value, int min, int max);
// Java: public static long clamp(long, long, long)
int64_t LIBMATTI_MC_Mth_ClampL(int64_t value, int64_t min, int64_t max);
// Java: public static float clamp(float, float, float)
float LIBMATTI_MC_Mth_Clamp(float value, float min, float max);
// Java: public static double clamp(double, double, double)
double LIBMATTI_MC_Mth_ClampD(double value, double min, double max);

// Java: public static double clampedLerp(double, double, double)
double LIBMATTI_MC_Mth_ClampedLerp(double delta, double start, double end);
// Java: public static float clampedLerp(float, float, float)
float LIBMATTI_MC_Mth_ClampedLerpF(float delta, float start, float end);

// Java: public static int absMax(int, int)
int LIBMATTI_MC_Mth_AbsMaxI(int a, int b);
// Java: public static float absMax(float, float)
float LIBMATTI_MC_Mth_AbsMax(float a, float b);
// Java: public static double absMax(double, double)
double LIBMATTI_MC_Mth_AbsMaxD(double a, double b);
// Java: public static int chessboardDistance(int, int, int, int)
int LIBMATTI_MC_Mth_ChessboardDistance(int x1, int z1, int x2, int z2);

// Java: public static int floorDiv(int, int)
int LIBMATTI_MC_Mth_FloorDiv(int x, int y);
// Java: public static boolean equal(float, float) / equal(double, double)
int LIBMATTI_MC_Mth_Equal(float a, float b);
int LIBMATTI_MC_Mth_EqualD(double a, double b);
// Java: public static int positiveModulo(int, int)
int LIBMATTI_MC_Mth_PositiveModuloI(int x, int y);
// Java: public static float positiveModulo(float, float)
float LIBMATTI_MC_Mth_PositiveModulo(float x, float y);
// Java: public static double positiveModulo(double, double)
double LIBMATTI_MC_Mth_PositiveModuloD(double x, double y);
// Java: public static boolean isMultipleOf(int, int)
int LIBMATTI_MC_Mth_IsMultipleOf(int x, int y);

// Java: public static byte packDegrees(float)
int8_t LIBMATTI_MC_Mth_PackDegrees(float value);
// Java: public static float unpackDegrees(byte)
float LIBMATTI_MC_Mth_UnpackDegrees(int8_t value);

// Java: public static int wrapDegrees(int)
int LIBMATTI_MC_Mth_WrapDegreesI(int degrees);
// Java: public static float wrapDegrees(long)
float LIBMATTI_MC_Mth_WrapDegreesL(int64_t degrees);
// Java: public static float wrapDegrees(float)
float LIBMATTI_MC_Mth_WrapDegrees(float degrees);
// Java: public static double wrapDegrees(double)
double LIBMATTI_MC_Mth_WrapDegreesD(double degrees);
// Java: public static float degreesDifference(float, float)
float LIBMATTI_MC_Mth_DegreesDifference(float from, float to);
// Java: public static float degreesDifferenceAbs(float, float)
float LIBMATTI_MC_Mth_DegreesDifferenceAbs(float from, float to);
// Java: public static float rotateIfNecessary(float current, float target, float maxDelta)
float LIBMATTI_MC_Mth_RotateIfNecessary(float current, float target, float maxDelta);
// Java: public static float approach(float, float, float)
float LIBMATTI_MC_Mth_Approach(float from, float to, float step);
// Java: public static float approachDegrees(float, float, float)
float LIBMATTI_MC_Mth_ApproachDegrees(float from, float to, float step);

// Java: public static int getInt(String, int) - NumberUtils.toInt
int LIBMATTI_MC_Mth_GetInt(const char *value, int fallback);

// Java: public static int smallestEncompassingPowerOfTwo(int)
int LIBMATTI_MC_Mth_SmallestEncompassingPowerOfTwo(int value);
// Java: public static int smallestSquareSide(int)
int LIBMATTI_MC_Mth_SmallestSquareSide(int itemCount);
// Java: public static boolean isPowerOfTwo(int)
int LIBMATTI_MC_Mth_IsPowerOfTwo(int value);
// Java: public static int ceillog2(int)
int LIBMATTI_MC_Mth_CeilLog2(int value);
// Java: public static int log2(int)
int LIBMATTI_MC_Mth_Log2(int value);

// Java: public static float frac(float) / frac(double)
float LIBMATTI_MC_Mth_Frac(float value);
double LIBMATTI_MC_Mth_FracD(double value);

// Java: public static long getSeed(int, int, int)
int64_t LIBMATTI_MC_Mth_GetSeed(int x, int y, int z);

// Java: public static double inverseLerp(double, double, double)
double LIBMATTI_MC_Mth_InverseLerp(double start, double end, double value);
// Java: public static float inverseLerp(float, float, float)
float LIBMATTI_MC_Mth_InverseLerpF(float start, float end, float value);

// Java: public static double atan2(double, double) - the ASIN_TAB fast path
double LIBMATTI_MC_Mth_Atan2(double y, double x);
// Java: public static float invSqrt(float)
float LIBMATTI_MC_Mth_InvSqrt(float value);
// Java: public static double invSqrt(double)
double LIBMATTI_MC_Mth_InvSqrtD(double value);
// Java: @Deprecated public static double fastInvSqrt(double)
double LIBMATTI_MC_Mth_FastInvSqrt(double value);
// Java: public static float fastInvCubeRoot(float)
float LIBMATTI_MC_Mth_FastInvCubeRoot(float value);

// Java: public static int hsvToRgb(float, float, float)
int LIBMATTI_MC_Mth_HsvToRgb(float hue, float saturation, float value);
// Java: public static int hsvToArgb(float, float, float, int)
int LIBMATTI_MC_Mth_HsvToArgb(float hue, float saturation, float value, int alpha);
// Java: public static int murmurHash3Mixer(int)
int LIBMATTI_MC_Mth_MurmurHash3Mixer(int value);

// Java: public static int lerpInt(float, int, int)
int LIBMATTI_MC_Mth_LerpInt(float delta, int start, int end);
// Java: public static int lerpDiscrete(float, int, int)
int LIBMATTI_MC_Mth_LerpDiscrete(float delta, int start, int end);
// Java: public static float lerp(float, float, float)
float LIBMATTI_MC_Mth_Lerp(float delta, float start, float end);
// Java: public static double lerp(double, double, double)
double LIBMATTI_MC_Mth_LerpD(double delta, double start, double end);
// Java: public static double lerp2(double, double, double, double, double, double)
double LIBMATTI_MC_Mth_Lerp2(double deltaX, double deltaY, double x00, double x10, double x01, double x11);
// Java: public static double lerp3(...)
double LIBMATTI_MC_Mth_Lerp3(double deltaX, double deltaY, double deltaZ, double x000, double x100,
                             double x010, double x110, double x001, double x101, double x011, double x111);
// Java: public static float catmullrom(float, float, float, float, float)
float LIBMATTI_MC_Mth_Catmullrom(float t, float p0, float p1, float p2, float p3);
// Java: public static double smoothstep(double)
double LIBMATTI_MC_Mth_Smoothstep(double value);
// Java: public static double smoothstepDerivative(double)
double LIBMATTI_MC_Mth_SmoothstepDerivative(double value);
// Java: public static int sign(double)
int LIBMATTI_MC_Mth_Sign(double value);
// Java: public static float rotLerp(float, float, float)
float LIBMATTI_MC_Mth_RotLerp(float delta, float start, float end);
// Java: public static double rotLerp(double, double, double)
double LIBMATTI_MC_Mth_RotLerpD(double delta, double start, double end);
// Java: public static float rotLerpRad(float, float, float)
float LIBMATTI_MC_Mth_RotLerpRad(float delta, float start, float end);
// Java: public static float triangleWave(float, float)
float LIBMATTI_MC_Mth_TriangleWave(float x, float period);
// Java: public static float square(float)
float LIBMATTI_MC_Mth_Square(float value);
// Java: public static float cube(float)
float LIBMATTI_MC_Mth_Cube(float value);
// Java: public static double square(double)
double LIBMATTI_MC_Mth_SquareD(double value);
// Java: public static int square(int)
int LIBMATTI_MC_Mth_SquareI(int value);
// Java: public static long square(long)
int64_t LIBMATTI_MC_Mth_SquareL(int64_t value);

// Java: public static double clampedMap(double, double, double, double, double)
double LIBMATTI_MC_Mth_ClampedMap(double input, double inMin, double inMax, double outMin, double outMax);
// Java: public static float clampedMap(float, ...)
float LIBMATTI_MC_Mth_ClampedMapF(float input, float inMin, float inMax, float outMin, float outMax);
// Java: public static double map(double, double, double, double, double)
double LIBMATTI_MC_Mth_Map(double input, double inMin, double inMax, double outMin, double outMax);
// Java: public static float map(float, ...)
float LIBMATTI_MC_Mth_MapF(float input, float inMin, float inMax, float outMin, float outMax);

// Java: public static int roundToward(int, int)
int LIBMATTI_MC_Mth_RoundToward(int value, int factor);
// Java: public static int positiveCeilDiv(int, int)
int LIBMATTI_MC_Mth_PositiveCeilDiv(int x, int y);

// Java: public static double lengthSquared(double, double)
double LIBMATTI_MC_Mth_LengthSquared(double x, double y);
// Java: public static double length(double, double)
double LIBMATTI_MC_Mth_Length(double x, double y);
// Java: public static float length(float, float)
float LIBMATTI_MC_Mth_LengthF(float x, float y);
// Java: public static double lengthSquared(double, double, double)
double LIBMATTI_MC_Mth_LengthSquared3(double x, double y, double z);
// Java: public static double length(double, double, double)
double LIBMATTI_MC_Mth_Length3(double x, double y, double z);
// Java: public static float lengthSquared(float, float, float)
float LIBMATTI_MC_Mth_LengthSquared3F(float x, float y, float z);

// Java: public static int quantize(double, int)
int LIBMATTI_MC_Mth_Quantize(double value, int factor);

#endif //MATTICRAFT_NET_MINECRAFT_UTIL_MTH_H
