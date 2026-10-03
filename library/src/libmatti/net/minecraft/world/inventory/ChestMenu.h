// Port of net.minecraft.world.inventory.ChestMenu - the GENERIC_9xN menu the
// P6.2 screens open (the one concrete menu the port carries for now).

#ifndef MATTICRAFT_MC_WORLD_INVENTORY_CHESTMENU_H
#define MATTICRAFT_MC_WORLD_INVENTORY_CHESTMENU_H

#include "libmatti/net/minecraft/world/inventory/AbstractContainerMenu.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class ChestMenu extends AbstractContainerMenu
typedef struct LIBMATTI_MC_ChestMenu
{
    LIBMATTI_MC_AbstractContainerMenu base;
    // Java: private final Container container
    struct LIBMATTI_MC_Container *container;
    // Java: private final int containerRows
    int containerRows;
} LIBMATTI_MC_ChestMenu;

// Java: public ChestMenu(MenuType<?>, int, Inventory, Container, int) - the
// port takes the rows + the container (the Inventory rides P6.3's port; the
// player inventory slots stage over a caller-owned container)
LIBMATTI_MC_ChestMenu *LIBMATTI_MC_ChestMenu_New(LIBMATTI_MC_MenuType *menuType, int containerId,
                                                 struct LIBMATTI_MC_Container *playerInventory,
                                                 struct LIBMATTI_MC_Container *container, int rows);
void LIBMATTI_MC_ChestMenu_Free(LIBMATTI_MC_ChestMenu *menu);

// Java: the one/five/six/three/four/twoRows statics - the typed factories
LIBMATTI_MC_ChestMenu *LIBMATTI_MC_ChestMenu_OneRow(int containerId, struct LIBMATTI_MC_Container *playerInventory,
                                                    struct LIBMATTI_MC_Container *container);
LIBMATTI_MC_ChestMenu *LIBMATTI_MC_ChestMenu_TwoRows(int containerId, struct LIBMATTI_MC_Container *playerInventory,
                                                     struct LIBMATTI_MC_Container *container);
LIBMATTI_MC_ChestMenu *LIBMATTI_MC_ChestMenu_ThreeRows(int containerId, struct LIBMATTI_MC_Container *playerInventory,
                                                       struct LIBMATTI_MC_Container *container);
LIBMATTI_MC_ChestMenu *LIBMATTI_MC_ChestMenu_FourRows(int containerId, struct LIBMATTI_MC_Container *playerInventory,
                                                      struct LIBMATTI_MC_Container *container);
LIBMATTI_MC_ChestMenu *LIBMATTI_MC_ChestMenu_FiveRows(int containerId, struct LIBMATTI_MC_Container *playerInventory,
                                                      struct LIBMATTI_MC_Container *container);
LIBMATTI_MC_ChestMenu *LIBMATTI_MC_ChestMenu_SixRows(int containerId, struct LIBMATTI_MC_Container *playerInventory,
                                                     struct LIBMATTI_MC_Container *container);

// Java: public Container getContainer() / public int getRowCount()
struct LIBMATTI_MC_Container *LIBMATTI_MC_ChestMenu_GetContainer(const LIBMATTI_MC_ChestMenu *menu);
int LIBMATTI_MC_ChestMenu_GetRowCount(const LIBMATTI_MC_ChestMenu *menu);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_INVENTORY_CHESTMENU_H
