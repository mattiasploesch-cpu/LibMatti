// Port of java.util.BitSet (implementation).

#include "libmatti/java/util/BitSet.h"

#include <stdlib.h>
#include <string.h>

#define WORDS_PER_BITS(nbits) (((nbits) + 63) / 64)

static void trim_words(LIBMATTI_JU_BitSet *set)
{
    // Java: wordsInUse = findHighestUsedWord + 1 - trailing zero words drop out
    while (set->wordsInUse > 0 && set->words[set->wordsInUse - 1] == 0)
        set->wordsInUse--;
}

static bool ensure_words(LIBMATTI_JU_BitSet *set, size_t wordsRequired)
{
    if (wordsRequired <= set->wordCount)
        return true;
    size_t newCount = set->wordCount > 0 ? set->wordCount : 1;
    while (newCount < wordsRequired)
        newCount *= 2;
    uint64_t *grown = realloc(set->words, newCount * sizeof(uint64_t));
    if (grown == NULL)
        return false;
    memset(grown + set->wordCount, 0, (newCount - set->wordCount) * sizeof(uint64_t));
    set->words = grown;
    set->wordCount = newCount;
    return true;
}

LIBMATTI_JU_BitSet *LIBMATTI_JU_BitSet_New(void)
{
    LIBMATTI_JU_BitSet *set = calloc(1, sizeof(LIBMATTI_JU_BitSet));
    return set;
}

LIBMATTI_JU_BitSet *LIBMATTI_JU_BitSet_NewWithSize(size_t nbits)
{
    LIBMATTI_JU_BitSet *set = LIBMATTI_JU_BitSet_New();
    if (set != NULL)
        ensure_words(set, WORDS_PER_BITS(nbits));
    return set;
}

LIBMATTI_JU_BitSet *LIBMATTI_JU_BitSet_ValueOf(const uint64_t *longs, size_t count)
{
    LIBMATTI_JU_BitSet *set = LIBMATTI_JU_BitSet_New();
    if (set == NULL)
        return NULL;
    // Java: valueOf trims trailing zero words on the way in
    while (count > 0 && longs[count - 1] == 0)
        count--;
    if (count > 0 && ensure_words(set, count))
    {
        memcpy(set->words, longs, count * sizeof(uint64_t));
        set->wordsInUse = count;
    }
    return set;
}

void LIBMATTI_JU_BitSet_Free(LIBMATTI_JU_BitSet *set)
{
    if (set == NULL)
        return;
    free(set->words);
    free(set);
}

void LIBMATTI_JU_BitSet_Set(LIBMATTI_JU_BitSet *set, size_t bitIndex)
{
    LIBMATTI_JU_BitSet_SetValue(set, bitIndex, true);
}

void LIBMATTI_JU_BitSet_SetValue(LIBMATTI_JU_BitSet *set, size_t bitIndex, bool value)
{
    size_t word = bitIndex / 64;
    if (!ensure_words(set, word + 1))
        return;
    uint64_t mask = 1ULL << (bitIndex % 64);
    if (value)
    {
        set->words[word] |= mask;
        if (word + 1 > set->wordsInUse)
            set->wordsInUse = word + 1;
    }
    else
    {
        set->words[word] &= ~mask;
        trim_words(set);
    }
}

void LIBMATTI_JU_BitSet_Clear(LIBMATTI_JU_BitSet *set, size_t bitIndex)
{
    LIBMATTI_JU_BitSet_SetValue(set, bitIndex, false);
}

void LIBMATTI_JU_BitSet_ClearAll(LIBMATTI_JU_BitSet *set)
{
    // Java: clear() zeroes words[0..wordsInUse) and resets wordsInUse
    if (set->wordsInUse > 0)
        memset(set->words, 0, set->wordsInUse * sizeof(uint64_t));
    set->wordsInUse = 0;
}

void LIBMATTI_JU_BitSet_Flip(LIBMATTI_JU_BitSet *set, size_t bitIndex)
{
    size_t word = bitIndex / 64;
    if (!ensure_words(set, word + 1))
        return;
    set->words[word] ^= 1ULL << (bitIndex % 64);
    if (word + 1 > set->wordsInUse)
        set->wordsInUse = word + 1;
    trim_words(set);
}

bool LIBMATTI_JU_BitSet_Get(const LIBMATTI_JU_BitSet *set, size_t bitIndex)
{
    size_t word = bitIndex / 64;
    if (word >= set->wordsInUse)
        return false;
    return (set->words[word] & (1ULL << (bitIndex % 64))) != 0;
}

ptrdiff_t LIBMATTI_JU_BitSet_NextSetBit(const LIBMATTI_JU_BitSet *set, size_t fromIndex)
{
    if (set == NULL || set->wordsInUse == 0)
        return -1;
    size_t word = fromIndex / 64;
    if (word >= set->wordsInUse)
        return -1;
    uint64_t w = set->words[word] & (~0ULL << (fromIndex % 64));
    for (;;)
    {
        if (w != 0)
            return (ptrdiff_t) (word * 64 + (size_t) __builtin_ctzll(w));
        word++;
        if (word >= set->wordsInUse)
            return -1;
        w = set->words[word];
    }
}

size_t LIBMATTI_JU_BitSet_NextClearBit(const LIBMATTI_JU_BitSet *set, size_t fromIndex)
{
    if (set == NULL)
        return fromIndex;
    size_t word = fromIndex / 64;
    for (;;)
    {
        uint64_t w = (word < set->wordsInUse ? set->words[word] : 0) & (~0ULL << (fromIndex % 64));
        if (w != ~0ULL)
            return word * 64 + (size_t) __builtin_ctzll(~w);
        word++;
        fromIndex = word * 64;
    }
}

