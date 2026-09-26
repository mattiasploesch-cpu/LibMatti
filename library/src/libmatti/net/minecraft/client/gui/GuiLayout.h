// The HUD layout unit (Java: net.minecraft.client.gui.Gui) - the crosshair and
// hotbar geometry as the Java path computes it, folded into pure functions so
// the layout stays testable without a GL context (Java keeps it inline in the
// GuiGraphics calls). The port renders the HUD into the earlydisplay layout
// space (the 854x480 the skeleton drives), so the width/height arguments are
// that fixed 854x480, like the title line.

#ifndef MATTICRAFT_NET_MINECRAFT_CLIENT_GUI_GUILAYOUT_H
#define MATTICRAFT_NET_MINECRAFT_CLIENT_GUI_GUILAYOUT_H

#include "libmatti/defines.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Java: Gui.renderCrosshair - the 15x15 crosshair sprite centred over the
// screen (guiWidth - 15) / 2, (guiHeight - 15) / 2.
void LIBMATTI_MC_GuiLayout_CrosshairRect(int guiWidth, int guiHeight, int *outX, int *outY, int *outW, int *outH);

// Java: Gui.renderItemHotbar - the 182x22 hotbar sprite at (w/2 - 91, h - 22).
void LIBMATTI_MC_GuiLayout_HotbarRect(int guiWidth, int guiHeight, int *outX, int *outY, int *outW, int *outH);

// Java: the 24x23 selection sprite at (w/2 - 92 + slot*20, h - 23); the slot
// clamps into the 0..8 the Inventory.selectedSlot carries.
void LIBMATTI_MC_GuiLayout_HotbarSelectionRect(int guiWidth, int guiHeight, int selectedSlot,
                                               int *outX, int *outY, int *outW, int *outH);

// Java: the renderSlot item quad - 16x16 at (w/2 - 90 + slot*20 + 3, h - 19);
// the slot clamps like the selection rect.
void LIBMATTI_MC_GuiLayout_HotbarSlotRect(int guiWidth, int guiHeight, int slot, int *outX, int *outY, int *outW, int *outH);

// Java: Gui.renderSelectedItemName - the name line centred over the hotbar at
// h - 59 (the creative canHurtPlayer false path lifts it by the 14 pixels).
// Returns the drawn x (the name's left edge for the given pixel width).
int LIBMATTI_MC_GuiLayout_SelectedItemNameRect(int guiWidth, int guiHeight, int textWidth, int *outY);

#ifdef __cplusplus
}
#endif

#endif
