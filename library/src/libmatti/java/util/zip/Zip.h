// Port of java.util.zip (the surface the loader and game need).

#ifndef MATTICRAFT_JAVA_UTIL_ZIP_H
#define MATTICRAFT_JAVA_UTIL_ZIP_H

#include <stddef.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
// Java: java.util.zip.ZipException (extends IOException)
// The port models exceptions as Throwables; the helpers build/cast them.
// ---------------------------------------------------------------------------

// Java: public ZipException(String s)
struct LIBMATTI_JL_Throwable;
struct LIBMATTI_JL_Throwable *LIBMATTI_JU_ZipException_New(const char *message);
// Java: instanceof check over the Throwable class name
int LIBMATTI_JU_ZipException_IsInstance(const struct LIBMATTI_JL_Throwable *throwable);

// ---------------------------------------------------------------------------
// Java: java.util.zip.Deflater
// ---------------------------------------------------------------------------

// Java: public static final int DEFAULT_COMPRESSION
#define LIBMATTI_JU_Deflater_DEFAULT_COMPRESSION (-1)

typedef struct LIBMATTI_JU_Deflater LIBMATTI_JU_Deflater;

// Java: public Deflater(int level, boolean nowrap)
LIBMATTI_JU_Deflater *LIBMATTI_JU_Deflater_New(int level, int nowrap);
void LIBMATTI_JU_Deflater_Free(LIBMATTI_JU_Deflater *deflater);

// Java: public int deflate(byte[] output) - compresses as much as fits
size_t LIBMATTI_JU_Deflater_Deflate(LIBMATTI_JU_Deflater *deflater,
                                    unsigned char *output, size_t outputLength);
// Java: public void setInput(byte[] input)
void LIBMATTI_JU_Deflater_SetInput(LIBMATTI_JU_Deflater *deflater,
                                   const unsigned char *input, size_t length);
// Java: public boolean finished()
int LIBMATTI_JU_Deflater_Finished(const LIBMATTI_JU_Deflater *deflater);
// Java: public void finish()
void LIBMATTI_JU_Deflater_Finish(LIBMATTI_JU_Deflater *deflater);

// Convenience: one-shot deflate - caller frees, *outLength set
unsigned char *LIBMATTI_JU_Deflater_Compress(const unsigned char *input, size_t length, size_t *outLength);

// ---------------------------------------------------------------------------
// Java: java.util.zip.Inflater
// ---------------------------------------------------------------------------

typedef struct LIBMATTI_JU_Inflater LIBMATTI_JU_Inflater;

// Java: public Inflater(boolean nowrap)
LIBMATTI_JU_Inflater *LIBMATTI_JU_Inflater_New(int nowrap);
void LIBMATTI_JU_Inflater_Free(LIBMATTI_JU_Inflater *inflater);

// Java: public void setInput(byte[] input)
void LIBMATTI_JU_Inflater_SetInput(LIBMATTI_JU_Inflater *inflater,
                                   const unsigned char *input, size_t length);
// Java: public int inflate(byte[] output) - returns the bytes written; 0 means
// either needs-input or finished (check needsInput/finished)
size_t LIBMATTI_JU_Inflater_Inflate(LIBMATTI_JU_Inflater *inflater,
                                    unsigned char *output, size_t outputLength);
// Java: public boolean finished()
int LIBMATTI_JU_Inflater_Finished(const LIBMATTI_JU_Inflater *inflater);
// Java: public boolean needsInput()
int LIBMATTI_JU_Inflater_NeedsInput(const LIBMATTI_JU_Inflater *inflater);

// Convenience: one-shot inflate. *outLength is the decompressed size; the
// caller guesses the capacity in outCapacityHint (0 = 4x input). Returns NULL
// on a corrupt stream (Java: throws DataFormatException).
unsigned char *LIBMATTI_JU_Inflater_Decompress(const unsigned char *input, size_t length,
                                               size_t outCapacityHint, size_t *outLength);

#endif //MATTICRAFT_JAVA_UTIL_ZIP_H
