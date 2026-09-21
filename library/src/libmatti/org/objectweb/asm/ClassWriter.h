// Port of org.objectweb.asm.ClassWriter.
// Java's ClassWriter collects the class through the ClassVisitor callbacks of
// ClassNode.accept(writer). In the C port the writer serializes the ClassNode it
// was created for (ModLauncher always creates the writer from the very node it
// then accepts into it), which is why New() takes the node.

#ifndef MATTICRAFT_ASM_CLASSWRITER_H
#define MATTICRAFT_ASM_CLASSWRITER_H

#include "libmatti/org/objectweb/asm/tree/ClassNode.h"

#include <stddef.h>

typedef struct LIBMATTI_ASM_ClassWriter LIBMATTI_ASM_ClassWriter;

// Java: protected ClassWriter(int flags)
LIBMATTI_ASM_ClassWriter *LIBMATTI_ASM_ClassWriter_New(int flags, LIBMATTI_ASMT_ClassNode *clazz);
void LIBMATTI_ASM_ClassWriter_Free(LIBMATTI_ASM_ClassWriter *writer);

// Java: public void visit(...) - the C port records the node instead
void LIBMATTI_ASM_ClassWriter_Accept(LIBMATTI_ASM_ClassWriter *writer, LIBMATTI_ASMT_ClassNode *node);

// Java: public byte[] toByteArray()
unsigned char *LIBMATTI_ASM_ClassWriter_ToByteArray(LIBMATTI_ASM_ClassWriter *writer, size_t *length);

int LIBMATTI_ASM_ClassWriter_GetFlags(const LIBMATTI_ASM_ClassWriter *writer);
LIBMATTI_ASMT_ClassNode *LIBMATTI_ASM_ClassWriter_GetClass(const LIBMATTI_ASM_ClassWriter *writer);

// Java: protected String getCommonSuperClass(String type1, String type2) is provided by
// TransformerClassWriter (see TransformerClassWriter.h); the C ClassWriter does not
// recompute frames, so it never calls it.

#endif //MATTICRAFT_ASM_CLASSWRITER_H
