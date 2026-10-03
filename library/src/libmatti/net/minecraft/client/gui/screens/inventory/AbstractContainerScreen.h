// Port of net.minecraft.client.gui.screens.inventory.AbstractContainerScreen -
// the container screen base: the menu binding, the leftPos/topPos layout over
// the 176x166 image, the hovered-slot tracking and the click routing into the
// menu's clicked() (the slot index or -999 outside). The carried-item/snapback
// render tails ride the client renderer.

#ifndef MATTICRAFT_MC_CLIENT_GUI_SCREENS_INVENTORY_ABSTRACTCONTAINERSCREEN_H
#define MATTICRAFT_MC_CLIENT_GUI_SCREENS_INVENTORY_ABSTRACTCONTAINERSCREEN_H

#include "libmatti/net/minecraft/client/gui/screens/Screen.h"
#include "libmatti/net/minecraft/world/inventory/AbstractContainerMenu.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: protected int imageWidth = 176 / imageHeight = 166
#define LIBMATTI_MC_AbstractContainerScreen_IMAGE_WIDTH 176
#define LIBMATTI_MC_AbstractContainerScreen_IMAGE_HEIGHT 166

// Java: public abstract class AbstractContainerScreen extends Screen
typedef struct LIBMATTI_MC_AbstractContainerScreen
{
    LIBMATTI_MC_Screen base;
    // Java: protected final T menu
    LIBMATTI_MC_AbstractContainerMenu *menu;
    // Java: protected int leftPos / topPos - the panel origin
    int leftPos;
    int topPos;
    // Java: protected int imageWidth / imageHeight
    int imageWidth;
    int imageHeight;
    // Java: protected @Nullable Slot hoveredSlot
    LIBMATTI_MC_Slot *hoveredSlot;
    // Java: protected int titleLabelX / titleLabelY + inventoryLabelX/Y
    int titleLabelX;
    int titleLabelY;
    int inventoryLabelX;
    int inventoryLabelY;
    // Java: private boolean isQuickCrafting / doubleclick bookkeeping
    bool isQuickCrafting;
    int lastClickSlot;
    int lastClickTime;
} LIBMATTI_MC_AbstractContainerScreen;

// Java: protected AbstractContainerScreen(T menu, Inventory, Component title)
void LIBMATTI_MC_AbstractContainerScreen_Init(LIBMATTI_MC_AbstractContainerScreen *screen,
                                              struct LIBMATTI_MC_Minecraft *minecraft,
                                              LIBMATTI_MC_AbstractContainerMenu *menu, const char *title);
void LIBMATTI_MC_AbstractContainerScreen_Free(LIBMATTI_MC_AbstractContainerScreen *screen);

// Java: protected void init() - the leftPos/topPos over the image size
void LIBMATTI_MC_AbstractContainerScreen_Layout(LIBMATTI_MC_AbstractContainerScreen *screen, int width, int height);

// Java: private @Nullable Slot getHoveredSlot(double, double) - the active
// slot over the 16x16 cell (the screen-relative coords)
LIBMATTI_MC_Slot *LIBMATTI_MC_AbstractContainerScreen_GetHoveredSlot(const LIBMATTI_MC_AbstractContainerScreen *screen,
                                                                     double mouseX, double mouseY);
// Java: protected boolean isHovering(int x, int y, int w, int h, double, double)
bool LIBMATTI_MC_AbstractContainerScreen_IsHovering(const LIBMATTI_MC_AbstractContainerScreen *screen, int x, int y,
                                                    int w, int h, double mouseX, double mouseY);
// Java: protected boolean hasClickedOutside(double, double, int, int)
bool LIBMATTI_MC_AbstractContainerScreen_HasClickedOutside(const LIBMATTI_MC_AbstractContainerScreen *screen,
                                                           double mouseX, double mouseY);
// Java: public boolean mouseClicked(MouseButtonEvent, boolean) - the slot
// routing into menu.clicked(index, button, PICKUP/QUICK_MOVE, player); the
// outside click rides -999, the quick-move rides the right button
bool LIBMATTI_MC_AbstractContainerScreen_MouseClickedScreen(LIBMATTI_MC_AbstractContainerScreen *screen,
                                                            double mouseX, double mouseY, int button,
                                                            struct LIBMATTI_MC_Player *player);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_GUI_SCREENS_INVENTORY_ABSTRACTCONTAINERSCREEN_H
