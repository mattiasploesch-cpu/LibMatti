// The GUI renderer (Java: GuiGraphics' blit path over RenderPipelines
// GUI_TEXTURED) - a POSITION_COLOR_TEXTURE quad batcher: positions in screen
// pixels (y-down), the shader folds them into the clip space exactly like the
// font shader (position / screenSize * 2 - 1 with the y negated), the RGBA
// colour tints the white 1x1 fallback texture the quads sample.

#ifndef MATTICRAFT_NET_MINECRAFT_CLIENT_GUI_GUIRENDERER_H
#define MATTICRAFT_NET_MINECRAFT_CLIENT_GUI_GUIRENDERER_H

#include "libmatti/defines.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Java: the GUI_TEXTURED pipeline's vertex format (POSITION_COLOR_TEXTURE, the
// GUI vertex layout the blit path packs).
typedef struct LIBMATTI_MC_GuiVertex
{
    float x;
    float y;
    float r;
    float g;
    float b;
    float a;
    float u;
    float v;
} LIBMATTI_MC_GuiVertex;

typedef struct LIBMATTI_MC_GuiRenderer LIBMATTI_MC_GuiRenderer;

// Compiles the GUI program and allocates the batch buffers; NULL without a GL
// context or on a failed shader (the draw calls no-op on NULL).
LIBMATTI_MC_GuiRenderer *LIBMATTI_MC_GuiRenderer_New(void);
void LIBMATTI_MC_GuiRenderer_Free(LIBMATTI_MC_GuiRenderer *renderer);

// The program the draw runs (0 = not compiled - the draw path stays off, like
// the skeleton's font program gate).
unsigned int LIBMATTI_MC_GuiRenderer_Program(const LIBMATTI_MC_GuiRenderer *renderer);

// Java: the GUI_TEXTURED pass binds the sprite atlas before the blits
// (RenderSystem.setShaderTexture). Flush samples the set texture; 0 falls
// back to the white 1x1 (the tint-only path). The renderer does NOT take
// ownership - the atlas outlives the HUD pass.
void LIBMATTI_MC_GuiRenderer_SetTexture(LIBMATTI_MC_GuiRenderer *renderer, unsigned int texture);
// The texture the next Flush samples (0 = the white 1x1 fallback).
unsigned int LIBMATTI_MC_GuiRenderer_GetTexture(const LIBMATTI_MC_GuiRenderer *renderer);

// Batches one textured quad (the two triangles GuiGraphics.blitSprite packs).
// (x, y, width, height) is the screen rect in layout pixels (y-down), uv
// 0..1 over the bound texture, rgba the tint (255 = white, the vanilla
// blitSprite default).
void LIBMATTI_MC_GuiRenderer_BlitQuad(LIBMATTI_MC_GuiRenderer *renderer,
                                      float x, float y, float width, float height,
                                      float u0, float v0, float u1, float v1,
                                      unsigned int rgba);

// The pure packing math BlitQuad rides: the six vertices (two triangles,
// v0 v1 v2 + v0 v2 v3 like GuiGraphics.blitSprite's innerData) over the rect
// corners, the rgba packed ARGB tint folded into the vertex colours. Runs
// without a GL context (the harness drives it directly).
void LIBMATTI_MC_GuiRenderer_PackQuad(LIBMATTI_MC_GuiVertex *out, float x, float y, float width, float height,
                                      float u0, float v0, float u1, float v1,
                                      unsigned int rgba);

// Uploads and draws the batched quads through the GUI program; the blend and
// texture binding stay the caller's state (the font path sets them the same
// way). Flush samples the SetTexture texture (the white 1x1 while unset).
// Draws nothing on a NULL renderer or an empty batch.
void LIBMATTI_MC_GuiRenderer_Flush(LIBMATTI_MC_GuiRenderer *renderer, float screenSizeX, float screenSizeY);

#ifdef __cplusplus
}
#endif

#endif
