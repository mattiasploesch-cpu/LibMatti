// Port of java.security.MessageDigest (the "SHA-256" instance used by ModLauncher).

#ifndef MATTICRAFT_MESSAGEDIGEST_H
#define MATTICRAFT_MESSAGEDIGEST_H

#include <stddef.h>

typedef struct LIBMATTI_JS_MessageDigest LIBMATTI_JS_MessageDigest;

// Java: static MessageDigest getInstance(String algorithm)
LIBMATTI_JS_MessageDigest *LIBMATTI_JS_MessageDigest_GetInstance(const char *algorithm);
// Java: byte[] digest(byte[] input); caller frees
unsigned char *LIBMATTI_JS_MessageDigest_Digest(LIBMATTI_JS_MessageDigest *digest, const unsigned char *input,
                                                size_t length, size_t *outLength);
void LIBMATTI_JS_MessageDigest_Free(LIBMATTI_JS_MessageDigest *digest);

#endif //MATTICRAFT_MESSAGEDIGEST_H
