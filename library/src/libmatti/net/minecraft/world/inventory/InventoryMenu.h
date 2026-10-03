// Port of net.minecraft.world.inventory.InventoryMenu - the player's own menu
// (containerId 0): the 2x2 crafting grid, the 4 armor slots, the 27 main
// inventory slots, the 9 hotbar slots and the offhand slot. The recipe
// matching rides the crafting port; the menu keeps the slot layout + the
// quick-move routing the screens drive.

#ifndef MATTICRAFT_MC_WORLD_INVENTORY_INVENTORYMENU_H
#define MATTICRAFT_MC_WORLD_INVENTORY_INVENTORYMENU_H

#include "libmatti/net/minecraft/world/inventory/AbstractContainerMenu.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: the slot-index constants the quick-move routing and the screens use
#define LIBMATTI_MC_InventoryMenu_CONTAINER_ID 0
#define LIBMATTI_MC_InventoryMenu_RESULT_SLOT 0
#define LIBMATTI_MC_InventoryMenu_CRAFT_SLOT_START 1
#define LIBMATTI_MC_InventoryMenu_CRAFT_SLOT_COUNT 4
#define LIBMATTI_MC_InventoryMenu_CRAFT_SLOT_END 5
#define LIBMATTI_MC_InventoryMenu_ARMOR_SLOT_START 5
#define LIBMATTI_MC_InventoryMenu_ARMOR_SLOT_COUNT 4
#define LIBMATTI_MC_InventoryMenu_ARMOR_SLOT_END 9
#define LIBMATTI_MC_InventoryMenu_INV_SLOT_START 9
#define LIBMATTI_MC_InventoryMenu_INV_SLOT_END 36
#define LIBMATTI_MC_InventoryMenu_USE_ROW_SLOT_START 36
#define LIBMATTI_MC_InventoryMenu_USE_ROW_SLOT_END 45
#define LIBMATTI_MC_InventoryMenu_SHIELD_SLOT 45

// Java: public class InventoryMenu extends AbstractCraftingMenu
typedef struct LIBMATTI_MC_InventoryMenu
{
    LIBMATTI_MC_AbstractContainerMenu base;
    // Java: public final boolean active
    bool active;
    // Java: private final Player owner
    struct LIBMATTI_MC_Player *owner;
    // the crafting/storage containers the slots reference (menu-owned: the
    // result/craft slots stage over them)
    struct LIBMATTI_MC_Container *craftContainer;   // 4 cells (the 2x2 grid)
    struct LIBMATTI_MC_Container *resultContainer;  // 1 cell (the ResultContainer)
    struct LIBMATTI_MC_Container *inventoryContainer; // the player's 36 slots (P6.3 swaps in the real Inventory)
} LIBMATTI_MC_InventoryMenu;

// Java: public InventoryMenu(Inventory, boolean, Player) - the port takes the
// player's inventory container (36 slots) + the owner; the menu owns the
// craft/result staging containers
LIBMATTI_MC_InventoryMenu *LIBMATTI_MC_InventoryMenu_New(struct LIBMATTI_MC_Container *playerInventory,
                                                         bool active, struct LIBMATTI_MC_Player *owner);
void LIBMATTI_MC_InventoryMenu_Free(LIBMATTI_MC_InventoryMenu *menu);

// Java: public static boolean isHotbarSlot(int)
bool LIBMATTI_MC_InventoryMenu_IsHotbarSlot(int index);
// Java: public CraftingContainer getCraftSlots()
struct LIBMATTI_MC_Container *LIBMATTI_MC_InventoryMenu_GetCraftSlots(const LIBMATTI_MC_InventoryMenu *menu);
struct LIBMATTI_MC_Container *LIBMATTI_MC_InventoryMenu_GetResultSlots(const LIBMATTI_MC_InventoryMenu *menu);
// Java: public Slot getResultSlot() / List<Slot> getInputGridSlots()
LIBMATTI_MC_Slot *LIBMATTI_MC_InventoryMenu_GetResultSlot(const LIBMATTI_MC_InventoryMenu *menu);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_INVENTORY_INVENTORYMENU_H
