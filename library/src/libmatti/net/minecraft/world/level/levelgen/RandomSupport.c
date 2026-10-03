// Port of net.minecraft.world.level.levelgen.RandomSupport.
// The integer paths are the Java arithmetic 1:1 (the shifts ride the C
// unsigned promotion, the multiplies wrap); the MD5 is the RFC 1321 core over
// the UTF-8 bytes - Guava's hashString pads the same stream.

#include "libmatti/net/minecraft/world/level/levelgen/RandomSupport.h"

#include <stdlib.h>
#include <string.h>

int64_t LIBMATTI_MC_RandomSupport_MixStafford13(int64_t seed)
{
    uint64_t z = (uint64_t) seed;
    z = (z ^ (z >> 30)) * (uint64_t) (-4658895280553007687LL);
    z = (z ^ (z >> 27)) * (uint64_t) (-7723592293110705685LL);
    return (int64_t) (z ^ (z >> 31));
}

void LIBMATTI_MC_RandomSupport_UpgradeSeedTo128bitUnmixed(int64_t seed, int64_t *outLo, int64_t *outHi)
{
    int64_t i = seed ^ LIBMATTI_MC_RandomSupport_SILVER_RATIO_64;
    int64_t j = i + LIBMATTI_MC_RandomSupport_GOLDEN_RATIO_64;
    *outLo = i;
    *outHi = j;
}

void LIBMATTI_MC_RandomSupport_UpgradeSeedTo128bit(int64_t seed, int64_t *outLo, int64_t *outHi)
{
    int64_t lo, hi;
    LIBMATTI_MC_RandomSupport_UpgradeSeedTo128bitUnmixed(seed, &lo, &hi);
    *outLo = LIBMATTI_MC_RandomSupport_MixStafford13(lo);
    *outHi = LIBMATTI_MC_RandomSupport_MixStafford13(hi);
}

// RFC 1321: the message digest over the byte string. The digest words are
// little-endian; Java reads the 16 bytes big-endian into the two longs
// (Longs.fromBytes(a[0]..a[7]), Longs.fromBytes(a[8]..a[15])).
#define F1(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define G1(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define H1(x, y, z) ((x) ^ (y) ^ (z))
#define I1(x, y, z) ((y) ^ ((x) | ~(z)))
#define ROTL1(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

static const uint32_t md5_k[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
};
static const int md5_r[64] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                              5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
                              4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                              6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

void LIBMATTI_MC_RandomSupport_SeedFromHashOf(const char *string, int64_t *outLo, int64_t *outHi)
{
    size_t len = strlen(string);

    // the padded message: the byte + 0x80, the zeros, the 8-byte little-endian
    // bit length (the MD5 padding to 56 mod 64)
    size_t total = ((len + 8) / 64 + 1) * 64;
    uint8_t *msg = calloc(total, 1);
    if (msg == NULL)
    {
        *outLo = 0;
        *outHi = 0;
        return;
    }
    memcpy(msg, string, len);
    msg[len] = 0x80;
    uint64_t bits = (uint64_t) len * 8;
    memcpy(msg + total - 8, &bits, 8);

    uint32_t h0 = 0x67452301, h1 = 0xefcdab89, h2 = 0x98badcfe, h3 = 0x10325476;
    for (size_t off = 0; off < total; off += 64)
    {
        uint32_t w[16];
        memcpy(w, msg + off, 64); // little-endian words on LE hosts
        uint32_t a = h0, b = h1, c = h2, d = h3;
        for (int i = 0; i < 64; i++)
        {
            uint32_t f, g;
            if (i < 16) { f = F1(b, c, d); g = i; }
            else if (i < 32) { f = G1(b, c, d); g = (5 * i + 1) % 16; }
            else if (i < 48) { f = H1(b, c, d); g = (3 * i + 5) % 16; }
            else { f = I1(b, c, d); g = (7 * i) % 16; }
            f = f + a + md5_k[i] + w[g];
            a = d;
            d = c;
            c = b;
            b = b + ROTL1(f, md5_r[i]);
        }
        h0 += a; h1 += b; h2 += c; h3 += d;
    }
    free(msg);

    uint8_t digest[16];
    for (int i = 0; i < 4; i++)
    {
        digest[i] = (uint8_t) (h0 >> (8 * i));
        digest[4 + i] = (uint8_t) (h1 >> (8 * i));
        digest[8 + i] = (uint8_t) (h2 >> (8 * i));
        digest[12 + i] = (uint8_t) (h3 >> (8 * i));
    }
    // Java: Longs.fromBytes(abyte[0..7]) reads the bytes big-endian into the long
    uint64_t lo = 0, hi = 0;
    for (int i = 0; i < 8; i++)
        lo = (lo << 8) | digest[i];
    for (int i = 8; i < 16; i++)
        hi = (hi << 8) | digest[i];
    *outLo = (int64_t) lo;
    *outHi = (int64_t) hi;
}
