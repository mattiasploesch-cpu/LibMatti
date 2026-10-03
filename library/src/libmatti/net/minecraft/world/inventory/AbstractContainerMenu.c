// Port of net.minecraft.world.inventory.AbstractContainerMenu over the
// Slot/Container/ItemStack ports. The doClick branches follow Java's 1.21.11
// semantics: PICKUP (place/take/split/swap), QUICK_MOVE (the vtable's
// quickMoveStack), SWAP (the hotbar number keys), CLONE (creative middle
// click), THROW (drop), QUICK_CRAFT (the drag paint) and PICKUP_ALL
// (double click gather). The server synchronizer collapses into the
// lastSlots snapshot broadcast (single-player, no wire yet).

#include "libmatti/net/minecraft/world/inventory/AbstractContainerMenu.h"
#include "libmatti/net/minecraft/world/Container.h"
#include "libmatti/net/minecraft/world/entity/VanillaEntities.h"

#include <stdlib.h>
#include <string.h>

// Java: protected AbstractContainerMenu(MenuType<?>, int)
void LIBMATTI_MC_AbstractContainerMenu_Init(LIBMATTI_MC_AbstractContainerMenu *menu, LIBMATTI_MC_MenuType *menuType,
                                            int containerId)
{
    memset(menu, 0, sizeof(*menu));
    menu->menuType = menuType;
    menu->containerId = containerId;
    menu->carried = LIBMATTI_MC_ItemStack_Empty();
    menu->quickcraftType = -1; // Java: private int quickcraftType = -1
    menu->quickcraftStatus = 0;
}

LIBMATTI_MC_AbstractContainerMenu *_menu_cast(void *menu)
{
    return (LIBMATTI_MC_AbstractContainerMenu *) menu;
}

void LIBMATTI_MC_AbstractContainerMenu_Free(LIBMATTI_MC_AbstractContainerMenu *menu)
{
    if (menu == NULL)
        return;
    if (menu->slots != NULL)
    {
        for (int i = 0; i < menu->slotCount; i++)
            LIBMATTI_MC_Slot_Free(menu->slots[i]);
        free(menu->slots);
    }
    free(menu->lastSlots);
    free(menu->dataSlots);
    free(menu->quickcraftSlots);
    // Java: the carried stack dies with the menu (removed() cleaned it)
    if (menu->carried != NULL && menu->carried != LIBMATTI_MC_ItemStack_Empty())
        LIBMATTI_MC_ItemStack_Free(menu->carried);
    memset(menu, 0, sizeof(*menu));
}

// the growable array helper (Java: NonNullList.create())
static void **grow(void **array, int count, int *capacity, size_t elementSize)
{
    (void) array;
    (void) elementSize;
    if (count < *capacity)
        return NULL; // signal: no realloc needed
    int newCapacity = *capacity == 0 ? 8 : *capacity * 2;
    void **grown = realloc(array, (size_t) newCapacity * elementSize);
    if (grown != NULL)
        *capacity = newCapacity;
    return grown;
}

// Java: protected Slot addSlot(Slot) - the menu index assignment rides here
LIBMATTI_MC_Slot *LIBMATTI_MC_AbstractContainerMenu_AddSlot(LIBMATTI_MC_AbstractContainerMenu *menu,
                                                            LIBMATTI_MC_Slot *slot)
{
    if (menu == NULL || slot == NULL)
        return slot;
    void **grown = grow((void **) menu->slots, menu->slotCount, &menu->slotCapacity, sizeof(LIBMATTI_MC_Slot *));
    if (grown != NULL)
        menu->slots = (LIBMATTI_MC_Slot **) grown;
    // the lastSlots snapshot grows with its OWN capacity (Java: lastSlots.add(EMPTY))
    void **grownLast = grow((void **) menu->lastSlots, menu->slotCount, &menu->lastSlotCapacity,
                            sizeof(LIBMATTI_MC_ItemStack *));
    if (grownLast != NULL)
        menu->lastSlots = (LIBMATTI_MC_ItemStack **) grownLast;
    menu->lastSlots[menu->slotCount] = NULL; // EMPTY = NULL snapshot
    slot->index = menu->slotCount;
    menu->slots[menu->slotCount++] = slot;
    return slot;
}

void LIBMATTI_MC_AbstractContainerMenu_AddDataSlot(LIBMATTI_MC_AbstractContainerMenu *menu, LIBMATTI_MC_DataSlot slot)
{
    if (menu == NULL)
        return;
    void **grown = grow((void **) menu->dataSlots, menu->dataSlotCount, &menu->dataSlotCapacity,
                        sizeof(LIBMATTI_MC_DataSlot));
    if (grown != NULL)
        menu->dataSlots = (LIBMATTI_MC_DataSlot *) grown;
    menu->dataSlots[menu->dataSlotCount++] = slot;
}

