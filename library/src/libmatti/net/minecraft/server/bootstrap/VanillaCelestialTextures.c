// The vanilla celestial textures, generated at runtime. Java ships them in
// assets/minecraft/textures/environment/*.png stitched into the CELESTIALS
// atlas (AtlasIds.CELESTIALS); the port generates the sun and the eight moon
// phases procedurally into NativeImages and uploads the stitched page like
// VanillaBlockTextures does for the block atlas.
//
// Java equivalent: Minecraft's CELESTIALS atlas loading step
// (SpriteSourceList over assets/minecraft/atlases/celestials.json).

#include "libmatti/net/minecraft/server/bootstrap/VanillaCelestialTextures.h"

#include "libmatti/com/mojang/blaze3d/platform/NativeImage.h"
#include "libmatti/net/minecraft/client/renderer/texture/TextureAtlas.h"
#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/net/minecraft/world/level/MoonPhase.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The grid slot for the sprite at (column, row) on the 3x3 page of 32x32
// sprites (the sun and the eight moon phases fit the same cell size).
#define CELL 32

static const char *const MOON_NAMES[LIBMATTI_MC_MoonPhase_COUNT] = {
    "full_moon", "waning_gibbous", "third_quarter", "waning_crescent",
    "new_moon", "waxing_crescent", "first_quarter", "waxing_gibbous"};

// Java: the sun texture - a bright yellow-white disc on transparent ground
// (assets/minecraft/textures/environment/sun.png).
static void fill_sun(LIBMATTI_B3D_NativeImage *image)
{
    const float cx = 15.5f, cy = 15.5f;
    for (int y = 0; y < CELL; y++)
    {
        for (int x = 0; x < CELL; x++)
        {
            float dx = (float) x - cx, dy = (float) y - cy;
            float distSq = dx * dx + dy * dy;
            // The core stays white, the rim falls off to the yellow edge.
            float t = distSq / (13.0f * 13.0f);
            if (t > 1.0f)
            {
                LIBMATTI_B3D_NativeImage_SetPixel(image, x, y, 0);
                continue;
            }
            float shade = 1.0f - t * 0.35f;
            int r = (int) (255 * shade);
            int g = (int) (252 * shade);
            int b = (int) (214 * shade);
            unsigned int argb = 0xFF000000u | (unsigned int) (r << 16) | (unsigned int) (g << 8)
                                 | (unsigned int) b;
            LIBMATTI_B3D_NativeImage_SetPixel(image, x, y, argb);
        }
    }
}

// Java: the moon phase texture - the lit fraction of the disc
// (assets/minecraft/textures/environment/moon_phases.png's cell).
static void fill_moon_phase(LIBMATTI_B3D_NativeImage *image, int phase)
{
    const float cx = 15.5f, cy = 15.5f;
    const float radius = 13.0f;
    // The terminator offset: phase 0 (full) shows the full disc, phase 4
    // (new) nothing; the quarters split the disc at the centre.
    double lit;
    switch (phase)
    {
        case LIBMATTI_MC_MoonPhase_FULL_MOON: lit = 1.0; break;
        case LIBMATTI_MC_MoonPhase_WANING_GIBBOUS: lit = 0.75; break;
        case LIBMATTI_MC_MoonPhase_THIRD_QUARTER: lit = 0.5; break;
        case LIBMATTI_MC_MoonPhase_WANING_CRESCENT: lit = 0.25; break;
        case LIBMATTI_MC_MoonPhase_NEW_MOON: lit = 0.0; break;
        case LIBMATTI_MC_MoonPhase_WAXING_CRESCENT: lit = 0.25; break;
        case LIBMATTI_MC_MoonPhase_FIRST_QUARTER: lit = 0.5; break;
        default: lit = 0.75; break;
    }
    for (int y = 0; y < CELL; y++)
    {
        for (int x = 0; x < CELL; x++)
        {
            float dx = (float) x - cx, dy = (float) y - cy;
            float distSq = dx * dx + dy * dy;
            if (distSq > radius * radius)
            {
                LIBMATTI_B3D_NativeImage_SetPixel(image, x, y, 0);
                continue;
            }
            // The lit half: waning phases light the left side, waxing the
            // right; gibbous/crescent widen or shrink the lit sliver through
            // the ellipse term (|dx| <= halfWidth).
            int litSide = phase < LIBMATTI_MC_MoonPhase_NEW_MOON ? -1 : 1;
            float halfWidth = 0.0f;
            if (lit >= 1.0f)
                halfWidth = radius;
            else if (lit >= 0.5f)
                halfWidth = radius * (float) (2.0 * lit - 1.0);
            else if (lit > 0.0f)
                halfWidth = radius * (float) (1.0 - 2.0 * lit);
            int inside;
            if (lit >= 1.0f)
                inside = 1;
            else if (lit <= 0.0f)
                inside = 0;
            else if (lit >= 0.5f)
                inside = litSide < 0 ? (dx <= halfWidth) : (dx >= -halfWidth);
            else
                inside = litSide < 0 ? (-dx <= halfWidth) : (dx <= halfWidth);
            if (!inside)
            {
                LIBMATTI_B3D_NativeImage_SetPixel(image, x, y, 0);
                continue;
            }
            // The pale grey disc with a slight edge falloff.
            float t = distSq / (radius * radius);
            int shade = 232 - (int) (t * 24.0f);
            unsigned int argb = 0xFF000000u | (unsigned int) (shade << 16) | (unsigned int) (shade << 8)
                                 | (unsigned int) shade;
            LIBMATTI_B3D_NativeImage_SetPixel(image, x, y, argb);
        }
    }
}

