// Port of net.minecraft.client.renderer.entity.ItemRenderer (the GUI side):
// Java's renderItem projects the block model's three visible faces (top, the
// two south/east sides) into the 16x16 GUI cell - the isometric cube the
// hotbar shows for every block item. The port bakes that projection per item
// on the CPU out of the atlas texels (the top/side sheets the blocks'
// textures carry) into ONE shared icon page, then draws each icon as a plain
// quad over that page: the classic 3D block icon without a 3D pipeline.
// The count rides the font shader blits the HUD text path already drives -
// the caller draws it AFTER the icon batch flushes (Java:
// renderSlot -> renderItemDecorations over the item model).
//
// The bake inverts the vanilla flat-model projection the GUI renders carry:
// the top face squashed to the 2:1 diamond, the side faces sheared down and
// shaded (1.0 / 0.8 / 0.6 like the model quads' face brightness).

#include "libmatti/net/minecraft/client/renderer/entity/ItemRenderer.h"

#include "libmatti/net/minecraft/client/Minecraft.h"
#include "libmatti/net/minecraft/client/gui/GuiRenderer.h"
#include "libmatti/net/minecraft/client/gui/GuiLayout.h"
#include "libmatti/net/minecraft/client/renderer/texture/TextureAtlas.h"
#include "libmatti/net/minecraft/client/renderer/texture/AbstractTexture.h"
#include "libmatti/net/minecraft/client/resources/model/SpriteGetter.h"
#include "libmatti/net/minecraft/world/item/ItemStack.h"
#include "libmatti/net/minecraft/world/level/block/Block.h"
#include "libmatti/net/minecraft/resources/ResourceKey.h"
#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/net/neoforged/fml/earlydisplay/SimpleFont.h"
#include "libmatti/com/mojang/blaze3d/opengl/GlStateManager.h"
#include "libmatti/org/lwjgl/opengl/Constants.h"
#include "libmatti/org/lwjgl/opengl/GL.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: the icon page rides the atlas' stitched layout (one 4096 wide row of
// 32x32 icons covers the palette's items; the height grows with the count
// like the stitcher's page grows with the sprites).
#define ICON_SIZE 32
#define ICON_PAGE_WIDTH 4096
#define ICON_PAGE_HEIGHT 128
#define ICONS_PER_ROW (ICON_PAGE_WIDTH / ICON_SIZE)

typedef struct LIBMATTI_MC_ItemRenderer
{
    unsigned int pageTexture; // 0 until the first bake (the lazy GL init)
    int iconCount;            // the baked icons (the page's used cells)
    // Java: the baked icons map - the port keys by block path (linear scan;
    // the palette stays small).
    char(*iconKeys)[64];
    int iconCapacity;
    // Java: the ItemRenderer renders the count through the shared FontShader
    // program (the attach the Minecraft constructor runs once).
    unsigned int fontProgram;
    int fontScreenSizeLocation;
    int fontTexLocation;
} LIBMATTI_MC_ItemRenderer;

static LIBMATTI_MC_ItemRenderer itemRenderer;

// The pixel fetcher the face sampling rides (the face textures come out of
// the atlas readback below).
typedef struct AtlasPixels
{
    unsigned char *rgba; // width * height * 4
    int width;
    int height;
} AtlasPixels;