void LIBMATTI_MC_AbstractContainerMenu_AddDataSlots(LIBMATTI_MC_AbstractContainerMenu *menu, int *data, int count)
{
    if (menu == NULL || data == NULL)
        return;
    for (int i = 0; i < count; i++)
    {
        LIBMATTI_MC_DataSlot slot = LIBMATTI_MC_DataSlot_Shared(data, i);
        LIBMATTI_MC_AbstractContainerMenu_AddDataSlot(menu, slot);
    }
}

// Java: protected void addInventoryHotbarSlots(Container, int x, int y)
void LIBMATTI_MC_AbstractContainerMenu_AddInventoryHotbarSlots(LIBMATTI_MC_AbstractContainerMenu *menu,
                                                               struct LIBMATTI_MC_Container *container, int x, int y)
{
    for (int i = 0; i < 9; i++)
        LIBMATTI_MC_AbstractContainerMenu_AddSlot(menu, LIBMATTI_MC_Slot_New(container, i, x + i * 18, y));
}

// Java: protected void addInventoryExtendedSlots(Container, int x, int y)
void LIBMATTI_MC_AbstractContainerMenu_AddInventoryExtendedSlots(LIBMATTI_MC_AbstractContainerMenu *menu,
                                                                 struct LIBMATTI_MC_Container *container, int x, int y)
{
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 9; j++)
            LIBMATTI_MC_AbstractContainerMenu_AddSlot(menu, LIBMATTI_MC_Slot_New(container, j + (i + 1) * 9,
                                                                                 x + j * 18, y + i * 18));
}

// Java: protected void addStandardInventorySlots(Container, int x, int y)
void LIBMATTI_MC_AbstractContainerMenu_AddStandardInventorySlots(LIBMATTI_MC_AbstractContainerMenu *menu,
                                                                 struct LIBMATTI_MC_Container *container, int x, int y)
{
    LIBMATTI_MC_AbstractContainerMenu_AddInventoryExtendedSlots(menu, container, x, y);
    LIBMATTI_MC_AbstractContainerMenu_AddInventoryHotbarSlots(menu, container, x, y + 58);
}

LIBMATTI_MC_MenuType *LIBMATTI_MC_AbstractContainerMenu_GetType(const LIBMATTI_MC_AbstractContainerMenu *menu)
{
    return menu != NULL ? menu->menuType : NULL;
}

LIBMATTI_MC_Slot *LIBMATTI_MC_AbstractContainerMenu_GetSlot(const LIBMATTI_MC_AbstractContainerMenu *menu, int index)
{
    if (menu == NULL || index < 0 || index >= menu->slotCount)
        return NULL;
    return menu->slots[index];
}

bool LIBMATTI_MC_AbstractContainerMenu_IsValidSlotIndex(const LIBMATTI_MC_AbstractContainerMenu *menu, int index)
{
    return index == -1 || index == LIBMATTI_MC_AbstractContainerMenu_SLOT_CLICKED_OUTSIDE
           || (menu != NULL && index < menu->slotCount);
}

LIBMATTI_MC_ItemStack **LIBMATTI_MC_AbstractContainerMenu_GetItems(const LIBMATTI_MC_AbstractContainerMenu *menu,
                                                                   int *count)
{
    if (menu == NULL || count == NULL)
        return NULL;
    LIBMATTI_MC_ItemStack **items = malloc((size_t) (menu->slotCount > 0 ? menu->slotCount : 1)
                                           * sizeof(LIBMATTI_MC_ItemStack *));
    if (items == NULL)
        return NULL;
    for (int i = 0; i < menu->slotCount; i++)
        items[i] = LIBMATTI_MC_Slot_GetItem(menu->slots[i]);
    *count = menu->slotCount;
    return items;
}

void LIBMATTI_MC_AbstractContainerMenu_SetCarried(LIBMATTI_MC_AbstractContainerMenu *menu, LIBMATTI_MC_ItemStack *stack)
{
    if (menu == NULL)
        return;
    // Java: the carried field reference-swap - the OLD stack is dropped only
    // when it is not the same object being stored (the slot setters hand the
    // same stack back on the merge paths)
    if (menu->carried != NULL && menu->carried != LIBMATTI_MC_ItemStack_Empty() && menu->carried != stack)
        LIBMATTI_MC_ItemStack_Free(menu->carried);
    menu->carried = stack != NULL ? stack : LIBMATTI_MC_ItemStack_Empty();
}

LIBMATTI_MC_ItemStack *LIBMATTI_MC_AbstractContainerMenu_GetCarried(const LIBMATTI_MC_AbstractContainerMenu *menu)
{
    if (menu == NULL || menu->carried == NULL)
        return LIBMATTI_MC_ItemStack_Empty();
    return menu->carried;
}

int LIBMATTI_MC_AbstractContainerMenu_GetStateId(const LIBMATTI_MC_AbstractContainerMenu *menu)
{
    return menu != NULL ? menu->stateId : 0;
}

int LIBMATTI_MC_AbstractContainerMenu_IncrementStateId(LIBMATTI_MC_AbstractContainerMenu *menu)
{
    if (menu == NULL)
        return 0;
    menu->stateId = (menu->stateId + 1) & 32767; // Java: + 1 & 32767
    return menu->stateId;
}

