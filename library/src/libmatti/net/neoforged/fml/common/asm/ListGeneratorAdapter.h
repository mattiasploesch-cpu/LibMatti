// Port of net.neoforged.fml.common.asm.ListGeneratorAdapter.
// Java: ListGeneratorAdapter extends GeneratorAdapter. Its constructor calls
// super(Opcodes.ASM9, null, 0, "", "()V") and then points the visitor output at the passed InsnList
// (mv = new MethodNode() with instructions = insnList).
// The C GeneratorAdapter already writes into an InsnList, so the subclass only provides the
// constructor and keeps the inherited insnList field.

#ifndef MATTICRAFT_FML_COMMON_ASM_LISTGENERATORADAPTER_H
#define MATTICRAFT_FML_COMMON_ASM_LISTGENERATORADAPTER_H

#include "libmatti/org/objectweb/asm/commons/GeneratorAdapter.h"

// Java: public class ListGeneratorAdapter extends GeneratorAdapter
typedef LIBMATTI_ASM_COMMONS_GeneratorAdapter LIBMATTI_FML_ListGeneratorAdapter;

// Java: public ListGeneratorAdapter(InsnList insnList)
LIBMATTI_FML_ListGeneratorAdapter *LIBMATTI_FML_ListGeneratorAdapter_New(LIBMATTI_ASMT_InsnList *insnList);
void LIBMATTI_FML_ListGeneratorAdapter_Free(LIBMATTI_FML_ListGeneratorAdapter *adapter);

#endif //MATTICRAFT_FML_COMMON_ASM_LISTGENERATORADAPTER_H