// The readback the bake rides (glGetTexImage over the bound atlas). The bake
// runs MID-PASS (the GUI draws ride the layout FBO), so the draw/read
// framebuffer bindings go back before the caller continues - an unbound FBO
// here sent the rest of the frame into the window back buffer (the all-black
// capture the first bake run showed).
static int atlas_pixels_bind(const LIBMATTI_MC_TextureAtlas *atlas, AtlasPixels *out)
{
    if (atlas == NULL || atlas->base.texture == 0)
        return 0;
    int width = LIBMATTI_MC_TextureAtlas_GetWidth(atlas);
    int height = LIBMATTI_MC_TextureAtlas_GetHeight(atlas);
    if (width <= 0 || height <= 0)
        return 0;
    unsigned char *pixels = malloc((size_t) width * (size_t) height * 4u);
    if (pixels == NULL)
        return 0;
    // GL_DRAW_FRAMEBUFFER (36009) / GL_READ_FRAMEBUFFER (36008) - the same
    // targets the screenshot path queries.
    unsigned int drawFbo = LIBMATTI_B3D_GlStateManager_GetFrameBuffer(36009);
    unsigned int readFbo = LIBMATTI_B3D_GlStateManager_GetFrameBuffer(36008);
    LIBMATTI_B3D_GlStateManager_ActiveTexture(LIBMATTI_GL_GL_TEXTURE0);
    LIBMATTI_B3D_GlStateManager_BindTexture((int) atlas->base.texture);
    LIBMATTI_GL_glPixelStorei(LIBMATTI_GL_GL_PACK_ALIGNMENT, 1);
    LIBMATTI_GL_glGetTexImage(LIBMATTI_GL_GL_TEXTURE_2D, 0, LIBMATTI_GL_GL_RGBA,
                              LIBMATTI_GL_GL_UNSIGNED_BYTE, pixels);
    LIBMATTI_GL_glPixelStorei(LIBMATTI_GL_GL_PACK_ALIGNMENT, 4);
    LIBMATTI_B3D_GlStateManager_BindTexture(0);
    LIBMATTI_B3D_GlStateManager_BindFramebuffer(36009, drawFbo);
    LIBMATTI_B3D_GlStateManager_BindFramebuffer(36008, readFbo);
    out->rgba = pixels;
    out->width = width;
    out->height = height;
    return 1;
}

static void atlas_pixels_release(AtlasPixels *pixels)
{
    free(pixels->rgba);
    pixels->rgba = NULL;
}

static void atlas_pixel(const AtlasPixels *pixels, int x, int y, unsigned char out[4])
{
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if (x >= pixels->width)
        x = pixels->width - 1;
    if (y >= pixels->height)
        y = pixels->height - 1;
    const unsigned char *p = pixels->rgba + ((size_t) y * (size_t) pixels->width + (size_t) x) * 4u;
    out[0] = p[0];
    out[1] = p[1];
    out[2] = p[2];
    out[3] = p[3];
}

// Java: SpriteContents' pixel fetch - the sprite's top-left + the atlas row
// stride resolve the sprite-local texel; sx/sy tile (sand/gravel repeat the
// sheet), shade is the face brightness the model quads ride.
static void sprite_pixel(const AtlasPixels *atlas, float u0, float v0, int sx, int sy, float shade,
                         unsigned char out[4])
{
    int u = sx & 15;
    int v = sy & 15;
    unsigned char p[4];
    atlas_pixel(atlas, (int) (u0 * (float) atlas->width) + u, (int) (v0 * (float) atlas->height) + v, p);
    out[0] = (unsigned char) ((float) p[0] * shade);
    out[1] = (unsigned char) ((float) p[1] * shade);
    out[2] = (unsigned char) ((float) p[2] * shade);
    out[3] = p[3];
}

// The face rect lookup (Java: the block model's texture references). The
// atlas keys the sprites "block/<path>" (the model texture ids the flat
// path rides) - the <block>_top sheets the log models reference, the base
// sheet everywhere else.
static int block_face_rects(const LIBMATTI_MC_TextureAtlas *atlas, const char *blockPath,
                            float top[4], float side[4])
{
    if (atlas == NULL || blockPath == NULL)
        return 0;
    // Java: ModelBakery's block states - the port keys the top sheets off the
    // path suffixes the vanilla block models reference ("oak_log" renders
    // the "oak_log_top" sheet on the diamond).
    static const char *const logSuffix = "_log";
    size_t pathLen = strlen(blockPath);
    size_t logLen = strlen(logSuffix);
    if (pathLen > logLen && strcmp(blockPath + pathLen - logLen, logSuffix) == 0)
    {
        char topId[80];
        if (snprintf(topId, sizeof(topId), "block/%s_top", blockPath) < (int) sizeof(topId)
            && LIBMATTI_MC_SpriteGetter_SpriteRect(atlas, topId, top))
        {
            char sideId[80];
            if (snprintf(sideId, sizeof(sideId), "block/%s", blockPath) < (int) sizeof(sideId)
                && LIBMATTI_MC_SpriteGetter_SpriteRect(atlas, sideId, side))
                return 1;
        }
    }
    // the base sheet on all three faces (stone, dirt, planks, sand, ...)
    char baseId[80];
    if (snprintf(baseId, sizeof(baseId), "block/%s", blockPath) >= (int) sizeof(baseId))
        return 0;
    if (!LIBMATTI_MC_SpriteGetter_SpriteRect(atlas, baseId, top))
        return 0;
    side[0] = top[0];
    side[1] = top[1];
    side[2] = top[2];
    side[3] = top[3];
    return 1;
}