// ---------------------------------------------------------------------------
// the quickcraft statics
// ---------------------------------------------------------------------------

int LIBMATTI_MC_AbstractContainerMenu_GetQuickcraftType(int headerValue)
{
    return (headerValue >> 2) & 3;
}

int LIBMATTI_MC_AbstractContainerMenu_GetQuickcraftHeader(int headerValue)
{
    return headerValue & 3;
}

int LIBMATTI_MC_AbstractContainerMenu_GetQuickcraftMask(int header, int type)
{
    return (header & 3) | ((type & 3) << 2);
}

bool LIBMATTI_MC_AbstractContainerMenu_IsValidQuickcraftType(int quickcraftType, const struct LIBMATTI_MC_Player *player)
{
    if (quickcraftType == LIBMATTI_MC_AbstractContainerMenu_QUICKCRAFT_TYPE_CHARITABLE)
        return true;
    if (quickcraftType == LIBMATTI_MC_AbstractContainerMenu_QUICKCRAFT_TYPE_GREEDY)
        return true;
    // CLONE rides the creative instabuild ability
    return quickcraftType == LIBMATTI_MC_AbstractContainerMenu_QUICKCRAFT_TYPE_CLONE
           && player != NULL && LIBMATTI_MC_Player_GetAbilities((LIBMATTI_MC_Player *) player)->instabuild;
}

int LIBMATTI_MC_AbstractContainerMenu_GetQuickcraftPlaceCount(int quickcraftSlotCount, int quickcraftType,
                                                              const LIBMATTI_MC_ItemStack *stack)
{
    int count = LIBMATTI_MC_ItemStack_GetCount(stack);
    switch (quickcraftType)
    {
    case LIBMATTI_MC_AbstractContainerMenu_QUICKCRAFT_TYPE_CHARITABLE:
        return quickcraftSlotCount > 0 ? count / quickcraftSlotCount : 0; // Mth.floor(int/int)
    case LIBMATTI_MC_AbstractContainerMenu_QUICKCRAFT_TYPE_GREEDY:
        return 1;
    case LIBMATTI_MC_AbstractContainerMenu_QUICKCRAFT_TYPE_CLONE:
        return LIBMATTI_MC_ItemStack_GetMaxStackSize(stack);
    default:
        return count;
    }
}

bool LIBMATTI_MC_AbstractContainerMenu_CanItemQuickReplace(const LIBMATTI_MC_Slot *slot,
                                                           const LIBMATTI_MC_ItemStack *stack, bool checkCount)
{
    bool empty = slot == NULL || !LIBMATTI_MC_Slot_HasItem(slot);
    if (!empty && LIBMATTI_MC_ItemStack_IsSameItemSameComponents(stack, LIBMATTI_MC_Slot_GetItem(slot)))
    {
        if (!checkCount)
            return true;
        return LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Slot_GetItem(slot)) + LIBMATTI_MC_ItemStack_GetCount(stack)
               <= LIBMATTI_MC_ItemStack_GetMaxStackSize(stack);
    }
    return empty;
}

int LIBMATTI_MC_AbstractContainerMenu_FindSlot(const LIBMATTI_MC_AbstractContainerMenu *menu,
                                               const struct LIBMATTI_MC_Container *container, int containerSlot)
{
    if (menu == NULL)
        return -1;
    for (int i = 0; i < menu->slotCount; i++)
    {
        const LIBMATTI_MC_Slot *slot = menu->slots[i];
        if (slot->container == container && containerSlot == LIBMATTI_MC_Slot_GetContainerSlot(slot))
            return i;
    }
    return -1;
}

void LIBMATTI_MC_AbstractContainerMenu_SlotsChanged(LIBMATTI_MC_AbstractContainerMenu *menu,
                                                    struct LIBMATTI_MC_Container *container)
{
    (void) container;
    LIBMATTI_MC_AbstractContainerMenu_BroadcastChanges(menu);
}

void LIBMATTI_MC_AbstractContainerMenu_BroadcastChanges(LIBMATTI_MC_AbstractContainerMenu *menu)
{
    if (menu == NULL)
        return;
    // Java: the lastSlots walk fires the ContainerListeners; the port keeps the
    // snapshot (matches() = the item/count identity) and bumps the stateId.
    for (int i = 0; i < menu->slotCount; i++)
    {
        LIBMATTI_MC_ItemStack *current = LIBMATTI_MC_Slot_GetItem(menu->slots[i]);
        LIBMATTI_MC_ItemStack *snapshot = menu->lastSlots[i];
        bool same = LIBMATTI_MC_ItemStack_Matches(snapshot != NULL ? snapshot : LIBMATTI_MC_ItemStack_Empty(), current);
        if (!same)
        {
            LIBMATTI_MC_ItemStack_Free(snapshot);
            menu->lastSlots[i] = LIBMATTI_MC_ItemStack_IsEmpty(current) ? NULL : LIBMATTI_MC_ItemStack_Copy(current);
            LIBMATTI_MC_AbstractContainerMenu_IncrementStateId(menu);
        }
    }
}

