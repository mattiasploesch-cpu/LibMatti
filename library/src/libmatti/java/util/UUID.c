// Port of java.util.UUID (implementation).

#include "libmatti/java/util/UUID.h"

#include "libmatti/java/util/Random.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Java: public UUID(long mostSigBits, long leastSigBits)
LIBMATTI_JU_UUID LIBMATTI_JU_UUID_FromBits(uint64_t mostSigBits, uint64_t leastSigBits)
{
    LIBMATTI_JU_UUID uuid = {mostSigBits, leastSigBits};
    return uuid;
}

// The MD5 core (Java: UUID.nameUUIDFromBytes runs MessageDigest.getInstance("MD5")).
// A minimal, self-contained MD5 over one block-aligned buffer.
static void md5(const uint8_t *data, size_t length, uint8_t digest[16])
{
    static const uint32_t K[64] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
    };
    static const int S[64] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                              5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
                              4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                              6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

    uint32_t h0 = 0x67452301, h1 = 0xefcdab89, h2 = 0x98badcfe, h3 = 0x10325476;

    // one-shot: the port pads to a single message (lengths stay small)
    size_t total = ((length + 8) / 64 + 1) * 64;
    uint8_t *msg = calloc(total, 1);
    if (msg == NULL)
    {
        memset(digest, 0, 16);
        return;
    }
    memcpy(msg, data, length);
    msg[length] = 0x80;
    uint64_t bits = (uint64_t) length * 8;
    memcpy(msg + total - 8, &bits, 8);

    for (size_t off = 0; off < total; off += 64)
    {
        uint32_t m[16];
        memcpy(m, msg + off, 64);
        uint32_t a = h0, b = h1, c = h2, d = h3;
        for (int i = 0; i < 64; i++)
        {
            uint32_t f, g;
            if (i < 16)
            {
                f = (b & c) | (~b & d);
                g = i;
            }
            else if (i < 32)
            {
                f = (d & b) | (~d & c);
                g = (5 * i + 1) % 16;
            }
            else if (i < 48)
            {
                f = b ^ c ^ d;
                g = (3 * i + 5) % 16;
            }
            else
            {
                f = c ^ (b | ~d);
                g = (7 * i) % 16;
            }
            f = f + a + K[i] + m[g];
            a = d;
            d = c;
            c = b;
            b = b + ((f << S[i]) | (f >> (32 - S[i])));
        }
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
    }
    free(msg);

    memcpy(digest + 0, &h0, 4);
    memcpy(digest + 4, &h1, 4);
    memcpy(digest + 8, &h2, 4);
    memcpy(digest + 12, &h3, 4);
}

// Java: public static UUID nameUUIDFromBytes(byte[] name) - the MD5 digest with
// version 3 and the IETF variant stamped in.
LIBMATTI_JU_UUID LIBMATTI_JU_UUID_NameUUIDFromBytes(const uint8_t *bytes, size_t length)
{
    uint8_t digest[16];
    md5(bytes, length, digest);
    uint64_t most = 0, least = 0;
    memcpy(&most, digest, 8);
    memcpy(&least, digest + 8, 8);
    most &= 0xFFFFFFFFFFFF0FFFULL; // clear the version nibble
    most |= 0x0000000000003000ULL; // version 3
    least &= 0x3FFFFFFFFFFFFFFFULL; // clear the variant bits
    least |= 0x8000000000000000ULL; // IETF variant
    return LIBMATTI_JU_UUID_FromBits(most, least);
}

// Java: public static UUID randomUUID() - SecureRandom over 16 bytes; the port
// draws through the java/util Random port (time-seeded) so runs stay reproducible.
LIBMATTI_JU_UUID LIBMATTI_JU_UUID_RandomUUID(void)
{
    static LIBMATTI_JU_Random *rng = NULL;
    if (rng == NULL)
        rng = LIBMATTI_JU_Random_NewSeeded((uint64_t) time(NULL));
    LIBMATTI_JU_UUID uuid;
    uint64_t most = (uint64_t) LIBMATTI_JU_Random_NextLong(rng);
    uint64_t least = (uint64_t) LIBMATTI_JU_Random_NextLong(rng);
    most &= 0xFFFFFFFFFFFF0FFFULL;
    most |= 0x0000000000004000ULL; // version 4
    least &= 0x3FFFFFFFFFFFFFFFULL;
    least |= 0x8000000000000000ULL;
    uuid.mostSigBits = most;
    uuid.leastSigBits = least;
    return uuid;
}

LIBMATTI_JU_UUID LIBMATTI_JU_UUID_Zero(void)
{
    return LIBMATTI_JU_UUID_FromBits(0, 0);
}

int LIBMATTI_JU_UUID_Version(const LIBMATTI_JU_UUID *uuid)
{
    // Java: (int)((mostSigBits >> 12) & 0x0f)
    return (int) ((uuid->mostSigBits >> 12) & 0x0fULL);
}

