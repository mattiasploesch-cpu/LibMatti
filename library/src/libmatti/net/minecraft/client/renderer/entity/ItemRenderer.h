// Port of net.minecraft.client.renderer.entity.ItemRenderer (the GUI side):
// Java rides GuiGraphics.renderItem (the isometric block-icon quads) +
// renderItemDecorations (the count text) over the hotbar/inventory slots.
// The port bakes the isometric cube projection per item into a shared icon
// page (see ItemRenderer.c), so the cells show the classic 3D block icons,
// and the count rides the font shader blits the HUD text path drives.

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_ENTITY_ITEMRENDERER_H
#define MATTICRAFT_MC_CLIENT_RENDERER_ENTITY_ITEMRENDERER_H

#include "libmatti/defines.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct LIBMATTI_MC_GuiRenderer;
struct LIBMATTI_FML_SimpleFont;
struct LIBMATTI_MC_TextureAtlas;
struct LIBMATTI_MC_Minecraft;
struct LIBMATTI_MC_ItemStack;

// Java: the ItemRenderer is a Minecraft field built once with the game - the
// icon page bakes lazily on the first GUI item draw (the GL context is up).
void LIBMATTI_MC_ItemRenderer_Init(struct LIBMATTI_MC_Minecraft *minecraft);
void LIBMATTI_MC_ItemRenderer_Free(void);

// Java: the count draws ride the FontShader program (the compile the
// Minecraft constructor runs) - the attach carries program + uniforms.
void LIBMATTI_MC_ItemRenderer_AttachFont(unsigned int program, int screenSizeLocation, int texLocation);

// Java: GuiGraphics.renderItem(x, y, stack) - the isometric block icon (the
// baked icon quad, packed into the gui batch; the caller flushes).
void LIBMATTI_MC_ItemRenderer_RenderGuiItem(struct LIBMATTI_MC_GuiRenderer *renderer,
                                            struct LIBMATTI_MC_TextureAtlas *atlas,
                                            const struct LIBMATTI_MC_ItemStack *stack, float x, float y,
                                            float scale);

// Java: GuiGraphics.renderItemCount - the immediate font draw (the caller
// runs it AFTER the icon batch flushed so the count lands over the item).
void LIBMATTI_MC_ItemRenderer_RenderGuiCount(struct LIBMATTI_FML_SimpleFont *font,
                                             const struct LIBMATTI_MC_ItemStack *stack, float x, float y,
                                             float scale, float screenW, float screenH);

// Java: Gui.renderHotbar - the widget blit + the cell shading + the icons +
// the counts, one module like the vanilla renderHotbar body.
void LIBMATTI_MC_ItemRenderer_RenderHotbar(struct LIBMATTI_MC_GuiRenderer *renderer,
                                           struct LIBMATTI_FML_SimpleFont *font,
                                           struct LIBMATTI_MC_TextureAtlas *atlas,
                                           struct LIBMATTI_MC_ItemStack **hotbarItems, int hotbarSize,
                                           int selectedSlot, float guiWidth, float guiHeight, float scale);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_ENTITY_ITEMRENDERER_H
