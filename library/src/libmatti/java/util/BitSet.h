// Port of java.util.BitSet. Java backs the set with a growable long[] words
// array (64 bits per word, little word order); the C port keeps the same
// layout so the save-format serialisation (states, sections) can mirror the
// Java word stream.

#ifndef MATTICRAFT_JAVA_UTIL_BITSET_H
#define MATTICRAFT_JAVA_UTIL_BITSET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class BitSet implements Cloneable, java.io.Serializable
typedef struct LIBMATTI_JU_BitSet
{
    // Java: private static final int BITS_PER_WORD = 64; the words array with
    // the trailing zero words trimmed (wordsInUse bookkeeping)
    uint64_t *words;
    size_t wordCount;    // allocated words
    size_t wordsInUse;   // Java: wordsInUse - the index of the highest used word + 1
} LIBMATTI_JU_BitSet;

// Java: public BitSet() / BitSet(int nbits)
LIBMATTI_JU_BitSet *LIBMATTI_JU_BitSet_New(void);
LIBMATTI_JU_BitSet *LIBMATTI_JU_BitSet_NewWithSize(size_t nbits);
// Java: public static BitSet valueOf(long[] longs) - the word array becomes the set
LIBMATTI_JU_BitSet *LIBMATTI_JU_BitSet_ValueOf(const uint64_t *longs, size_t count);
void LIBMATTI_JU_BitSet_Free(LIBMATTI_JU_BitSet *set);

// Java: public void set(int bitIndex) / set(int, boolean)
void LIBMATTI_JU_BitSet_Set(LIBMATTI_JU_BitSet *set, size_t bitIndex);
void LIBMATTI_JU_BitSet_SetValue(LIBMATTI_JU_BitSet *set, size_t bitIndex, bool value);
// Java: public void clear(int bitIndex) / public void clear()
void LIBMATTI_JU_BitSet_Clear(LIBMATTI_JU_BitSet *set, size_t bitIndex);
void LIBMATTI_JU_BitSet_ClearAll(LIBMATTI_JU_BitSet *set);
// Java: public void flip(int bitIndex)
void LIBMATTI_JU_BitSet_Flip(LIBMATTI_JU_BitSet *set, size_t bitIndex);
// Java: public boolean get(int bitIndex)
bool LIBMATTI_JU_BitSet_Get(const LIBMATTI_JU_BitSet *set, size_t bitIndex);

// Java: public int nextSetBit(int fromIndex) - -1 when none
ptrdiff_t LIBMATTI_JU_BitSet_NextSetBit(const LIBMATTI_JU_BitSet *set, size_t fromIndex);
// Java: public int nextClearBit(int fromIndex)
size_t LIBMATTI_JU_BitSet_NextClearBit(const LIBMATTI_JU_BitSet *set, size_t fromIndex);
// Java: public int cardinality() - the popcount over the used words
size_t LIBMATTI_JU_BitSet_Cardinality(const LIBMATTI_JU_BitSet *set);
// Java: public boolean isEmpty()
bool LIBMATTI_JU_BitSet_IsEmpty(const LIBMATTI_JU_BitSet *set);
// Java: public int length() - the highest set bit + 1 (0 when empty)
size_t LIBMATTI_JU_BitSet_Length(const LIBMATTI_JU_BitSet *set);
// Java: public int size() - the words array capacity in bits
size_t LIBMATTI_JU_BitSet_Size(const LIBMATTI_JU_BitSet *set);

// Java: public void and(BitSet) / or(BitSet) / xor(BitSet) / andNot(BitSet)
void LIBMATTI_JU_BitSet_And(LIBMATTI_JU_BitSet *set, const LIBMATTI_JU_BitSet *other);
void LIBMATTI_JU_BitSet_Or(LIBMATTI_JU_BitSet *set, const LIBMATTI_JU_BitSet *other);
void LIBMATTI_JU_BitSet_Xor(LIBMATTI_JU_BitSet *set, const LIBMATTI_JU_BitSet *other);
void LIBMATTI_JU_BitSet_AndNot(LIBMATTI_JU_BitSet *set, const LIBMATTI_JU_BitSet *other);
// Java: public boolean intersects(BitSet)
bool LIBMATTI_JU_BitSet_Intersects(const LIBMATTI_JU_BitSet *a, const LIBMATTI_JU_BitSet *b);

// Java: public long[] toLongArray() - the used words (caller frees)
uint64_t *LIBMATTI_JU_BitSet_ToLongArray(const LIBMATTI_JU_BitSet *set, size_t *outCount);
// Java: public boolean equals(Object) / public int hashCode()
bool LIBMATTI_JU_BitSet_Equals(const LIBMATTI_JU_BitSet *a, const LIBMATTI_JU_BitSet *b);
uint32_t LIBMATTI_JU_BitSet_HashCode(const LIBMATTI_JU_BitSet *set);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_JAVA_UTIL_BITSET_H
