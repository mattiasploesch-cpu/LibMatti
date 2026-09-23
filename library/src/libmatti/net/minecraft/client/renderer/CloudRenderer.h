// Port of net.minecraft.client.renderer.CloudRenderer (1.21.11).
//
// Java packs the clouds.png cells into long cell data (color + the four
// neighbour-emptiness flags), builds the cell mesh around the camera cell
// (flat top quads for FAST, extruded boxes for FANCY, faces culled against
// the neighbour flags) and scrolls it with the game time. The port keeps
// every constant and the mesh algorithm 1:1; the texture source is the
// generated 256x256 clouds image (the same generation path the celestial
// atlas uses - the vendor repo carries no binary assets).
//
// The draw rides the position+texture cloud program: Java's clouds.vsh/fsh
// transform the cell quads with the clip matrix and tint through CloudInfo;
// the port bakes the world-space positions into the mesh and sets the tint
// as a uniform (the same DynamicTransforms equivalence the SkyRenderer uses).

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_CLOUDRENDERER_H
#define MATTICRAFT_MC_CLIENT_RENDERER_CLOUDRENDERER_H

#include "libmatti/org/joml/Matrix4f.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: private static final float CELL_SIZE_IN_BLOCKS = 12.0F
#define LIBMATTI_MC_Cloud_CELL_SIZE_IN_BLOCKS 12.0f
// Java: private static final int TICKS_PER_CELL = 400
#define LIBMATTI_MC_Cloud_TICKS_PER_CELL 400
// Java: private static final float BLOCKS_PER_SECOND = 0.6F
#define LIBMATTI_MC_Cloud_BLOCKS_PER_SECOND 0.6f

// Java: public enum CloudStatus (net.minecraft.client.CloudStatus)
typedef enum LIBMATTI_MC_CloudStatus
{
    LIBMATTI_MC_CloudStatus_OFF = 0,
    LIBMATTI_MC_CloudStatus_FAST = 1,
    LIBMATTI_MC_CloudStatus_FANCY = 2
} LIBMATTI_MC_CloudStatus;

// Java: private enum RelativeCameraPos
typedef enum LIBMATTI_MC_Cloud_RelativeCameraPos
{
    LIBMATTI_MC_Cloud_ABOVE_CLOUDS = 0,
    LIBMATTI_MC_Cloud_INSIDE_CLOUDS = 1,
    LIBMATTI_MC_Cloud_BELOW_CLOUDS = 2
} LIBMATTI_MC_Cloud_RelativeCameraPos;

// Java: public class CloudRenderer
typedef struct LIBMATTI_MC_CloudRenderer LIBMATTI_MC_CloudRenderer;

// Java: the reload listener's prepare/apply - the texture cells build once
// (the port generates the image, Java reads textures/environment/clouds.png).
LIBMATTI_MC_CloudRenderer *LIBMATTI_MC_CloudRenderer_New(void);
void LIBMATTI_MC_CloudRenderer_Free(LIBMATTI_MC_CloudRenderer *renderer);

// Java: public void render(int cloudColor, CloudStatus, float cloudHeight,
// Vec3 cameraPos, long gameTime, float partialTick) - the cell-mesh rebuild
// on cell change and the indexed quad draw through the cloud program.
// The clip matrix is projection * view (the world-anchored transform).
void LIBMATTI_MC_CloudRenderer_Render(LIBMATTI_MC_CloudRenderer *renderer,
                                      LIBMATTI_MC_CloudStatus status,
                                      float cloudColorR, float cloudColorG, float cloudColorB,
                                      float cloudColorA,
                                      float cloudHeight,
                                      double camX, double camY, double camZ,
                                      long gameTime, float partialTick,
                                      const LIBMATTI_JOML_Matrix4f *view,
                                      const LIBMATTI_JOML_Matrix4f *projection,
                                      int cloudRangeCells);

// Java: public void markForRebuild()
void LIBMATTI_MC_CloudRenderer_MarkForRebuild(LIBMATTI_MC_CloudRenderer *renderer);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_CLOUDRENDERER_H
