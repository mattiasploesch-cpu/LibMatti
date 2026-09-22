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

// Java: the individual sprite preparation - wraps a NativeImage as a
// TextureAtlasSprite on the atlas page (the 16x16 sprite at the stitch slot).
// The port stitches the two sprites side by side on a 32x16 page.
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

    // The two procedural sprites on a 32x16 page.
    LIBMATTI_B3D_NativeImage *stone = LIBMATTI_B3D_NativeImage_New(16, 16, 1);
    LIBMATTI_B3D_NativeImage *dirt = LIBMATTI_B3D_NativeImage_New(16, 16, 1);
    fill_stone(stone);
    fill_dirt(dirt);

    LIBMATTI_MC_TextureAtlasSprite *stoneSprite = make_sprite(atlasId, "block/stone", stone, 0, 0);
    LIBMATTI_MC_TextureAtlasSprite *dirtSprite = make_sprite(atlasId, "block/dirt", dirt, 16, 0);
    if (stoneSprite == NULL || dirtSprite == NULL)
    {
        LIBMATTI_MC_TextureAtlas_Free(atlas);
        return NULL;
    }

    // Java: the atlas takes the sprites over (texturesByName).
    atlas->sprites = calloc(2, sizeof(LIBMATTI_MC_TextureAtlasSprite *));
    atlas->sprites[0] = stoneSprite;
    atlas->sprites[1] = dirtSprite;
    atlas->spriteCount = 2;
    atlas->width = 32;
    atlas->height = 16;

    // Java: the GL upload of the stitched page (the port composes the 32x16
    // RGBA page from the two images and pushes it through the binding).
    LIBMATTI_B3D_NativeImage *page = LIBMATTI_B3D_NativeImage_New(32, 16, 1);
    for (int y = 0; y < 16; y++)
    {
        for (int x = 0; x < 16; x++)
        {
            uint32_t s = stone->pixels[y * 16 + x];
            uint32_t d = dirt->pixels[y * 16 + x];
            page->pixels[y * 32 + x] = s;
            page->pixels[y * 32 + x + 16] = d;
        }
    }
    atlas->base.pixels = page; // the DynamicTexture upload path owns it
    atlas->base.kind = LIBMATTI_MC_TextureKind_ATLAS;

    return atlas;
}
