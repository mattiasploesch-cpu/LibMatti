// Port of net.minecraft.client.renderer.entity.ItemRenderer (the GUI side):
// the item sprite quad + count text the hotbar/inventory slot renders share
// (Java: GuiGraphics.renderItem + renderItemCount over the atlas).

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_ENTITY_ITEMRENDERER_H
#define MATTICRAFT_MC_CLIENT_RENDERER_ENTITY_ITEMRENDERER_H

#include "libmatti/defines.h"

struct LIBMATTI_MC_GuiRenderer;
struct LIBMATTI_FML_SimpleFont;
struct LIBMATTI_MC_TextureAtlas;
struct LIBMATTI_MC_ItemStack;

// Java: GuiGraphics.renderItem(stack, x, y) - the 16x16 sprite quad with the
// count text when count > 1 (the caller's layout scale rides x/y/scale).
void LIBMATTI_MC_ItemRenderer_RenderGuiItem(struct LIBMATTI_MC_GuiRenderer *renderer,
                                            struct LIBMATTI_FML_SimpleFont *font,
                                            const struct LIBMATTI_MC_TextureAtlas *atlas,
                                            const struct LIBMATTI_MC_ItemStack *stack,
                                            float x, float y, float scale);

// Java: Gui.renderHotbar - the widget blit + cell shading + item sprites +
// selection frame (the whole hotbar row the HUD rides; guiWidth/guiHeight are
// the layout size, scale the guiScale).
void LIBMATTI_MC_ItemRenderer_RenderHotbar(struct LIBMATTI_MC_GuiRenderer *renderer,
                                           struct LIBMATTI_FML_SimpleFont *font,
                                           const struct LIBMATTI_MC_TextureAtlas *atlas,
                                           struct LIBMATTI_MC_ItemStack **hotbarItems, int hotbarSize,
                                           int selectedSlot, float guiWidth, float guiHeight, float scale);

#endif //MATTICRAFT_MC_CLIENT_RENDERER_ENTITY_ITEMRENDERER_H
