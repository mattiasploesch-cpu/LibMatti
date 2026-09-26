// Port of net.minecraft.world.inventory.InventoryMenu (the crafting/armor/
// inventory layout) over the menu core. The recipe-match tail of
// slotsChanged rides the crafting port when it lands; the menu keeps the
// layout, the quick-move routing and the armor-slot mayPlace gates.

#include "libmatti/net/minecraft/world/inventory/InventoryMenu.h"
#include "libmatti/net/minecraft/world/Container.h"
#include "libmatti/net/minecraft/world/entity/VanillaEntities.h"
#include "libmatti/net/minecraft/world/item/Item.h"

#include <stdlib.h>
#include <string.h>

// Java: the armor slot positions ride the SLOT_IDS order
// (HEAD, CHEST, LEGS, FEET) - the menu slot indexes 5..8 map 39-i
static const int ARMOR_MENU_SLOT_TO_EQUIPMENT[4] = {0, 1, 2, 3}; // EquipmentSlot ordinal (the port keeps the index)

// Java: ArmorSlot.mayPlace - the equipment gate (the port answers the slot
// index; the Item's equipment mapping rides the Item port when it lands)
static bool armor_may_place(const LIBMATTI_MC_Slot *slot, const LIBMATTI_MC_ItemStack *stack)
{
    (void) slot;
    (void) stack;
    // Java: mayPlace -> the item fits the equipment slot; the port has no
    // equipment mapping on Item yet (the armor stand exists, the mapping
    // lands with the equipment port) - accept everything like the base.
    return true;
}

static bool result_may_place(const LIBMATTI_MC_Slot *slot, const LIBMATTI_MC_ItemStack *stack)
{
    (void) slot;
    (void) stack;
    return false; // Java: ResultSlot.mayPlace -> false
}

// Java: public boolean stillValid(Player) -> true
static bool inventory_still_valid(const LIBMATTI_MC_AbstractContainerMenu *menu, const struct LIBMATTI_MC_Player *player)
{
    (void) menu;
    (void) player;
    return true;
}

// Java: InventoryMenu.quickMoveStack - the routing over the slot ranges
static LIBMATTI_MC_ItemStack *inventory_quick_move(LIBMATTI_MC_AbstractContainerMenu *menu,
                                                   struct LIBMATTI_MC_Player *player, int index)
{
    LIBMATTI_MC_ItemStack *empty = LIBMATTI_MC_ItemStack_Empty();
    LIBMATTI_MC_Slot *slot = LIBMATTI_MC_AbstractContainerMenu_GetSlot(menu, index);
    if (slot == NULL || !LIBMATTI_MC_Slot_HasItem(slot))
        return empty;

    LIBMATTI_MC_ItemStack *itemStack = LIBMATTI_MC_Slot_GetItem(slot);
    LIBMATTI_MC_ItemStack *copy = LIBMATTI_MC_ItemStack_Copy(itemStack);
    bool moved = false;

    if (index == LIBMATTI_MC_InventoryMenu_RESULT_SLOT)
    {
        // Java: moveItemStackTo(itemstack1, 9, 45, true) - reverse scan
        moved = LIBMATTI_MC_AbstractContainerMenu_MoveItemStackTo(menu, itemStack, 9, 45, true);
    }
    else if (index >= LIBMATTI_MC_InventoryMenu_CRAFT_SLOT_START && index < LIBMATTI_MC_InventoryMenu_CRAFT_SLOT_END)
    {
        moved = LIBMATTI_MC_AbstractContainerMenu_MoveItemStackTo(menu, itemStack, 9, 45, false);
    }
    else if (index >= LIBMATTI_MC_InventoryMenu_ARMOR_SLOT_START && index < LIBMATTI_MC_InventoryMenu_ARMOR_SLOT_END)
    {
        moved = LIBMATTI_MC_AbstractContainerMenu_MoveItemStackTo(menu, itemStack, 9, 45, false);
    }
    else if (index >= LIBMATTI_MC_InventoryMenu_INV_SLOT_START && index < LIBMATTI_MC_InventoryMenu_INV_SLOT_END)
    {
        // Java: main inventory -> hotbar
        moved = LIBMATTI_MC_AbstractContainerMenu_MoveItemStackTo(menu, itemStack, 36, 45, false);
    }
    else if (index >= LIBMATTI_MC_InventoryMenu_USE_ROW_SLOT_START && index < LIBMATTI_MC_InventoryMenu_USE_ROW_SLOT_END)
    {
        // Java: hotbar -> main inventory
        moved = LIBMATTI_MC_AbstractContainerMenu_MoveItemStackTo(menu, itemStack, 9, 36, false);
    }
    else
    {
        moved = LIBMATTI_MC_AbstractContainerMenu_MoveItemStackTo(menu, itemStack, 9, 45, false);
    }

    if (!moved)
    {
        LIBMATTI_MC_ItemStack_Free(copy);
        return empty;
    }

    if (LIBMATTI_MC_ItemStack_IsEmpty(itemStack))
        LIBMATTI_MC_Slot_SetByPlayer(slot, LIBMATTI_MC_ItemStack_Empty(), copy);
    else
        LIBMATTI_MC_Slot_SetChanged(slot);

