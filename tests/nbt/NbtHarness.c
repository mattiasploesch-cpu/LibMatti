// NBT harness: builds a compound covering every tag kind, checks the SNBT round trip
// (parse -> print -> parse), the binary round trip (write -> read) and the gzip round
// trip of NbtIo, and the ListTag wrapper/unwrap behaviour.

#include "libmatti/net/minecraft/nbt/CompoundTag.h"
#include "libmatti/net/minecraft/nbt/ListTag.h"
#include "libmatti/net/minecraft/nbt/NbtIo.h"
#include "libmatti/net/minecraft/nbt/NbtUtils.h"
#include "libmatti/net/minecraft/nbt/TagParser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void check(int condition, const char *what)
{
    printf("%s: %s\n", condition ? "ok  " : "FAIL", what);
    if (!condition)
        failures++;
}

// Java: NbtIo round trips a compound covering every tag id
static LIBMATTI_MC_Nbt_CompoundTag *build_sample(void)
{
    LIBMATTI_MC_Nbt_CompoundTag *compound = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_PutByte(compound, "byte", -5);
    LIBMATTI_MC_Nbt_CompoundTag_PutShort(compound, "short", 1234);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(compound, "int", -123456);
    LIBMATTI_MC_Nbt_CompoundTag_PutLong(compound, "long", 9876543210LL);
    LIBMATTI_MC_Nbt_CompoundTag_PutFloat(compound, "float", 1.5f);
    LIBMATTI_MC_Nbt_CompoundTag_PutDouble(compound, "double", 2.25);
    LIBMATTI_MC_Nbt_CompoundTag_PutString(compound, "string", "hello \"world\"");
    LIBMATTI_MC_Nbt_CompoundTag_PutBoolean(compound, "flag", 1);

    int8_t bytes[] = {1, -2, 3};
    LIBMATTI_MC_Nbt_CompoundTag_PutByteArray(compound, "bytes", bytes, 3);
    int32_t ints[] = {10, 20, 30};
    LIBMATTI_MC_Nbt_CompoundTag_PutIntArray(compound, "ints", ints, 3);
    int64_t longs[] = {100, -200};
    LIBMATTI_MC_Nbt_CompoundTag_PutLongArray(compound, "longs", longs, 2);

    LIBMATTI_MC_Nbt_ListTag *list = LIBMATTI_MC_Nbt_ListTag_New();
    LIBMATTI_MC_Nbt_ListTag_Add(list, LIBMATTI_MC_Nbt_IntTag_Of(7));
    LIBMATTI_MC_Nbt_ListTag_Add(list, LIBMATTI_MC_Nbt_IntTag_Of(8));
    LIBMATTI_MC_Nbt_CompoundTag_Put(compound, "list", (LIBMATTI_MC_Nbt_Tag *) list);

    LIBMATTI_MC_Nbt_CompoundTag *nested = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_PutString(nested, "name", "inner");
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(nested, "value", 42);
    LIBMATTI_MC_Nbt_CompoundTag_Put(compound, "nested", (LIBMATTI_MC_Nbt_Tag *) nested);
    return compound;
}