void LIBMATTI_MC_AbstractContainerMenu_SetItem(LIBMATTI_MC_AbstractContainerMenu *menu, int slotIndex, int stateId,
                                               LIBMATTI_MC_ItemStack *stack)
{
    if (menu == NULL)
        return;
    LIBMATTI_MC_Slot *slot = LIBMATTI_MC_AbstractContainerMenu_GetSlot(menu, slotIndex);
    if (slot != NULL)
        LIBMATTI_MC_Slot_Set(slot, stack);
    menu->stateId = stateId;
}

void LIBMATTI_MC_AbstractContainerMenu_Removed(LIBMATTI_MC_AbstractContainerMenu *menu, struct LIBMATTI_MC_Player *player)
{
    (void) player;
    if (menu == NULL)
        return;
    // Java: the ServerPlayer branch drops/places the carried stack back; the
    // client skeleton clears it (the local player never disconnects mid-menu).
    if (menu->carried != NULL && menu->carried != LIBMATTI_MC_ItemStack_Empty())
    {
        LIBMATTI_MC_ItemStack_Free(menu->carried);
        menu->carried = LIBMATTI_MC_ItemStack_Empty();
    }
}

// ---------------------------------------------------------------------------
// moveItemStackTo - the quick-move scan (Java: protected)
// ---------------------------------------------------------------------------

bool LIBMATTI_MC_AbstractContainerMenu_MoveItemStackTo(LIBMATTI_MC_AbstractContainerMenu *menu,
                                                       LIBMATTI_MC_ItemStack *stack, int fromIndex, int endIndex,
                                                       bool reverse)
{
    if (menu == NULL || stack == NULL || LIBMATTI_MC_ItemStack_IsEmpty(stack))
        return false;
    bool moved = false;
    int i = reverse ? endIndex - 1 : fromIndex;

    // the first pass: merge into the same-item stacks
    if (LIBMATTI_MC_ItemStack_IsStackable(stack))
    {
        while (!LIBMATTI_MC_ItemStack_IsEmpty(stack) && (reverse ? i >= fromIndex : i < endIndex))
        {
            LIBMATTI_MC_Slot *slot = menu->slots[i];
            LIBMATTI_MC_ItemStack *current = LIBMATTI_MC_Slot_GetItem(slot);
            if (!LIBMATTI_MC_ItemStack_IsEmpty(current)
                && LIBMATTI_MC_ItemStack_IsSameItemSameComponents(stack, current))
            {
                int merged = LIBMATTI_MC_ItemStack_GetCount(current) + LIBMATTI_MC_ItemStack_GetCount(stack);
                int max = LIBMATTI_MC_Slot_GetMaxStackSizeFor(slot, current);
                if (merged <= max)
                {
                    LIBMATTI_MC_ItemStack_SetCount(stack, 0);
                    LIBMATTI_MC_ItemStack_SetCount(current, merged);
                    LIBMATTI_MC_Slot_SetChanged(slot);
                    moved = true;
                }
                else if (LIBMATTI_MC_ItemStack_GetCount(current) < max)
                {
                    LIBMATTI_MC_ItemStack_Shrink(stack, max - LIBMATTI_MC_ItemStack_GetCount(current));
                    LIBMATTI_MC_ItemStack_SetCount(current, max);
                    LIBMATTI_MC_Slot_SetChanged(slot);
                    moved = true;
                }
            }

            i += reverse ? -1 : 1;
        }
    }

    // the second pass: the first empty mayPlace slot takes the split
    if (!LIBMATTI_MC_ItemStack_IsEmpty(stack))
    {
        i = reverse ? endIndex - 1 : fromIndex;
        while (reverse ? i >= fromIndex : i < endIndex)
        {
            LIBMATTI_MC_Slot *slot = menu->slots[i];
            LIBMATTI_MC_ItemStack *current = LIBMATTI_MC_Slot_GetItem(slot);
            if (LIBMATTI_MC_ItemStack_IsEmpty(current) && LIBMATTI_MC_Slot_MayPlace(slot, stack))
            {
                int max = LIBMATTI_MC_Slot_GetMaxStackSizeFor(slot, stack);
                int take = LIBMATTI_MC_ItemStack_GetCount(stack) < max ? LIBMATTI_MC_ItemStack_GetCount(stack) : max;
                LIBMATTI_MC_ItemStack *part = LIBMATTI_MC_ItemStack_Split(stack, take);
                LIBMATTI_MC_Slot_SetByPlayer(slot, part, NULL);
                LIBMATTI_MC_Slot_SetChanged(slot);
                moved = true;
                break;
            }

            i += reverse ? -1 : 1;
        }
    }

    return moved;
}

// ---------------------------------------------------------------------------
// doClick - the click dispatch
// ---------------------------------------------------------------------------

