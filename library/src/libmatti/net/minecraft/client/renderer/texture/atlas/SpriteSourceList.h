// Port of net.minecraft.client.renderer.texture.atlas.SpriteSourceList,
// SpriteSource, SpriteResourceLoader and the sources DirectoryLister,
// SingleFile and SourceFilter. The atlas definition JSON (atlases/*.json)
// drives which resources become sprites.

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_ATLAS_SPRITESOURCELIST_H
#define MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_ATLAS_SPRITESOURCELIST_H

#include "libmatti/net/minecraft/client/renderer/texture/TextureAtlasSprite.h"
#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/net/minecraft/server/packs/resources/MultiPackResourceManager.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Java: interface SpriteSource - one entry of an atlas definition
typedef enum LIBMATTI_MC_SpriteSource_Kind
{
    LIBMATTI_MC_SpriteSource_SINGLE_FILE = 0,
    LIBMATTI_MC_SpriteSource_DIRECTORY_LISTER,
    LIBMATTI_MC_SpriteSource_SOURCE_FILTER
} LIBMATTI_MC_SpriteSource_Kind;

typedef struct LIBMATTI_MC_SpriteSource
{
    LIBMATTI_MC_SpriteSource_Kind kind;
    // SingleFile: the resource id + optional sprite id
    LIBMATTI_MC_Identifier *resourceId;
    LIBMATTI_MC_Identifier *spriteId; // NULL = use resourceId
    // DirectoryLister: sourcePath + prefix
    char *sourcePath;
    char *idPrefix;
    // SourceFilter: the pattern the sprite ids must match (ns:path glob)
    char *filterPattern;
} LIBMATTI_MC_SpriteSource;

void LIBMATTI_MC_SpriteSource_Free(LIBMATTI_MC_SpriteSource *source);

// Java: SpriteSourceList.load(ResourceManager, atlasId) - parses the
// "sources" array of atlases/<name>.json
LIBMATTI_MC_SpriteSource **LIBMATTI_MC_SpriteSourceList_Load(
    const LIBMATTI_MC_MultiPackResourceManager *resourceManager, const LIBMATTI_MC_Identifier *atlasId,
    size_t *outCount);
void LIBMATTI_MC_SpriteSourceList_Free(LIBMATTI_MC_SpriteSource **sources, size_t count);

// Java: SpriteSourceList.list(ResourceManager) - runs the sources into the
// sprite map and appends the missing sprite; the returned pairs are
// (identifier, resource) the SpriteResourceLoader decodes.
typedef struct LIBMATTI_MC_SpriteResourceEntry
{
    LIBMATTI_MC_Identifier *spriteId;
    LIBMATTI_MC_Resource *resource; // owned by the entry
} LIBMATTI_MC_SpriteResourceEntry;

LIBMATTI_MC_SpriteResourceEntry *LIBMATTI_MC_SpriteSourceList_Run(
    LIBMATTI_MC_SpriteSource **sources, size_t sourceCount,
    const LIBMATTI_MC_MultiPackResourceManager *resourceManager, size_t *outCount);
void LIBMATTI_MC_SpriteResourceEntries_Free(LIBMATTI_MC_SpriteResourceEntry *entries, size_t count);

// Java: SpriteResourceLoader.loadSprite(Identifier, Resource) - decodes the
// PNG, applies the animation metadata and builds the SpriteContents.
// NULL on a decode error (Java logs and returns null).
LIBMATTI_MC_SpriteContents *LIBMATTI_MC_SpriteResourceLoader_LoadSprite(const LIBMATTI_MC_Identifier *spriteId,
                                                                        LIBMATTI_MC_Resource *resource);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_ATLAS_SPRITESOURCELIST_H
