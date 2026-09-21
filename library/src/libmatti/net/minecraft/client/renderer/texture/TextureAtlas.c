// Implementation of net.minecraft.client.renderer.texture.TextureAtlas.

#include "libmatti/net/minecraft/client/renderer/texture/TextureAtlas.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/com/mojang/blaze3d/opengl/GlStateManager.h"
#include "libmatti/net/minecraft/client/renderer/texture/MissingTextureAtlasSprite.h"
#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/org/lwjgl/opengl/Constants.h"
#include "libmatti/org/lwjgl/opengl/GL.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOGGER() LIBMATTI_ML_LogManager_GetLogger()

// Java: public static final Identifier LOCATION_BLOCKS
static LIBMATTI_MC_Identifier *location_blocks = NULL;
const LIBMATTI_MC_Identifier *const LIBMATTI_MC_TextureAtlas_LOCATION_BLOCKS = NULL;

static const LIBMATTI_MC_Identifier *get_location_blocks(void)
{
    if (location_blocks == NULL)
        location_blocks = LIBMATTI_MC_Identifier_WithDefaultNamespace("textures/atlas/blocks.png");
    return location_blocks;
}

// Java: private void createTexture(int width, int height, int mipLevel)
static void create_texture(LIBMATTI_MC_TextureAtlas *atlas, int width, int height, int mipLevel)
{
    char message[256];
    snprintf(message, sizeof(message), "Created: %dx%dx%d atlas", width, height, mipLevel);
    LIBMATTI_ML_Logger_Info(LOGGER(), NULL, message);

    // Java: this.close(); texture = createTexture(..., mipLevel + 1)
    LIBMATTI_MC_AbstractTexture_Close(&atlas->base);
    atlas->base.texture = LIBMATTI_B3D_GlStateManager_GenTexture();
    atlas->base.textureView = atlas->base.texture;
    atlas->width = width;
    atlas->height = height;
    atlas->maxMipLevel = mipLevel;

    // Java: the sampler is getClampToEdge(NEAREST)
    atlas->base.addressModeClamp = 1;
    atlas->base.filterLinear = 0;
}

LIBMATTI_MC_TextureAtlas *LIBMATTI_MC_TextureAtlas_New(const LIBMATTI_MC_Identifier *location,
                                                       int maxSupportedTextureSize)
{
    LIBMATTI_MC_TextureAtlas *atlas = calloc(1, sizeof(LIBMATTI_MC_TextureAtlas));
    atlas->base.kind = LIBMATTI_MC_TextureKind_ATLAS;
    atlas->base.tick = (LIBMATTI_MC_TickableTexture_Tick) LIBMATTI_MC_TextureAtlas_Tick;
    atlas->location = LIBMATTI_MC_Identifier_FromNamespaceAndPath(location->namespace, location->path);
    atlas->maxSupportedTextureSize = maxSupportedTextureSize;
    return atlas;
}

// Java: public void upload(SpriteLoader.Preparations)
int LIBMATTI_MC_TextureAtlas_Upload(LIBMATTI_MC_TextureAtlas *atlas, LIBMATTI_MC_SpriteLoader_Preparations *preparations)
{
    create_texture(atlas, preparations->width, preparations->height, preparations->mipLevel);
    LIBMATTI_MC_TextureAtlas_ClearTextureData(atlas);

    // Java: this.texturesByName = Map.copyOf(regions); missingSprite = get(missingno)
    atlas->spriteCount = preparations->spriteCount;
    atlas->sprites = calloc(atlas->spriteCount > 0 ? atlas->spriteCount : 1, sizeof(LIBMATTI_MC_TextureAtlasSprite *));
    for (size_t i = 0; i < atlas->spriteCount; i++)
        atlas->sprites[i] = preparations->sprites[i];

    LIBMATTI_MC_Identifier *missingLocation = LIBMATTI_MC_MissingTextureAtlasSprite_GetLocation();
    atlas->missingSprite = LIBMATTI_MC_TextureAtlas_GetSprite(atlas, missingLocation);
    LIBMATTI_MC_Identifier_Free(missingLocation);

    if (atlas->missingSprite == NULL)
    {
        // Java: throw new IllegalStateException("Atlas ... has no missing texture sprite")
        atlas->spriteCount = 0;
        free(atlas->sprites);
        atlas->sprites = NULL;
        return 0;
    }

    // Java: uploadInitialContents - the port uploads every sprite's first mip
    // frame into the atlas region directly (the Java blit runs through the
    // ANIMATE_SPRITE_BLIT render pass, a game-port pipeline).
    LIBMATTI_GL_glPixelStorei(LIBMATTI_GL_GL_UNPACK_ALIGNMENT, 1);
    LIBMATTI_B3D_GlStateManager_BindTexture(atlas->base.texture);
    for (size_t i = 0; i < atlas->spriteCount; i++)
    {
        LIBMATTI_MC_TextureAtlasSprite *sprite = atlas->sprites[i];
        LIBMATTI_B3D_NativeImage *image = sprite->contents->byMipLevel[0];
        if (image == NULL)
            continue;

        // Java: the first frame blits the sprite into its atlas region -
        // TexSubImage2D at (x, y) with the sprite size.
        LIBMATTI_GL_glTexSubImage2D(LIBMATTI_GL_GL_TEXTURE_2D, 0, sprite->x, sprite->y,
                                    image->width, image->height, LIBMATTI_GL_GL_RGBA,
                                    LIBMATTI_GL_GL_UNSIGNED_BYTE, image->pixels);
    }
    return 1;
}

// Java: public TextureAtlasSprite getSprite(Identifier)
LIBMATTI_MC_TextureAtlasSprite *LIBMATTI_MC_TextureAtlas_GetSprite(const LIBMATTI_MC_TextureAtlas *atlas,
                                                                   const LIBMATTI_MC_Identifier *spriteId)
{
    for (size_t i = 0; i < atlas->spriteCount; i++)
    {
        LIBMATTI_MC_Identifier *name = atlas->sprites[i]->contents->name;
        if (strcmp(name->namespace, spriteId->namespace) == 0 && strcmp(name->path, spriteId->path) == 0)
            return atlas->sprites[i];
    }
    // Java: getOrDefault(sprite, missingSprite)
    return atlas->missingSprite;
}

// Java: public void clearTextureData() - the sprites return to the loader's
// ownership in the port (the preparations free them), so the atlas only drops
// its references here when told.
void LIBMATTI_MC_TextureAtlas_ClearTextureData(LIBMATTI_MC_TextureAtlas *atlas)
{
    atlas->sprites = NULL;
    atlas->spriteCount = 0;
    atlas->missingSprite = NULL;
}

// Java: public void tick() { cycleAnimationFrames(); } - the animated frames
// run through the sprite-blit pipeline (game-port part; nothing to cycle yet)
void LIBMATTI_MC_TextureAtlas_Tick(LIBMATTI_MC_TextureAtlas *atlas)
{
    (void) atlas;
}

int LIBMATTI_MC_TextureAtlas_GetWidth(const LIBMATTI_MC_TextureAtlas *atlas)
{
    return atlas->width;
}

int LIBMATTI_MC_TextureAtlas_GetHeight(const LIBMATTI_MC_TextureAtlas *atlas)
{
    return atlas->height;
}

void LIBMATTI_MC_TextureAtlas_Free(LIBMATTI_MC_TextureAtlas *atlas)
{
    if (atlas == NULL)
        return;
    LIBMATTI_MC_AbstractTexture_Close(&atlas->base);
    free(atlas->sprites);
    LIBMATTI_MC_Identifier_Free(atlas->location);
    free(atlas);
}
