// Port of java.security.MessageDigest("SHA-256").

#include "libmatti/java/security/MessageDigest.h"

#include <stdlib.h>
#include <string.h>

typedef struct
{
    unsigned int state[8];
    unsigned long long bitLength;
    unsigned char buffer[64];
    size_t bufferLength;
} Sha256;

struct LIBMATTI_JS_MessageDigest
{
    Sha256 sha256;
};

static const unsigned int K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static unsigned int rotr(unsigned int x, int n)
{
    return (x >> n) | (x << (32 - n));
}

static void sha256_reset(Sha256 *sha)
{
    sha->state[0] = 0x6a09e667;
    sha->state[1] = 0xbb67ae85;
    sha->state[2] = 0x3c6ef372;
    sha->state[3] = 0xa54ff53a;
    sha->state[4] = 0x510e527f;
    sha->state[5] = 0x9b05688c;
    sha->state[6] = 0x1f83d9ab;
    sha->state[7] = 0x5be0cd19;
    sha->bitLength = 0;
    sha->bufferLength = 0;
}

static void sha256_block(Sha256 *sha, const unsigned char *block)
{
    unsigned int w[64];

    for (int i = 0; i < 16; i++)
        w[i] = ((unsigned int)block[i * 4] << 24) | ((unsigned int)block[i * 4 + 1] << 16) |
               ((unsigned int)block[i * 4 + 2] << 8) | block[i * 4 + 3];

    for (int i = 16; i < 64; i++)
    {
        unsigned int s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
        unsigned int s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    unsigned int a = sha->state[0], b = sha->state[1], c = sha->state[2], d = sha->state[3];
    unsigned int e = sha->state[4], f = sha->state[5], g = sha->state[6], h = sha->state[7];

    for (int i = 0; i < 64; i++)
    {
        unsigned int s1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
        unsigned int ch = (e & f) ^ (~e & g);
        unsigned int temp1 = h + s1 + ch + K[i] + w[i];
        unsigned int s0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
        unsigned int maj = (a & b) ^ (a & c) ^ (b & c);
        unsigned int temp2 = s0 + maj;

        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    sha->state[0] += a;
    sha->state[1] += b;
    sha->state[2] += c;
    sha->state[3] += d;
    sha->state[4] += e;
    sha->state[5] += f;
    sha->state[6] += g;
    sha->state[7] += h;
}

static void sha256_update(Sha256 *sha, const unsigned char *data, size_t length)
{
    sha->bitLength += (unsigned long long)length * 8;

    while (length > 0)
    {
        size_t take = 64 - sha->bufferLength;
        if (take > length) take = length;

        memcpy(sha->buffer + sha->bufferLength, data, take);
        sha->bufferLength += take;
        data += take;
        length -= take;

        if (sha->bufferLength == 64)
        {
            sha256_block(sha, sha->buffer);
            sha->bufferLength = 0;
        }
    }
}

static void sha256_final(Sha256 *sha, unsigned char *out)
{
    unsigned long long bitLength = sha->bitLength;

    unsigned char padding = 0x80;
    sha256_update(sha, &padding, 1);

    unsigned char zero = 0;
    while (sha->bufferLength != 56) sha256_update(sha, &zero, 1);

    unsigned char lengthBytes[8];
    for (int i = 0; i < 8; i++) lengthBytes[i] = (unsigned char)(bitLength >> (56 - i * 8));
    sha256_update(sha, lengthBytes, 8);

    for (int i = 0; i < 8; i++)
    {
        out[i * 4] = (unsigned char)(sha->state[i] >> 24);
        out[i * 4 + 1] = (unsigned char)(sha->state[i] >> 16);
        out[i * 4 + 2] = (unsigned char)(sha->state[i] >> 8);
        out[i * 4 + 3] = (unsigned char)sha->state[i];
    }
}

LIBMATTI_JS_MessageDigest *LIBMATTI_JS_MessageDigest_GetInstance(const char *algorithm)
{
    if (strcmp(algorithm, "SHA-256") != 0) return NULL;

    LIBMATTI_JS_MessageDigest *digest = calloc(1, sizeof(LIBMATTI_JS_MessageDigest));
    // Java: MessageDigest is reset on creation
    sha256_reset(&digest->sha256);
    return digest;
}

unsigned char *LIBMATTI_JS_MessageDigest_Digest(LIBMATTI_JS_MessageDigest *digest, const unsigned char *input,
                                                size_t length, size_t *outLength)
{
    // Java: digest(input) resets the digest afterwards
    sha256_reset(&digest->sha256);
    if (length > 0) sha256_update(&digest->sha256, input, length);

    unsigned char *out = malloc(32);
    sha256_final(&digest->sha256, out);
    sha256_reset(&digest->sha256);

    *outLength = 32;
    return out;
}

void LIBMATTI_JS_MessageDigest_Free(LIBMATTI_JS_MessageDigest *digest)
{
    free(digest);
}
