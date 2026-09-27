// Port of net.minecraft.client.renderer.entity.ItemRenderer (the GUI side):
// Java rides GuiGraphics.renderItem (the ItemRenderer.getFoilBufferDirect
// model quads over the atlas) + renderItemCount over the hotbar/inventory
// slots. The port keeps the sprite-quad + count-text layer: the block item's
// model sprite ("block/<key>") samples the block atlas, the missing sprite
// (missingno) covers unknown ids, the count rides the SimpleFont blits the
// HUD text path already drives.

#include "libmatti/net/minecraft/client/renderer/entity/ItemRenderer.h"

#include "libmatti/net/minecraft/client/gui/GuiRenderer.h"
#include "libmatti/net/minecraft/client/gui/GuiLayout.h"
#include "libmatti/net/minecraft/client/renderer/texture/TextureAtlas.h"
#include "libmatti/net/minecraft/client/resources/model/SpriteGetter.h"
#include "libmatti/net/minecraft/world/item/ItemStack.h"
#include "libmatti/net/minecraft/world/level/block/Block.h"
#include "libmatti/net/minecraft/resources/ResourceKey.h"
#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/net/neoforged/fml/earlydisplay/SimpleFont.h"

#include <stdio.h>

// Java: private void renderQuad(...) - the single tinted sprite quad.
static void render_item_quad(LIBMATTI_MC_GuiRenderer *renderer, const LIBMATTI_MC_TextureAtlas *atlas,
                             const LIBMATTI_MC_ItemStack *stack, float x, float y, float scale)
{
    LIBMATTI_MC_Item *item = LIBMATTI_MC_ItemStack_GetItem(stack);
    if (item == NULL || item->block == NULL)
        return;
    LIBMATTI_MC_ResourceKey *key = LIBMATTI_MC_Block_GetKey((const LIBMATTI_MC_Block *) item->block);
    if (key == NULL)
        return;

    char textureId[128];
    snprintf(textureId, sizeof(textureId), "block/%s", LIBMATTI_MC_Identifier_GetPath(key->identifier));
    float uv[4] = {0.0f, 0.0f, 1.0f, 1.0f};
    if (!LIBMATTI_MC_SpriteGetter_SpriteRect(atlas, textureId, uv))
    {
        // Java: the missingSprite fallback (the checkerboard).
        if (!LIBMATTI_MC_SpriteGetter_SpriteRect(atlas, "missingno", uv))
            return;
    }
    LIBMATTI_MC_GuiRenderer_BlitQuad(renderer, x, y, 16.0f * scale, 16.0f * scale,
                                     uv[0], uv[1], uv[2], uv[3], 0xFFFFFFFFu);
}

// Java: GuiGraphics.renderItemCount - the white count with the dark shadow at
// the cell's bottom right (right-aligned to x+16, baseline y+16).
static void render_item_count(LIBMATTI_MC_GuiRenderer *renderer, LIBMATTI_FML_SimpleFont *font,
                              const char *countText, float x, float y, float scale)
{
    if (font == NULL)
        return;
    int textWidth = LIBMATTI_FML_SimpleFont_StringWidth(font, countText);
    // Java: the shadow first (offset +1,+1, ARGB 0x202020 baked dark)
    LIBMATTI_FML_SimpleFont_DisplayText shadow[1] = {{countText, 0x40202020u}};
    LIBMATTI_FML_SimpleFont_DisplayText text[1] = {{countText, 0xFFFFFFFFu}};
    // Java: the count renders through the font shader (the port's DrawTexts
    // rides the caller's program); the GuiRenderer program stays bound so the
    // blits below keep the batch. The text blits at layout scale.
    LIBMATTI_FML_SimpleFont_DrawTexts(font, x + (16.0f - (float) textWidth) * scale,
                                      y + 8.0f * scale, shadow, 1);
    LIBMATTI_FML_SimpleFont_DrawTexts(font, x + (16.0f - (float) textWidth - 1.0f) * scale,
                                      y + 8.0f * scale, text, 1);
}

