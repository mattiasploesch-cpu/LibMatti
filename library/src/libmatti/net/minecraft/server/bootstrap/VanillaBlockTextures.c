// The vanilla block textures (implementation). The stitch normally runs over
// the resource pack; the port generates the demo textures procedurally into
// NativeImages, wraps them into sprite sources the SpriteLoader accepts and
// uploads the stitched page into the atlas.
//
// Java equivalent: Minecraft's MODEL_ATLAS loading step
// (SpriteSourceList over assets/minecraft/atlases/blocks.json).

#include "libmatti/net/minecraft/server/bootstrap/VanillaBlockTextures.h"

#include "libmatti/com/mojang/blaze3d/platform/NativeImage.h"
#include "libmatti/net/minecraft/client/renderer/texture/TextureAtlas.h"
#include "libmatti/net/minecraft/client/resources/metadata/TextureMetadataSections.h"
#include "libmatti/net/minecraft/resources/Identifier.h"

#include <stdlib.h>
#include <string.h>

// Java: the missing-no texture - the 2x2 magenta/black checkerboard tiled
// over the sprite (MissingTextureAtlasSprite.generateMissingImage). The
// terrain and the HUD sample it for every block whose texture id is not
// stitched, never the full-atlas rect (the page smear the demo rendered).
static void fill_missing(LIBMATTI_B3D_NativeImage *image)
{
    for (int y = 0; y < 16; y++)
    {
        for (int x = 0; x < 16; x++)
        {
            int checker = ((x >> 3) + (y >> 3)) & 1;
            int argb = checker ? 0xFF000000u : 0xFFF800F8u;
            LIBMATTI_B3D_NativeImage_SetPixel(image, x, y, argb);
        }
    }
}

// Java: the stone texture - grayscale noise around #7d7d7d.
static void fill_stone(LIBMATTI_B3D_NativeImage *image)
{
    unsigned int seed = 0x517cc1b7; // a fixed seed keeps the texture stable
    for (int y = 0; y < 16; y++)
    {
        for (int x = 0; x < 16; x++)
        {
            seed = seed * 1103515245 + 12345;
            int shade = 118 + (int) ((seed >> 16) % 24) - 12;
            int argb = 0xFF000000u | (unsigned int) (shade << 16) | (unsigned int) (shade << 8)
                       | (unsigned int) shade;
            LIBMATTI_B3D_NativeImage_SetPixel(image, x, y, argb);
        }
    }
}

// Java: the dirt texture - brown noise around #866043.
static void fill_dirt(LIBMATTI_B3D_NativeImage *image)
{
    unsigned int seed = 0x2f9e4d1a;
    for (int y = 0; y < 16; y++)
    {
        for (int x = 0; x < 16; x++)
        {
            seed = seed * 1103515245 + 12345;
            int variant = (int) ((seed >> 16) % 30) - 15;
            int r = 134 + variant;
            int g = 96 + variant;
            int b = 67 + variant / 2;
            int argb = 0xFF000000u | (unsigned int) (r << 16) | (unsigned int) (g << 8) | (unsigned int) b;
            LIBMATTI_B3D_NativeImage_SetPixel(image, x, y, argb);
        }
    }
}

// Java: cobblestone - the pebble noise over the dark mortar seams (#7a7a7a).
static void fill_cobblestone(LIBMATTI_B3D_NativeImage *image)
{
    unsigned int seed = 0x1a2b3c4d;
    for (int y = 0; y < 16; y++)
    {
        for (int x = 0; x < 16; x++)
        {
            seed = seed * 1103515245 + 12345;
            int shade = 122 + (int) ((seed >> 16) % 20) - 10;
            // the mortar seams (the pebble edges)
            if ((x + y) % 5 == 0 || x % 7 == 0 || y % 6 == 0)
                shade = 78;
            int argb = 0xFF000000u | (unsigned int) (shade << 16) | (unsigned int) (shade << 8)
                       | (unsigned int) shade;
            LIBMATTI_B3D_NativeImage_SetPixel(image, x, y, argb);
        }
    }
}