static void test_binary_round_trip(void)
{
    LIBMATTI_MC_Nbt_CompoundTag *sample = build_sample();

    uint8_t *data;
    size_t length;
    check(LIBMATTI_MC_Nbt_NbtIo_Write(sample, &data, &length), "NbtIo.write succeeds");

    LIBMATTI_MC_Nbt_CompoundTag *parsed = LIBMATTI_MC_Nbt_NbtIo_Read(data, length, NULL);
    check(parsed != NULL, "NbtIo.read succeeds");
    if (parsed != NULL)
    {
        check(LIBMATTI_MC_Nbt_CompoundTag_GetByteOr(parsed, "byte", 0) == -5, "byte round trip");
        check(LIBMATTI_MC_Nbt_CompoundTag_GetShortOr(parsed, "short", 0) == 1234, "short round trip");
        check(LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(parsed, "int", 0) == -123456, "int round trip");
        check(LIBMATTI_MC_Nbt_CompoundTag_GetLongOr(parsed, "long", 0) == 9876543210LL, "long round trip");
        check(LIBMATTI_MC_Nbt_CompoundTag_GetFloatOr(parsed, "float", 0) == 1.5f, "float round trip");
        check(LIBMATTI_MC_Nbt_CompoundTag_GetDoubleOr(parsed, "double", 0) == 2.25, "double round trip");
        check(strcmp(LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(parsed, "string", ""), "hello \"world\"") == 0,
              "string round trip");
        check(LIBMATTI_MC_Nbt_CompoundTag_GetBooleanOr(parsed, "flag", 0) == 1, "boolean round trip");

        const int32_t *intData;
        size_t intCount;
        check(LIBMATTI_MC_Nbt_CompoundTag_GetIntArray(parsed, "ints", &intData, &intCount) && intCount == 3 &&
                  intData[1] == 20,
              "int array round trip");
        const int64_t *longData;
        size_t longCount;
        check(LIBMATTI_MC_Nbt_CompoundTag_GetLongArray(parsed, "longs", &longData, &longCount) && longCount == 2 &&
                  longData[1] == -200,
              "long array round trip");

        LIBMATTI_MC_Nbt_ListTag *list;
        check(LIBMATTI_MC_Nbt_CompoundTag_GetList(parsed, "list", &list) &&
                  LIBMATTI_MC_Nbt_ListTag_GetIntOr(list, 1, 0) == 8,
              "list round trip (unwrapped)");

        LIBMATTI_MC_Nbt_CompoundTag *nested;
        check(LIBMATTI_MC_Nbt_CompoundTag_GetCompound(parsed, "nested", &nested) &&
                  strcmp(LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(nested, "name", ""), "inner") == 0,
              "nested compound round trip");
        LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(parsed);
        free(parsed);
    }
    free(data);
    LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(sample);
    free(sample);
}

static void test_gzip_round_trip(void)
{
    LIBMATTI_MC_Nbt_CompoundTag *sample = build_sample();
    uint8_t *data;
    size_t length;
    check(LIBMATTI_MC_Nbt_NbtIo_WriteCompressed(sample, &data, &length), "NbtIo.writeCompressed succeeds");
    check(length > 2 && data[0] == 0x1f && data[1] == 0x8b, "gzip magic bytes present");

    LIBMATTI_MC_Nbt_CompoundTag *parsed = LIBMATTI_MC_Nbt_NbtIo_ReadCompressed(data, length, NULL);
    check(parsed != NULL, "NbtIo.readCompressed succeeds");
    if (parsed != NULL)
    {
        check(LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(parsed, "int", 0) == -123456, "gzip int round trip");
        LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(parsed);
        free(parsed);
    }
    free(data);
    LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(sample);
    free(sample);

    // the file variants
    check(LIBMATTI_MC_Nbt_NbtIo_WriteCompressedFile(build_sample(), "nbt-test.dat"),
          "NbtIo.writeCompressedFile succeeds");
    LIBMATTI_MC_Nbt_CompoundTag *fromFile = LIBMATTI_MC_Nbt_NbtIo_ReadCompressedFile("nbt-test.dat", NULL);
    check(fromFile != NULL && LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(fromFile, "int", 0) == -123456,
          "file round trip");
    if (fromFile != NULL)
    {
        LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(fromFile);
        free(fromFile);
    }
    remove("nbt-test.dat");
}

