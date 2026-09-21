// Port of org.objectweb.asm.ClassReader.
// Java accepts into a ClassVisitor; ModLauncher (and therefore this port) only
// ever accepts into a ClassNode, so the C port accepts a ClassNode directly.

#ifndef MATTICRAFT_ASM_CLASSREADER_H
#define MATTICRAFT_ASM_CLASSREADER_H

#include "libmatti/org/objectweb/asm/tree/ClassNode.h"

#include <stddef.h>

// Java: public class ClassReader
typedef struct LIBMATTI_ASM_ClassReader
{
    const unsigned char *data;
    size_t length;
    int flags;
} LIBMATTI_ASM_ClassReader;

// Java: public ClassReader(byte[] classFile)
LIBMATTI_ASM_ClassReader *LIBMATTI_ASM_ClassReader_New(const unsigned char *data, size_t length, int flags);
void LIBMATTI_ASM_ClassReader_Free(LIBMATTI_ASM_ClassReader *reader);

// Java: public void accept(ClassVisitor classVisitor, int parsingOptions)
void LIBMATTI_ASM_ClassReader_Accept(LIBMATTI_ASM_ClassReader *reader, LIBMATTI_ASMT_ClassNode *node);

#endif //MATTICRAFT_ASM_CLASSREADER_H