// Java: oak_planks - the horizontal boards with the dark seams (#9c7f4e).
static void fill_oak_planks(LIBMATTI_B3D_NativeImage *image)
{
    unsigned int seed = 0x7d4e2b1a;
    for (int y = 0; y < 16; y++)
    {
        for (int x = 0; x < 16; x++)
        {
            seed = seed * 1103515245 + 12345;
            int variant = (int) ((seed >> 16) % 18) - 9;
            int r = 156 + variant;
            int g = 127 + variant;
            int b = 78 + variant / 2;
            // the plank seams every 4 rows (the board edges)
            if (y % 4 == 3)
            {
                r = 108;
                g = 86;
                b = 52;
            }
            int argb = 0xFF000000u | (unsigned int) (r << 16) | (unsigned int) (g << 8) | (unsigned int) b;
            LIBMATTI_B3D_NativeImage_SetPixel(image, x, y, argb);
        }
    }
}

// Java: glass - the pale pane with the border highlights (the alpha the
// cutout discard punches out).
static void fill_glass(LIBMATTI_B3D_NativeImage *image)
{
    for (int y = 0; y < 16; y++)
    {
        for (int x = 0; x < 16; x++)
        {
            int border = x == 0 || y == 0 || x == 15 || y == 15;
            // the corner highlights the vanilla pane carries
            int highlight = (x < 3 && y < 3) || (x > 12 && y > 12);
            int argb;
            if (border)
                argb = 0xFFC0F5FAu;
            else if (highlight)
                argb = 0x90FFFFFFu;
            else
                argb = 0x30FFFFFFu;
            LIBMATTI_B3D_NativeImage_SetPixel(image, x, y, argb);
        }
    }
}

// Java: bricks - the red bricks over the pale mortar grid (#985e43).
static void fill_bricks(LIBMATTI_B3D_NativeImage *image)
{
    unsigned int seed = 0x3c6e2f1d;
    for (int y = 0; y < 16; y++)
    {
        for (int x = 0; x < 16; x++)
        {
            seed = seed * 1103515245 + 12345;
            int variant = (int) ((seed >> 16) % 14) - 7;
            int r = 152 + variant;
            int g = 94 + variant;
            int b = 67 + variant / 2;
            // the mortar grid (offset rows like the vanilla texture)
            int row = y / 4;
            int offset = (row % 2) * 4;
            if (y % 4 == 3 || (x + offset) % 8 == 7)
            {
                r = 143;
                g = 143;
                b = 143;
            }
            int argb = 0xFF000000u | (unsigned int) (r << 16) | (unsigned int) (g << 8) | (unsigned int) b;
            LIBMATTI_B3D_NativeImage_SetPixel(image, x, y, argb);
        }
    }
}

// Java: sand - the pale grain noise around #dbd3a0.
static void fill_sand(LIBMATTI_B3D_NativeImage *image)
{
    unsigned int seed = 0x5e1d3b7a;
    for (int y = 0; y < 16; y++)
    {
        for (int x = 0; x < 16; x++)
        {
            seed = seed * 1103515245 + 12345;
            int variant = (int) ((seed >> 16) % 16) - 8;
            int r = 219 + variant;
            int g = 211 + variant;
            int b = 160 + variant;
            int argb = 0xFF000000u | (unsigned int) (r << 16) | (unsigned int) (g << 8) | (unsigned int) b;
            LIBMATTI_B3D_NativeImage_SetPixel(image, x, y, argb);
        }
    }
}

// Java: gravel - the coarse gray/brown pebble noise around #7f7f7f.
static void fill_gravel(LIBMATTI_B3D_NativeImage *image)
{
    unsigned int seed = 0x2b1e4f6d;
    for (int y = 0; y < 16; y++)
    {
        for (int x = 0; x < 16; x++)
        {
            seed = seed * 1103515245 + 12345;
            int shade = 100 + (int) ((seed >> 16) % 70) - 35;
            int brown = (int) ((seed >> 24) % 3) == 0;
            int r = brown ? shade : shade - 6;
            int g = shade - 4;
            int b = shade - 8;
            int argb = 0xFF000000u | (unsigned int) (r << 16) | (unsigned int) (g << 8) | (unsigned int) b;
            LIBMATTI_B3D_NativeImage_SetPixel(image, x, y, argb);
        }
    }
}