// Java: the sprite wrapper over one 32x32 procedural image (the stitch slot
// on the 96x96 page: 3 columns x 3 rows).
static LIBMATTI_MC_TextureAtlasSprite *make_sprite(const LIBMATTI_MC_Identifier *atlas, const char *path,
                                                   LIBMATTI_B3D_NativeImage *image, int column, int row)
{
    LIBMATTI_MC_FrameSize frameSize = {CELL, CELL};
    LIBMATTI_MC_Identifier *name = LIBMATTI_MC_Identifier_FromNamespaceAndPath("minecraft", path);
    LIBMATTI_MC_SpriteContents *contents = LIBMATTI_MC_SpriteContents_New(name, frameSize, image, NULL);
    if (contents == NULL)
        return NULL;
    return LIBMATTI_MC_TextureAtlasSprite_New((LIBMATTI_MC_Identifier *) atlas, contents, 3 * CELL, 3 * CELL,
                                              column * CELL, row * CELL, 0);
}

// Java: the CELESTIALS atlas load (SpriteLoader + TextureAtlas.upload).
LIBMATTI_MC_TextureAtlas *LIBMATTI_MC_VanillaCelestialTextures_Bootstrap(int maxTextureSize)
{
    (void) maxTextureSize;
    LIBMATTI_MC_Identifier *atlasId = LIBMATTI_MC_Identifier_WithDefaultNamespace("celestials");
    LIBMATTI_MC_TextureAtlas *atlas = LIBMATTI_MC_TextureAtlas_New(atlasId, 1024);
    if (atlas == NULL)
        return NULL;

    // Java: the sun sprite (minecraft:sun) plus one sprite per moon phase
    // (minecraft:moon/<phase name>), 9 sprites on a 96x96 page.
    LIBMATTI_B3D_NativeImage *sun = LIBMATTI_B3D_NativeImage_New(CELL, CELL, 1);
    fill_sun(sun);
    LIBMATTI_MC_TextureAtlasSprite *sunSprite = make_sprite(atlasId, "environment/sun", sun, 0, 0);

    LIBMATTI_MC_TextureAtlasSprite *moonSprites[LIBMATTI_MC_MoonPhase_COUNT];
    for (int p = 0; p < LIBMATTI_MC_MoonPhase_COUNT; p++)
    {
        LIBMATTI_B3D_NativeImage *moon = LIBMATTI_B3D_NativeImage_New(CELL, CELL, 1);
        fill_moon_phase(moon, p);
        char path[64];
        snprintf(path, sizeof(path), "environment/moon/%s", MOON_NAMES[p]);
        // The grid: the sun in (0,0), the phases running left-to-right,
        // top-to-bottom from (1,0).
        int column = (p + 1) % 3;
        int row = (p + 1) / 3;
        moonSprites[p] = make_sprite(atlasId, path, moon, column, row);
        if (moonSprites[p] == NULL)
        {
            LIBMATTI_MC_TextureAtlas_Free(atlas);
            return NULL;
        }
    }
    if (sunSprite == NULL)
    {
        LIBMATTI_MC_TextureAtlas_Free(atlas);
        return NULL;
    }

    // Java: the atlas takes the sprites over (texturesByName).
    atlas->spriteCount = 1 + LIBMATTI_MC_MoonPhase_COUNT;
    atlas->sprites = calloc(atlas->spriteCount, sizeof(LIBMATTI_MC_TextureAtlasSprite *));
    atlas->sprites[0] = sunSprite;
    for (int p = 0; p < LIBMATTI_MC_MoonPhase_COUNT; p++)
        atlas->sprites[1 + p] = moonSprites[p];
    atlas->width = 3 * CELL;
    atlas->height = 3 * CELL;

    // Java: the GL upload of the stitched page (the port composes the page
    // from the sprite images and pushes it through the DynamicTexture path).
    LIBMATTI_B3D_NativeImage *page = LIBMATTI_B3D_NativeImage_New(3 * CELL, 3 * CELL, 1);
    memset(page->pixels, 0, (size_t) (3 * CELL * 3 * CELL) * sizeof(uint32_t));
    for (size_t i = 0; i < atlas->spriteCount; i++)
    {
        LIBMATTI_MC_TextureAtlasSprite *sprite = atlas->sprites[i];
        LIBMATTI_B3D_NativeImage *image = sprite->contents->byMipLevel[0];
        for (int y = 0; y < image->height; y++)
        {
            for (int x = 0; x < image->width; x++)
                page->pixels[(sprite->y + y) * 3 * CELL + sprite->x + x] = image->pixels[y * image->width + x];
        }
    }
    atlas->base.pixels = page; // the DynamicTexture upload path owns it
    atlas->base.kind = LIBMATTI_MC_TextureKind_ATLAS;

    return atlas;
}
