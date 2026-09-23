// Port of net.minecraft.client.renderer.SkyRenderer (1.21.11).
//
// Java builds the static vertex arrays once per renderer: the sky disc, the
// sunrise position circle fan, sun/moon quads and the 1500 stars. The port
// keeps the same buffers and the draw order SKY_DISC -> SUNRISE/SUNSET ->
// SUN -> MOON -> STARS -> DARK_DISC (Java: LevelRenderer.renderLevel's sky
// pass). Every draw rides a 1:1 mapping of Java's render pass flow: the
// model-view stack pose multiplied onto the projection, the tint through the
// DynamicTransforms equivalent (the color uniform).

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_SKYRENDERER_H
#define MATTICRAFT_MC_CLIENT_RENDERER_SKYRENDERER_H

#include "libmatti/net/minecraft/world/level/MoonPhase.h"
#include "libmatti/org/joml/Matrix4f.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: private static final float SKY_DISC_RADIUS -> the disc rim at 512
#define LIBMATTI_MC_SKY_SKY_DISC_RADIUS 512.0f
// Java: private static final int STAR_COUNT = 1500
#define LIBMATTI_MC_SKY_STAR_COUNT 1500
// Java: private static final float SUN_SIZE = 30.0F / SUN_HEIGHT = 100.0F
#define LIBMATTI_MC_SKY_SUN_SIZE 30.0f
#define LIBMATTI_MC_SKY_SUN_HEIGHT 100.0f
// Java: private static final float MOON_SIZE = 20.0F / MOON_HEIGHT = 100.0F
#define LIBMATTI_MC_SKY_MOON_SIZE 20.0f
#define LIBMATTI_MC_SKY_MOON_HEIGHT 100.0f

// Java: public class SkyRenderer
typedef struct LIBMATTI_MC_SkyRenderer LIBMATTI_MC_SkyRenderer;

// Java: public SkyRenderer(TextureManager, AtlasManager) - the sky disc and
// buffers build once. The celestials atlas rides in through SetAtlas (the
// CELESTIALS atlas load step).
LIBMATTI_MC_SkyRenderer *LIBMATTI_MC_SkyRenderer_New(void);
void LIBMATTI_MC_SkyRenderer_SetAtlas(LIBMATTI_MC_SkyRenderer *renderer, void *celestialsAtlas);
void LIBMATTI_MC_SkyRenderer_Free(LIBMATTI_MC_SkyRenderer *renderer);

// Java: public void renderSkyDisc(int color) - the top disc through the SKY
// pipeline with the sky-color tint.
void LIBMATTI_MC_SkyRenderer_DrawSkyDisc(LIBMATTI_MC_SkyRenderer *renderer, const LIBMATTI_JOML_Matrix4f *modelView,
                                         const LIBMATTI_JOML_Matrix4f *projection, float r, float g, float b,
                                         float a);

// Java: public void renderDarkDisc() - the bottom disc, translated +12 on y.
void LIBMATTI_MC_SkyRenderer_DrawDarkDisc(LIBMATTI_MC_SkyRenderer *renderer, const LIBMATTI_JOML_Matrix4f *modelView,
                                          const LIBMATTI_JOML_Matrix4f *projection);

// Java: public void renderSunMoonAndStars(PoseStack, sunAngle, moonAngle,
// starAngle, moonPhase, rainBrightness, starBrightness)
void LIBMATTI_MC_SkyRenderer_RenderSunMoonAndStars(LIBMATTI_MC_SkyRenderer *renderer,
                                                   const LIBMATTI_JOML_Matrix4f *modelView,
                                                   const LIBMATTI_JOML_Matrix4f *projection, float sunAngle,
                                                   float moonAngle, float starAngle,
                                                   LIBMATTI_MC_MoonPhase moonPhase, float rainBrightness,
                                                   float starBrightness);

// Java: public void renderSunriseAndSunset(PoseStack, sunAngle, color)
void LIBMATTI_MC_SkyRenderer_DrawSunriseSunset(LIBMATTI_MC_SkyRenderer *renderer,
                                               const LIBMATTI_JOML_Matrix4f *modelView,
                                               const LIBMATTI_JOML_Matrix4f *projection, float angleRadians,
                                               float alpha);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_SKYRENDERER_H