// Java: oak_log - the vertical bark ridges around #6b5232 (the column's side
// texture; the demo atlas keeps one texture per block, so the ends ride it
// too until the cube_column's two sprites stitch).
static void fill_oak_log(LIBMATTI_B3D_NativeImage *image)
{
    unsigned int seed = 0x4a6d2f13;
    for (int y = 0; y < 16; y++)
    {
        for (int x = 0; x < 16; x++)
        {
            seed = seed * 1103515245 + 12345;
            int variant = (int) ((seed >> 16) % 22) - 11;
            int r = 107 + variant;
            int g = 82 + variant;
            int b = 50 + variant / 2;
            // the vertical bark ridges (x keeps the column structure)
            if (x % 5 == 2)
            {
                r = 84;
                g = 64;
                b = 40;
            }
            int argb = 0xFF000000u | (unsigned int) (r << 16) | (unsigned int) (g << 8) | (unsigned int) b;
            LIBMATTI_B3D_NativeImage_SetPixel(image, x, y, argb);
        }
    }
}

// Java: the individual sprite preparation - wraps a NativeImage as a
// TextureAtlasSprite on the atlas page (the 16x16 sprite at the stitch slot).
static LIBMATTI_MC_TextureAtlasSprite *make_sprite(const LIBMATTI_MC_Identifier *atlas, const char *path,
                                                   LIBMATTI_B3D_NativeImage *image, int x, int y)
{
    LIBMATTI_MC_FrameSize frameSize = {16, 16};
    LIBMATTI_MC_Identifier *name = LIBMATTI_MC_Identifier_FromNamespaceAndPath("minecraft", path);
    LIBMATTI_MC_SpriteContents *contents = LIBMATTI_MC_SpriteContents_New(name, frameSize, image, NULL);
    if (contents == NULL)
        return NULL;
    return LIBMATTI_MC_TextureAtlasSprite_New((LIBMATTI_MC_Identifier *) atlas, contents, 32, 16, x, y, 0);
}