// The isometric projection the GUI item renders carry - the flat model's
// three visible faces over the 2:1 dimetric axes (cos30 on x, 0.5 on y): the
// top diamond, the two side faces sheared down. The port scans every
// destination pixel back through the projection into the face textures (the
// inverse the GPU interpolators forward-interpolate):
//
//   forward (cube-local, the apex at (0,0)):
//     top face:   x = (u - v) * 0.8660254, y = (u + v) * 0.5      u,v in [0,16]
//     left face:  x = u * 0.8660254 - 13.856, y = 8 + u * 0.5 + w u,w in [0,16]
//     right face: x = u * 0.8660254, y = 16 - u * 0.5 + w         u,w in [0,16]
//
//   inverse (what the scan solves per pixel):
//     top:   u = y + x * 0.57735, v = y - x * 0.57735   (0.57735 = 1/(2*cos30))
//     left:  u = (x + 13.856) / 0.8660254, w = y - 8 - u * 0.5
//     right: u = x / 0.8660254, w = y - 16 + u * 0.5
//
// The cube spans x in [-13.856, +13.856], y in [0, 32] - the scan centres it
// at icon px 16 with the apex at row 0 (the vanilla cell layout).
static void bake_icon_pixels(const AtlasPixels *atlas, float top[4], float side[4],
                             unsigned char *iconRgba /* ICON_SIZE*ICON_SIZE*4 */)
{
    memset(iconRgba, 0, (size_t) ICON_SIZE * ICON_SIZE * 4);
    for (int y = 0; y < ICON_SIZE; y++)
    {
        for (int x = 0; x < ICON_SIZE; x++)
        {
            float bx = (float) x - 16.0f;
            float by = (float) y;
            float u = 0.0f, coord = 0.0f;
            int face = -1;
            float shade = 1.0f;
            const float *rect = top;
            // the top diamond: u,v in [0,16) over the sheet
            u = by + bx * 0.57735f;
            coord = by - bx * 0.57735f;
            if (u >= 0.0f && u < 16.0f && coord >= 0.0f && coord < 16.0f)
                face = 0;
            if (face < 0)
            {
                // the left (south) face: u along the top edge, w down
                u = (bx + 13.8564f) / 0.8660254f;
                coord = by - 8.0f - u * 0.5f;
                if (u >= 0.0f && u < 16.0f && coord >= 0.0f && coord < 16.0f)
                {
                    face = 1;
                    shade = 0.8f;
                    rect = side;
                }
            }
            if (face < 0)
            {
                // the right (east) face: u along the top edge, w down
                u = bx / 0.8660254f;
                coord = by - 16.0f + u * 0.5f;
                if (u >= 0.0f && u < 16.0f && coord >= 0.0f && coord < 16.0f)
                {
                    face = 2;
                    shade = 0.6f;
                    rect = side;
                }
            }
            if (face < 0)
                continue; // the transparent cell corners
            unsigned char p[4];
            sprite_pixel(atlas, rect[0], rect[1], (int) u, (int) coord, shade, p);
            unsigned char *dst = iconRgba + ((size_t) y * ICON_SIZE + (size_t) x) * 4u;
            dst[0] = p[0];
            dst[1] = p[1];
            dst[2] = p[2];
            dst[3] = p[3];
        }
    }
}