static void test_snbt(void)
{
    // Java: TagParser.parseCompoundFully covers the SNBT surface
    LIBMATTI_MC_Nbt_CompoundTag *parsed = LIBMATTI_MC_Nbt_TagParser_ParseCompoundFully(
        "{byte: 1b, short: 2s, int: 3, long: 4L, float: 0.5f, double: 0.25d, str: \"hi\", flag: true, "
        "list: [1, 2, 3], bytes: [B; 1, 2], ints: [I; 3, 4], longs: [L; 5L], "
        "nested: {a: 1}, quoted-key: 9, hex: 0xFF, 'single': 'x'}");
    check(parsed != NULL, "SNBT parse succeeds");
    if (parsed == NULL)
    {
        printf("     error: %s\n", LIBMATTI_MC_Nbt_TagParser_LastError());
        return;
    }
    check(LIBMATTI_MC_Nbt_CompoundTag_GetByteOr(parsed, "byte", 0) == 1, "SNBT byte suffix");
    check(LIBMATTI_MC_Nbt_CompoundTag_GetShortOr(parsed, "short", 0) == 2, "SNBT short suffix");
    check(LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(parsed, "int", 0) == 3, "SNBT plain int");
    check(LIBMATTI_MC_Nbt_CompoundTag_GetLongOr(parsed, "long", 0) == 4, "SNBT long suffix");
    check(LIBMATTI_MC_Nbt_CompoundTag_GetFloatOr(parsed, "float", 0) == 0.5f, "SNBT float suffix");
    check(LIBMATTI_MC_Nbt_CompoundTag_GetDoubleOr(parsed, "double", 0) == 0.25, "SNBT double suffix");
    check(strcmp(LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(parsed, "str", ""), "hi") == 0, "SNBT quoted string");
    check(LIBMATTI_MC_Nbt_CompoundTag_GetBooleanOr(parsed, "flag", 0) == 1, "SNBT true builtin");
    check(LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(parsed, "hex", 0) == 255, "SNBT hex literal");
    check(strcmp(LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(parsed, "single", ""), "x") == 0,
          "SNBT single-quoted string");
    check(strcmp(LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(parsed, "quoted-key", ""), "") != 0 ||
              LIBMATTI_MC_Nbt_CompoundTag_Contains(parsed, "quoted-key"),
          "SNBT unquoted key with dash");

    LIBMATTI_MC_Nbt_ListTag *list;
    check(LIBMATTI_MC_Nbt_CompoundTag_GetList(parsed, "list", &list) && LIBMATTI_MC_Nbt_ListTag_Size(list) == 3,
          "SNBT list");
    const int8_t *byteData;
    size_t byteCount;
    check(LIBMATTI_MC_Nbt_CompoundTag_GetByteArray(parsed, "bytes", &byteData, &byteCount) && byteCount == 2 &&
              byteData[1] == 2,
          "SNBT byte array");
    LIBMATTI_MC_Nbt_CompoundTag *nested;
    check(LIBMATTI_MC_Nbt_CompoundTag_GetCompound(parsed, "nested", &nested) &&
              LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(nested, "a", 0) == 1,
          "SNBT nested compound");
    LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(parsed);
    free(parsed);

    // the underscore literals (Java: NumberRunParseRule allows single separators)
    LIBMATTI_MC_Nbt_CompoundTag *underscores = LIBMATTI_MC_Nbt_TagParser_ParseCompoundFully("{a: 1_0}");
    check(underscores != NULL, "SNBT underscore digits");
    if (underscores != NULL)
    {
        check(LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(underscores, "a", 0) == 10, "underscore 1_0 = 10");
        LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(underscores);
        free(underscores);
    }

    // Java: ERROR_UNDESCORE_NOT_ALLOWED - doubled or edge underscores are rejected
    check(LIBMATTI_MC_Nbt_TagParser_ParseCompoundFully("{a: 1__0}") == NULL, "doubled underscore rejected");

    // Java: ERROR_LEADING_ZERO_NOT_ALLOWED
    check(LIBMATTI_MC_Nbt_TagParser_ParseCompoundFully("{a: 007}") == NULL, "leading zero rejected");
    // Java: ERROR_TRAILING_DATA
    check(LIBMATTI_MC_Nbt_TagParser_ParseCompoundFully("{a: 1} x") == NULL, "trailing data rejected");

    // the print side: CompoundTag.toString sorts the keys
    LIBMATTI_MC_Nbt_CompoundTag *sample = build_sample();
    char *plain = LIBMATTI_MC_Nbt_StringTagVisitor_Visit((LIBMATTI_MC_Nbt_Tag *) sample);
    check(plain != NULL && strstr(plain, "nested:") != NULL && plain[0] == '{', "StringTagVisitor prints");
    check(strstr(plain, "9876543210L") != NULL, "long printed with L suffix");
    check(strstr(plain, "[B;") != NULL, "byte array printed with [B;");
    free(plain);

    char *pretty = LIBMATTI_MC_Nbt_TextComponentTagVisitor_Visit("    ", (LIBMATTI_MC_Nbt_Tag *) sample);
    check(pretty != NULL && strstr(pretty, "\n") != NULL, "pretty printer wraps");
    free(pretty);
    LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(sample);
    free(sample);

    // the SNBT round trip through the plain printer
    LIBMATTI_MC_Nbt_CompoundTag *original = build_sample();
    char *printed = LIBMATTI_MC_Nbt_StringTagVisitor_Visit((LIBMATTI_MC_Nbt_Tag *) original);
    LIBMATTI_MC_Nbt_CompoundTag *reparsed = LIBMATTI_MC_Nbt_TagParser_ParseCompoundFully(printed);
    check(reparsed != NULL, "SNBT print -> parse round trip");
    if (reparsed != NULL)
    {
        LIBMATTI_MC_Nbt_Tag *a = LIBMATTI_MC_Nbt_CompoundTag_Get(original, "nested");
        LIBMATTI_MC_Nbt_Tag *b = LIBMATTI_MC_Nbt_CompoundTag_Get(reparsed, "nested");
        check(LIBMATTI_MC_Nbt_Tag_Equals(a, b), "nested compound equal after round trip");
        LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(reparsed);
        free(reparsed);
    }
    free(printed);
    LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(original);
    free(original);
}

