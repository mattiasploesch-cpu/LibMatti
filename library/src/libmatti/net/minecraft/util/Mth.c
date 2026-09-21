// Port of net.minecraft.util.Mth.

#include "libmatti/net/minecraft/util/Mth.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final int SIN_QUANTIZATION = 65536, SIN_MASK = 65535
// Java: private static final int COS_OFFSET = 16384
// Java: private static final double SIN_SCALE = 10430.378350470453
#define SIN_MASK 65535
#define COS_OFFSET 16384
#define SIN_SCALE 10430.378350470453

static float sinTable[65536];
static int sinTableReady = 0;

// Java: ASIN_TAB/COS_TAB are filled in the static initialiser
// Java: private static final double FRAC_BIAS = Double.longBitsToDouble(4805340802404319232L)
#define FRAC_BIAS 651.7997231005632

static double asinTab[257];
static double cosTab[257];
static int lookupTablesReady = 0;

static void ensure_tables(void)
{
    if (!sinTableReady)
    {
        for (int i = 0; i < 65536; i++)
            sinTable[i] = (float) sin((double) i / SIN_SCALE);
        sinTableReady = 1;
    }
    if (!lookupTablesReady)
    {
        for (int i = 0; i < 257; i++)
        {
            double d1 = asin((double) i / 256.0);
            cosTab[i] = cos(d1);
            asinTab[i] = d1;
        }
        lookupTablesReady = 1;
    }
}

// Java: public static float sin(double value)
float LIBMATTI_MC_Mth_Sin(double value)
{
    ensure_tables();
    return sinTable[(int) ((int64_t) (value * SIN_SCALE) & SIN_MASK)];
}

// Java: public static float cos(double value)
float LIBMATTI_MC_Mth_Cos(double value)
{
    ensure_tables();
    return sinTable[(int) ((int64_t) (value * SIN_SCALE + COS_OFFSET) & SIN_MASK)];
}

// Java: public static float sqrt(float value)
float LIBMATTI_MC_Mth_Sqrt(float value)
{
    return (float) sqrt((double) value);
}

// Java: public static int floor(float value)
int LIBMATTI_MC_Mth_Floor(float value)
{
    int i = (int) value;
    return value < (float) i ? i - 1 : i;
}

// Java: public static int floor(double value)
int LIBMATTI_MC_Mth_FloorD(double value)
{
    int i = (int) value;
    return value < (double) i ? i - 1 : i;
}

// Java: public static long lfloor(double value)
int64_t LIBMATTI_MC_Mth_LFloor(double value)
{
    int64_t i = (int64_t) value;
    return value < (double) i ? i - 1 : i;
}

// Java: public static float abs(float value)
float LIBMATTI_MC_Mth_Abs(float value)
{
    return fabsf(value);
}

// Java: public static int abs(int value)
int LIBMATTI_MC_Mth_AbsI(int value)
{
    return value < 0 ? -value : value;
}

// Java: public static int ceil(float value)
int LIBMATTI_MC_Mth_Ceil(float value)
{
    int i = (int) value;
    return value > (float) i ? i + 1 : i;
}

// Java: public static int ceil(double value)
int LIBMATTI_MC_Mth_CeilD(double value)
{
    int i = (int) value;
    return value > (double) i ? i + 1 : i;
}

// Java: public static long ceilLong(double value)
int64_t LIBMATTI_MC_Mth_CeilLong(double value)
{
    int64_t i = (int64_t) value;
    return value > (double) i ? i + 1 : i;
}

// Java: public static int clamp(int value, int min, int max)
int LIBMATTI_MC_Mth_ClampI(int value, int min, int max)
{
    return value < min ? min : (value > max ? max : value);
}

// Java: public static long clamp(long value, long min, long max)
int64_t LIBMATTI_MC_Mth_ClampL(int64_t value, int64_t min, int64_t max)
{
    return value < min ? min : (value > max ? max : value);
}

// Java: public static float clamp(float value, float min, float max)
float LIBMATTI_MC_Mth_Clamp(float value, float min, float max)
{
    return value < min ? min : (value > max ? max : value);
}

// Java: public static double clamp(double value, double min, double max)
double LIBMATTI_MC_Mth_ClampD(double value, double min, double max)
{
    return value < min ? min : (value > max ? max : value);
}

