// Port of net.minecraft.world.SimpleContainer + ContainerHelper over the
// ItemStack port. The container OWNS its stacks (Java: the NonNullList holds
// the references; the port frees what it stores, callers copy when they keep).

#include "libmatti/net/minecraft/world/Container.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_MC_Container *LIBMATTI_MC_Container_New(int size)
{
    if (size <= 0)
        return NULL;
    LIBMATTI_MC_Container *container = calloc(1, sizeof(LIBMATTI_MC_Container));
    if (container == NULL)
        return NULL;
    container->size = size;
    container->items = calloc((size_t) size, sizeof(LIBMATTI_MC_ItemStack *));
    if (container->items == NULL)
    {
        free(container);
        return NULL;
    }
    // Java: NonNullList.withSize(size, ItemStack.EMPTY) - the port leaves the
    // slots NULL (IsEmpty treats NULL as empty) so the EMPTY singleton stays
    // shared and the ownership stays unambiguous.
    return container;
}

void LIBMATTI_MC_Container_Free(LIBMATTI_MC_Container *container)
{
    if (container == NULL)
        return;
    if (container->items != NULL)
    {
        for (int i = 0; i < container->size; i++)
            LIBMATTI_MC_ItemStack_Free(container->items[i]);
        free(container->items);
    }
    free(container);
}

LIBMATTI_MC_ItemStack *LIBMATTI_MC_Container_GetItem(const LIBMATTI_MC_Container *container, int index)
{
    if (container == NULL || container->items == NULL || index < 0 || index >= container->size)
        return LIBMATTI_MC_ItemStack_Empty();
    LIBMATTI_MC_ItemStack *stack = container->items[index];
    return stack != NULL ? stack : LIBMATTI_MC_ItemStack_Empty();
}

// Java: ContainerHelper.removeItem - p_18971_.get(p_18971_).split(p_18972_)
LIBMATTI_MC_ItemStack *LIBMATTI_MC_Container_RemoveItem(LIBMATTI_MC_Container *container, int index, int amount)
{
    if (container == NULL || index < 0 || index >= container->size)
        return LIBMATTI_MC_ItemStack_Empty();
    LIBMATTI_MC_ItemStack *stack = LIBMATTI_MC_ContainerHelper_RemoveItem(container->items, container->size, index, amount);
    if (stack != NULL && !LIBMATTI_MC_ItemStack_IsEmpty(stack))
        LIBMATTI_MC_Container_SetChanged(container);
    return stack;
}

LIBMATTI_MC_ItemStack *LIBMATTI_MC_Container_RemoveItemNoUpdate(LIBMATTI_MC_Container *container, int index)
{
    if (container == NULL || container->items == NULL || index < 0 || index >= container->size)
        return LIBMATTI_MC_ItemStack_Empty();
    LIBMATTI_MC_ItemStack *stack = container->items[index];
    if (stack == NULL || LIBMATTI_MC_ItemStack_IsEmpty(stack))
        return LIBMATTI_MC_ItemStack_Empty();
    container->items[index] = NULL;
    return stack;
}

void LIBMATTI_MC_Container_SetItem(LIBMATTI_MC_Container *container, int index, LIBMATTI_MC_ItemStack *stack)
{
    if (container == NULL || container->items == NULL || index < 0 || index >= container->size)
        return;
    // Java: items.set(index, stack); stack.limitSize(getMaxStackSize(stack))
    if (stack != NULL && !LIBMATTI_MC_ItemStack_IsEmpty(stack))
    {
        int max = LIBMATTI_MC_Container_GetMaxStackSize(container) < LIBMATTI_MC_ItemStack_GetMaxStackSize(stack)
                      ? LIBMATTI_MC_Container_GetMaxStackSize(container)
                      : LIBMATTI_MC_ItemStack_GetMaxStackSize(stack);
        if (LIBMATTI_MC_ItemStack_GetCount(stack) > max)
            LIBMATTI_MC_ItemStack_SetCount(stack, max);
    }
    // the container owns the slot - the PREVIOUS stack dies here, but never
    // the one being stored (Java aliases both to the same reference)
    if (container->items[index] != NULL && container->items[index] != stack)
        LIBMATTI_MC_ItemStack_Free(container->items[index]);
    container->items[index] = stack;
    LIBMATTI_MC_Container_SetChanged(container);
}