// Java: resetQuickCraft
static void reset_quickcraft(LIBMATTI_MC_AbstractContainerMenu *menu)
{
    menu->quickcraftStatus = 0;
    menu->quickcraftSlotCount = 0;
}

// the quickcraft drag-set membership (Java: the Set<Slot>)
static int quickcraft_find(const LIBMATTI_MC_AbstractContainerMenu *menu, const LIBMATTI_MC_Slot *slot)
{
    for (int i = 0; i < menu->quickcraftSlotCount; i++)
        if (menu->quickcraftSlots[i] == slot)
            return i;
    return -1;
}

static void quickcraft_add(LIBMATTI_MC_AbstractContainerMenu *menu, LIBMATTI_MC_Slot *slot)
{
    if (quickcraft_find(menu, slot) >= 0)
        return;
    void **grown = grow((void **) menu->quickcraftSlots, menu->quickcraftSlotCount, &menu->quickcraftSlotCapacity,
                        sizeof(LIBMATTI_MC_Slot *));
    if (grown != NULL)
        menu->quickcraftSlots = (LIBMATTI_MC_Slot **) grown;
    menu->quickcraftSlots[menu->quickcraftSlotCount++] = slot;
}

// Java: the QUICK_CRAFT status==2 tail - the painted slots take their share
static void quickcraft_place(LIBMATTI_MC_AbstractContainerMenu *menu, struct LIBMATTI_MC_Player *player)
{
    (void) player;
    if (menu->quickcraftSlotCount == 0)
        return;
    if (menu->quickcraftSlotCount == 1)
    {
        // Java: the single-slot drag collapses into one PICKUP
        int index = menu->quickcraftSlots[0]->index;
        int type = menu->quickcraftType;
        reset_quickcraft(menu);
        LIBMATTI_MC_AbstractContainerMenu_Clicked(menu, index, type, LIBMATTI_MC_ClickType_PICKUP, player);
        return;
    }

    LIBMATTI_MC_ItemStack *carried = LIBMATTI_MC_AbstractContainerMenu_GetCarried(menu);
    if (LIBMATTI_MC_ItemStack_IsEmpty(carried))
    {
        reset_quickcraft(menu);
        return;
    }
    // Java: itemstack3 = carried.copy() - the paint source
    LIBMATTI_MC_ItemStack *copy = LIBMATTI_MC_ItemStack_Copy(carried);
    int remaining = LIBMATTI_MC_ItemStack_GetCount(carried);

    for (int i = 0; i < menu->quickcraftSlotCount; i++)
    {
        LIBMATTI_MC_Slot *slot = menu->quickcraftSlots[i];
        if (LIBMATTI_MC_AbstractContainerMenu_CanItemQuickReplace(slot, carried, true)
            && LIBMATTI_MC_Slot_MayPlace(slot, carried)
            && (menu->quickcraftType == LIBMATTI_MC_AbstractContainerMenu_QUICKCRAFT_TYPE_CLONE
                || LIBMATTI_MC_ItemStack_GetCount(carried) >= menu->quickcraftSlotCount)
            && (menu->canDragTo == NULL || menu->canDragTo(menu, slot)))
        {
            int current = LIBMATTI_MC_Slot_HasItem(slot) ? LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Slot_GetItem(slot)) : 0;
            int max = LIBMATTI_MC_ItemStack_GetMaxStackSize(copy) < LIBMATTI_MC_Slot_GetMaxStackSizeFor(slot, copy)
                          ? LIBMATTI_MC_ItemStack_GetMaxStackSize(copy)
                          : LIBMATTI_MC_Slot_GetMaxStackSizeFor(slot, copy);
            int place = LIBMATTI_MC_AbstractContainerMenu_GetQuickcraftPlaceCount(
                            menu->quickcraftSlotCount, menu->quickcraftType, copy)
                        + current;
            if (place > max)
                place = max;
            remaining -= place - current;
            LIBMATTI_MC_Slot_SetByPlayer(slot, LIBMATTI_MC_ItemStack_CopyWithCount(copy, place), NULL);
        }
    }

    // Java: itemstack3.setCount(k1); this.setCarried(itemstack3) - the copy
    // BECOMES the carried (SetCarried frees the old reference, never the new)
    LIBMATTI_MC_ItemStack_SetCount(copy, remaining);
    LIBMATTI_MC_AbstractContainerMenu_SetCarried(menu, copy);
    reset_quickcraft(menu);
}

// Java: the QUICK_MOVE branch tail (quickMoveStack loop until the item stops)
static void quick_move(LIBMATTI_MC_AbstractContainerMenu *menu, int slotIndex, struct LIBMATTI_MC_Player *player)
{
    LIBMATTI_MC_Slot *slot = LIBMATTI_MC_AbstractContainerMenu_GetSlot(menu, slotIndex);
    if (slot == NULL || !LIBMATTI_MC_Slot_MayPickup(slot, player))
        return;
    LIBMATTI_MC_ItemStack *moved = LIBMATTI_MC_AbstractContainerMenu_QuickMoveStack(menu, player, slotIndex);
    while (!LIBMATTI_MC_ItemStack_IsEmpty(moved)
           && LIBMATTI_MC_ItemStack_IsSameItem(LIBMATTI_MC_Slot_GetItem(slot), moved))
    {
        LIBMATTI_MC_ItemStack_Free(moved);
        moved = LIBMATTI_MC_AbstractContainerMenu_QuickMoveStack(menu, player, slotIndex);
    }
    LIBMATTI_MC_ItemStack_Free(moved);
}