// Java: public static double clampedLerp(double delta, double start, double end)
double LIBMATTI_MC_Mth_ClampedLerp(double delta, double start, double end)
{
    if (delta < 0.0)
        return start;
    return delta > 1.0 ? end : LIBMATTI_MC_Mth_LerpD(delta, start, end);
}

// Java: public static float clampedLerp(float delta, float start, float end)
float LIBMATTI_MC_Mth_ClampedLerpF(float delta, float start, float end)
{
    if (delta < 0.0f)
        return start;
    return delta > 1.0f ? end : LIBMATTI_MC_Mth_Lerp(delta, start, end);
}

// Java: public static int absMax(int a, int b)
int LIBMATTI_MC_Mth_AbsMaxI(int a, int b)
{
    int absA = LIBMATTI_MC_Mth_AbsI(a);
    int absB = LIBMATTI_MC_Mth_AbsI(b);
    return absA > absB ? absA : absB;
}

// Java: public static float absMax(float a, float b)
float LIBMATTI_MC_Mth_AbsMax(float a, float b)
{
    float absA = LIBMATTI_MC_Mth_Abs(a);
    float absB = LIBMATTI_MC_Mth_Abs(b);
    return absA > absB ? absA : absB;
}

// Java: public static double absMax(double a, double b)
double LIBMATTI_MC_Mth_AbsMaxD(double a, double b)
{
    double absA = fabs(a);
    double absB = fabs(b);
    return absA > absB ? absA : absB;
}

// Java: public static int chessboardDistance(int x1, int z1, int x2, int z2)
int LIBMATTI_MC_Mth_ChessboardDistance(int x1, int z1, int x2, int z2)
{
    return LIBMATTI_MC_Mth_AbsMaxI(x2 - x1, z2 - z1);
}

// Java: public static int floorDiv(int x, int y)
int LIBMATTI_MC_Mth_FloorDiv(int x, int y)
{
    int quotient = x / y;
    if ((x ^ y) < 0 && quotient * y != x)
        quotient--;
    return quotient;
}

// Java: public static boolean equal(float a, float b)
int LIBMATTI_MC_Mth_Equal(float a, float b)
{
    return fabsf(b - a) < 1.0E-5f;
}

// Java: public static boolean equal(double a, double b)
int LIBMATTI_MC_Mth_EqualD(double a, double b)
{
    return fabs(b - a) < 1.0E-5;
}

// Java: public static int positiveModulo(int x, int y) - Math.floorMod
int LIBMATTI_MC_Mth_PositiveModuloI(int x, int y)
{
    int remainder = x % y;
    if (remainder != 0 && (remainder ^ y) < 0)
        remainder += y;
    return remainder;
}

// Java: public static float positiveModulo(float x, float y)
float LIBMATTI_MC_Mth_PositiveModulo(float x, float y)
{
    return fmodf(fmodf(x, y) + y, y);
}

// Java: public static double positiveModulo(double x, double y)
double LIBMATTI_MC_Mth_PositiveModuloD(double x, double y)
{
    return fmod(fmod(x, y) + y, y);
}

// Java: public static boolean isMultipleOf(int x, int y)
int LIBMATTI_MC_Mth_IsMultipleOf(int x, int y)
{
    return x % y == 0;
}

// Java: public static byte packDegrees(float value)
int8_t LIBMATTI_MC_Mth_PackDegrees(float value)
{
    return (int8_t) LIBMATTI_MC_Mth_Floor(value * 256.0f / 360.0f);
}

// Java: public static float unpackDegrees(byte value)
float LIBMATTI_MC_Mth_UnpackDegrees(int8_t value)
{
    return (float) value * 360 / 256.0f;
}

// Java: public static int wrapDegrees(int degrees)
int LIBMATTI_MC_Mth_WrapDegreesI(int degrees)
{
    int i = degrees % 360;
    if (i >= 180)
        i -= 360;
    if (i < -180)
        i += 360;
    return i;
}

// Java: public static float wrapDegrees(long degrees)
float LIBMATTI_MC_Mth_WrapDegreesL(int64_t degrees)
{
    float f = (float) (degrees % 360);
    if (f >= 180.0f)
        f -= 360.0f;
    if (f < -180.0f)
        f += 360.0f;
    return f;
}

// Java: public static float wrapDegrees(float degrees)
float LIBMATTI_MC_Mth_WrapDegrees(float degrees)
{
    float f = fmodf(degrees, 360.0f);
    if (f >= 180.0f)
        f -= 360.0f;
    if (f < -180.0f)
        f += 360.0f;
    return f;
}