static int ensure_page_texture(void)
{
    if (itemRenderer.pageTexture != 0)
        return 1;
    unsigned int texture = 0;
    LIBMATTI_GL_glGenTextures(1, &texture);
    if (texture == 0)
        return 0;
    LIBMATTI_B3D_GlStateManager_BindTexture((int) texture);
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_MIN_FILTER,
                                LIBMATTI_GL_GL_NEAREST);
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_MAG_FILTER,
                                LIBMATTI_GL_GL_NEAREST);
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_WRAP_S,
                                LIBMATTI_GL_GL_CLAMP_TO_EDGE);
    LIBMATTI_GL_glTexParameteri(LIBMATTI_GL_GL_TEXTURE_2D, LIBMATTI_GL_GL_TEXTURE_WRAP_T,
                                LIBMATTI_GL_GL_CLAMP_TO_EDGE);
    LIBMATTI_GL_glTexImage2D(LIBMATTI_GL_GL_TEXTURE_2D, 0, LIBMATTI_GL_GL_RGBA8, ICON_PAGE_WIDTH,
                             ICON_PAGE_HEIGHT, 0, LIBMATTI_GL_GL_RGBA, LIBMATTI_GL_GL_UNSIGNED_BYTE, NULL);
    LIBMATTI_B3D_GlStateManager_BindTexture(0);
    itemRenderer.pageTexture = texture;
    return 1;
}

// Java: the baked icon lookup (the block path -> the icon index).
static int find_icon(const char *blockPath)
{
    for (int i = 0; i < itemRenderer.iconCount; i++)
        if (strncmp(itemRenderer.iconKeys[i], blockPath, sizeof(itemRenderer.iconKeys[0]) - 1) == 0)
            return i;
    return -1;
}

