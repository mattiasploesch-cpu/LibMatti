// ASM constant pool (ClassReader/ClassWriter internal representation).

#include "libmatti/org/objectweb/asm/ConstantPool.h"

#include <stdlib.h>
#include <string.h>

// the JVM constant pool tags
#define CP_UTF8 1
#define CP_INTEGER 3
#define CP_FLOAT 4
#define CP_LONG 5
#define CP_DOUBLE 6
#define CP_CLASS 7
#define CP_STRING 8
#define CP_FIELDREF 9
#define CP_METHODREF 10
#define CP_INTERFACE_METHODREF 11
#define CP_NAME_AND_TYPE 12

LIBMATTI_ASM_ConstantPool *LIBMATTI_ASM_ConstantPool_New(void)
{
    LIBMATTI_ASM_ConstantPool *pool = calloc(1, sizeof(LIBMATTI_ASM_ConstantPool));
    // Java: index 0 is unused
    pool->count = 1;
    pool->entries = calloc(1, sizeof(LIBMATTI_ASM_ConstantPoolEntry));
    return pool;
}

LIBMATTI_ASM_ConstantPool *LIBMATTI_ASM_ConstantPool_Clone(const LIBMATTI_ASM_ConstantPool *pool)
{
    LIBMATTI_ASM_ConstantPool *clone = calloc(1, sizeof(LIBMATTI_ASM_ConstantPool));
    clone->count = pool->count;
    clone->entries = calloc(clone->count, sizeof(LIBMATTI_ASM_ConstantPoolEntry));

    for (size_t i = 0; i < pool->count; i++)
    {
        clone->entries[i] = pool->entries[i];
        if (pool->entries[i].utf8 != NULL)
        {
            clone->entries[i].utf8 = malloc(pool->entries[i].utf8Length + 1);
            memcpy(clone->entries[i].utf8, pool->entries[i].utf8, pool->entries[i].utf8Length + 1);
        }
    }

    return clone;
}

void LIBMATTI_ASM_ConstantPool_Free(LIBMATTI_ASM_ConstantPool *pool)
{
    if (pool == NULL) return;

    for (size_t i = 0; i < pool->count; i++) free(pool->entries[i].utf8);
    free(pool->entries);
    free(pool);
}

const char *LIBMATTI_ASM_ConstantPool_Utf8(const LIBMATTI_ASM_ConstantPool *pool, int index)
{
    if (index <= 0 || (size_t)index >= pool->count) return NULL;
    return pool->entries[index].utf8;
}

const char *LIBMATTI_ASM_ConstantPool_ClassName(const LIBMATTI_ASM_ConstantPool *pool, int index)
{
    if (index <= 0 || (size_t)index >= pool->count) return NULL;
    if (pool->entries[index].tag != CP_CLASS) return NULL;
    return LIBMATTI_ASM_ConstantPool_Utf8(pool, pool->entries[index].index1);
}

static int pool_append(LIBMATTI_ASM_ConstantPool *pool, LIBMATTI_ASM_ConstantPoolEntry entry)
{
    pool->entries = realloc(pool->entries, sizeof(entry) * (pool->count + 2));
    pool->entries[pool->count] = entry;

    int index = (int)pool->count;
    pool->count++;

    // Java: a Long/Double constant occupies two pool slots
    if (entry.tag == CP_LONG || entry.tag == CP_DOUBLE)
    {
        pool->entries[pool->count] = (LIBMATTI_ASM_ConstantPoolEntry){0};
        pool->count++;
    }

    return index;
}

int LIBMATTI_ASM_ConstantPool_NewUtf8(LIBMATTI_ASM_ConstantPool *pool, const char *value)
{
    size_t length = strlen(value);

    for (size_t i = 1; i < pool->count; i++)
    {
        if (pool->entries[i].tag != CP_UTF8) continue;
        if (pool->entries[i].utf8Length != length) continue;
        if (memcmp(pool->entries[i].utf8, value, length) != 0) continue;
        return (int)i;
    }

    LIBMATTI_ASM_ConstantPoolEntry entry = {0};
    entry.tag = CP_UTF8;
    entry.utf8Length = length;
    entry.utf8 = malloc(length + 1);
    memcpy(entry.utf8, value, length + 1);

    return pool_append(pool, entry);
}

int LIBMATTI_ASM_ConstantPool_NewClass(LIBMATTI_ASM_ConstantPool *pool, const char *internalName)
{
    int nameIndex = LIBMATTI_ASM_ConstantPool_NewUtf8(pool, internalName);

    for (size_t i = 1; i < pool->count; i++)
        if (pool->entries[i].tag == CP_CLASS && pool->entries[i].index1 == nameIndex) return (int)i;

    LIBMATTI_ASM_ConstantPoolEntry entry = {0};
    entry.tag = CP_CLASS;
    entry.index1 = nameIndex;

    return pool_append(pool, entry);
}