// Java: public static double wrapDegrees(double degrees)
double LIBMATTI_MC_Mth_WrapDegreesD(double degrees)
{
    double d0 = fmod(degrees, 360.0);
    if (d0 >= 180.0)
        d0 -= 360.0;
    if (d0 < -180.0)
        d0 += 360.0;
    return d0;
}

// Java: public static float degreesDifference(float from, float to)
float LIBMATTI_MC_Mth_DegreesDifference(float from, float to)
{
    return LIBMATTI_MC_Mth_WrapDegrees(to - from);
}

// Java: public static float degreesDifferenceAbs(float from, float to)
float LIBMATTI_MC_Mth_DegreesDifferenceAbs(float from, float to)
{
    return LIBMATTI_MC_Mth_Abs(LIBMATTI_MC_Mth_DegreesDifference(from, to));
}

// Java: public static float rotateIfNecessary(float current, float target, float maxDelta)
float LIBMATTI_MC_Mth_RotateIfNecessary(float current, float target, float maxDelta)
{
    float f = LIBMATTI_MC_Mth_DegreesDifference(current, target);
    float f1 = LIBMATTI_MC_Mth_Clamp(f, -maxDelta, maxDelta);
    return target - f1;
}

// Java: public static float approach(float from, float to, float step)
float LIBMATTI_MC_Mth_Approach(float from, float to, float step)
{
    step = LIBMATTI_MC_Mth_Abs(step);
    return from < to ? LIBMATTI_MC_Mth_Clamp(from + step, from, to)
                     : LIBMATTI_MC_Mth_Clamp(from - step, to, from);
}

// Java: public static float approachDegrees(float from, float to, float step)
float LIBMATTI_MC_Mth_ApproachDegrees(float from, float to, float step)
{
    float f = LIBMATTI_MC_Mth_DegreesDifference(from, to);
    return LIBMATTI_MC_Mth_Approach(from, from + f, step);
}

// Java: public static int getInt(String value, int fallback) - NumberUtils.toInt
int LIBMATTI_MC_Mth_GetInt(const char *value, int fallback)
{
    if (value == NULL || *value == '\0')
        return fallback;
    char *end = NULL;
    long parsed = strtol(value, &end, 10);
    if (end == value || *end != '\0')
        return fallback;
    return (int) parsed;
}

// Java: public static int smallestEncompassingPowerOfTwo(int value)
int LIBMATTI_MC_Mth_SmallestEncompassingPowerOfTwo(int value)
{
    int i = value - 1;
    i |= i >> 1;
    i |= i >> 2;
    i |= i >> 4;
    i |= i >> 8;
    i |= i >> 16;
    return i + 1;
}

// Java: public static int smallestSquareSide(int itemCount) - throws on negative
int LIBMATTI_MC_Mth_SmallestSquareSide(int itemCount)
{
    if (itemCount < 0)
    {
        // Java: throw new IllegalArgumentException("itemCount must be greater than or equal to zero")
        return 0;
    }
    return LIBMATTI_MC_Mth_CeilD(sqrt((double) itemCount));
}

// Java: public static boolean isPowerOfTwo(int value)
int LIBMATTI_MC_Mth_IsPowerOfTwo(int value)
{
    return value != 0 && (value & value - 1) == 0;
}

// Java: private static final int[] MULTIPLY_DE_BRUIJN_BIT_POSITION
static const int MULTIPLY_DE_BRUIJN_BIT_POSITION[32] = {
    0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
    31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9};

// Java: public static int ceillog2(int value)
int LIBMATTI_MC_Mth_CeilLog2(int value)
{
    value = LIBMATTI_MC_Mth_IsPowerOfTwo(value) ? value : LIBMATTI_MC_Mth_SmallestEncompassingPowerOfTwo(value);
    return MULTIPLY_DE_BRUIJN_BIT_POSITION[(int) ((int64_t) value * 125613361L >> 27) & 31];
}

// Java: public static int log2(int value)
int LIBMATTI_MC_Mth_Log2(int value)
{
    return LIBMATTI_MC_Mth_CeilLog2(value) - (LIBMATTI_MC_Mth_IsPowerOfTwo(value) ? 0 : 1);
}

// Java: public static float frac(float value)
float LIBMATTI_MC_Mth_Frac(float value)
{
    return value - (float) LIBMATTI_MC_Mth_Floor(value);
}