int LIBMATTI_MC_Container_GetContainerSize(const LIBMATTI_MC_Container *container)
{
    return container != NULL ? container->size : 0;
}

bool LIBMATTI_MC_Container_IsEmpty(const LIBMATTI_MC_Container *container)
{
    if (container == NULL || container->items == NULL)
        return true;
    for (int i = 0; i < container->size; i++)
        if (!LIBMATTI_MC_ItemStack_IsEmpty(container->items[i]))
            return false;
    return true;
}

int LIBMATTI_MC_Container_GetMaxStackSize(const LIBMATTI_MC_Container *container)
{
    (void) container;
    return LIBMATTI_MC_Container_DEFAULT_MAX_STACK_SIZE;
}

void LIBMATTI_MC_Container_SetChanged(LIBMATTI_MC_Container *container)
{
    (void) container; // the listener ping lands with the menu wiring (P6.2)
}

bool LIBMATTI_MC_Container_StillValid(const LIBMATTI_MC_Container *container, const struct LIBMATTI_MC_Player *player)
{
    (void) container;
    (void) player;
    return true; // Java: SimpleContainer.stillValid
}

void LIBMATTI_MC_Container_ClearContent(LIBMATTI_MC_Container *container)
{
    if (container == NULL || container->items == NULL)
        return;
    for (int i = 0; i < container->size; i++)
    {
        LIBMATTI_MC_ItemStack_Free(container->items[i]);
        container->items[i] = NULL;
    }
}

// Java: moveItemsBetweenStacks - the same-item merge the addItem path rides
static void move_between_stacks(LIBMATTI_MC_Container *container, LIBMATTI_MC_ItemStack *from,
                                LIBMATTI_MC_ItemStack *into)
{
    int max = LIBMATTI_MC_ItemStack_GetMaxStackSize(into);
    int amount = LIBMATTI_MC_ItemStack_GetCount(from);
    if (max - LIBMATTI_MC_ItemStack_GetCount(into) < amount)
        amount = max - LIBMATTI_MC_ItemStack_GetCount(into);
    if (amount > 0)
    {
        LIBMATTI_MC_ItemStack_Grow(into, amount);
        LIBMATTI_MC_ItemStack_Shrink(from, amount);
        LIBMATTI_MC_Container_SetChanged(container);
    }
}

LIBMATTI_MC_ItemStack *LIBMATTI_MC_Container_AddItem(LIBMATTI_MC_Container *container, LIBMATTI_MC_ItemStack *stack)
{
    if (container == NULL || stack == NULL || LIBMATTI_MC_ItemStack_IsEmpty(stack))
        return LIBMATTI_MC_ItemStack_Empty();
    // Java: the copy rides the slots (the caller keeps its stack)
    LIBMATTI_MC_ItemStack *copy = LIBMATTI_MC_ItemStack_Copy(stack);

    // Java: moveItemToOccupiedSlotsWithSameType
    for (int i = 0; i < container->size && !LIBMATTI_MC_ItemStack_IsEmpty(copy); i++)
    {
        LIBMATTI_MC_ItemStack *slot = container->items[i];
        if (slot != NULL && LIBMATTI_MC_ItemStack_IsSameItemSameComponents(slot, copy))
            move_between_stacks(container, copy, slot);
    }
    // Java: moveItemToEmptySlots
    for (int i = 0; i < container->size; i++)
    {
        if (LIBMATTI_MC_ItemStack_IsEmpty(container->items[i]))
        {
            container->items[i] = copy;
            LIBMATTI_MC_Container_SetChanged(container);
            return LIBMATTI_MC_ItemStack_Empty(); // everything placed
        }
    }
    return copy; // the remainder
}

