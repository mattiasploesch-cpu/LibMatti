// Port of org.objectweb.asm.tree.InsnList.

#ifndef MATTICRAFT_ASMT_INSNLIST_H
#define MATTICRAFT_ASMT_INSNLIST_H

#include "libmatti/org/objectweb/asm/tree/AbstractInsnNode.h"

#include <stddef.h>

// Java: public class InsnList
typedef struct LIBMATTI_ASMT_InsnList
{
    LIBMATTI_ASMT_AbstractInsnNode *first;
    LIBMATTI_ASMT_AbstractInsnNode *last;
    size_t size;
    // Java: InsnList has no such flag; it marks that the parsed code must be
    // re-serialized instead of reusing the original Code attribute bytes.
    int modified;
} LIBMATTI_ASMT_InsnList;

// Java: public int size()
size_t LIBMATTI_ASM_InsnList_Size(const LIBMATTI_ASMT_InsnList *list);
// Java: public AbstractInsnNode get(int index)
LIBMATTI_ASMT_AbstractInsnNode *LIBMATTI_ASM_InsnList_Get(const LIBMATTI_ASMT_InsnList *list, size_t index);
// Java: public void add(AbstractInsnNode insn)
void LIBMATTI_ASM_InsnList_Add(LIBMATTI_ASMT_InsnList *list, LIBMATTI_ASMT_AbstractInsnNode *node);
// Java: public void insert(AbstractInsnNode insn)
void LIBMATTI_ASM_InsnList_Insert(LIBMATTI_ASMT_InsnList *list, LIBMATTI_ASMT_AbstractInsnNode *node);
// Java: public void insertBefore(AbstractInsnNode location, AbstractInsnNode insn)
void LIBMATTI_ASM_InsnList_InsertBefore(LIBMATTI_ASMT_InsnList *list, LIBMATTI_ASMT_AbstractInsnNode *location,
                                        LIBMATTI_ASMT_AbstractInsnNode *node);
// Java: public void insertBefore(AbstractInsnNode location, InsnList insns)
void LIBMATTI_ASM_InsnList_InsertBeforeList(LIBMATTI_ASMT_InsnList *list, LIBMATTI_ASMT_AbstractInsnNode *location,
                                            LIBMATTI_ASMT_InsnList *insns);
// Java: public void remove(AbstractInsnNode insn)
void LIBMATTI_ASM_InsnList_Remove(LIBMATTI_ASMT_InsnList *list, LIBMATTI_ASMT_AbstractInsnNode *node);
// Java: public void clear()
void LIBMATTI_ASM_InsnList_Clear(LIBMATTI_ASMT_InsnList *list);
// Java: public AbstractInsnNode[] toArray()
LIBMATTI_ASMT_AbstractInsnNode **LIBMATTI_ASM_InsnList_ToArray(const LIBMATTI_ASMT_InsnList *list, size_t *count);

void LIBMATTI_ASM_InsnList_Free(LIBMATTI_ASMT_InsnList *list);

#endif //MATTICRAFT_ASMT_INSNLIST_H