// Java: public static double frac(double value)
double LIBMATTI_MC_Mth_FracD(double value)
{
    return value - (double) LIBMATTI_MC_Mth_LFloor(value);
}

// Java: public static long getSeed(int x, int y, int z)
int64_t LIBMATTI_MC_Mth_GetSeed(int x, int y, int z)
{
    int64_t i = (int64_t) x * 3129871 ^ (int64_t) z * 116129781L ^ (int64_t) y;
    i = i * i * 42317861L + i * 11L;
    return i >> 16;
}

// Java: public static double inverseLerp(double start, double end, double value)
double LIBMATTI_MC_Mth_InverseLerp(double start, double end, double value)
{
    return (start - end) / (value - end);
}

// Java: public static float inverseLerp(float start, float end, float value)
float LIBMATTI_MC_Mth_InverseLerpF(float start, float end, float value)
{
    return (start - end) / (value - end);
}

// Java: public static double atan2(double y, double x) - the ASIN_TAB fast path
double LIBMATTI_MC_Mth_Atan2(double y, double x)
{
    ensure_tables();

    double d0 = x * x + y * y;
    if (isnan(d0))
        return NAN;

    int negativeY = y < 0.0;
    if (negativeY)
        y = -y;

    int negativeX = x < 0.0;
    if (negativeX)
        x = -x;

    int swapped = y > x;
    if (swapped)
    {
        double swap = x;
        x = y;
        y = swap;
    }

    double d9 = LIBMATTI_MC_Mth_FastInvSqrt(d0);
    x *= d9;
    y *= d9;

    double d2 = FRAC_BIAS + y;
    int i = (int) *(int64_t *) &d2;
    double d3 = asinTab[i];
    double d4 = cosTab[i];
    double d5 = d2 - FRAC_BIAS;
    double d6 = y * d4 - x * d5;
    double d7 = (6.0 + d6 * d6) * d6 * 0.16666666666666666;
    double d8 = d3 + d7;

    if (swapped)
        d8 = (M_PI / 2) - d8;
    if (negativeX)
        d8 = M_PI - d8;
    if (negativeY)
        d8 = -d8;

    return d8;
}

// Java: public static float invSqrt(float value) - org.joml.Math.invsqrt
float LIBMATTI_MC_Mth_InvSqrt(float value)
{
    return 1.0f / sqrtf(value);
}

// Java: public static double invSqrt(double value)
double LIBMATTI_MC_Mth_InvSqrtD(double value)
{
    return 1.0 / sqrt(value);
}

// Java: @Deprecated public static double fastInvSqrt(double value)
double LIBMATTI_MC_Mth_FastInvSqrt(double value)
{
    double d0 = 0.5 * value;
    int64_t i = *(int64_t *) &value;
    i = 6910469410427058090L - (i >> 1);
    double result = *(double *) &i;
    return result * (1.5 - d0 * result * result);
}

// Java: public static float fastInvCubeRoot(float value)
float LIBMATTI_MC_Mth_FastInvCubeRoot(float value)
{
    int i = *(int *) &value;
    i = 1419967116 - i / 3;
    float f = *(float *) &i;
    f = 0.6666667f * f + 1.0f / (3.0f * f * f * value);
    return 0.6666667f * f + 1.0f / (3.0f * f * f * value);
}

// Java: public static int hsvToRgb(float hue, float saturation, float value)
int LIBMATTI_MC_Mth_HsvToRgb(float hue, float saturation, float value)
{
    return LIBMATTI_MC_Mth_HsvToArgb(hue, saturation, value, 0);
}

// Java: public static int hsvToArgb(float hue, float saturation, float value, int alpha)
int LIBMATTI_MC_Mth_HsvToArgb(float hue, float saturation, float value, int alpha)
{
    int i = (int) (hue * 6.0f) % 6;
    float f = hue * 6.0f - (float) i;
    float f1 = value * (1.0f - saturation);
    float f2 = value * (1.0f - f * saturation);
    float f3 = value * (1.0f - (1.0f - f) * saturation);
    float f4;
    float f5;
    float f6;
    switch (i)
    {
    case 0:
        f4 = value;
        f5 = f3;
        f6 = f1;
        break;
    case 1:
        f4 = f2;
        f5 = value;
        f6 = f1;
        break;
    case 2:
        f4 = f1;
        f5 = value;
        f6 = f3;
        break;
    case 3:
        f4 = f1;
        f5 = f2;
        f6 = value;
        break;
    case 4:
        f4 = f3;
        f5 = f1;
        f6 = value;
        break;
    case 5:
        f4 = value;
        f5 = f1;
        f6 = f2;
        break;
    default:
        return 0;
    }
    int r = LIBMATTI_MC_Mth_ClampI((int) (f4 * 255.0f), 0, 255);
    int g = LIBMATTI_MC_Mth_ClampI((int) (f5 * 255.0f), 0, 255);
    int b = LIBMATTI_MC_Mth_ClampI((int) (f6 * 255.0f), 0, 255);
    return (alpha & 0xFF) << 24 | r << 16 | g << 8 | b;
}

