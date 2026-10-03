// The HUD layout unit (see GuiLayout.h) - the Java math folded out of
// Gui.renderCrosshair / Gui.renderItemHotbar / Gui.renderSelectedItemName.

#include "libmatti/net/minecraft/client/gui/GuiLayout.h"

// Java: Inventory.getSelectionSize() - the hotbar carries 9 slots; the
// selectedSlot field clamps into the 0..8 the vanilla hotbar indexes.
#define HOTBAR_SLOT_COUNT 9

static int clamp_slot(int slot)
{
    if (slot < 0)
        return 0;
    if (slot > HOTBAR_SLOT_COUNT - 1)
        return HOTBAR_SLOT_COUNT - 1;
    return slot;
}

void LIBMATTI_MC_GuiLayout_CrosshairRect(int guiWidth, int guiHeight, int *outX, int *outY, int *outW, int *outH)
{
    // Java: blitSprite(CROSSHAIR, (guiWidth - 15) / 2, (guiHeight - 15) / 2, 15, 15)
    if (outX != NULL)
        *outX = (guiWidth - 15) / 2;
    if (outY != NULL)
        *outY = (guiHeight - 15) / 2;
    if (outW != NULL)
        *outW = 15;
    if (outH != NULL)
        *outH = 15;
}

void LIBMATTI_MC_GuiLayout_HotbarRect(int guiWidth, int guiHeight, int *outX, int *outY, int *outW, int *outH)
{
    // Java: int i = guiWidth / 2; blitSprite(HOTBAR, i - 91, guiHeight - 22, 182, 22)
    if (outX != NULL)
        *outX = guiWidth / 2 - 91;
    if (outY != NULL)
        *outY = guiHeight - 22;
    if (outW != NULL)
        *outW = 182;
    if (outH != NULL)
        *outH = 22;
}

void LIBMATTI_MC_GuiLayout_HotbarSelectionRect(int guiWidth, int guiHeight, int selectedSlot,
                                               int *outX, int *outY, int *outW, int *outH)
{
    // Java: blitSprite(HOTBAR_SELECTION, i - 91 - 1 + selectedSlot * 20, guiHeight - 22 - 1, 24, 23)
    int slot = clamp_slot(selectedSlot);
    if (outX != NULL)
        *outX = guiWidth / 2 - 91 - 1 + slot * 20;
    if (outY != NULL)
        *outY = guiHeight - 22 - 1;
    if (outW != NULL)
        *outW = 24;
    if (outH != NULL)
        *outH = 23;
}

void LIBMATTI_MC_GuiLayout_HotbarSlotRect(int guiWidth, int guiHeight, int slot, int *outX, int *outY, int *outW, int *outH)
{
    // Java: renderSlot(..., i - 90 + i1 * 20 + 2, guiHeight - 16 - 3, ...) -
    // the 16x16 item quad the renderSlot path draws at.
    int clamped = clamp_slot(slot);
    if (outX != NULL)
        *outX = guiWidth / 2 - 90 + clamped * 20 + 2;
    if (outY != NULL)
        *outY = guiHeight - 16 - 3;
    if (outW != NULL)
        *outW = 16;
    if (outH != NULL)
        *outH = 16;
}

int LIBMATTI_MC_GuiLayout_SelectedItemNameRect(int guiWidth, int guiHeight, int textWidth, int *outY)
{
    // Java: int i = (guiWidth - font.width(name)) / 2; int k = guiHeight - 59;
    // the skeleton runs creative (gameMode.canHurtPlayer() false), so the name
    // lifts by the 14 pixels the survival bars would occupy.
    if (outY != NULL)
        *outY = guiHeight - 59 + 14;
    return (guiWidth - textWidth) / 2;
}