// Java: MODEL_ATLAS's SpriteLoader + TextureAtlas.upload
LIBMATTI_MC_TextureAtlas *LIBMATTI_MC_VanillaBlockTextures_Bootstrap(int maxTextureSize)
{
    (void) maxTextureSize;
    LIBMATTI_MC_Identifier *atlasId = LIBMATTI_MC_Identifier_WithDefaultNamespace("blocks");
    LIBMATTI_MC_TextureAtlas *atlas = LIBMATTI_MC_TextureAtlas_New(atlasId, 1024);
    if (atlas == NULL)
        return NULL;

    // Java: the vanilla sprites the demo level renders (the hotbar palette +
    // the terrain) plus the missing-no fallback, on a 160x16 page (10 cells).
    LIBMATTI_B3D_NativeImage *missing = LIBMATTI_B3D_NativeImage_New(16, 16, 1);
    LIBMATTI_B3D_NativeImage *stone = LIBMATTI_B3D_NativeImage_New(16, 16, 1);
    LIBMATTI_B3D_NativeImage *dirt = LIBMATTI_B3D_NativeImage_New(16, 16, 1);
    LIBMATTI_B3D_NativeImage *cobblestone = LIBMATTI_B3D_NativeImage_New(16, 16, 1);
    LIBMATTI_B3D_NativeImage *oakPlanks = LIBMATTI_B3D_NativeImage_New(16, 16, 1);
    LIBMATTI_B3D_NativeImage *glass = LIBMATTI_B3D_NativeImage_New(16, 16, 1);
    LIBMATTI_B3D_NativeImage *bricks = LIBMATTI_B3D_NativeImage_New(16, 16, 1);
    LIBMATTI_B3D_NativeImage *sand = LIBMATTI_B3D_NativeImage_New(16, 16, 1);
    LIBMATTI_B3D_NativeImage *gravel = LIBMATTI_B3D_NativeImage_New(16, 16, 1);
    LIBMATTI_B3D_NativeImage *oakLog = LIBMATTI_B3D_NativeImage_New(16, 16, 1);
    fill_missing(missing);
    fill_stone(stone);
    fill_dirt(dirt);
    fill_cobblestone(cobblestone);
    fill_oak_planks(oakPlanks);
    fill_glass(glass);
    fill_bricks(bricks);
    fill_sand(sand);
    fill_gravel(gravel);
    fill_oak_log(oakLog);

    LIBMATTI_MC_TextureAtlasSprite *missingSprite = make_sprite(atlasId, "missingno", missing, 0, 0);
    LIBMATTI_MC_TextureAtlasSprite *stoneSprite = make_sprite(atlasId, "block/stone", stone, 16, 0);
    LIBMATTI_MC_TextureAtlasSprite *dirtSprite = make_sprite(atlasId, "block/dirt", dirt, 32, 0);
    LIBMATTI_MC_TextureAtlasSprite *cobblestoneSprite = make_sprite(atlasId, "block/cobblestone", cobblestone, 48, 0);
    LIBMATTI_MC_TextureAtlasSprite *oakPlanksSprite = make_sprite(atlasId, "block/oak_planks", oakPlanks, 64, 0);
    LIBMATTI_MC_TextureAtlasSprite *glassSprite = make_sprite(atlasId, "block/glass", glass, 80, 0);
    LIBMATTI_MC_TextureAtlasSprite *bricksSprite = make_sprite(atlasId, "block/bricks", bricks, 96, 0);
    LIBMATTI_MC_TextureAtlasSprite *sandSprite = make_sprite(atlasId, "block/sand", sand, 112, 0);
    LIBMATTI_MC_TextureAtlasSprite *gravelSprite = make_sprite(atlasId, "block/gravel", gravel, 128, 0);
    LIBMATTI_MC_TextureAtlasSprite *oakLogSprite = make_sprite(atlasId, "block/oak_log", oakLog, 144, 0);
    if (missingSprite == NULL || stoneSprite == NULL || dirtSprite == NULL || cobblestoneSprite == NULL
        || oakPlanksSprite == NULL || glassSprite == NULL || bricksSprite == NULL || sandSprite == NULL
        || gravelSprite == NULL || oakLogSprite == NULL)
    {
        LIBMATTI_MC_TextureAtlas_Free(atlas);
        return NULL;
    }

    // Java: the atlas takes the sprites over (texturesByName) + the missing
    // fallback (getOrDefault's target for unknown texture ids).
    atlas->sprites = calloc(10, sizeof(LIBMATTI_MC_TextureAtlasSprite *));
    atlas->sprites[0] = missingSprite;
    atlas->sprites[1] = stoneSprite;
    atlas->sprites[2] = dirtSprite;
    atlas->sprites[3] = cobblestoneSprite;
    atlas->sprites[4] = oakPlanksSprite;
    atlas->sprites[5] = glassSprite;
    atlas->sprites[6] = bricksSprite;
    atlas->sprites[7] = sandSprite;
    atlas->sprites[8] = gravelSprite;
    atlas->sprites[9] = oakLogSprite;
    atlas->spriteCount = 10;
    atlas->missingSprite = missingSprite;
    atlas->width = 160;
    atlas->height = 16;

    // Java: the GL upload of the stitched page (the port composes the 160x16
    // RGBA page from the sprite images and pushes it through the binding).
    LIBMATTI_B3D_NativeImage *page = LIBMATTI_B3D_NativeImage_New(160, 16, 1);
    LIBMATTI_B3D_NativeImage *cells[10] = {missing,  stone,  dirt,       cobblestone, oakPlanks,
                                           glass,    bricks, sand,       gravel,      oakLog};
    for (int cell = 0; cell < 10; cell++)
    {
        for (int y = 0; y < 16; y++)
        {
            for (int x = 0; x < 16; x++)
            {
                // Java: NativeImage - the pixels ride as ABGR ints; the page
                // copies the raw channels (the same format, no swap).
                page->pixels[(size_t) y * 160 + (size_t) cell * 16 + x] =
                    cells[cell]->pixels[(size_t) y * 16 + x];
            }
        }
    }
    atlas->base.pixels = page; // the DynamicTexture upload path owns it
    atlas->base.kind = LIBMATTI_MC_TextureKind_ATLAS;

    return atlas;
}
