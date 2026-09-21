// Port of org.objectweb.asm.Label.

#ifndef MATTICRAFT_ASM_LABEL_H
#define MATTICRAFT_ASM_LABEL_H

// Java: public class Label
typedef struct LIBMATTI_ASM_Label
{
    // Java: int bytecodeOffset
    int offset;
    // Java: int position (index in the instruction list)
    int position;
    // Java: private boolean resolved
    int resolved;
} LIBMATTI_ASM_Label;

LIBMATTI_ASM_Label *LIBMATTI_ASM_Label_New(void);
void LIBMATTI_ASM_Label_Free(LIBMATTI_ASM_Label *label);

// Java: public int getOffset()
int LIBMATTI_ASM_Label_GetOffset(const LIBMATTI_ASM_Label *label);

#endif //MATTICRAFT_ASM_LABEL_H
