// GUI textures for the C menus.
//
// Texturing stays on the C side: the menus name a vanilla texture path
// ("gui/title/minecraft.png"), this module finds the PNG in the game's asset
// root (--assetsDir / $MATTI_ASSETS_DIR / ./assets), falls back to the copy
// bundled into the executable, decodes it and hands out ARGB pixels. The
// drawing entry points live in bridge.h next to the framebuffer access.

#ifndef MATTICRAFT_MATTI_UI_TEXTURE_H
#define MATTICRAFT_MATTI_UI_TEXTURE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct LIBMATTI_UI_Texture
{
    int width;
    int height;
    // 0xAARRGGBB, straight alpha, rows top-to-bottom.
    uint32_t *pixels;
} LIBMATTI_UI_Texture;

// Loads "minecraft:textures/<name>" or a bare "<name>". Returns NULL when the
// texture is in neither the asset root nor the embedded bundle. Loaded
// textures are cached and shared; do not free them.
LIBMATTI_UI_Texture *LIBMATTI_UI_Texture_Load(const char *name);

int LIBMATTI_UI_Texture_Width(const LIBMATTI_UI_Texture *texture);
int LIBMATTI_UI_Texture_Height(const LIBMATTI_UI_Texture *texture);
const uint32_t *LIBMATTI_UI_Texture_Pixels(const LIBMATTI_UI_Texture *texture);

// The root the next load looks in first (may be NULL). The menus read
// LIBMATTI_UI_AssetRoot or the asset directory once at startup.
const char *LIBMATTI_UI_Texture_AssetRoot(void);
void LIBMATTI_UI_Texture_SetAssetRoot(const char *root);

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_MATTI_UI_TEXTURE_H
