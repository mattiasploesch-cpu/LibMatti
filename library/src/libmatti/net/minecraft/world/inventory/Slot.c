// Port of net.minecraft.world.inventory.Slot over the Container/ItemStack
// ports. The override hooks ride the struct (see Slot.h); every accessor
// follows the Java default when the hook is NULL.

#include "libmatti/net/minecraft/world/inventory/Slot.h"
#include "libmatti/net/minecraft/world/Container.h"

#include <stdlib.h>

LIBMATTI_MC_Slot *LIBMATTI_MC_Slot_New(struct LIBMATTI_MC_Container *container, int containerSlot, int x, int y)
{
    LIBMATTI_MC_Slot *slot = calloc(1, sizeof(LIBMATTI_MC_Slot));
    if (slot == NULL)
        return NULL;
    slot->container = container;
    slot->slot = containerSlot;
    slot->index = -1; // addSlot assigns the menu index (Java: p_38898_.index)
    slot->x = x;
    slot->y = y;
    return slot;
}

void LIBMATTI_MC_Slot_Free(LIBMATTI_MC_Slot *slot)
{
    free(slot);
}

LIBMATTI_MC_ItemStack *LIBMATTI_MC_Slot_GetItem(const LIBMATTI_MC_Slot *slot)
{
    if (slot == NULL || slot->container == NULL)
        return LIBMATTI_MC_ItemStack_Empty();
    return LIBMATTI_MC_Container_GetItem(slot->container, slot->slot);
}

bool LIBMATTI_MC_Slot_HasItem(const LIBMATTI_MC_Slot *slot)
{
    return !LIBMATTI_MC_ItemStack_IsEmpty(LIBMATTI_MC_Slot_GetItem(slot));
}

void LIBMATTI_MC_Slot_SetChanged(LIBMATTI_MC_Slot *slot)
{
    if (slot == NULL || slot->container == NULL)
        return;
    LIBMATTI_MC_Container_SetChanged(slot->container);
}

void LIBMATTI_MC_Slot_Set(LIBMATTI_MC_Slot *slot, LIBMATTI_MC_ItemStack *stack)
{
    if (slot == NULL || slot->container == NULL)
        return;
    LIBMATTI_MC_Container_SetItem(slot->container, slot->slot, stack);
    LIBMATTI_MC_Slot_SetChanged(slot);
}

void LIBMATTI_MC_Slot_SetByPlayer(LIBMATTI_MC_Slot *slot, LIBMATTI_MC_ItemStack *stack, LIBMATTI_MC_ItemStack *oldStack)
{
    (void) oldStack; // Java: setByPlayer(stack, old) - the armor/offhand equip
    // hooks land with the equipment port; the default rides set(stack)
    LIBMATTI_MC_Slot_Set(slot, stack);
}

LIBMATTI_MC_ItemStack *LIBMATTI_MC_Slot_Remove(LIBMATTI_MC_Slot *slot, int amount)
{
    if (slot == NULL || slot->container == NULL)
        return LIBMATTI_MC_ItemStack_Empty();
    return LIBMATTI_MC_Container_RemoveItem(slot->container, slot->slot, amount);
}

int LIBMATTI_MC_Slot_GetMaxStackSize(const LIBMATTI_MC_Slot *slot)
{
    if (slot == NULL || slot->container == NULL)
        return 0;
    return LIBMATTI_MC_Container_GetMaxStackSize(slot->container);
}

int LIBMATTI_MC_Slot_GetMaxStackSizeFor(const LIBMATTI_MC_Slot *slot, const LIBMATTI_MC_ItemStack *stack)
{
    int containerMax = LIBMATTI_MC_Slot_GetMaxStackSize(slot);
    int stackMax = LIBMATTI_MC_ItemStack_GetMaxStackSize(stack);
    return containerMax < stackMax ? containerMax : stackMax;
}

int LIBMATTI_MC_Slot_GetContainerSlot(const LIBMATTI_MC_Slot *slot)
{
    return slot != NULL ? slot->slot : -1;
}

bool LIBMATTI_MC_Slot_MayPlace(const LIBMATTI_MC_Slot *slot, const LIBMATTI_MC_ItemStack *stack)
{
    if (slot == NULL)
        return false;
    if (slot->mayPlace != NULL)
        return slot->mayPlace(slot, stack);
    return true; // Java: the base default
}

bool LIBMATTI_MC_Slot_MayPickup(const LIBMATTI_MC_Slot *slot, const struct LIBMATTI_MC_Player *player)
{
    if (slot == NULL)
        return false;
    if (slot->mayPickup != NULL)
        return slot->mayPickup(slot, player);
    return true;
}

void LIBMATTI_MC_Slot_OnTake(LIBMATTI_MC_Slot *slot, struct LIBMATTI_MC_Player *player, LIBMATTI_MC_ItemStack *stack)
{
    if (slot == NULL)
        return;
    if (slot->onTake != NULL)
    {
        slot->onTake(slot, player, stack);
        return;
    }
    LIBMATTI_MC_Slot_SetChanged(slot); // Java: the base onTake
}

