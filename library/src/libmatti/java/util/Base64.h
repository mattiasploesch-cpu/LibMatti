// Port of java.util.Base64 (basic, MIME and URL schemes).

#ifndef MATTICRAFT_JAVA_UTIL_BASE64_H
#define MATTICRAFT_JAVA_UTIL_BASE64_H

#include <stddef.h>
#include <stdint.h>

// Java: public static Base64.Encoder getEncoder() / getMimeEncoder() / getUrlEncoder()
// The port models the encoder/decoder as plain function parameters instead of objects.

// Java: public byte[] encode(byte[] src) - caller frees, *outLength set
// mimeLineLength: 0 = basic (no line breaks), 76 = MIME, -1 = URL (same alphabet as basic)
unsigned char *LIBMATTI_JU_Base64_Encode(const unsigned char *src, size_t length, size_t *outLength,
                                         int mimeLineLength, int urlAlphabet);
// Java: public byte[] decode(byte[] src) - caller frees, *outLength set; NULL on illegal input
unsigned char *LIBMATTI_JU_Base64_Decode(const unsigned char *src, size_t length, size_t *outLength,
                                         int mimeLineLength, int urlAlphabet);

// Convenience wrappers used all over the codebase:
// Java: Base64.getEncoder().encodeToString(...)
char *LIBMATTI_JU_Base64_EncodeToString(const unsigned char *src, size_t length);
// Java: Base64.getDecoder().decode(String)
unsigned char *LIBMATTI_JU_Base64_DecodeString(const char *src, size_t *outLength);

#endif //MATTICRAFT_JAVA_UTIL_BASE64_H
