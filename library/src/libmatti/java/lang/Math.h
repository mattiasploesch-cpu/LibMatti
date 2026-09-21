// Port of java.lang.Math.
// C has no overloading, so the numeric type becomes a name suffix:
// AbsD(double), AbsF(float), AbsI(int), AbsL(long) ...

#ifndef MATTICRAFT_JAVA_LANG_MATH_H
#define MATTICRAFT_JAVA_LANG_MATH_H

// Java: public static final double E / PI
#define LIBMATTI_JL_Math_E 2.718281828459045
#define LIBMATTI_JL_Math_PI 3.141592653589793

// Java: public static int abs(int a)
int LIBMATTI_JL_Math_AbsI(int a);
// Java: public static long abs(long a)
long LIBMATTI_JL_Math_AbsL(long a);
// Java: public static float abs(float a)
float LIBMATTI_JL_Math_AbsF(float a);
// Java: public static double abs(double a)
double LIBMATTI_JL_Math_AbsD(double a);

// Java: public static int max(int a, int b)
int LIBMATTI_JL_Math_MaxI(int a, int b);
// Java: public static long max(long a, long b)
long LIBMATTI_JL_Math_MaxL(long a, long b);
// Java: public static float max(float a, float b)
float LIBMATTI_JL_Math_MaxF(float a, float b);
// Java: public static double max(double a, double b)
double LIBMATTI_JL_Math_MaxD(double a, double b);

// Java: public static int min(int a, int b)
int LIBMATTI_JL_Math_MinI(int a, int b);
// Java: public static long min(long a, long b)
long LIBMATTI_JL_Math_MinL(long a, long b);
// Java: public static float min(float a, float b)
float LIBMATTI_JL_Math_MinF(float a, float b);
// Java: public static double min(double a, double b)
double LIBMATTI_JL_Math_MinD(double a, double b);

// Java: public static double floor(double a)
double LIBMATTI_JL_Math_Floor(double a);
// Java: public static double ceil(double a)
double LIBMATTI_JL_Math_Ceil(double a);
// Java: public static double sqrt(double a)
double LIBMATTI_JL_Math_Sqrt(double a);
// Java: public static double cbrt(double a)
double LIBMATTI_JL_Math_Cbrt(double a);
// Java: public static double pow(double a, double b)
double LIBMATTI_JL_Math_Pow(double a, double b);
// Java: public static double sin(double a)
double LIBMATTI_JL_Math_Sin(double a);
// Java: public static double cos(double a)
double LIBMATTI_JL_Math_Cos(double a);
// Java: public static double tan(double a)
double LIBMATTI_JL_Math_Tan(double a);
// Java: public static double asin(double a)
double LIBMATTI_JL_Math_Asin(double a);
// Java: public static double acos(double a)
double LIBMATTI_JL_Math_Acos(double a);
// Java: public static double atan(double a)
double LIBMATTI_JL_Math_Atan(double a);
// Java: public static double atan2(double y, double x)
double LIBMATTI_JL_Math_Atan2(double y, double x);
// Java: public static double log(double a)
double LIBMATTI_JL_Math_Log(double a);
// Java: public static double log10(double a)
double LIBMATTI_JL_Math_Log10(double a);
// Java: public static double exp(double a)
double LIBMATTI_JL_Math_Exp(double a);
// Java: public static double hypot(double x, double y)
double LIBMATTI_JL_Math_Hypot(double x, double y);

// Java: public static double signum(double d)
double LIBMATTI_JL_Math_Signum(double d);
// Java: public static float signum(float f)
float LIBMATTI_JL_Math_SignumF(float f);

// Java: public static long round(double a)
long LIBMATTI_JL_Math_Round(double a);
// Java: public static int round(float a)
int LIBMATTI_JL_Math_RoundF(float a);

// Java: public static int floorDiv(int x, int y)
int LIBMATTI_JL_Math_FloorDivI(int x, int y);
// Java: public static long floorDiv(long x, long y)
long LIBMATTI_JL_Math_FloorDivL(long x, long y);
// Java: public static int floorMod(int x, int y)
int LIBMATTI_JL_Math_FloorModI(int x, int y);
// Java: public static long floorMod(long x, long y)
long LIBMATTI_JL_Math_FloorModL(long x, long y);

// Java: public static double toRadians(double angdeg)
double LIBMATTI_JL_Math_ToRadians(double angdeg);
// Java: public static double toDegrees(double angrad)
double LIBMATTI_JL_Math_ToDegrees(double angrad);

// Java: public static double random()
double LIBMATTI_JL_Math_Random(void);

// Java 21: public static int clamp(int value, int min, int max)
int LIBMATTI_JL_Math_ClampI(int value, int min, int max);
// Java 21: public static long clamp(long value, long min, long max)
long LIBMATTI_JL_Math_ClampL(long value, long min, long max);
// Java 21: public static float clamp(float value, float min, float max)
float LIBMATTI_JL_Math_ClampF(float value, float min, float max);
// Java 21: public static double clamp(double value, double min, double max)
double LIBMATTI_JL_Math_ClampD(double value, double min, double max);

// Java: public static int compare(int x, int y) / long / float / double
int LIBMATTI_JL_Math_CompareI(int x, int y);
int LIBMATTI_JL_Math_CompareL(long x, long y);
int LIBMATTI_JL_Math_CompareF(float x, float y);
int LIBMATTI_JL_Math_CompareD(double x, double y);

#endif //MATTICRAFT_JAVA_LANG_MATH_H
