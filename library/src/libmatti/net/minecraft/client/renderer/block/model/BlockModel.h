// Port of net.minecraft.client.renderer.block.model.BlockModel (the unbaked
// JSON model) plus the TextureSlots.Data slice the bake needs. The port keeps
// the parent chain as an owned string ("block/block" etc.) and the texture map
// as insertion-ordered string pairs - references ("#side") resolve against it
// at bake time like Java's TextureSlots.Resolver chain.

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_BLOCK_MODEL_BLOCKMODEL_H
#define MATTICRAFT_MC_CLIENT_RENDERER_BLOCK_MODEL_BLOCKMODEL_H

#include "libmatti/com/google/gson/JsonElement.h"
#include "libmatti/net/minecraft/client/renderer/block/model/BlockElement.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LIBMATTI_MC_BlockModel
{
    // Java: @Nullable Identifier parent - owned.
    char *parent;
    // Java: TextureSlots.Data - the "textures" member, insertion ordered.
    size_t textureCount;
    char **textureNames;
    char **textureValues; // "#ref" or "block/stone" - owned.
    // Java: List<BlockElement> elements (NULL elements = the model only
    // inherits, like Java's getElements() returning null when absent).
    int elementsPresent;
    size_t elementCount;
    LIBMATTI_MC_BlockElement *elements;
    // Java: boolean gui3d / hasAmbientOcclusion - the flags the bake reads.
    int ambientOcclusion;
} LIBMATTI_MC_BlockModel;

// Java: BlockModel.fromString / the Deserializer - parse the model JSON text.
// Returns NULL on malformed input (Java: JsonParseException / IO errors).
LIBMATTI_MC_BlockModel *LIBMATTI_MC_BlockModel_Parse(const char *jsonText);

// Java: TextureSlots.Resolver.resolve(String) - follow "#ref" references
// through the parent chain until a real texture id comes out; NULL when the
// slot cannot resolve (Java throws MissingTextureAtlasException at bake).
char *LIBMATTI_MC_BlockModel_ResolveTexture(const LIBMATTI_MC_BlockModel *model, const char *slotName);

// The depth-limited parent walk (Java: the parent field, resolved by the
// ModelDiscovery loop) - the port exposes the chain link for the baker.
const char *LIBMATTI_MC_BlockModel_Parent(const LIBMATTI_MC_BlockModel *model);

void LIBMATTI_MC_BlockModel_Free(LIBMATTI_MC_BlockModel *model);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_BLOCK_MODEL_BLOCKMODEL_H
