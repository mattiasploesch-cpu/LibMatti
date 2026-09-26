// Port of net.minecraft.world.inventory.ChestMenu over the menu core.

#include "libmatti/net/minecraft/world/inventory/ChestMenu.h"
#include "libmatti/net/minecraft/world/Container.h"

#include <stdlib.h>

// Java: private void addChestGrid(Container, int x, int y) - the 9xrows grid
static void add_chest_grid(LIBMATTI_MC_AbstractContainerMenu *menu, struct LIBMATTI_MC_Container *container,
                           int rows, int x, int y)
{
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < 9; j++)
            LIBMATTI_MC_AbstractContainerMenu_AddSlot(menu, LIBMATTI_MC_Slot_New(container, j + i * 9,
                                                                                 x + j * 18, y + i * 18));
}

// Java: public boolean stillValid(Player) -> container.stillValid(player)
static bool chest_still_valid(const LIBMATTI_MC_AbstractContainerMenu *menu, const struct LIBMATTI_MC_Player *player)
{
    LIBMATTI_MC_ChestMenu *chest = (LIBMATTI_MC_ChestMenu *) menu;
    return LIBMATTI_MC_Container_StillValid(chest->container, player);
}

// Java: public ItemStack quickMoveStack(Player, int) - chest <-> inventory
static LIBMATTI_MC_ItemStack *chest_quick_move(LIBMATTI_MC_AbstractContainerMenu *menu,
                                               struct LIBMATTI_MC_Player *player, int index)
{
    (void) player;
    LIBMATTI_MC_ChestMenu *chest = (LIBMATTI_MC_ChestMenu *) menu;
    LIBMATTI_MC_ItemStack *empty = LIBMATTI_MC_ItemStack_Empty();
    LIBMATTI_MC_Slot *slot = LIBMATTI_MC_AbstractContainerMenu_GetSlot(menu, index);
    if (slot == NULL || !LIBMATTI_MC_Slot_HasItem(slot))
        return empty;

    LIBMATTI_MC_ItemStack *itemStack = LIBMATTI_MC_Slot_GetItem(slot);
    LIBMATTI_MC_ItemStack *copy = LIBMATTI_MC_ItemStack_Copy(itemStack);
    int chestSlots = chest->containerRows * 9;
    if (index < chestSlots)
    {
        // Java: moveItemStackTo(itemstack1, rows*9, slots.size(), true)
        if (!LIBMATTI_MC_AbstractContainerMenu_MoveItemStackTo(menu, itemStack, chestSlots, menu->slotCount, true))
        {
            LIBMATTI_MC_ItemStack_Free(copy);
            return empty;
        }
    }
    else if (!LIBMATTI_MC_AbstractContainerMenu_MoveItemStackTo(menu, itemStack, 0, chestSlots, false))
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
        return empty; // Java: nothing moved
    }

    LIBMATTI_MC_Slot_OnTake(slot, (struct LIBMATTI_MC_Player *) player, itemStack);
    return copy;
}

LIBMATTI_MC_ChestMenu *LIBMATTI_MC_ChestMenu_New(LIBMATTI_MC_MenuType *menuType, int containerId,
                                                 struct LIBMATTI_MC_Container *playerInventory,
                                                 struct LIBMATTI_MC_Container *container, int rows)
{
    LIBMATTI_MC_ChestMenu *menu = calloc(1, sizeof(LIBMATTI_MC_ChestMenu));
    if (menu == NULL)
        return NULL;
    LIBMATTI_MC_AbstractContainerMenu_Init(&menu->base, menuType, containerId);
    menu->base.quickMoveStack = chest_quick_move;
    menu->base.stillValid = chest_still_valid;
    menu->container = container;
    menu->containerRows = rows;

    // Java: checkContainerSize(container, rows * 9) - the port asserts softly
    if (LIBMATTI_MC_Container_GetContainerSize(container) < rows * 9)
    {
        LIBMATTI_MC_AbstractContainerMenu_Free(&menu->base);
        free(menu);
        return NULL;
    }

    add_chest_grid(&menu->base, container, rows, 8, 18);
    // Java: int j = 18 + containerRows * 18 + 13 - the inventory below the grid
    LIBMATTI_MC_AbstractContainerMenu_AddStandardInventorySlots(&menu->base, playerInventory, 8,
                                                                18 + rows * 18 + 13);
    return menu;
}

void LIBMATTI_MC_ChestMenu_Free(LIBMATTI_MC_ChestMenu *menu)
{
    if (menu == NULL)
        return;
    LIBMATTI_MC_AbstractContainerMenu_Free(&menu->base);
    free(menu);
}

// Java: the rows statics - one factory per MenuType constant
LIBMATTI_MC_ChestMenu *LIBMATTI_MC_ChestMenu_OneRow(int containerId, struct LIBMATTI_MC_Container *playerInventory,
                                                    struct LIBMATTI_MC_Container *container)
{
    return LIBMATTI_MC_ChestMenu_New(LIBMATTI_MC_MenuType_GENERIC_9x1(), containerId, playerInventory, container, 1);
}

LIBMATTI_MC_ChestMenu *LIBMATTI_MC_ChestMenu_TwoRows(int containerId, struct LIBMATTI_MC_Container *playerInventory,
                                                     struct LIBMATTI_MC_Container *container)
{
    return LIBMATTI_MC_ChestMenu_New(LIBMATTI_MC_MenuType_GENERIC_9x2(), containerId, playerInventory, container, 2);
}

LIBMATTI_MC_ChestMenu *LIBMATTI_MC_ChestMenu_ThreeRows(int containerId, struct LIBMATTI_MC_Container *playerInventory,
                                                       struct LIBMATTI_MC_Container *container)
{
    return LIBMATTI_MC_ChestMenu_New(LIBMATTI_MC_MenuType_GENERIC_9x3(), containerId, playerInventory, container, 3);
}

LIBMATTI_MC_ChestMenu *LIBMATTI_MC_ChestMenu_FourRows(int containerId, struct LIBMATTI_MC_Container *playerInventory,
                                                      struct LIBMATTI_MC_Container *container)
{
    return LIBMATTI_MC_ChestMenu_New(LIBMATTI_MC_MenuType_GENERIC_9x4(), containerId, playerInventory, container, 4);
}

LIBMATTI_MC_ChestMenu *LIBMATTI_MC_ChestMenu_FiveRows(int containerId, struct LIBMATTI_MC_Container *playerInventory,
                                                      struct LIBMATTI_MC_Container *container)
{
    return LIBMATTI_MC_ChestMenu_New(LIBMATTI_MC_MenuType_GENERIC_9x5(), containerId, playerInventory, container, 5);
}

LIBMATTI_MC_ChestMenu *LIBMATTI_MC_ChestMenu_SixRows(int containerId, struct LIBMATTI_MC_Container *playerInventory,
                                                     struct LIBMATTI_MC_Container *container)
{
    return LIBMATTI_MC_ChestMenu_New(LIBMATTI_MC_MenuType_GENERIC_9x6(), containerId, playerInventory, container, 6);
}

struct LIBMATTI_MC_Container *LIBMATTI_MC_ChestMenu_GetContainer(const LIBMATTI_MC_ChestMenu *menu)
{
    return menu != NULL ? menu->container : NULL;
}

int LIBMATTI_MC_ChestMenu_GetRowCount(const LIBMATTI_MC_ChestMenu *menu)
{
    return menu != NULL ? menu->containerRows : 0;
}
