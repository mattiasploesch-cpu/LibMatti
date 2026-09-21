// The org.objectweb.asm.ClassReader/ClassWriter constant pool representation.
// ASM keeps this internal; the C port exposes it so that ClassNode can carry the
// pool it was parsed from and ClassWriter can re-use the pool indices (which
// keeps raw, unparsed attributes valid).

#ifndef MATTICRAFT_ASM_CONSTANTPOOL_H
#define MATTICRAFT_ASM_CONSTANTPOOL_H

#include <stddef.h>

typedef struct
{
    int tag;
    int index1;
    int index2;
    long long bits;
    char *utf8;
    size_t utf8Length;
} LIBMATTI_ASM_ConstantPoolEntry;

// Java: ClassReader/ClassWriter keep a ConstantPool; index 0 is unused (Java 1-based)
typedef struct
{
    LIBMATTI_ASM_ConstantPoolEntry *entries;
    size_t count;
} LIBMATTI_ASM_ConstantPool;

LIBMATTI_ASM_ConstantPool *LIBMATTI_ASM_ConstantPool_New(void);
LIBMATTI_ASM_ConstantPool *LIBMATTI_ASM_ConstantPool_Clone(const LIBMATTI_ASM_ConstantPool *pool);
void LIBMATTI_ASM_ConstantPool_Free(LIBMATTI_ASM_ConstantPool *pool);

// Java: ClassReader.readUTF8(index)
const char *LIBMATTI_ASM_ConstantPool_Utf8(const LIBMATTI_ASM_ConstantPool *pool, int index);
// Java: ClassReader.readClass(index) - the internal name
const char *LIBMATTI_ASM_ConstantPool_ClassName(const LIBMATTI_ASM_ConstantPool *pool, int index);

// Java: SymbolTable.addConstant...
int LIBMATTI_ASM_ConstantPool_NewUtf8(LIBMATTI_ASM_ConstantPool *pool, const char *value);
int LIBMATTI_ASM_ConstantPool_NewClass(LIBMATTI_ASM_ConstantPool *pool, const char *internalName);
int LIBMATTI_ASM_ConstantPool_NewString(LIBMATTI_ASM_ConstantPool *pool, const char *value);
int LIBMATTI_ASM_ConstantPool_NewNameAndType(LIBMATTI_ASM_ConstantPool *pool, const char *name, const char *descriptor);
int LIBMATTI_ASM_ConstantPool_NewFieldref(LIBMATTI_ASM_ConstantPool *pool, const char *owner, const char *name,
                                          const char *descriptor);
int LIBMATTI_ASM_ConstantPool_NewMethodref(LIBMATTI_ASM_ConstantPool *pool, const char *owner, const char *name,
                                           const char *descriptor, int itf);
int LIBMATTI_ASM_ConstantPool_NewInteger(LIBMATTI_ASM_ConstantPool *pool, int value);
int LIBMATTI_ASM_ConstantPool_NewFloat(LIBMATTI_ASM_ConstantPool *pool, float value);
int LIBMATTI_ASM_ConstantPool_NewLong(LIBMATTI_ASM_ConstantPool *pool, long long value);
int LIBMATTI_ASM_ConstantPool_NewDouble(LIBMATTI_ASM_ConstantPool *pool, double value);

#endif //MATTICRAFT_ASM_CONSTANTPOOL_H
