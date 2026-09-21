// Port of org.objectweb.asm.Label.

#include "libmatti/org/objectweb/asm/Label.h"

#include <stdlib.h>

LIBMATTI_ASM_Label *LIBMATTI_ASM_Label_New(void)
{
    return calloc(1, sizeof(LIBMATTI_ASM_Label));
}

void LIBMATTI_ASM_Label_Free(LIBMATTI_ASM_Label *label)
{
    free(label);
}

int LIBMATTI_ASM_Label_GetOffset(const LIBMATTI_ASM_Label *label)
{
    return label->offset;
}