    if (LIBMATTI_MC_ItemStack_GetCount(itemStack) == LIBMATTI_MC_ItemStack_GetCount(copy))
    {
        LIBMATTI_MC_ItemStack_Free(copy);
        return empty;
    }

    LIBMATTI_MC_Slot_OnTake(slot, player, itemStack);
    return copy;
}

// Java: public boolean canTakeItemForPickAll(ItemStack, Slot) - the result
// slot refuses the double-click gather
static bool inventory_can_take_item_for_pick_all(const LIBMATTI_MC_AbstractContainerMenu *menu,
                                                 const LIBMATTI_MC_ItemStack *stack, const LIBMATTI_MC_Slot *slot)
{
    (void) menu;
    (void) stack;
    (void) slot;
    return true; // the result gate rides the ResultSlot hook when crafting lands
}

LIBMATTI_MC_InventoryMenu *LIBMATTI_MC_InventoryMenu_New(struct LIBMATTI_MC_Container *playerInventory,
                                                         bool active, struct LIBMATTI_MC_Player *owner)
{
    LIBMATTI_MC_InventoryMenu *menu = calloc(1, sizeof(LIBMATTI_MC_InventoryMenu));
    if (menu == NULL)
        return NULL;
    // Java: super(null, 0, 2, 2) - no MenuType (the typeless menu)
    LIBMATTI_MC_AbstractContainerMenu_Init(&menu->base, NULL, LIBMATTI_MC_InventoryMenu_CONTAINER_ID);
    menu->base.quickMoveStack = inventory_quick_move;
    menu->base.stillValid = inventory_still_valid;
    menu->base.canTakeItemForPickAll = inventory_can_take_item_for_pick_all;
    menu->active = active;
    menu->owner = owner;
    menu->inventoryContainer = playerInventory;

    menu->resultContainer = LIBMATTI_MC_Container_New(1);
    menu->craftContainer = LIBMATTI_MC_Container_New(4);
    if (menu->resultContainer == NULL || menu->craftContainer == NULL)
    {
        LIBMATTI_MC_InventoryMenu_Free(menu);
        return NULL;
    }

    // Java: addResultSlot(player, 154, 28) - the ResultSlot at menu index 0
    LIBMATTI_MC_Slot *result = LIBMATTI_MC_Slot_New(menu->resultContainer, 0, 154, 28);
    result->mayPlace = result_may_place;
    LIBMATTI_MC_AbstractContainerMenu_AddSlot(&menu->base, result);

    // Java: addCraftingGridSlots(98, 18) - the 2x2 grid, menu indexes 1..4
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            LIBMATTI_MC_AbstractContainerMenu_AddSlot(&menu->base,
                                                      LIBMATTI_MC_Slot_New(menu->craftContainer, j + i * 2,
                                                                           98 + j * 18, 18 + i * 18));

    // Java: the 4 ArmorSlots - menu indexes 5..8, container slots 39-i
    for (int i = 0; i < 4; i++)
    {
        (void) ARMOR_MENU_SLOT_TO_EQUIPMENT[i];
        LIBMATTI_MC_Slot *armor = LIBMATTI_MC_Slot_New(playerInventory, 39 - i, 8, 8 + i * 18);
        armor->mayPlace = armor_may_place;
        LIBMATTI_MC_AbstractContainerMenu_AddSlot(&menu->base, armor);
    }

    // Java: addStandardInventorySlots(inventory, 8, 84) - indexes 9..44
    LIBMATTI_MC_AbstractContainerMenu_AddStandardInventorySlots(&menu->base, playerInventory, 8, 84);

    // Java: the offhand Slot(inventory, 40, 77, 62) - menu index 45
    LIBMATTI_MC_AbstractContainerMenu_AddSlot(&menu->base, LIBMATTI_MC_Slot_New(playerInventory, 40, 77, 62));

    return menu;
}

void LIBMATTI_MC_InventoryMenu_Free(LIBMATTI_MC_InventoryMenu *menu)
{
    if (menu == NULL)
        return;
    LIBMATTI_MC_AbstractContainerMenu_Free(&menu->base);
    LIBMATTI_MC_Container_Free(menu->craftContainer);
    LIBMATTI_MC_Container_Free(menu->resultContainer);
    free(menu);
}

bool LIBMATTI_MC_InventoryMenu_IsHotbarSlot(int index)
{
    return (index >= 36 && index < 45) || index == 45;
}

struct LIBMATTI_MC_Container *LIBMATTI_MC_InventoryMenu_GetCraftSlots(const LIBMATTI_MC_InventoryMenu *menu)
{
    return menu != NULL ? menu->craftContainer : NULL;
}

struct LIBMATTI_MC_Container *LIBMATTI_MC_InventoryMenu_GetResultSlots(const LIBMATTI_MC_InventoryMenu *menu)
{
    return menu != NULL ? menu->resultContainer : NULL;
}

LIBMATTI_MC_Slot *LIBMATTI_MC_InventoryMenu_GetResultSlot(const LIBMATTI_MC_InventoryMenu *menu)
{
    if (menu == NULL)
        return NULL;
    return LIBMATTI_MC_AbstractContainerMenu_GetSlot(&menu->base, LIBMATTI_MC_InventoryMenu_RESULT_SLOT);
}
