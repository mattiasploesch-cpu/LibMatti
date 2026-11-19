// C side of the Rust UI (library/src/libmatti/rs/src/ui).
//
// C keeps the API: the launcher chain, the early window, the render backends,
// the block/item registries and the texture/font resources. Rust owns the game
// window after the early window, the screen stack and every menu, and draws
// through these functions only.
//
// Coordinates are logical framebuffer pixels (top-left origin), the same space
// LIBMATTI_Render_* uses. Colours are 0xAARRGGBB.

#ifndef MATTICRAFT_MATTI_UI_BRIDGE_H
#define MATTICRAFT_MATTI_UI_BRIDGE_H

#include <stdint.h>

#include "libmatti/matti/ui/texture.h"

#ifdef __cplusplus
extern "C"
{
#endif

// GLFW key codes (identical to org.lwjgl.glfw.GLFW's values).
#define LIBMATTI_UI_KEY_ESCAPE 256
#define LIBMATTI_UI_KEY_ENTER 257
#define LIBMATTI_UI_KEY_TAB 258
#define LIBMATTI_UI_KEY_RIGHT 262
#define LIBMATTI_UI_KEY_LEFT 263
#define LIBMATTI_UI_KEY_DOWN 264
#define LIBMATTI_UI_KEY_UP 265

// GLFW mouse buttons.
#define LIBMATTI_UI_MOUSE_LEFT 0
#define LIBMATTI_UI_MOUSE_RIGHT 1

typedef struct LIBMATTI_UI_WindowConfig
{
    int width;
    int height;
    int fullscreen;
    int vsync;
    // "native:opengl", "sdl:vulkan", ...; NULL uses MATTI_RENDERER and then
    // the platform default from LIBMATTI_Platform_DefaultRenderer.
    const char *selection;
    const char *title;
} LIBMATTI_UI_WindowConfig;

// Closes the early window and opens the game window through the render API.
// Returns 0 on failure (see LIBMATTI_UI_LastError).
int LIBMATTI_UI_WindowOpen(const LIBMATTI_UI_WindowConfig *config);
void LIBMATTI_UI_WindowClose(void);

int LIBMATTI_UI_WindowWidth(void);
int LIBMATTI_UI_WindowHeight(void);
// The renderer in use ("opengl", "vulkan", "sdl3").
const char *LIBMATTI_UI_BackendName(void);
// The layer:api string the window was opened with ("native:vulkan", ...).
const char *LIBMATTI_UI_RequestedSelection(void);
// Compatibility alias for the old name; new code uses RequestedSelection.
const char *LIBMATTI_UI_RequestedRenderer(void);
const char *LIBMATTI_UI_LastError(void);

void LIBMATTI_UI_PollEvents(void);
int LIBMATTI_UI_ShouldClose(void);
void LIBMATTI_UI_RequestClose(void);
void LIBMATTI_UI_SetTitle(const char *title);
double LIBMATTI_UI_Time(void);

// Drawing. LIBMATTI_UI_Clear fills the whole framebuffer.
void LIBMATTI_UI_Clear(uint32_t argb);
void LIBMATTI_UI_FillRect(int x, int y, int width, int height, uint32_t argb);
// FillRect with source-over alpha (the framebuffer itself stays opaque).
void LIBMATTI_UI_FillRectBlend(int x, int y, int width, int height, uint32_t argb);
void LIBMATTI_UI_DrawPixel(int x, int y, uint32_t argb);
void LIBMATTI_UI_Present(void);

// Debug helper: write the current framebuffer as PPM (used by --ui-once).
int LIBMATTI_UI_SaveScreenshot(const char *path);

// Textures (decoded by matti/ui/texture.c). Coordinates are framebuffer
// pixels; the region variants take a source rectangle in texture pixels and a
// tint (0xAARRGGBB, 0xFFFFFFFF = unchanged). The whole GUI of the menus is
// built from these: buttons/widgets.png, the logo, the panorama faces and the
// tiled dirt background.
void LIBMATTI_UI_DrawTexture(const LIBMATTI_UI_Texture *texture, int x, int y, int width, int height);
void LIBMATTI_UI_DrawTextureRegion(const LIBMATTI_UI_Texture *texture, int x, int y, int width, int height,
                                   int source_x, int source_y, int source_width, int source_height,
                                   uint32_t tint);
void LIBMATTI_UI_DrawTextureTiled(const LIBMATTI_UI_Texture *texture, int x, int y, int width, int height,
                                  int tile);

// Text through the C font stack (Monocraft, rasterised into the framebuffer).
int LIBMATTI_UI_TextHeight(int scale);
int LIBMATTI_UI_TextWidth(const char *text, int scale);
// (x, y) is the top-left corner of the first line. scale 1 is 16 px tall.
void LIBMATTI_UI_DrawText(const char *text, int x, int y, uint32_t argb, int scale, int shadow);

// Input (all backends; SDL3 events are translated to the GLFW numbering).
int LIBMATTI_UI_KeyDown(int key);
int LIBMATTI_UI_MouseX(void);
int LIBMATTI_UI_MouseY(void);
int LIBMATTI_UI_MouseDown(int button);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MATTI_UI_BRIDGE_H