// The bake path: the atlas readback -> the projection scan -> the page upload
// -> the icon index (or -1; the caller skips the icon).
static int bake_icon(LIBMATTI_MC_TextureAtlas *atlas, const char *blockPath)
{
    if (!ensure_page_texture())
        return -1;
    int index = find_icon(blockPath);
    if (index >= 0)
        return index;

    AtlasPixels atlasPixels;
    if (!atlas_pixels_bind(atlas, &atlasPixels))
        return -1;

    float top[4], side[4];
    int haveFaces = block_face_rects(atlas, blockPath, top, side);
    // MATTI_GUI_DEBUG: the bake audit - which sheets resolved per icon (the
    // missingno intrusions the pixel verification hunts) + the atlas/icon
    // page dumps the texel-space verification rides (the page dump overwrites
    // per bake; the last bake's file carries the complete page).
    if (getenv("MATTI_GUI_DEBUG") != NULL)
    {
        fprintf(stderr, "[ICONBAKE] %s faces=%d top=(%.3f,%.3f) side=(%.3f,%.3f)\n", blockPath, haveFaces,
                haveFaces ? top[0] : -1.0f, haveFaces ? top[1] : -1.0f,
                haveFaces ? side[0] : -1.0f, haveFaces ? side[1] : -1.0f);
        if (itemRenderer.iconCount == 0)
        {
            FILE *atlasDump = fopen("/tmp/atlaspage.ppm", "wb");
            if (atlasDump != NULL)
            {
                fprintf(atlasDump, "P6\n%d %d\n255\n", atlasPixels.width, atlasPixels.height);
                for (int y = 0; y < atlasPixels.height; y++)
                    for (int x = 0; x < atlasPixels.width; x++)
                        fwrite(atlasPixels.rgba + ((size_t) y * (size_t) atlasPixels.width + (size_t) x) * 4u,
                               1, 3, atlasDump);
                fclose(atlasDump);
                fprintf(stderr, "[ICONBAKE] wrote /tmp/atlaspage.ppm (%dx%d)\n", atlasPixels.width,
                        atlasPixels.height);
            }
        }
    }
    unsigned char *icon = malloc((size_t) ICON_SIZE * ICON_SIZE * 4);
    if (icon == NULL)
    {
        atlas_pixels_release(&atlasPixels);
        return -1;
    }
    if (haveFaces)
    {
        bake_icon_pixels(&atlasPixels, top, side, icon);
    }
    else
    {
        // the missing-model fallback: the flat sprite sheet (the missingno
        // checkerboard the atlas carries) scaled into the cell.
        memset(icon, 0, (size_t) ICON_SIZE * ICON_SIZE * 4);
        float uv[4] = {0.0f, 0.0f, 1.0f, 1.0f};
        if (LIBMATTI_MC_SpriteGetter_SpriteRect(atlas, "missingno", uv))
        {
            for (int y = 0; y < ICON_SIZE; y++)
                for (int x = 0; x < ICON_SIZE; x++)
                {
                    unsigned char p[4];
                    sprite_pixel(&atlasPixels, uv[0], uv[1], (x * 16) / ICON_SIZE, (y * 16) / ICON_SIZE,
                                 1.0f, p);
                    unsigned char *dst = icon + ((size_t) y * ICON_SIZE + (size_t) x) * 4u;
                    dst[0] = p[0];
                    dst[1] = p[1];
                    dst[2] = p[2];
                    dst[3] = 255;
                }
        }
    }
    atlas_pixels_release(&atlasPixels);

    // the page cell (the stitcher's next free slot)
    int slot = itemRenderer.iconCount;
    if (slot >= ICONS_PER_ROW * (ICON_PAGE_HEIGHT / ICON_SIZE))
    {
        free(icon);
        return -1;
    }
    LIBMATTI_B3D_GlStateManager_BindTexture((int) itemRenderer.pageTexture);
    LIBMATTI_GL_glPixelStorei(LIBMATTI_GL_GL_UNPACK_ALIGNMENT, 1);
    LIBMATTI_GL_glTexSubImage2D(LIBMATTI_GL_GL_TEXTURE_2D, 0, (slot % ICONS_PER_ROW) * ICON_SIZE,
                                (slot / ICONS_PER_ROW) * ICON_SIZE, ICON_SIZE, ICON_SIZE,
                                LIBMATTI_GL_GL_RGBA, LIBMATTI_GL_GL_UNSIGNED_BYTE, icon);
    LIBMATTI_GL_glPixelStorei(LIBMATTI_GL_GL_UNPACK_ALIGNMENT, 4);
    LIBMATTI_B3D_GlStateManager_BindTexture(0);
    free(icon);

    // the cache entry
    if (itemRenderer.iconCount >= itemRenderer.iconCapacity)
    {
        int capacity = itemRenderer.iconCapacity != 0 ? itemRenderer.iconCapacity * 2 : 32;
        char(*grown)[64] = realloc(itemRenderer.iconKeys, (size_t) capacity * sizeof(*grown));
        if (grown == NULL)
            return -1;
        itemRenderer.iconKeys = grown;
        itemRenderer.iconCapacity = capacity;
    }
    snprintf(itemRenderer.iconKeys[itemRenderer.iconCount], sizeof(itemRenderer.iconKeys[0]), "%s",
             blockPath);
    itemRenderer.iconCount++;
    // MATTI_GUI_DEBUG: the per-bake page dump (the texel-space verification -
    // the bake pixels readable without any screen mapping; the LAST bake's
    // file carries the complete page).
    if (getenv("MATTI_GUI_DEBUG") != NULL)
    {
        unsigned char *page = malloc((size_t) ICON_PAGE_WIDTH * ICON_PAGE_HEIGHT * 4u);
        if (page != NULL)
        {
            LIBMATTI_B3D_GlStateManager_BindTexture((int) itemRenderer.pageTexture);
            LIBMATTI_GL_glPixelStorei(LIBMATTI_GL_GL_PACK_ALIGNMENT, 1);
            LIBMATTI_GL_glGetTexImage(LIBMATTI_GL_GL_TEXTURE_2D, 0, LIBMATTI_GL_GL_RGBA,
                                      LIBMATTI_GL_GL_UNSIGNED_BYTE, page);
            LIBMATTI_GL_glPixelStorei(LIBMATTI_GL_GL_PACK_ALIGNMENT, 4);
            LIBMATTI_B3D_GlStateManager_BindTexture(0);
            FILE *dump = fopen("/tmp/iconpage.ppm", "wb");
            if (dump != NULL)
            {
                fprintf(dump, "P6\n%d %d\n255\n", ICON_PAGE_WIDTH, ICON_PAGE_HEIGHT);
                for (int y = 0; y < ICON_PAGE_HEIGHT; y++)
                    for (int x = 0; x < ICON_PAGE_WIDTH; x++)
                        fwrite(page + ((size_t) y * ICON_PAGE_WIDTH + x) * 4u, 1, 3, dump);
                fclose(dump);
                fprintf(stderr, "[ICONBAKE] wrote /tmp/iconpage.ppm\n");
            }
            free(page);
        }
    }
    return slot;
}