bool LIBMATTI_MC_Container_CanAddItem(const LIBMATTI_MC_Container *container, const LIBMATTI_MC_ItemStack *stack)
{
    if (container == NULL || stack == NULL || LIBMATTI_MC_ItemStack_IsEmpty(stack))
        return false;
    for (int i = 0; i < container->size; i++)
    {
        LIBMATTI_MC_ItemStack *slot = container->items[i];
        if (slot == NULL || LIBMATTI_MC_ItemStack_IsEmpty(slot))
            return true;
        if (LIBMATTI_MC_ItemStack_IsSameItemSameComponents(slot, stack)
            && LIBMATTI_MC_ItemStack_GetCount(slot) < LIBMATTI_MC_ItemStack_GetMaxStackSize(slot))
            return true;
    }
    return false;
}

LIBMATTI_MC_ItemStack *LIBMATTI_MC_Container_RemoveItemType(LIBMATTI_MC_Container *container,
                                                            const LIBMATTI_MC_Item *item, int amount)
{
    if (container == NULL || item == NULL || amount <= 0)
        return LIBMATTI_MC_ItemStack_Empty();
    // Java: new ItemStack(item, 0) - the accumulator the splits grow
    LIBMATTI_MC_ItemStack *remainder = LIBMATTI_MC_ItemStack_NewWithCount((LIBMATTI_MC_Item *) item, 0);
    if (remainder == NULL)
        return LIBMATTI_MC_ItemStack_Empty();
    for (int i = container->size - 1; i >= 0; i--)
    {
        LIBMATTI_MC_ItemStack *slot = container->items[i];
        if (slot != NULL && LIBMATTI_MC_ItemStack_Is(slot, item))
        {
            int stillNeeded = amount - LIBMATTI_MC_ItemStack_GetCount(remainder);
            LIBMATTI_MC_ItemStack *part = LIBMATTI_MC_ItemStack_Split(slot, stillNeeded);
            if (part != NULL && !LIBMATTI_MC_ItemStack_IsEmpty(part))
                LIBMATTI_MC_ItemStack_Grow(remainder, LIBMATTI_MC_ItemStack_GetCount(part));
            LIBMATTI_MC_ItemStack_Free(part);
            if (LIBMATTI_MC_ItemStack_GetCount(remainder) == amount)
                break;
        }
    }
    if (!LIBMATTI_MC_ItemStack_IsEmpty(remainder))
        LIBMATTI_MC_Container_SetChanged(container);
    return remainder;
}

void LIBMATTI_MC_Container_AddListener(LIBMATTI_MC_Container *container, void *listener)
{
    if (container != NULL)
        container->listener = listener;
}

void LIBMATTI_MC_Container_RemoveListener(LIBMATTI_MC_Container *container, void *listener)
{
    if (container != NULL && container->listener == listener)
        container->listener = NULL;
}

// ---------------------------------------------------------------------------
// ContainerHelper
// ---------------------------------------------------------------------------

LIBMATTI_MC_ItemStack *LIBMATTI_MC_ContainerHelper_RemoveItem(LIBMATTI_MC_ItemStack **items, int size,
                                                              int index, int amount)
{
    if (items == NULL || index < 0 || index >= size || amount <= 0)
        return LIBMATTI_MC_ItemStack_Empty();
    LIBMATTI_MC_ItemStack *stack = items[index];
    if (stack == NULL || LIBMATTI_MC_ItemStack_IsEmpty(stack))
        return LIBMATTI_MC_ItemStack_Empty();
    return LIBMATTI_MC_ItemStack_Split(stack, amount);
}

LIBMATTI_MC_ItemStack *LIBMATTI_MC_ContainerHelper_TakeItem(LIBMATTI_MC_ItemStack **items, int size, int index)
{
    if (items == NULL || index < 0 || index >= size)
        return LIBMATTI_MC_ItemStack_Empty();
    LIBMATTI_MC_ItemStack *stack = items[index];
    items[index] = NULL;
    return stack != NULL ? stack : LIBMATTI_MC_ItemStack_Empty();
}
