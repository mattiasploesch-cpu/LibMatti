// Port of net.minecraft.client.gui.screens.inventory.InventoryScreen over the
// container screen base. The layout rides the InventoryMenu's 46 slots; the
// title ("container.crafting") renders at the vanilla label position.

#include "libmatti/net/minecraft/client/gui/screens/inventory/InventoryScreen.h"

#include <stdlib.h>

LIBMATTI_MC_InventoryScreen *LIBMATTI_MC_InventoryScreen_New(struct LIBMATTI_MC_Minecraft *minecraft,
                                                             struct LIBMATTI_MC_Player *player,
                                                             struct LIBMATTI_MC_Container *playerInventory)
{
    LIBMATTI_MC_InventoryScreen *screen = calloc(1, sizeof(LIBMATTI_MC_InventoryScreen));
    if (screen == NULL)
        return NULL;
    // Java: super(player.inventoryMenu, ..., Component.translatable("container.crafting"))
    screen->inventoryMenu = LIBMATTI_MC_InventoryMenu_New(playerInventory, true, player);
    if (screen->inventoryMenu == NULL)
    {
        free(screen);
        return NULL;
    }
    LIBMATTI_MC_AbstractContainerScreen_Init(&screen->base, minecraft, &screen->inventoryMenu->base,
                                             "container.crafting");
    // Java: this.titleLabelX = 97 (the crafting title rides the grid)
    screen->base.titleLabelX = 97;
    screen->base.titleLabelY = 28;
    return screen;
}

void LIBMATTI_MC_InventoryScreen_Free(LIBMATTI_MC_InventoryScreen *screen)
{
    if (screen == NULL)
        return;
    // the menu/children die through the base cleanup; the SUBCLASS struct
    // (the allocation the base rides in) dies here - the base free no longer
    // frees the struct itself (the interior-pointer SIGSEGV fix)
    LIBMATTI_MC_AbstractContainerScreen_Free(&screen->base);
    free(screen);
}
