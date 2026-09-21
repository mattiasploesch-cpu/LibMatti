// Port of net.neoforged.fml.common.asm.ListGeneratorAdapter.

#include "libmatti/net/neoforged/fml/common/asm/ListGeneratorAdapter.h"

// Java: public ListGeneratorAdapter(InsnList insnList)
LIBMATTI_FML_ListGeneratorAdapter *LIBMATTI_FML_ListGeneratorAdapter_New(LIBMATTI_ASMT_InsnList *insnList)
{
    // Java: super(Opcodes.ASM9, null, 0, "", "()V"); this.insnList = insnList;
    return LIBMATTI_ASM_COMMONS_GeneratorAdapter_New(LIBMATTI_ASM_ASM9, 0, "", "()V", insnList);
}

void LIBMATTI_FML_ListGeneratorAdapter_Free(LIBMATTI_FML_ListGeneratorAdapter *adapter)
{
    LIBMATTI_ASM_COMMONS_GeneratorAdapter_Free(adapter);
}
