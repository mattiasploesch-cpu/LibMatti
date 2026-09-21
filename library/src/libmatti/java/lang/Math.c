#include "libmatti/java/lang/Math.h"

#include <math.h>
#include <stdlib.h>

int LIBMATTI_JL_Math_AbsI(int a)
{
    return a < 0 ? -a : a;
}

long LIBMATTI_JL_Math_AbsL(long a)
{
    return a < 0 ? -a : a;
}

float LIBMATTI_JL_Math_AbsF(float a)
{
    return fabsf(a);
}

double LIBMATTI_JL_Math_AbsD(double a)
{
    return fabs(a);
}

int LIBMATTI_JL_Math_MaxI(int a, int b)
{
    return a > b ? a : b;
}

long LIBMATTI_JL_Math_MaxL(long a, long b)
{
    return a > b ? a : b;
}

float LIBMATTI_JL_Math_MaxF(float a, float b)
{
    return fmaxf(a, b);
}

double LIBMATTI_JL_Math_MaxD(double a, double b)
{
    return fmax(a, b);
}

int LIBMATTI_JL_Math_MinI(int a, int b)
{
    return a < b ? a : b;
}

long LIBMATTI_JL_Math_MinL(long a, long b)
{
    return a < b ? a : b;
}

float LIBMATTI_JL_Math_MinF(float a, float b)
{
    return fminf(a, b);
}

double LIBMATTI_JL_Math_MinD(double a, double b)
{
    return fmin(a, b);
}

double LIBMATTI_JL_Math_Floor(double a)
{
    return floor(a);
}

double LIBMATTI_JL_Math_Ceil(double a)
{
    return ceil(a);
}

double LIBMATTI_JL_Math_Sqrt(double a)
{
    return sqrt(a);
}

double LIBMATTI_JL_Math_Cbrt(double a)
{
    return cbrt(a);
}

double LIBMATTI_JL_Math_Pow(double a, double b)
{
    return pow(a, b);
}

double LIBMATTI_JL_Math_Sin(double a)
{
    return sin(a);
}

double LIBMATTI_JL_Math_Cos(double a)
{
    return cos(a);
}

double LIBMATTI_JL_Math_Tan(double a)
{
    return tan(a);
}

double LIBMATTI_JL_Math_Asin(double a)
{
    return asin(a);
}

double LIBMATTI_JL_Math_Acos(double a)
{
    return acos(a);
}

double LIBMATTI_JL_Math_Atan(double a)
{
    return atan(a);
}

double LIBMATTI_JL_Math_Atan2(double y, double x)
{
    return atan2(y, x);
}

double LIBMATTI_JL_Math_Log(double a)
{
    return log(a);
}

double LIBMATTI_JL_Math_Log10(double a)
{
    return log10(a);
}

double LIBMATTI_JL_Math_Exp(double a)
{
    return exp(a);
}

double LIBMATTI_JL_Math_Hypot(double x, double y)
{
    return hypot(x, y);
}

double LIBMATTI_JL_Math_Signum(double d)
{
    return (double) LIBMATTI_JL_Math_SignumF((float) d);
}

float LIBMATTI_JL_Math_SignumF(float f)
{
    if (isnan(f)) return NAN;
    if (f == 0.0f) return f;
    return f > 0.0f ? 1.0f : -1.0f;
}

long LIBMATTI_JL_Math_Round(double a)
{
    // Java: floor(a + 0.5d), with NaN mapping to 0
    if (isnan(a)) return 0;
    return (long) floor(a + 0.5);
}

int LIBMATTI_JL_Math_RoundF(float a)
{
    if (isnan(a)) return 0;
    return (int) floorf(a + 0.5f);
}

int LIBMATTI_JL_Math_FloorDivI(int x, int y)
{
    int r = x / y;
    if ((x ^ y) < 0 && r * y != x)
        r--;
    return r;
}

long LIBMATTI_JL_Math_FloorDivL(long x, long y)
{
    long r = x / y;
    if ((x ^ y) < 0 && r * y != x)
        r--;
    return r;
}

int LIBMATTI_JL_Math_FloorModI(int x, int y)
{
    return x - LIBMATTI_JL_Math_FloorDivI(x, y) * y;
}

long LIBMATTI_JL_Math_FloorModL(long x, long y)
{
    return x - LIBMATTI_JL_Math_FloorDivL(x, y) * y;
}

double LIBMATTI_JL_Math_ToRadians(double angdeg)
{
    return angdeg / 180.0 * LIBMATTI_JL_Math_PI;
}

double LIBMATTI_JL_Math_ToDegrees(double angrad)
{
    return angrad * 180.0 / LIBMATTI_JL_Math_PI;
}

double LIBMATTI_JL_Math_Random(void)
{
    // Java: an internally seeded Random.nextDouble()
    return (double) rand() / ((double) RAND_MAX + 1.0);
}

int LIBMATTI_JL_Math_ClampI(int value, int min, int max)
{
    return value < min ? min : (value > max ? max : value);
}

long LIBMATTI_JL_Math_ClampL(long value, long min, long max)
{
    return value < min ? min : (value > max ? max : value);
}

float LIBMATTI_JL_Math_ClampF(float value, float min, float max)
{
    return value < min ? min : (value > max ? max : value);
}

double LIBMATTI_JL_Math_ClampD(double value, double min, double max)
{
    return value < min ? min : (value > max ? max : value);
}

int LIBMATTI_JL_Math_CompareI(int x, int y)
{
    return x < y ? -1 : (x == y ? 0 : 1);
}

int LIBMATTI_JL_Math_CompareL(long x, long y)
{
    return x < y ? -1 : (x == y ? 0 : 1);
}

int LIBMATTI_JL_Math_CompareF(float x, float y)
{
    // Java: -1.0f is considered less than values >= 0.0f; NaN is greater than everything
    if (x < y) return -1;
    if (x > y) return 1;
    // NaN == NaN compares equal in C but Java orders NaN greater
    if (isnan(x) && isnan(y)) return 0;
    if (isnan(x)) return 1;
    if (isnan(y)) return -1;
    // -0.0f < 0.0f
    if (x == 0.0f && y == 0.0f)
        return signbit(x) ? -1 : (signbit(y) ? 1 : 0);
    return 0;
}

int LIBMATTI_JL_Math_CompareD(double x, double y)
{
    if (x < y) return -1;
    if (x > y) return 1;
    if (isnan(x) && isnan(y)) return 0;
    if (isnan(x)) return 1;
    if (isnan(y)) return -1;
    if (x == 0.0 && y == 0.0)
        return signbit(x) ? -1 : (signbit(y) ? 1 : 0);
    return 0;
}
