// Port of net.neoforged.fml.loading.TracingPrintStream.
// Java extends java.io.PrintStream (which the port does not model) and prefixes every logged line
// with the caller's origin, taken from Thread.currentThread().getStackTrace().

#ifndef MATTICRAFT_FML_LOADING_TRACINGPRINTSTREAM_H
#define MATTICRAFT_FML_LOADING_TRACINGPRINTSTREAM_H

#include "libmatti/cpw/modlauncher/LogManager.h"

typedef struct LIBMATTI_FML_TracingPrintStream LIBMATTI_FML_TracingPrintStream;

// Java: public TracingPrintStream(Logger logger, PrintStream original)
LIBMATTI_FML_TracingPrintStream *LIBMATTI_FML_TracingPrintStream_New(LIBMATTI_ML_Logger *logger);
void LIBMATTI_FML_TracingPrintStream_Free(LIBMATTI_FML_TracingPrintStream *stream);

// Java: @Override public void println(Object o) - String.valueOf(o)
void LIBMATTI_FML_TracingPrintStream_PrintlnObject(LIBMATTI_FML_TracingPrintStream *stream, const char *value);
// Java: @Override public void println(String s)
void LIBMATTI_FML_TracingPrintStream_PrintlnString(LIBMATTI_FML_TracingPrintStream *stream, const char *value);
// Java: @Override public void println(boolean x)
void LIBMATTI_FML_TracingPrintStream_PrintlnBoolean(LIBMATTI_FML_TracingPrintStream *stream, int value);
// Java: @Override public void println(char x)
void LIBMATTI_FML_TracingPrintStream_PrintlnChar(LIBMATTI_FML_TracingPrintStream *stream, char value);
// Java: @Override public void println(int x)
void LIBMATTI_FML_TracingPrintStream_PrintlnInt(LIBMATTI_FML_TracingPrintStream *stream, int value);
// Java: @Override public void println(long x)
void LIBMATTI_FML_TracingPrintStream_PrintlnLong(LIBMATTI_FML_TracingPrintStream *stream, long long value);
// Java: @Override public void println(float x)
void LIBMATTI_FML_TracingPrintStream_PrintlnFloat(LIBMATTI_FML_TracingPrintStream *stream, float value);
// Java: @Override public void println(double x)
void LIBMATTI_FML_TracingPrintStream_PrintlnDouble(LIBMATTI_FML_TracingPrintStream *stream, double value);
// Java: @Override public void println(char[] x)
void LIBMATTI_FML_TracingPrintStream_PrintlnChars(LIBMATTI_FML_TracingPrintStream *stream, const char *value);

#endif //MATTICRAFT_FML_LOADING_TRACINGPRINTSTREAM_H