const char *LIBMATTI_MC_Slot_GetNoItemIcon(const LIBMATTI_MC_Slot *slot)
{
    if (slot == NULL)
        return NULL;
    if (slot->getNoItemIcon != NULL)
        return slot->getNoItemIcon(slot);
    return NULL; // Java: the base answers null
}

bool LIBMATTI_MC_Slot_IsActive(const LIBMATTI_MC_Slot *slot)
{
    if (slot == NULL)
        return false;
    if (slot->isActive != NULL)
        return slot->isActive(slot);
    return true;
}

// Java: public Optional<ItemStack> tryRemove(int, int, Player) - EMPTY on the
// refusals, the removed part otherwise (and the emptied slot resets to EMPTY)
LIBMATTI_MC_ItemStack *LIBMATTI_MC_Slot_TryRemove(LIBMATTI_MC_Slot *slot, int amount, int max,
                                                  struct LIBMATTI_MC_Player *player)
{
    if (slot == NULL || !LIBMATTI_MC_Slot_MayPickup(slot, player))
        return LIBMATTI_MC_ItemStack_Empty();
    if (!LIBMATTI_MC_Slot_AllowModification(slot, player) && LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Slot_GetItem(slot)) > max)
        return LIBMATTI_MC_ItemStack_Empty();
    if (amount > max)
        amount = max;
    LIBMATTI_MC_ItemStack *removed = LIBMATTI_MC_Slot_Remove(slot, amount);
    if (LIBMATTI_MC_ItemStack_IsEmpty(removed))
        return LIBMATTI_MC_ItemStack_Empty();
    if (LIBMATTI_MC_ItemStack_IsEmpty(LIBMATTI_MC_Slot_GetItem(slot)))
        LIBMATTI_MC_Slot_SetByPlayer(slot, LIBMATTI_MC_ItemStack_Empty(), removed);
    return removed;
}

LIBMATTI_MC_ItemStack *LIBMATTI_MC_Slot_SafeTake(LIBMATTI_MC_Slot *slot, int amount, int max,
                                                 struct LIBMATTI_MC_Player *player)
{
    LIBMATTI_MC_ItemStack *removed = LIBMATTI_MC_Slot_TryRemove(slot, amount, max, player);
    if (!LIBMATTI_MC_ItemStack_IsEmpty(removed))
        LIBMATTI_MC_Slot_OnTake(slot, player, removed);
    return removed;
}

// Java: public ItemStack safeInsert(ItemStack, int) - the insert returns the
// stack REMAINDER (the input stack mutates like Java's)
LIBMATTI_MC_ItemStack *LIBMATTI_MC_Slot_SafeInsertAmount(LIBMATTI_MC_Slot *slot, LIBMATTI_MC_ItemStack *stack, int amount)
{
    if (slot == NULL || LIBMATTI_MC_ItemStack_IsEmpty(stack) || !LIBMATTI_MC_Slot_MayPlace(slot, stack))
        return stack;
    LIBMATTI_MC_ItemStack *current = LIBMATTI_MC_Slot_GetItem(slot);
    int max = LIBMATTI_MC_Slot_GetMaxStackSizeFor(slot, stack) - LIBMATTI_MC_ItemStack_GetCount(current);
    int insert = LIBMATTI_MC_ItemStack_GetCount(stack);
    if (insert > amount)
        insert = amount;
    if (insert > max)
        insert = max;
    if (insert <= 0)
        return stack;
    if (LIBMATTI_MC_ItemStack_IsEmpty(current))
    {
        LIBMATTI_MC_ItemStack *part = LIBMATTI_MC_ItemStack_Split(stack, insert);
        LIBMATTI_MC_Slot_SetByPlayer(slot, part, NULL);
    }
    else if (LIBMATTI_MC_ItemStack_IsSameItemSameComponents(current, stack))
    {
        LIBMATTI_MC_ItemStack_Shrink(stack, insert);
        LIBMATTI_MC_ItemStack_Grow(current, insert);
    }
    return stack;
}

LIBMATTI_MC_ItemStack *LIBMATTI_MC_Slot_SafeInsert(LIBMATTI_MC_Slot *slot, LIBMATTI_MC_ItemStack *stack)
{
    return LIBMATTI_MC_Slot_SafeInsertAmount(slot, stack, LIBMATTI_MC_ItemStack_GetCount(stack));
}

bool LIBMATTI_MC_Slot_AllowModification(LIBMATTI_MC_Slot *slot, struct LIBMATTI_MC_Player *player)
{
    return LIBMATTI_MC_Slot_MayPickup(slot, player) && LIBMATTI_MC_Slot_MayPlace(slot, LIBMATTI_MC_Slot_GetItem(slot));
}