size_t LIBMATTI_JU_BitSet_Cardinality(const LIBMATTI_JU_BitSet *set)
{
    if (set == NULL)
        return 0;
    size_t count = 0;
    for (size_t i = 0; i < set->wordsInUse; i++)
        count += (size_t) __builtin_popcountll(set->words[i]);
    return count;
}

bool LIBMATTI_JU_BitSet_IsEmpty(const LIBMATTI_JU_BitSet *set)
{
    return set == NULL || set->wordsInUse == 0;
}

size_t LIBMATTI_JU_BitSet_Length(const LIBMATTI_JU_BitSet *set)
{
    if (set == NULL || set->wordsInUse == 0)
        return 0;
    // Java: 64*(wordsInUse-1) + (64 - numberOfLeadingZeros(words[wordsInUse-1]))
    uint64_t top = set->words[set->wordsInUse - 1];
    return (set->wordsInUse - 1) * 64 + 64 - (size_t) __builtin_clzll(top);
}

size_t LIBMATTI_JU_BitSet_Size(const LIBMATTI_JU_BitSet *set)
{
    return set != NULL ? set->wordCount * 64 : 0;
}

void LIBMATTI_JU_BitSet_And(LIBMATTI_JU_BitSet *set, const LIBMATTI_JU_BitSet *other)
{
    if (set == NULL)
        return;
    if (other == NULL || other->wordsInUse == 0)
    {
        LIBMATTI_JU_BitSet_ClearAll(set);
        return;
    }
    size_t n = set->wordsInUse < other->wordsInUse ? set->wordsInUse : other->wordsInUse;
    for (size_t i = 0; i < n; i++)
        set->words[i] &= other->words[i];
    for (size_t i = n; i < set->wordsInUse; i++)
        set->words[i] = 0;
    trim_words(set);
}

void LIBMATTI_JU_BitSet_Or(LIBMATTI_JU_BitSet *set, const LIBMATTI_JU_BitSet *other)
{
    if (set == NULL || other == NULL || other->wordsInUse == 0)
        return;
    if (!ensure_words(set, other->wordsInUse))
        return;
    for (size_t i = 0; i < other->wordsInUse; i++)
        set->words[i] |= other->words[i];
    if (other->wordsInUse > set->wordsInUse)
        set->wordsInUse = other->wordsInUse;
}

void LIBMATTI_JU_BitSet_Xor(LIBMATTI_JU_BitSet *set, const LIBMATTI_JU_BitSet *other)
{
    if (set == NULL || other == NULL || other->wordsInUse == 0)
        return;
    if (!ensure_words(set, other->wordsInUse))
        return;
    for (size_t i = 0; i < other->wordsInUse; i++)
        set->words[i] ^= other->words[i];
    if (other->wordsInUse > set->wordsInUse)
        set->wordsInUse = other->wordsInUse;
    trim_words(set);
}

void LIBMATTI_JU_BitSet_AndNot(LIBMATTI_JU_BitSet *set, const LIBMATTI_JU_BitSet *other)
{
    if (set == NULL || other == NULL || other->wordsInUse == 0 || set->wordsInUse == 0)
        return;
    size_t n = set->wordsInUse < other->wordsInUse ? set->wordsInUse : other->wordsInUse;
    for (size_t i = 0; i < n; i++)
        set->words[i] &= ~other->words[i];
    trim_words(set);
}

bool LIBMATTI_JU_BitSet_Intersects(const LIBMATTI_JU_BitSet *a, const LIBMATTI_JU_BitSet *b)
{
    if (a == NULL || b == NULL)
        return false;
    size_t n = a->wordsInUse < b->wordsInUse ? a->wordsInUse : b->wordsInUse;
    for (size_t i = 0; i < n; i++)
    {
        if ((a->words[i] & b->words[i]) != 0)
            return true;
    }
    return false;
}

uint64_t *LIBMATTI_JU_BitSet_ToLongArray(const LIBMATTI_JU_BitSet *set, size_t *outCount)
{
    if (outCount != NULL)
        *outCount = 0;
    if (set == NULL)
        return NULL;
    if (set->wordsInUse == 0)
        return calloc(1, sizeof(uint64_t)); // Java: empty long[] when nothing is set
    uint64_t *copy = malloc(set->wordsInUse * sizeof(uint64_t));
    if (copy != NULL)
    {
        memcpy(copy, set->words, set->wordsInUse * sizeof(uint64_t));
        if (outCount != NULL)
            *outCount = set->wordsInUse;
    }
    return copy;
}

bool LIBMATTI_JU_BitSet_Equals(const LIBMATTI_JU_BitSet *a, const LIBMATTI_JU_BitSet *b)
{
    if (a == b)
        return true;
    if (a == NULL || b == NULL)
        return false;
    if (a->wordsInUse != b->wordsInUse)
        return false;
    return memcmp(a->words, b->words, a->wordsInUse * sizeof(uint64_t)) == 0;
}

uint32_t LIBMATTI_JU_BitSet_HashCode(const LIBMATTI_JU_BitSet *set)
{
    // Java: hashCode xor-folds each word rotated by 64*wordIndex % 32
    if (set == NULL)
        return 0;
    uint32_t h = 1234;
    for (size_t i = 0; i < set->wordsInUse; i++)
    {
        uint64_t w = set->words[i];
        if (w == 0)
            continue;
        h ^= (uint32_t) ((w & 0xFFFFFFFFULL) ^ (w >> 32)) * (uint32_t) (i % 2 == 0 ? 1 : 0x9E3779B9ULL);
    }
    return h;
}