void LIBMATTI_MC_ItemRenderer_Init(struct LIBMATTI_MC_Minecraft *minecraft)
{
    (void) minecraft;
    memset(&itemRenderer, 0, sizeof(itemRenderer));
}

void LIBMATTI_MC_ItemRenderer_Free(void)
{
    if (itemRenderer.pageTexture != 0)
        LIBMATTI_GL_glDeleteTextures(1, &itemRenderer.pageTexture);
    free(itemRenderer.iconKeys);
    memset(&itemRenderer, 0, sizeof(itemRenderer));
}

// Java: the ItemRenderer renders the count through the shared FontShader
// program - the attach carries the program + the two uniforms the text path
// needs (the compile the Minecraft constructor runs).
void LIBMATTI_MC_ItemRenderer_AttachFont(unsigned int program, int screenSizeLocation, int texLocation)
{
    itemRenderer.fontProgram = program;
    itemRenderer.fontScreenSizeLocation = screenSizeLocation;
    itemRenderer.fontTexLocation = texLocation;
}

// Java: GuiGraphics.renderItemCount - the white count with the dark shadow at
// the cell's bottom right (right-aligned to x+16, baseline y+16). The draw is
// IMMEDIATE (the font program's own batch) - the caller runs it after the
// icon batch flushed so the count lands over the item like vanilla's
// renderSlot -> renderItemDecorations order.
void LIBMATTI_MC_ItemRenderer_RenderGuiCount(struct LIBMATTI_FML_SimpleFont *font,
                                             const struct LIBMATTI_MC_ItemStack *stack, float x, float y,
                                             float scale, float screenW, float screenH)
{
    if (font == NULL || stack == NULL || stack->count <= 1)
        return;
    if (itemRenderer.fontProgram == 0)
        return;
    char countText[16];
    snprintf(countText, sizeof(countText), "%d", stack->count);
    int textWidth = LIBMATTI_FML_SimpleFont_StringWidth(font, countText);
    // Java: the shadow first (offset +1,+1, ARGB 0x202020 baked dark)
    LIBMATTI_FML_SimpleFont_DisplayText shadow[1] = {{countText, 0x40202020u}};
    LIBMATTI_FML_SimpleFont_DisplayText text[1] = {{countText, 0xFFFFFFFFu}};
    LIBMATTI_GL_glUseProgram(itemRenderer.fontProgram);
    if (itemRenderer.fontScreenSizeLocation >= 0)
        LIBMATTI_GL_glUniform2f(itemRenderer.fontScreenSizeLocation, screenW, screenH);
    LIBMATTI_B3D_GlStateManager_ActiveTexture(LIBMATTI_GL_GL_TEXTURE0);
    LIBMATTI_B3D_GlStateManager_BindTexture((int) LIBMATTI_FML_SimpleFont_TextureId(font));
    if (itemRenderer.fontTexLocation >= 0)
        LIBMATTI_GL_glUniform1i(itemRenderer.fontTexLocation, 0);
    LIBMATTI_FML_SimpleFont_DrawTexts(font, x + (16.0f - (float) textWidth) * scale,
                                      y + 8.0f * scale, shadow, 1);
    LIBMATTI_FML_SimpleFont_DrawTexts(font, x + (16.0f - (float) textWidth - 1.0f) * scale,
                                      y + 8.0f * scale, text, 1);
    LIBMATTI_GL_glUseProgram(0);
}

