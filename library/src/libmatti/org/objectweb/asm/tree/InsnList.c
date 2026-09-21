// Port of org.objectweb.asm.tree.InsnList.

#include "libmatti/org/objectweb/asm/tree/InsnList.h"

#include "libmatti/org/objectweb/asm/tree/InsnNodes.h"

#include <stdlib.h>

size_t LIBMATTI_ASM_InsnList_Size(const LIBMATTI_ASMT_InsnList *list)
{
    return list->size;
}

LIBMATTI_ASMT_AbstractInsnNode *LIBMATTI_ASM_InsnList_Get(const LIBMATTI_ASMT_InsnList *list, size_t index)
{
    LIBMATTI_ASMT_AbstractInsnNode *node = list->first;
    for (size_t i = 0; node != NULL && i < index; i++) node = node->next;
    return node;
}

void LIBMATTI_ASM_InsnList_Add(LIBMATTI_ASMT_InsnList *list, LIBMATTI_ASMT_AbstractInsnNode *node)
{
    node->prev = list->last;
    node->next = NULL;

    if (list->last != NULL) list->last->next = node;
    else list->first = node;

    list->last = node;
    list->size++;
    list->modified = 1;
}

void LIBMATTI_ASM_InsnList_Insert(LIBMATTI_ASMT_InsnList *list, LIBMATTI_ASMT_AbstractInsnNode *node)
{
    node->prev = NULL;
    node->next = list->first;

    if (list->first != NULL) list->first->prev = node;
    else list->last = node;

    list->first = node;
    list->size++;
    list->modified = 1;
}

// Java: public void insertBefore(AbstractInsnNode location, AbstractInsnNode insn)
void LIBMATTI_ASM_InsnList_InsertBefore(LIBMATTI_ASMT_InsnList *list, LIBMATTI_ASMT_AbstractInsnNode *location,
                                        LIBMATTI_ASMT_AbstractInsnNode *node)
{
    // Java: insertBefore(location, insn.getPrevious()) - the node is appended before the location
    node->prev = location != NULL ? location->prev : list->last;
    node->next = location;

    if (node->prev != NULL) node->prev->next = node;
    else list->first = node;

    if (location != NULL) location->prev = node;
    else list->last = node;

    list->size++;
    list->modified = 1;
}

// Java: public void insertBefore(AbstractInsnNode location, InsnList insns)
void LIBMATTI_ASM_InsnList_InsertBeforeList(LIBMATTI_ASMT_InsnList *list, LIBMATTI_ASMT_AbstractInsnNode *location,
                                            LIBMATTI_ASMT_InsnList *insns)
{
    while (insns->first != NULL)
    {
        LIBMATTI_ASMT_AbstractInsnNode *node = insns->first;
        LIBMATTI_ASM_InsnList_Remove(insns, node);
        LIBMATTI_ASM_InsnList_InsertBefore(list, location, node);
    }
}

void LIBMATTI_ASM_InsnList_Remove(LIBMATTI_ASMT_InsnList *list, LIBMATTI_ASMT_AbstractInsnNode *node)
{
    if (node->prev != NULL) node->prev->next = node->next;
    else list->first = node->next;

    if (node->next != NULL) node->next->prev = node->prev;
    else list->last = node->prev;

    list->size--;
    list->modified = 1;
}

void LIBMATTI_ASM_InsnList_Clear(LIBMATTI_ASMT_InsnList *list)
{
    LIBMATTI_ASMT_AbstractInsnNode *node = list->first;
    while (node != NULL)
    {
        LIBMATTI_ASMT_AbstractInsnNode *next = node->next;
        LIBMATTI_ASMT_AbstractInsnNode_Free(node);
        node = next;
    }

    list->first = NULL;
    list->last = NULL;
    list->size = 0;
    list->modified = 1;
}

LIBMATTI_ASMT_AbstractInsnNode **LIBMATTI_ASM_InsnList_ToArray(const LIBMATTI_ASMT_InsnList *list, size_t *count)
{
    LIBMATTI_ASMT_AbstractInsnNode **array = malloc(sizeof(*array) * (list->size > 0 ? list->size : 1));

    size_t index = 0;
    for (LIBMATTI_ASMT_AbstractInsnNode *node = list->first; node != NULL; node = node->next) array[index++] = node;

    *count = index;
    return array;
}

void LIBMATTI_ASM_InsnList_Free(LIBMATTI_ASMT_InsnList *list)
{
    LIBMATTI_ASM_InsnList_Clear(list);
}