// Java: public static int murmurHash3Mixer(int value)
int LIBMATTI_MC_Mth_MurmurHash3Mixer(int value)
{
    value ^= (unsigned int) value >> 16;
    value *= -2048144789;
    value ^= (unsigned int) value >> 13;
    value *= -1028477387;
    return value ^ (unsigned int) value >> 16;
}

// Java: public static int lerpInt(float delta, int start, int end)
int LIBMATTI_MC_Mth_LerpInt(float delta, int start, int end)
{
    return start + LIBMATTI_MC_Mth_Floor(delta * (float) (end - start));
}

// Java: public static int lerpDiscrete(float delta, int start, int end)
int LIBMATTI_MC_Mth_LerpDiscrete(float delta, int start, int end)
{
    int i = end - start;
    return start + LIBMATTI_MC_Mth_Floor(delta * (float) (i - 1)) + (delta > 0.0f ? 1 : 0);
}

// Java: public static float lerp(float delta, float start, float end)
float LIBMATTI_MC_Mth_Lerp(float delta, float start, float end)
{
    return start + delta * (end - start);
}

// Java: public static double lerp(double delta, double start, double end)
double LIBMATTI_MC_Mth_LerpD(double delta, double start, double end)
{
    return start + delta * (end - start);
}

// Java: public static double lerp2(double deltaX, double deltaY, ...)
double LIBMATTI_MC_Mth_Lerp2(double deltaX, double deltaY, double x00, double x10, double x01, double x11)
{
    return LIBMATTI_MC_Mth_LerpD(deltaY,
                                 LIBMATTI_MC_Mth_LerpD(deltaX, x00, x10),
                                 LIBMATTI_MC_Mth_LerpD(deltaX, x01, x11));
}

// Java: public static double lerp3(...)
double LIBMATTI_MC_Mth_Lerp3(double deltaX, double deltaY, double deltaZ, double x000, double x100,
                             double x010, double x110, double x001, double x101, double x011, double x111)
{
    return LIBMATTI_MC_Mth_LerpD(deltaZ,
                                 LIBMATTI_MC_Mth_Lerp2(deltaX, deltaY, x000, x100, x010, x110),
                                 LIBMATTI_MC_Mth_Lerp2(deltaX, deltaY, x001, x101, x011, x111));
}

// Java: public static float catmullrom(float t, float p0, float p1, float p2, float p3)
float LIBMATTI_MC_Mth_Catmullrom(float t, float p0, float p1, float p2, float p3)
{
    return 0.5f
           * (2.0f * p1
              + (p2 - p0) * t
              + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t * t
              + (3.0f * p1 - p0 - 3.0f * p2 + p3) * t * t * t);
}

// Java: public static double smoothstep(double value)
double LIBMATTI_MC_Mth_Smoothstep(double value)
{
    return value * value * value * (value * (value * 6.0 - 15.0) + 10.0);
}

// Java: public static double smoothstepDerivative(double value)
double LIBMATTI_MC_Mth_SmoothstepDerivative(double value)
{
    return 30.0 * value * value * (value - 1.0) * (value - 1.0);
}

// Java: public static int sign(double value)
int LIBMATTI_MC_Mth_Sign(double value)
{
    if (value == 0.0)
        return 0;
    return value > 0.0 ? 1 : -1;
}

// Java: public static float rotLerp(float delta, float start, float end)
float LIBMATTI_MC_Mth_RotLerp(float delta, float start, float end)
{
    return start + delta * LIBMATTI_MC_Mth_WrapDegrees(end - start);
}

// Java: public static double rotLerp(double delta, double start, double end)
double LIBMATTI_MC_Mth_RotLerpD(double delta, double start, double end)
{
    return start + delta * LIBMATTI_MC_Mth_WrapDegreesD(end - start);
}