void LIBMATTI_MC_ItemRenderer_RenderGuiItem(LIBMATTI_MC_GuiRenderer *renderer, LIBMATTI_FML_SimpleFont *font,
                                            const LIBMATTI_MC_TextureAtlas *atlas,
                                            const LIBMATTI_MC_ItemStack *stack, float x, float y, float scale)
{
    if (renderer == NULL || stack == NULL)
        return;
    if (LIBMATTI_MC_ItemStack_IsEmpty(stack))
        return;

    // Java: the item quad first, the count over it.
    render_item_quad(renderer, atlas, stack, x, y, scale);
    if (stack->count > 1)
    {
        char countText[16];
        snprintf(countText, sizeof(countText), "%d", stack->count);
        render_item_count(renderer, font, countText, x, y, scale);
    }
}

void LIBMATTI_MC_ItemRenderer_RenderHotbar(LIBMATTI_MC_GuiRenderer *renderer, LIBMATTI_FML_SimpleFont *font,
                                           const LIBMATTI_MC_TextureAtlas *atlas,
                                           LIBMATTI_MC_ItemStack **hotbarItems, int hotbarSize,
                                           int selectedSlot, float guiWidth, float guiHeight, float scale)
{
    if (renderer == NULL || hotbarItems == NULL)
        return;

    // Java: Gui.renderHotbar - the widget blit + the cell shading (the
    // tint-only quads sample the white fallback, so no texture rides here).
    int x = 0, y = 0, w = 0, h = 0;
    LIBMATTI_MC_GuiLayout_HotbarRect(guiWidth, guiHeight, &x, &y, &w, &h);
    LIBMATTI_MC_GuiRenderer_BlitQuad(renderer, (float) x * scale, (float) y * scale,
                                     (float) w * scale, (float) h * scale,
                                     0.0f, 0.0f, 1.0f, 1.0f, 0xA0202020u);
    for (int slot = 0; slot < hotbarSize; slot++)
    {
        LIBMATTI_MC_GuiLayout_HotbarSlotRect(guiWidth, guiHeight, slot, &x, &y, &w, &h);
        LIBMATTI_MC_GuiRenderer_BlitQuad(renderer, (float) (x - 1) * scale, (float) (y - 1) * scale,
                                         (float) (w + 2) * scale, (float) (h + 2) * scale,
                                         0.0f, 0.0f, 1.0f, 1.0f, 0x50000000u);
    }
    LIBMATTI_MC_GuiRenderer_Flush(renderer, (float) guiWidth * scale, (float) guiHeight * scale);

    // Java: the item sprites over the cells (one atlas batch for the row).
    if (atlas != NULL)
        LIBMATTI_MC_GuiRenderer_SetTexture(renderer, atlas->base.texture);
    for (int slot = 0; slot < hotbarSize; slot++)
    {
        const LIBMATTI_MC_ItemStack *stack = hotbarItems[slot];
        if (stack == NULL)
            continue;
        LIBMATTI_MC_GuiLayout_HotbarSlotRect(guiWidth, guiHeight, slot, &x, &y, &w, &h);
        LIBMATTI_MC_ItemRenderer_RenderGuiItem(renderer, font, atlas, stack,
                                               (float) x * scale, (float) y * scale, scale);
    }
    LIBMATTI_MC_GuiRenderer_Flush(renderer, (float) guiWidth * scale, (float) guiHeight * scale);
    LIBMATTI_MC_GuiRenderer_SetTexture(renderer, 0);

    // Java: the selection frame AFTER the items (the white frame overlaps the
    // cell and the item like the vanilla path).
    LIBMATTI_MC_GuiLayout_HotbarSelectionRect(guiWidth, guiHeight, selectedSlot, &x, &y, &w, &h);
    LIBMATTI_MC_GuiRenderer_BlitQuad(renderer, (float) x * scale, (float) y * scale,
                                     (float) w * scale, (float) h * scale,
                                     0.0f, 0.0f, 1.0f, 1.0f, 0xE0FFFFFFu);
    LIBMATTI_MC_GuiRenderer_Flush(renderer, (float) guiWidth * scale, (float) guiHeight * scale);
}
