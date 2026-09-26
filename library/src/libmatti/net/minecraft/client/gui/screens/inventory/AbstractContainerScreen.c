// Port of net.minecraft.client.gui.screens.inventory.AbstractContainerScreen
// over the Screen base + the menu port. The layout follows Java: the panel
// centres over the 176x166 image, the slot hover rides the 16x16 cell inside
// the 18px pitch, the click routes menu.clicked(index, button, type, player)
// with -999 for the outside drops (the slot coordinates are screen-relative:
// mouse - leftPos / mouse - topPos).

#include "libmatti/net/minecraft/client/gui/screens/inventory/AbstractContainerScreen.h"

#include <stdlib.h>
#include <string.h>

// Java: protected AbstractContainerScreen(T menu, Inventory, Component title)
void LIBMATTI_MC_AbstractContainerScreen_Init(LIBMATTI_MC_AbstractContainerScreen *screen,
                                              struct LIBMATTI_MC_Minecraft *minecraft,
                                              LIBMATTI_MC_AbstractContainerMenu *menu, const char *title)
{
    if (screen == NULL)
        return;
    LIBMATTI_MC_Screen_Init(&screen->base, minecraft, title);
    screen->menu = menu;
    screen->imageWidth = LIBMATTI_MC_AbstractContainerScreen_IMAGE_WIDTH;
    screen->imageHeight = LIBMATTI_MC_AbstractContainerScreen_IMAGE_HEIGHT;
    // Java: this.titleLabelX = 8 / titleLabelY = 6; the inventory label rides
    // imageHeight - 94
    screen->titleLabelX = 8;
    screen->titleLabelY = 6;
    screen->inventoryLabelX = 8;
    screen->inventoryLabelY = screen->imageHeight - 94;
}

void LIBMATTI_MC_AbstractContainerScreen_Free(LIBMATTI_MC_AbstractContainerScreen *screen)
{
    if (screen == NULL)
        return;
    // Java: the teardown rides removed() + the menu close. The struct itself
    // rides the SUBCLASS allocation (C has no destructor chaining) - freeing
    // the base pointer here would free an interior pointer of the
    // InventoryScreen allocation (the SIGSEGV on the first close).
    LIBMATTI_MC_Screen_Cleanup(&screen->base);
    // Java: the menu dies with the screen close (Minecraft.setScreen(null)
    // runs removed() -> menu.removed(player))
    if (screen->menu != NULL)
    {
        LIBMATTI_MC_AbstractContainerMenu_Removed(screen->menu, NULL);
        LIBMATTI_MC_AbstractContainerMenu_Free(screen->menu);
        screen->menu = NULL;
    }
}

void LIBMATTI_MC_AbstractContainerScreen_Layout(LIBMATTI_MC_AbstractContainerScreen *screen, int width, int height)
{
    if (screen == NULL)
        return;
    screen->base.width = width;
    screen->base.height = height;
    // Java: this.leftPos = (this.width - this.imageWidth) / 2
    screen->leftPos = (width - screen->imageWidth) / 2;
    screen->topPos = (height - screen->imageHeight) / 2;
}

// Java: protected boolean isHovering(int x, int y, int w, int h, double, double)
// - the coords go screen-relative first
bool LIBMATTI_MC_AbstractContainerScreen_IsHovering(const LIBMATTI_MC_AbstractContainerScreen *screen, int x, int y,
                                                    int w, int h, double mouseX, double mouseY)
{
    if (screen == NULL)
        return false;
    double relX = mouseX - screen->leftPos;
    double relY = mouseY - screen->topPos;
    return relX >= (double) x - 1.0 && relX < (double) (x + w) + 1.0 && relY >= (double) y - 1.0
           && relY < (double) (y + h) + 1.0;
}

// Java: private @Nullable Slot getHoveredSlot(double, double) - the first
// ACTIVE slot whose 16x16 cell hits
LIBMATTI_MC_Slot *LIBMATTI_MC_AbstractContainerScreen_GetHoveredSlot(const LIBMATTI_MC_AbstractContainerScreen *screen,
                                                                     double mouseX, double mouseY)
{
    if (screen == NULL || screen->menu == NULL)
        return NULL;
    for (int i = 0; i < screen->menu->slotCount; i++)
    {
        LIBMATTI_MC_Slot *slot = screen->menu->slots[i];
        if (LIBMATTI_MC_Slot_IsActive(slot)
            && LIBMATTI_MC_AbstractContainerScreen_IsHovering(screen, slot->x, slot->y, 16, 16, mouseX, mouseY))
            return slot;
    }
    return NULL;
}

// Java: protected boolean hasClickedOutside(double, double, int, int)
bool LIBMATTI_MC_AbstractContainerScreen_HasClickedOutside(const LIBMATTI_MC_AbstractContainerScreen *screen,
                                                           double mouseX, double mouseY)
{
    if (screen == NULL)
        return true;
    return mouseX < (double) screen->leftPos || mouseY < (double) screen->topPos
           || mouseX >= (double) (screen->leftPos + screen->imageWidth)
           || mouseY >= (double) (screen->topPos + screen->imageHeight);
}

// Java: public boolean mouseClicked(MouseButtonEvent, boolean) - the slot
// routing (the PICKUP type for the left button, QUICK_MOVE for the right;
// Java rides ClickAction.PRIMARY/SECONDARY over the same button split)
bool LIBMATTI_MC_AbstractContainerScreen_MouseClickedScreen(LIBMATTI_MC_AbstractContainerScreen *screen,
                                                            double mouseX, double mouseY, int button,
                                                            struct LIBMATTI_MC_Player *player)
{
    if (screen == NULL || screen->menu == NULL)
        return false;
    LIBMATTI_MC_Slot *slot = LIBMATTI_MC_AbstractContainerScreen_GetHoveredSlot(screen, mouseX, mouseY);
    int index = -1;
    if (slot != NULL)
        index = slot->index;
    // Java: the outside click rides -999 (the carried drop)
    if (LIBMATTI_MC_AbstractContainerScreen_HasClickedOutside(screen, mouseX, mouseY))
        index = LIBMATTI_MC_AbstractContainerMenu_SLOT_CLICKED_OUTSIDE;
    if (index == -1)
        return false;
    LIBMATTI_MC_ClickType type = button == 1 ? LIBMATTI_MC_ClickType_QUICK_MOVE : LIBMATTI_MC_ClickType_PICKUP;
    LIBMATTI_MC_AbstractContainerMenu_Clicked(screen->menu, index, button, type, player);
    return true;
}