int LIBMATTI_JU_UUID_Variant(const LIBMATTI_JU_UUID *uuid)
{
    // Java: the variant table over the top bits of leastSigBits
    uint64_t least = uuid->leastSigBits;
    if ((least & 0x8000000000000000ULL) == 0)
        return 0; // NCS backward compatibility
    if ((least & 0x4000000000000000ULL) == 0)
        return 2; // IETF RFC 4122
    if ((least & 0x2000000000000000ULL) == 0)
        return 6; // Microsoft
    return 7;     // reserved
}

uint64_t LIBMATTI_JU_UUID_GetMostSignificantBits(const LIBMATTI_JU_UUID *uuid)
{
    return uuid->mostSigBits;
}

uint64_t LIBMATTI_JU_UUID_GetLeastSignificantBits(const LIBMATTI_JU_UUID *uuid)
{
    return uuid->leastSigBits;
}

// Java: public long timestamp() - version 1 layout of mostSigBits (invalid for
// other versions in Java; the port reproduces the bit surgery)
uint64_t LIBMATTI_JU_UUID_Timestamp(const LIBMATTI_JU_UUID *uuid)
{
    uint64_t most = uuid->mostSigBits;
    return ((most & 0x0000000000000FFFULL) << 48) | (((most >> 16) & 0xFFFFULL) << 32) | (most >> 32);
}

int LIBMATTI_JU_UUID_ClockSequence(const LIBMATTI_JU_UUID *uuid)
{
    // Java: (int)((leastSigBits & 0x3FFF000000000000L) >>> 48)
    return (int) ((uuid->leastSigBits & 0x3FFF000000000000ULL) >> 48);
}

uint64_t LIBMATTI_JU_UUID_Node(const LIBMATTI_JU_UUID *uuid)
{
    return uuid->leastSigBits & 0x0000FFFFFFFFFFFFULL;
}

static void hex16(char *out, uint16_t value)
{
    static const char digits[] = "0123456789abcdef";
    for (int i = 3; i >= 0; i--)
    {
        out[i] = digits[value & 0xf];
        value >>= 4;
    }
}

void LIBMATTI_JU_UUID_ToString(const LIBMATTI_JU_UUID *uuid, char *out, size_t outSize)
{
    if (out == NULL || outSize < 37)
        return;
    uint32_t a = (uint32_t) (uuid->mostSigBits >> 32);
    uint16_t b = (uint16_t) (uuid->mostSigBits >> 16);
    uint16_t c = (uint16_t) uuid->mostSigBits;
    uint16_t d = (uint16_t) (uuid->leastSigBits >> 48);
    uint16_t e1 = (uint16_t) (uuid->leastSigBits >> 32);
    uint16_t e2 = (uint16_t) (uuid->leastSigBits >> 16);
    uint16_t e3 = (uint16_t) uuid->leastSigBits;
    hex16(out, (uint16_t) (a >> 16));
    hex16(out + 4, (uint16_t) a);
    out[8] = '-';
    hex16(out + 9, b);
    out[13] = '-';
    hex16(out + 14, c);
    out[18] = '-';
    hex16(out + 19, d);
    out[23] = '-';
    hex16(out + 24, e1);
    hex16(out + 28, e2);
    hex16(out + 32, e3);
    out[36] = '\0';
}

// Java: public static UUID fromString(String) - the dashed 5-group hex form
bool LIBMATTI_JU_UUID_FromString(const char *input, LIBMATTI_JU_UUID *out)
{
    if (input == NULL || out == NULL)
        return false;
    uint64_t words[5] = {0, 0, 0, 0, 0};
    int lengths[5] = {8, 4, 4, 4, 12};
    int group = 0, pos = 0;
    for (const char *p = input; *p != '\0'; p++)
    {
        char ch = *p;
        if (ch == '-')
        {
            // Java: the parse uses Long.parseLong(group, 16) - a short group is
            // right-padded semantics through parseLong, so digits count must match
            if (pos != lengths[group] || group >= 4)
                return false;
            group++;
            pos = 0;
            continue;
        }
        int value;
        if (ch >= '0' && ch <= '9')
            value = ch - '0';
        else if (ch >= 'a' && ch <= 'f')
            value = ch - 'a' + 10;
        else if (ch >= 'A' && ch <= 'F')
            value = ch - 'A' + 10;
        else
            return false;
        if (pos >= lengths[group])
            return false;
        words[group] = (words[group] << 4) | (uint64_t) value;
        pos++;
    }
    if (group != 4 || pos != lengths[4])
        return false;
    uint64_t most = (words[0] << 32) | (words[1] << 16) | words[2];
    uint64_t least = (words[3] << 48) | (words[4]);
    *out = LIBMATTI_JU_UUID_FromBits(most, least);
    return true;
}

bool LIBMATTI_JU_UUID_Equals(const LIBMATTI_JU_UUID *a, const LIBMATTI_JU_UUID *b)
{
    if (a == NULL || b == NULL)
        return a == b;
    return a->mostSigBits == b->mostSigBits && a->leastSigBits == b->leastSigBits;
}