int LIBMATTI_ASM_ConstantPool_NewString(LIBMATTI_ASM_ConstantPool *pool, const char *value)
{
    int utf8Index = LIBMATTI_ASM_ConstantPool_NewUtf8(pool, value);

    for (size_t i = 1; i < pool->count; i++)
        if (pool->entries[i].tag == CP_STRING && pool->entries[i].index1 == utf8Index) return (int)i;

    LIBMATTI_ASM_ConstantPoolEntry entry = {0};
    entry.tag = CP_STRING;
    entry.index1 = utf8Index;

    return pool_append(pool, entry);
}

int LIBMATTI_ASM_ConstantPool_NewNameAndType(LIBMATTI_ASM_ConstantPool *pool, const char *name, const char *descriptor)
{
    int nameIndex = LIBMATTI_ASM_ConstantPool_NewUtf8(pool, name);
    int descriptorIndex = LIBMATTI_ASM_ConstantPool_NewUtf8(pool, descriptor);

    for (size_t i = 1; i < pool->count; i++)
        if (pool->entries[i].tag == CP_NAME_AND_TYPE && pool->entries[i].index1 == nameIndex &&
            pool->entries[i].index2 == descriptorIndex)
            return (int)i;

    LIBMATTI_ASM_ConstantPoolEntry entry = {0};
    entry.tag = CP_NAME_AND_TYPE;
    entry.index1 = nameIndex;
    entry.index2 = descriptorIndex;

    return pool_append(pool, entry);
}

static int new_member_ref(LIBMATTI_ASM_ConstantPool *pool, int tag, const char *owner, const char *name,
                          const char *descriptor)
{
    int ownerIndex = LIBMATTI_ASM_ConstantPool_NewClass(pool, owner);
    int nameAndTypeIndex = LIBMATTI_ASM_ConstantPool_NewNameAndType(pool, name, descriptor);

    for (size_t i = 1; i < pool->count; i++)
        if (pool->entries[i].tag == tag && pool->entries[i].index1 == ownerIndex &&
            pool->entries[i].index2 == nameAndTypeIndex)
            return (int)i;

    LIBMATTI_ASM_ConstantPoolEntry entry = {0};
    entry.tag = tag;
    entry.index1 = ownerIndex;
    entry.index2 = nameAndTypeIndex;

    return pool_append(pool, entry);
}

int LIBMATTI_ASM_ConstantPool_NewFieldref(LIBMATTI_ASM_ConstantPool *pool, const char *owner, const char *name,
                                          const char *descriptor)
{
    return new_member_ref(pool, CP_FIELDREF, owner, name, descriptor);
}

int LIBMATTI_ASM_ConstantPool_NewMethodref(LIBMATTI_ASM_ConstantPool *pool, const char *owner, const char *name,
                                           const char *descriptor, int itf)
{
    return new_member_ref(pool, itf ? CP_INTERFACE_METHODREF : CP_METHODREF, owner, name, descriptor);
}

int LIBMATTI_ASM_ConstantPool_NewInteger(LIBMATTI_ASM_ConstantPool *pool, int value)
{
    long long bits = (unsigned int)value;

    for (size_t i = 1; i < pool->count; i++)
        if (pool->entries[i].tag == CP_INTEGER && pool->entries[i].bits == bits) return (int)i;

    LIBMATTI_ASM_ConstantPoolEntry entry = {0};
    entry.tag = CP_INTEGER;
    entry.bits = bits;

    return pool_append(pool, entry);
}

int LIBMATTI_ASM_ConstantPool_NewFloat(LIBMATTI_ASM_ConstantPool *pool, float value)
{
    int bits = 0;
    memcpy(&bits, &value, sizeof(bits));
    long long bits64 = (unsigned int)bits;

    for (size_t i = 1; i < pool->count; i++)
        if (pool->entries[i].tag == CP_FLOAT && pool->entries[i].bits == bits64) return (int)i;

    LIBMATTI_ASM_ConstantPoolEntry entry = {0};
    entry.tag = CP_FLOAT;
    entry.bits = bits64;

    return pool_append(pool, entry);
}

int LIBMATTI_ASM_ConstantPool_NewLong(LIBMATTI_ASM_ConstantPool *pool, long long value)
{
    for (size_t i = 1; i < pool->count; i++)
        if (pool->entries[i].tag == CP_LONG && pool->entries[i].bits == value) return (int)i;

    LIBMATTI_ASM_ConstantPoolEntry entry = {0};
    entry.tag = CP_LONG;
    entry.bits = value;

    return pool_append(pool, entry);
}

int LIBMATTI_ASM_ConstantPool_NewDouble(LIBMATTI_ASM_ConstantPool *pool, double value)
{
    long long bits = 0;
    memcpy(&bits, &value, sizeof(bits));

    for (size_t i = 1; i < pool->count; i++)
        if (pool->entries[i].tag == CP_DOUBLE && pool->entries[i].bits == bits) return (int)i;

    LIBMATTI_ASM_ConstantPoolEntry entry = {0};
    entry.tag = CP_DOUBLE;
    entry.bits = bits;

    return pool_append(pool, entry);
}