// Java: the PICKUP branch (the primary/secondary click semantics)
static void pickup(LIBMATTI_MC_AbstractContainerMenu *menu, int slotIndex, int button, struct LIBMATTI_MC_Player *player)
{
    LIBMATTI_MC_Slot *slot = LIBMATTI_MC_AbstractContainerMenu_GetSlot(menu, slotIndex);
    if (slot == NULL)
        return;
    LIBMATTI_MC_ItemStack *slotStack = LIBMATTI_MC_Slot_GetItem(slot);
    LIBMATTI_MC_ItemStack *carried = LIBMATTI_MC_AbstractContainerMenu_GetCarried(menu);
    bool primary = button == 0;

    if (LIBMATTI_MC_ItemStack_IsEmpty(slotStack))
    {
        if (!LIBMATTI_MC_ItemStack_IsEmpty(carried))
        {
            int amount = primary ? LIBMATTI_MC_ItemStack_GetCount(carried) : 1;
            // Java: setCarried(slot.safeInsert(carried, amount)) - the remainder
            LIBMATTI_MC_ItemStack *remainder = LIBMATTI_MC_Slot_SafeInsertAmount(slot, carried, amount);
            LIBMATTI_MC_AbstractContainerMenu_SetCarried(menu, remainder);
        }
    }
    else if (LIBMATTI_MC_Slot_MayPickup(slot, player))
    {
        if (LIBMATTI_MC_ItemStack_IsEmpty(carried))
        {
            // Java: the take - primary takes all, secondary takes the half
            int amount = primary ? LIBMATTI_MC_ItemStack_GetCount(slotStack)
                                 : (LIBMATTI_MC_ItemStack_GetCount(slotStack) + 1) / 2;
            LIBMATTI_MC_ItemStack *removed = LIBMATTI_MC_Slot_TryRemove(slot, amount, 0x7FFFFFFF, player);
            if (!LIBMATTI_MC_ItemStack_IsEmpty(removed))
            {
                LIBMATTI_MC_AbstractContainerMenu_SetCarried(menu, removed);
                LIBMATTI_MC_Slot_OnTake(slot, player, removed);
            }
        }
        else if (LIBMATTI_MC_Slot_MayPlace(slot, carried))
        {
            if (LIBMATTI_MC_ItemStack_IsSameItemSameComponents(slotStack, carried))
            {
                int amount = primary ? LIBMATTI_MC_ItemStack_GetCount(carried) : 1;
                LIBMATTI_MC_ItemStack *remainder = LIBMATTI_MC_Slot_SafeInsertAmount(slot, carried, amount);
                LIBMATTI_MC_AbstractContainerMenu_SetCarried(menu, remainder);
            }
            else if (LIBMATTI_MC_ItemStack_GetCount(carried) <= LIBMATTI_MC_Slot_GetMaxStackSizeFor(slot, carried))
            {
                // Java: the swap - carried <-> slot
                LIBMATTI_MC_ItemStack *slotCopy = LIBMATTI_MC_ItemStack_Copy(slotStack);
                LIBMATTI_MC_ItemStack *carriedCopy = LIBMATTI_MC_ItemStack_Copy(carried);
                LIBMATTI_MC_Slot_SetByPlayer(slot, carriedCopy, slotStack);
                LIBMATTI_MC_AbstractContainerMenu_SetCarried(menu, slotCopy);
            }
        }
        else if (LIBMATTI_MC_ItemStack_IsSameItemSameComponents(slotStack, carried))
        {
            // Java: the mayPlace-refused merge - the carried grows from the slot
            int room = LIBMATTI_MC_ItemStack_GetMaxStackSize(carried) - LIBMATTI_MC_ItemStack_GetCount(carried);
            LIBMATTI_MC_ItemStack *removed = LIBMATTI_MC_Slot_TryRemove(
                slot, LIBMATTI_MC_ItemStack_GetCount(slotStack), room > 0 ? room : 0, player);
            if (!LIBMATTI_MC_ItemStack_IsEmpty(removed))
            {
                LIBMATTI_MC_ItemStack_Grow(carried, LIBMATTI_MC_ItemStack_GetCount(removed));
                LIBMATTI_MC_Slot_OnTake(slot, player, removed);
                LIBMATTI_MC_ItemStack_Free(removed);
            }
        }
    }

    LIBMATTI_MC_Slot_SetChanged(slot);
}

