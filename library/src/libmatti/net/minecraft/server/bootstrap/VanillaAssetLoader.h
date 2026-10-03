// Port of Minecraft's asset reload slice (the ReloadListener over the
// ResourceManager): stitch the block atlas through SpriteLoader, bake the
// block models through ModelBaker and register everything - driven entirely
// by the resource pack (the embedded blob), so a new block ships as JSON +
// PNG and the pipeline picks it up without code changes.
//
// Java: the MODEL_ATLAS + MODEL_BAKERY steps of the startup reload, fed by
// the VanillaPackResources (the embedded blob rides the .a here).

#ifndef MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLAASSETLOADER_H
#define MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLAASSETLOADER_H

#include "libmatti/net/minecraft/client/resources/model/ModelManager.h"
#include "libmatti/net/minecraft/server/packs/embedded/EmbeddedPackResources.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Java: Minecraft's reload - build the resource manager over the embedded
// pack (the RAM-only blob pack), stitch atlases/blocks.json through the
// SpriteLoader and bake the block models the blockstates reference. The
// manager stays owned by the loader (the renderer paths read the atlas).
//
// Returns 1 when the atlas stitched (the world renders with the real
// textures); 0 leaves the caller's fallback (the procedural atlas) alive.
int LIBMATTI_MC_VanillaAssetLoader_Load(void);

// Java: the reload's products the renderer paths read (the atlas the
// section compiler + the HUD sample, the baked models the compiler walks).
LIBMATTI_MC_ModelManager *LIBMATTI_MC_VanillaAssetLoader_GetModelManager(void);
LIBMATTI_MC_MultiPackResourceManager *LIBMATTI_MC_VanillaAssetLoader_GetResourceManager(void);

// The entry count of the embedded pack (the smoke/log line the boot prints).
size_t LIBMATTI_MC_VanillaAssetLoader_PackEntryCount(void);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLAASSETLOADER_H
