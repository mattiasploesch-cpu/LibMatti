// Port of net.minecraft.client.gui.screens.inventory.InventoryScreen - the
// player's inventory screen over the InventoryMenu (the 2x2 crafting grid +
// armor + main inventory + hotbar). The creative redirect rides the
// instabuild gate the port keeps (the CreativeModeInventoryScreen lands with
// the creative menu port).

#ifndef MATTICRAFT_MC_CLIENT_GUI_SCREENS_INVENTORY_INVENTORYSCREEN_H
#define MATTICRAFT_MC_CLIENT_GUI_SCREENS_INVENTORY_INVENTORYSCREEN_H

#include "libmatti/net/minecraft/client/gui/screens/inventory/AbstractContainerScreen.h"
#include "libmatti/net/minecraft/world/inventory/InventoryMenu.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class InventoryScreen extends AbstractContainerScreen<InventoryMenu>
typedef struct LIBMATTI_MC_InventoryScreen
{
    LIBMATTI_MC_AbstractContainerScreen base;
    // Java: this.menu rides the base; the menu the screen owns
    LIBMATTI_MC_InventoryMenu *inventoryMenu;
} LIBMATTI_MC_InventoryScreen;

// Java: public InventoryScreen(Player) - the menu builds over the player's
// inventory container (the menu owns the craft/result staging containers)
LIBMATTI_MC_InventoryScreen *LIBMATTI_MC_InventoryScreen_New(struct LIBMATTI_MC_Minecraft *minecraft,
                                                             struct LIBMATTI_MC_Player *player,
                                                             struct LIBMATTI_MC_Container *playerInventory);
void LIBMATTI_MC_InventoryScreen_Free(LIBMATTI_MC_InventoryScreen *screen);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_GUI_SCREENS_INVENTORY_INVENTORYSCREEN_H
