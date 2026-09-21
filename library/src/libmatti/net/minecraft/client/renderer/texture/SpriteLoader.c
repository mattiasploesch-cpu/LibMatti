// Implementation of net.minecraft.client.renderer.texture.SpriteLoader:
// the sprite list -> mip level resolution -> Stitcher -> Preparations.
// Java's executor chain collapses into the synchronous call order.

#include "libmatti/net/minecraft/client/renderer/texture/SpriteLoader.h"

#include "libmatti/net/minecraft/client/renderer/texture/AbstractTexture.h"
#include "libmatti/net/minecraft/client/renderer/texture/MissingTextureAtlasSprite.h"
#include "libmatti/net/minecraft/client/renderer/texture/Stitcher.h"
#include "libmatti/net/minecraft/client/renderer/texture/atlas/SpriteSourceList.h"
#include "libmatti/net/minecraft/util/Mth.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_MC_SpriteLoader *LIBMATTI_MC_SpriteLoader_Create(const LIBMATTI_MC_Identifier *atlasLocation,
                                                          int maxSupportedTextureSize)
{
    LIBMATTI_MC_SpriteLoader *loader = calloc(1, sizeof(LIBMATTI_MC_SpriteLoader));
    loader->location = LIBMATTI_MC_Identifier_FromNamespaceAndPath(atlasLocation->namespace, atlasLocation->path);
    loader->maxSupportedTextureSize = maxSupportedTextureSize;
    return loader;
}

void LIBMATTI_MC_SpriteLoader_Free(LIBMATTI_MC_SpriteLoader *loader)
{
    if (loader == NULL)
        return;
    LIBMATTI_MC_Identifier_Free(loader->location);
    free(loader);
}

// The gather context maps a stitched Stitcher.Entry back to its
// SpriteContents (the entries carry only width/height/name) and builds
// the TextureAtlasSprites.
typedef struct GatherContext
{
    LIBMATTI_MC_SpriteContents **contents;
    size_t contentsCount;
    LIBMATTI_MC_SpriteLoader_Preparations *preparations;
    LIBMATTI_MC_Identifier *atlas;
} GatherContext;

// Java: map.put(p_448383_.name(), new TextureAtlasSprite(this.location,
//       p_448383_, i2, j2, p_448384_, p_448385_, p_448386_))
static void gather_callback(void *userData, const LIBMATTI_MC_Stitcher_Entry *entry, int x, int y, int padding)
{
    GatherContext *context = userData;

    LIBMATTI_MC_SpriteContents *matched = NULL;
    for (size_t i = 0; i < context->contentsCount; i++)
    {
        if (strcmp(context->contents[i]->name->path, entry->name) == 0)
        {
            matched = context->contents[i];
            break;
        }
    }
    if (matched == NULL)
        return;

    LIBMATTI_MC_Identifier *name =
        LIBMATTI_MC_Identifier_FromNamespaceAndPath(matched->name->namespace, matched->name->path);
    LIBMATTI_MC_TextureAtlasSprite *sprite =
        LIBMATTI_MC_TextureAtlasSprite_New(context->atlas, matched, context->preparations->width,
                                           context->preparations->height, x, y, padding);
    LIBMATTI_MC_Identifier_Free(name);
    // TextureAtlasSprite_New copies the atlas id but keeps the contents
    // pointer; the sprite takes the contents over (the port's ownership).
    (void) name;

    size_t index = context->preparations->spriteCount++;
    context->preparations->sprites[index] = sprite;
    context->preparations->spriteNames[index] =
        LIBMATTI_MC_Identifier_FromNamespaceAndPath(matched->name->namespace, matched->name->path);
}