// Java: the SWAP branch (the hotbar number keys / offhand)
static void swap(LIBMATTI_MC_AbstractContainerMenu *menu, int slotIndex, int button, struct LIBMATTI_MC_Player *player)
{
    // Java: inventory.getItem(button) - the player inventory rides the player's
    // container; the port stages the swap over the carried (the Inventory port
    // lands with P6.3 - the numeric swap stays a no-op without the inventory).
    (void) menu;
    (void) slotIndex;
    (void) button;
    (void) player;
}

// Java: public void clicked(int, int, ClickType, Player) -> doClick
void LIBMATTI_MC_AbstractContainerMenu_Clicked(LIBMATTI_MC_AbstractContainerMenu *menu, int slotIndex, int button,
                                               LIBMATTI_MC_ClickType clickType, struct LIBMATTI_MC_Player *player)
{
    if (menu == NULL || player == NULL)
        return;

    if (clickType == LIBMATTI_MC_ClickType_QUICK_CRAFT)
    {
        int previousStatus = menu->quickcraftStatus;
        menu->quickcraftStatus = LIBMATTI_MC_AbstractContainerMenu_GetQuickcraftHeader(button);
        // Java: the invalid transition guards
        if ((previousStatus != 1 || menu->quickcraftStatus != 2) && previousStatus != menu->quickcraftStatus)
        {
            reset_quickcraft(menu);
            return;
        }
        if (LIBMATTI_MC_ItemStack_IsEmpty(LIBMATTI_MC_AbstractContainerMenu_GetCarried(menu)))
        {
            reset_quickcraft(menu);
            return;
        }
        if (menu->quickcraftStatus == LIBMATTI_MC_AbstractContainerMenu_QUICKCRAFT_HEADER_START)
        {
            menu->quickcraftType = LIBMATTI_MC_AbstractContainerMenu_GetQuickcraftType(button);
            if (LIBMATTI_MC_AbstractContainerMenu_IsValidQuickcraftType(menu->quickcraftType, player))
            {
                menu->quickcraftStatus = 1;
                menu->quickcraftSlotCount = 0;
            }
            else
            {
                reset_quickcraft(menu);
            }
            return;
        }
        if (menu->quickcraftStatus == LIBMATTI_MC_AbstractContainerMenu_QUICKCRAFT_HEADER_CONTINUE)
        {
            LIBMATTI_MC_Slot *slot = LIBMATTI_MC_AbstractContainerMenu_GetSlot(menu, slotIndex);
            LIBMATTI_MC_ItemStack *carried = LIBMATTI_MC_AbstractContainerMenu_GetCarried(menu);
            if (slot != NULL
                && LIBMATTI_MC_AbstractContainerMenu_CanItemQuickReplace(slot, carried, true)
                && LIBMATTI_MC_Slot_MayPlace(slot, carried)
                && (menu->quickcraftType == LIBMATTI_MC_AbstractContainerMenu_QUICKCRAFT_TYPE_CLONE
                    || LIBMATTI_MC_ItemStack_GetCount(carried) > menu->quickcraftSlotCount)
                && (menu->canDragTo == NULL || menu->canDragTo(menu, slot)))
            {
                quickcraft_add(menu, slot);
            }
            return;
        }
        if (menu->quickcraftStatus == LIBMATTI_MC_AbstractContainerMenu_QUICKCRAFT_HEADER_END)
        {
            quickcraft_place(menu, player);
            return;
        }
        reset_quickcraft(menu);
        return;
    }

    if (menu->quickcraftStatus != 0)
    {
        reset_quickcraft(menu);
        return;
    }

    if ((clickType == LIBMATTI_MC_ClickType_PICKUP || clickType == LIBMATTI_MC_ClickType_QUICK_MOVE)
        && (button == 0 || button == 1))
    {
        if (slotIndex == LIBMATTI_MC_AbstractContainerMenu_SLOT_CLICKED_OUTSIDE)
        {
            // Java: the drop outside - primary drops the whole carried, secondary one
            LIBMATTI_MC_ItemStack *carried = LIBMATTI_MC_AbstractContainerMenu_GetCarried(menu);
            if (!LIBMATTI_MC_ItemStack_IsEmpty(carried))
            {
                if (button == 0)
                    LIBMATTI_MC_AbstractContainerMenu_SetCarried(menu, LIBMATTI_MC_ItemStack_Empty());
                else
                    LIBMATTI_MC_ItemStack_Shrink(carried, 1);
            }
            return;
        }
        if (slotIndex < 0)
            return;
        if (clickType == LIBMATTI_MC_ClickType_QUICK_MOVE)
        {
            quick_move(menu, slotIndex, player);
            return;
        }
        pickup(menu, slotIndex, button, player);
        return;
    }

    if (clickType == LIBMATTI_MC_ClickType_SWAP && ((button >= 0 && button < 9) || button == 40))
    {
        swap(menu, slotIndex, button, player);
        return;
    }

    if (clickType == LIBMATTI_MC_ClickType_CLONE && slotIndex >= 0)
    {
        LIBMATTI_MC_Player *typed = (LIBMATTI_MC_Player *) player;
        LIBMATTI_MC_ItemStack *carried = LIBMATTI_MC_AbstractContainerMenu_GetCarried(menu);
        if (LIBMATTI_MC_Player_GetAbilities(typed)->instabuild && LIBMATTI_MC_ItemStack_IsEmpty(carried))
        {
            LIBMATTI_MC_Slot *slot = LIBMATTI_MC_AbstractContainerMenu_GetSlot(menu, slotIndex);
            if (slot != NULL && LIBMATTI_MC_Slot_HasItem(slot))
            {
                LIBMATTI_MC_ItemStack *slotStack = LIBMATTI_MC_Slot_GetItem(slot);
                LIBMATTI_MC_AbstractContainerMenu_SetCarried(
                    menu, LIBMATTI_MC_ItemStack_CopyWithCount(slotStack, LIBMATTI_MC_ItemStack_GetMaxStackSize(slotStack)));
            }
        }
        return;
    }

    if (clickType == LIBMATTI_MC_ClickType_THROW && LIBMATTI_MC_ItemStack_IsEmpty(LIBMATTI_MC_AbstractContainerMenu_GetCarried(menu))
        && slotIndex >= 0)
    {
        LIBMATTI_MC_Slot *slot = LIBMATTI_MC_AbstractContainerMenu_GetSlot(menu, slotIndex);
        if (slot == NULL)
            return;
        int amount = button == 0 ? 1 : LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Slot_GetItem(slot));
        // Java: player.drop(slot.safeTake(...), true) - the port has no item
        // entity spawner yet; the take rides (the stack leaves the menu)
        LIBMATTI_MC_ItemStack *taken = LIBMATTI_MC_Slot_SafeTake(slot, amount, 0x7FFFFFFF, player);
        LIBMATTI_MC_ItemStack_Free(taken);
        return;
    }

    if (clickType == LIBMATTI_MC_ClickType_PICKUP_ALL && slotIndex >= 0)
    {
        LIBMATTI_MC_Slot *slot = LIBMATTI_MC_AbstractContainerMenu_GetSlot(menu, slotIndex);
        LIBMATTI_MC_ItemStack *carried = LIBMATTI_MC_AbstractContainerMenu_GetCarried(menu);
        if (slot == NULL || LIBMATTI_MC_ItemStack_IsEmpty(carried)
            || (LIBMATTI_MC_Slot_HasItem(slot) && LIBMATTI_MC_Slot_MayPickup(slot, player)))
            return;
        int start = button == 0 ? 0 : menu->slotCount - 1;
        int step = button == 0 ? 1 : -1;
        for (int pass = 0; pass < 2; pass++)
        {
            for (int i = start; i >= 0 && i < menu->slotCount
                 && LIBMATTI_MC_ItemStack_GetCount(carried) < LIBMATTI_MC_ItemStack_GetMaxStackSize(carried);
                 i += step)
            {
                LIBMATTI_MC_Slot *gather = menu->slots[i];
                if (LIBMATTI_MC_Slot_HasItem(gather)
                    && LIBMATTI_MC_AbstractContainerMenu_CanItemQuickReplace(gather, carried, true)
                    && LIBMATTI_MC_Slot_MayPickup(gather, player)
                    && (menu->canTakeItemForPickAll == NULL || menu->canTakeItemForPickAll(menu, carried, gather)))
                {
                    LIBMATTI_MC_ItemStack *gathered = LIBMATTI_MC_Slot_GetItem(gather);
                    if (pass != 0
                        || LIBMATTI_MC_ItemStack_GetCount(gathered) != LIBMATTI_MC_ItemStack_GetMaxStackSize(gathered))
                    {
                        LIBMATTI_MC_ItemStack *part = LIBMATTI_MC_Slot_SafeTake(
                            gather, LIBMATTI_MC_ItemStack_GetCount(gathered),
                            LIBMATTI_MC_ItemStack_GetMaxStackSize(carried) - LIBMATTI_MC_ItemStack_GetCount(carried),
                            player);
                        LIBMATTI_MC_ItemStack_Grow(carried, LIBMATTI_MC_ItemStack_GetCount(part));
                        LIBMATTI_MC_ItemStack_Free(part);
                    }
                }
            }
        }
        return;
    }
}

LIBMATTI_MC_ItemStack *LIBMATTI_MC_AbstractContainerMenu_QuickMoveStack(LIBMATTI_MC_AbstractContainerMenu *menu,
                                                                        struct LIBMATTI_MC_Player *player, int index)
{
    if (menu == NULL || menu->quickMoveStack == NULL)
        return LIBMATTI_MC_ItemStack_Empty();
    return menu->quickMoveStack(menu, player, index);
}

bool LIBMATTI_MC_AbstractContainerMenu_StillValid(const LIBMATTI_MC_AbstractContainerMenu *menu,
                                                  const struct LIBMATTI_MC_Player *player)
{
    if (menu == NULL || menu->stillValid == NULL)
        return true;
    return menu->stillValid(menu, player);
}