static void test_wrappers(void)
{
    // Java: a list of bytes stores wrapped compounds on the wire and unwraps on load
    LIBMATTI_MC_Nbt_ListTag *list = LIBMATTI_MC_Nbt_ListTag_New();
    LIBMATTI_MC_Nbt_ListTag_Add(list, LIBMATTI_MC_Nbt_StringTag_Of("a"));
    LIBMATTI_MC_Nbt_ListTag_Add(list, LIBMATTI_MC_Nbt_StringTag_Of("b"));

    LIBMATTI_MC_Nbt_CompoundTag *holder = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_Put(holder, "strings", (LIBMATTI_MC_Nbt_Tag *) list);

    uint8_t *data;
    size_t length;
    check(LIBMATTI_MC_Nbt_NbtIo_Write(holder, &data, &length), "wrapper write succeeds");

    LIBMATTI_MC_Nbt_CompoundTag *parsed = LIBMATTI_MC_Nbt_NbtIo_Read(data, length, NULL);
    check(parsed != NULL, "wrapper read succeeds");
    if (parsed != NULL)
    {
        LIBMATTI_MC_Nbt_ListTag *parsedList;
        check(LIBMATTI_MC_Nbt_CompoundTag_GetList(parsed, "strings", &parsedList) &&
                  LIBMATTI_MC_Nbt_ListTag_Size(parsedList) == 2 &&
                  strcmp(LIBMATTI_MC_Nbt_ListTag_GetStringOr(parsedList, 0, ""), "a") == 0,
              "string list unwraps on load");
        LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(parsed);
        free(parsed);
    }
    free(data);
    LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(holder);
    free(holder);

    // Java: ListTag.equals is order-sensitive, CompoundTag.equals is not
    LIBMATTI_MC_Nbt_CompoundTag *a = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag *b = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(a, "x", 1);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(a, "y", 2);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(b, "y", 2);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(b, "x", 1);
    check(LIBMATTI_MC_Nbt_Tag_Equals((LIBMATTI_MC_Nbt_Tag *) a, (LIBMATTI_MC_Nbt_Tag *) b),
          "compound equals ignores order");
    LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(a);
    free(a);
    LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(b);
    free(b);

    // Java: CompoundTag.merge
    LIBMATTI_MC_Nbt_CompoundTag *base = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag *baseNested = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(baseNested, "keep", 1);
    LIBMATTI_MC_Nbt_CompoundTag_Put(base, "nested", (LIBMATTI_MC_Nbt_Tag *) baseNested);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(base, "top", 5);

    LIBMATTI_MC_Nbt_CompoundTag *other = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag *otherNested = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(otherNested, "added", 2);
    LIBMATTI_MC_Nbt_CompoundTag_Put(other, "nested", (LIBMATTI_MC_Nbt_Tag *) otherNested);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(other, "top", 9);

    LIBMATTI_MC_Nbt_CompoundTag_Merge(base, other);
    LIBMATTI_MC_Nbt_CompoundTag *mergedNested;
    check(LIBMATTI_MC_Nbt_CompoundTag_GetCompound(base, "nested", &mergedNested) &&
              LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(mergedNested, "keep", 0) == 1 &&
              LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(mergedNested, "added", 0) == 2,
          "merge deepens compounds");
    check(LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(base, "top", 0) == 9, "merge replaces scalars");
    LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(base);
    free(base);
    LIBMATTI_MC_Nbt_CompoundTag_FreeEntries(other);
    free(other);
}

int main(void)
{
    test_binary_round_trip();
    test_gzip_round_trip();
    test_snbt();
    test_wrappers();
    printf("%s\n", failures == 0 ? "ALL OK" : "FAILURES");
    return failures == 0 ? 0 : 1;
}