// Java: GuiGraphics.renderItem - the isometric icon quad (the baked page
// sampling). The quad only PACKS into the gui batch - the caller flushes the
// row's icons as one atlas draw.
static void render_icon_quad(struct LIBMATTI_MC_GuiRenderer *renderer, struct LIBMATTI_MC_TextureAtlas *atlas,
                             const struct LIBMATTI_MC_ItemStack *stack, float x, float y, float scale)
{
    struct LIBMATTI_MC_Item *item = LIBMATTI_MC_ItemStack_GetItem(stack);
    if (item == NULL || item->block == NULL)
        return;
    struct LIBMATTI_MC_ResourceKey *key =
        LIBMATTI_MC_Block_GetKey((const struct LIBMATTI_MC_Block *) item->block);
    if (key == NULL)
        return;
    const char *blockPath = LIBMATTI_MC_Identifier_GetPath(key->identifier);

    int icon = bake_icon(atlas, blockPath);
    if (icon < 0)
        return;
    float u0 = (float) ((icon % ICONS_PER_ROW) * ICON_SIZE) / (float) ICON_PAGE_WIDTH;
    float v0 = (float) ((icon / ICONS_PER_ROW) * ICON_SIZE) / (float) ICON_PAGE_HEIGHT;
    float u1 = u0 + (float) ICON_SIZE / (float) ICON_PAGE_WIDTH;
    float v1 = v0 + (float) ICON_SIZE / (float) ICON_PAGE_HEIGHT;
    LIBMATTI_MC_GuiRenderer_SetTexture(renderer, itemRenderer.pageTexture);
    LIBMATTI_MC_GuiRenderer_BlitQuad(renderer, x, y, 16.0f * scale, 16.0f * scale,
                                     u0, v0, u1, v1, 0xFFFFFFFFu);
}

void LIBMATTI_MC_ItemRenderer_RenderGuiItem(struct LIBMATTI_MC_GuiRenderer *renderer,
                                            struct LIBMATTI_MC_TextureAtlas *atlas,
                                            const struct LIBMATTI_MC_ItemStack *stack, float x, float y,
                                            float scale)
{
    if (renderer == NULL || stack == NULL)
        return;
    if (LIBMATTI_MC_ItemStack_IsEmpty(stack))
        return;
    render_icon_quad(renderer, atlas, stack, x, y, scale);
}

void LIBMATTI_MC_ItemRenderer_RenderHotbar(struct LIBMATTI_MC_GuiRenderer *renderer,
                                           struct LIBMATTI_FML_SimpleFont *font,
                                           struct LIBMATTI_MC_TextureAtlas *atlas,
                                           struct LIBMATTI_MC_ItemStack **hotbarItems, int hotbarSize,
                                           int selectedSlot, float guiWidth, float guiHeight, float scale)
{
    if (renderer == NULL || hotbarItems == NULL)
        return;
    float screenW = guiWidth * scale, screenH = guiHeight * scale;

    // Java: Gui.renderHotbar - the widget blit + the cell shading + the
    // selection sprite (the tint-only quads sample the white fallback, so no
    // texture rides here).
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
    LIBMATTI_MC_GuiLayout_HotbarSelectionRect(guiWidth, guiHeight, selectedSlot, &x, &y, &w, &h);
    LIBMATTI_MC_GuiRenderer_BlitQuad(renderer, (float) x * scale, (float) y * scale,
                                     (float) w * scale, (float) h * scale,
                                     0.0f, 0.0f, 1.0f, 1.0f, 0xE0FFFFFFu);
    LIBMATTI_MC_GuiRenderer_Flush(renderer, screenW, screenH);

    // Java: the item icons over the cells (one icon-page batch for the row).
    if (atlas != NULL)
    {
        for (int slot = 0; slot < hotbarSize; slot++)
        {
            const struct LIBMATTI_MC_ItemStack *stack = hotbarItems[slot];
            if (stack == NULL)
                continue;
            LIBMATTI_MC_GuiLayout_HotbarSlotRect(guiWidth, guiHeight, slot, &x, &y, &w, &h);
            LIBMATTI_MC_ItemRenderer_RenderGuiItem(renderer, atlas, stack,
                                                   (float) x * scale, (float) y * scale, scale);
        }
        LIBMATTI_MC_GuiRenderer_Flush(renderer, screenW, screenH);
        LIBMATTI_MC_GuiRenderer_SetTexture(renderer, 0);
    }

    // Java: renderSlot -> renderItemDecorations - the counts over the icons
    // (the immediate font draws land after the icon batch).
    for (int slot = 0; slot < hotbarSize; slot++)
    {
        const struct LIBMATTI_MC_ItemStack *stack = hotbarItems[slot];
        if (stack == NULL)
            continue;
        LIBMATTI_MC_GuiLayout_HotbarSlotRect(guiWidth, guiHeight, slot, &x, &y, &w, &h);
        LIBMATTI_MC_ItemRenderer_RenderGuiCount(font, stack, (float) x * scale, (float) y * scale, scale,
                                                screenW, screenH);
    }
}