// Java: public static float rotLerpRad(float delta, float start, float end)
float LIBMATTI_MC_Mth_RotLerpRad(float delta, float start, float end)
{
    float f = end - start;
    while (f < (float) -M_PI)
        f += (float) (M_PI * 2);
    while (f >= (float) M_PI)
        f -= (float) (M_PI * 2);
    return start + delta * f;
}

// Java: public static float triangleWave(float x, float period)
float LIBMATTI_MC_Mth_TriangleWave(float x, float period)
{
    return (fabsf(fmodf(x, period) - period * 0.5f) - period * 0.25f) / (period * 0.25f);
}

// Java: public static float square(float value)
float LIBMATTI_MC_Mth_Square(float value)
{
    return value * value;
}

// Java: public static float cube(float value)
float LIBMATTI_MC_Mth_Cube(float value)
{
    return value * value * value;
}

// Java: public static double square(double value)
double LIBMATTI_MC_Mth_SquareD(double value)
{
    return value * value;
}

// Java: public static int square(int value)
int LIBMATTI_MC_Mth_SquareI(int value)
{
    return value * value;
}

// Java: public static long square(long value)
int64_t LIBMATTI_MC_Mth_SquareL(int64_t value)
{
    return value * value;
}

// Java: public static double clampedMap(...)
double LIBMATTI_MC_Mth_ClampedMap(double input, double inMin, double inMax, double outMin, double outMax)
{
    return LIBMATTI_MC_Mth_ClampedLerp(LIBMATTI_MC_Mth_InverseLerp(input, inMin, inMax), outMin, outMax);
}

// Java: public static float clampedMap(float, ...)
float LIBMATTI_MC_Mth_ClampedMapF(float input, float inMin, float inMax, float outMin, float outMax)
{
    return LIBMATTI_MC_Mth_ClampedLerpF(LIBMATTI_MC_Mth_InverseLerpF(input, inMin, inMax), outMin, outMax);
}

// Java: public static double map(...)
double LIBMATTI_MC_Mth_Map(double input, double inMin, double inMax, double outMin, double outMax)
{
    return LIBMATTI_MC_Mth_LerpD(LIBMATTI_MC_Mth_InverseLerp(input, inMin, inMax), outMin, outMax);
}

// Java: public static float map(float, ...)
float LIBMATTI_MC_Mth_MapF(float input, float inMin, float inMax, float outMin, float outMax)
{
    return LIBMATTI_MC_Mth_Lerp(LIBMATTI_MC_Mth_InverseLerpF(input, inMin, inMax), outMin, outMax);
}

// Java: public static int roundToward(int value, int factor)
int LIBMATTI_MC_Mth_RoundToward(int value, int factor)
{
    return LIBMATTI_MC_Mth_PositiveCeilDiv(value, factor) * factor;
}

// Java: public static int positiveCeilDiv(int x, int y) - -Math.floorDiv(-x, y)
int LIBMATTI_MC_Mth_PositiveCeilDiv(int x, int y)
{
    return -LIBMATTI_MC_Mth_FloorDiv(-x, y);
}

// Java: public static double lengthSquared(double x, double z)
double LIBMATTI_MC_Mth_LengthSquared(double x, double z)
{
    return x * x + z * z;
}

// Java: public static double length(double x, double z)
double LIBMATTI_MC_Mth_Length(double x, double z)
{
    return sqrt(LIBMATTI_MC_Mth_LengthSquared(x, z));
}

// Java: public static float length(float x, float z)
float LIBMATTI_MC_Mth_LengthF(float x, float z)
{
    return (float) sqrt((double) (x * x + z * z));
}

// Java: public static double lengthSquared(double x, double y, double z)
double LIBMATTI_MC_Mth_LengthSquared3(double x, double y, double z)
{
    return x * x + y * y + z * z;
}

// Java: public static double length(double x, double y, double z)
double LIBMATTI_MC_Mth_Length3(double x, double y, double z)
{
    return sqrt(LIBMATTI_MC_Mth_LengthSquared3(x, y, z));
}

// Java: public static float lengthSquared(float x, float y, float z)
float LIBMATTI_MC_Mth_LengthSquared3F(float x, float y, float z)
{
    return x * x + y * y + z * z;
}

// Java: public static int quantize(double value, int factor)
int LIBMATTI_MC_Mth_Quantize(double value, int factor)
{
    return LIBMATTI_MC_Mth_FloorD(value / (double) factor) * factor;
}
