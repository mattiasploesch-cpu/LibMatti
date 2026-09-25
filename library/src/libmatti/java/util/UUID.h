// Port of java.util.UUID. Java models the UUID as a 128-bit immutable value
// (mostSigBits/leastSigBits) with the version/variant bit layout; the C port
// keeps the same two words plus the canonical 8-4-4-4-12 string form used by
// the game protocol and the save format.

#ifndef MATTICRAFT_JAVA_UTIL_UUID_H
#define MATTICRAFT_JAVA_UTIL_UUID_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public final class UUID
typedef struct LIBMATTI_JU_UUID
{
    // Java: private final long mostSigBits / leastSigBits
    uint64_t mostSigBits;
    uint64_t leastSigBits;
} LIBMATTI_JU_UUID;

// Java: public UUID(long mostSigBits, long leastSigBits)
LIBMATTI_JU_UUID LIBMATTI_JU_UUID_FromBits(uint64_t mostSigBits, uint64_t leastSigBits);
// The name-based form (Java: UUID.nameUUIDFromBytes(byte[])) - MD5 over the
// bytes, version 3, variant IETF; the port reproduces the version/variant
// bit surgery over the hash words.
LIBMATTI_JU_UUID LIBMATTI_JU_UUID_NameUUIDFromBytes(const uint8_t *bytes, size_t length);
// Java: public static UUID randomUUID() - SecureRandom in Java; the port uses
// the java/util Random port seeded from the time so callers stay deterministic.
LIBMATTI_JU_UUID LIBMATTI_JU_UUID_RandomUUID(void);
// The version-0 zero UUID (Java models it through the all-zero bit pattern).
LIBMATTI_JU_UUID LIBMATTI_JU_UUID_Zero(void);

// Java: public int version() - the 4-bit version field (bits 15..12 of mostSigBits)
int LIBMATTI_JU_UUID_Version(const LIBMATTI_JU_UUID *uuid);
// Java: public int variant() - the variant field (top bits of leastSigBits)
int LIBMATTI_JU_UUID_Variant(const LIBMATTI_JU_UUID *uuid);
// Java: public long getMostSignificantBits() / getLeastSignificantBits()
uint64_t LIBMATTI_JU_UUID_GetMostSignificantBits(const LIBMATTI_JU_UUID *uuid);
uint64_t LIBMATTI_JU_UUID_GetLeastSignificantBits(const LIBMATTI_JU_UUID *uuid);

// Java: public long timestamp() / clockSequence() / node() - the version-1 fields
uint64_t LIBMATTI_JU_UUID_Timestamp(const LIBMATTI_JU_UUID *uuid);
int LIBMATTI_JU_UUID_ClockSequence(const LIBMATTI_JU_UUID *uuid);
uint64_t LIBMATTI_JU_UUID_Node(const LIBMATTI_JU_UUID *uuid);

// Java: public String toString() - the canonical 8-4-4-4-12 lowercase form
// (buffer must hold at least 37 bytes)
void LIBMATTI_JU_UUID_ToString(const LIBMATTI_JU_UUID *uuid, char *out, size_t outSize);
// Java: public static UUID fromString(String) - accepts the dashed lowercase/
// uppercase form; returns false when the input is not a valid UUID string
bool LIBMATTI_JU_UUID_FromString(const char *input, LIBMATTI_JU_UUID *out);
// Java: public boolean equals(Object) - the two words compare
bool LIBMATTI_JU_UUID_Equals(const LIBMATTI_JU_UUID *a, const LIBMATTI_JU_UUID *b);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_JAVA_UTIL_UUID_H