// Java: private SpriteLoader.Preparations stitch(List<SpriteContents>, int mipLevel, Executor)
static LIBMATTI_MC_SpriteLoader_Preparations *stitch(LIBMATTI_MC_SpriteLoader *loader,
                                                     LIBMATTI_MC_SpriteContents **contents, size_t count,
                                                     int requestedMipLevel, const LIBMATTI_MC_Identifier *atlas)
{
    int maxSupportedTextureSize = loader->maxSupportedTextureSize;
    // Java: int j = Integer.MAX_VALUE - the min width/height over all sprites
    int minimum = 0x7FFFFFFF;
    int k = 1 << requestedMipLevel;

    for (size_t i = 0; i < count; i++)
    {
        LIBMATTI_MC_SpriteContents *sprite = contents[i];
        int smaller = sprite->width < sprite->height ? sprite->width : sprite->height;
        if (smaller < minimum)
            minimum = smaller;
        // Java: Integer.lowestOneBit - the mip clamp per sprite
        int lowestW = sprite->width & -sprite->width;
        int lowestH = sprite->height & -sprite->height;
        int lowest = lowestW < lowestH ? lowestW : lowestH;
        if (lowest < k)
            k = lowest;
    }

    int j1 = minimum < k ? minimum : k;
    int k1 = LIBMATTI_MC_Mth_Log2(j1);
    int effectiveMipLevel = k1 < requestedMipLevel ? k1 : requestedMipLevel;

    LIBMATTI_MC_Stitcher *stitcher =
        LIBMATTI_MC_Stitcher_New(maxSupportedTextureSize, maxSupportedTextureSize, effectiveMipLevel, 0);

    for (size_t i = 0; i < count; i++)
    {
        LIBMATTI_MC_Stitcher_Entry entry = {contents[i]->width, contents[i]->height, contents[i]->name->path};
        LIBMATTI_MC_Stitcher_RegisterSprite(stitcher, &entry);
    }

    if (!LIBMATTI_MC_Stitcher_Stitch(stitcher))
    {
        // Java: StitcherException -> CrashReport("Stitching")
        LIBMATTI_MC_Stitcher_Free(stitcher);
        return NULL;
    }

    LIBMATTI_MC_SpriteLoader_Preparations *preparations = calloc(1, sizeof(LIBMATTI_MC_SpriteLoader_Preparations));
    preparations->width = LIBMATTI_MC_Stitcher_GetWidth(stitcher);
    preparations->height = LIBMATTI_MC_Stitcher_GetHeight(stitcher);
    preparations->mipLevel = effectiveMipLevel;
    preparations->sprites = calloc(count > 0 ? count : 1, sizeof(LIBMATTI_MC_TextureAtlasSprite *));
    preparations->spriteNames = calloc(count > 0 ? count : 1, sizeof(LIBMATTI_MC_Identifier *));

    GatherContext context = {contents, count, preparations, (LIBMATTI_MC_Identifier *) atlas};
    LIBMATTI_MC_Stitcher_GatherSprites(stitcher, &context, gather_callback);

    // Java: map.values().forEach(contents -> increaseMipLevel(l1)) - after
    // gathering, every sprite's mip chain grows to the effective level.
    for (size_t i = 0; i < preparations->spriteCount; i++)
        LIBMATTI_MC_SpriteContents_IncreaseMipLevel(preparations->sprites[i]->contents, effectiveMipLevel);

    LIBMATTI_MC_Stitcher_Free(stitcher);
    return preparations;
}

// Java: public CompletableFuture<Preparations> loadAndStitch(ResourceManager, Identifier, int)
LIBMATTI_MC_SpriteLoader_Preparations *LIBMATTI_MC_SpriteLoader_LoadAndStitch(
    LIBMATTI_MC_SpriteLoader *loader, const LIBMATTI_MC_MultiPackResourceManager *resourceManager,
    const LIBMATTI_MC_Identifier *atlasInfo, int mipLevel)
{
    // Java: SpriteSourceList.load(...).list(...) -> the resource entries
    size_t sourceCount = 0;
    LIBMATTI_MC_SpriteSource **sources = LIBMATTI_MC_SpriteSourceList_Load(resourceManager, atlasInfo, &sourceCount);

    size_t entryCount = 0;
    LIBMATTI_MC_SpriteResourceEntry *entries =
        LIBMATTI_MC_SpriteSourceList_Run(sources, sourceCount, resourceManager, &entryCount);

    // Java: builder.add(p -> MissingTextureAtlasSprite.create()) runs first
    size_t spriteCount = entryCount + 1;
    LIBMATTI_MC_SpriteContents **contents = calloc(spriteCount, sizeof(LIBMATTI_MC_SpriteContents *));

    // The missing sprite leads the list (Java: the builder adds it first)
    LIBMATTI_MC_Identifier *missingLocation = LIBMATTI_MC_MissingTextureAtlasSprite_GetLocation();
    contents[0] = LIBMATTI_MC_MissingTextureAtlasSprite_Create();

    size_t decoded = 1;
    for (size_t i = 0; i < entryCount; i++)
    {
        LIBMATTI_MC_SpriteContents *sprite =
            LIBMATTI_MC_SpriteResourceLoader_LoadSprite(entries[i].spriteId, entries[i].resource);
        if (sprite != NULL)
            contents[decoded++] = sprite;
    }
    LIBMATTI_MC_SpriteResourceEntries_Free(entries, entryCount);
    LIBMATTI_MC_SpriteSourceList_Free(sources, sourceCount);

    LIBMATTI_MC_SpriteLoader_Preparations *preparations =
        stitch(loader, contents, decoded, mipLevel, loader->location);

    // The sprites the stitcher rejected (or that were filtered) still own
    // their images: free everything that did not land in the preparations.
    for (size_t i = 0; i < decoded; i++)
    {
        int used = 0;
        if (preparations != NULL)
        {
            for (size_t s = 0; s < preparations->spriteCount; s++)
            {
                if (preparations->sprites[s]->contents == contents[i])
                {
                    used = 1;
                    break;
                }
            }
        }
        if (!used)
            LIBMATTI_MC_SpriteContents_Free(contents[i]);
    }
    free(contents);
    LIBMATTI_MC_Identifier_Free(missingLocation);
    return preparations;
}

void LIBMATTI_MC_SpriteLoader_Preparations_Free(LIBMATTI_MC_SpriteLoader_Preparations *preparations)
{
    if (preparations == NULL)
        return;
    for (size_t i = 0; i < preparations->spriteCount; i++)
    {
        LIBMATTI_MC_TextureAtlasSprite_Free(preparations->sprites[i]);
        LIBMATTI_MC_Identifier_Free(preparations->spriteNames[i]);
    }
    free(preparations->sprites);
    free(preparations->spriteNames);
    free(preparations);
}
