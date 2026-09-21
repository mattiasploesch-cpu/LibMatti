// Port of net.minecraft.client.renderer.texture.AbstractTexture,
// ReloadableTexture, SimpleTexture, DynamicTexture, TickableTexture and
// MissingTextureAtlasSprite. Java's abstract class family becomes one struct
// with a kind discriminator; the texture upload runs through GlStateManager.

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_ABSTRACTTEXTURE_H
#define MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_ABSTRACTTEXTURE_H

#include "libmatti/com/mojang/blaze3d/platform/NativeImage.h"
#include "libmatti/net/minecraft/client/resources/metadata/TextureMetadataSections.h"
#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/net/minecraft/server/packs/resources/MultiPackResourceManager.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Java: GpuTexture/GpuTextureView - the port keeps the GL texture names
// (the GL binding has no separate view objects).

// Java: class AbstractTexture implements AutoCloseable
typedef struct LIBMATTI_MC_AbstractTexture LIBMATTI_MC_AbstractTexture;

// Java: interface TickableTexture { void tick(); }
typedef int (*LIBMATTI_MC_TickableTexture_Tick)(LIBMATTI_MC_AbstractTexture *texture);

// Java: sealed class family - the port's kind discriminator
typedef enum LIBMATTI_MC_TextureKind
{
    LIBMATTI_MC_TextureKind_SIMPLE = 0,
    LIBMATTI_MC_TextureKind_DYNAMIC,
    LIBMATTI_MC_TextureKind_ATLAS
} LIBMATTI_MC_TextureKind;

struct LIBMATTI_MC_AbstractTexture
{
    LIBMATTI_MC_TextureKind kind;

    // Java: protected GpuTexture texture; GpuTextureView textureView
    unsigned int texture;      // the GL texture name, 0 until loaded
    unsigned int textureView;  // the port: the same GL name
    // Java: protected GpuSampler sampler (repeat/nearest by default)
    int addressModeClamp; // 0 = REPEAT (default), 1 = CLAMP_TO_EDGE
    int filterLinear;     // 0 = NEAREST (default), 1 = LINEAR

    // Java: ReloadableTexture - the resource this texture loads from
    LIBMATTI_MC_Identifier *resourceId;

    // Java: TickableTexture - the atlas overrides this
    LIBMATTI_MC_TickableTexture_Tick tick;

    // DynamicTexture: Java: private NativeImage pixels
    LIBMATTI_B3D_NativeImage *pixels;

    // TextureAtlas fields (only when kind == ATLAS)
    void *atlasData;
};

// Java: public void close()
void LIBMATTI_MC_AbstractTexture_Close(LIBMATTI_MC_AbstractTexture *texture);
// Java: public GpuTexture getTexture() - 0 before initialization
unsigned int LIBMATTI_MC_AbstractTexture_GetTexture(const LIBMATTI_MC_AbstractTexture *texture);

// ---------------------------------------------------------------------------
// TextureContents (record TextureContents(NativeImage, TextureMetadataSection))
// ---------------------------------------------------------------------------

typedef struct LIBMATTI_MC_TextureContents
{
    LIBMATTI_B3D_NativeImage *image;
    LIBMATTI_MC_TextureMetadataSection *metadata; // NULL when absent
} LIBMATTI_MC_TextureContents;

// Java: public static TextureContents load(ResourceManager, Identifier)
// Returns NULL when the resource is missing or undecodable.
LIBMATTI_MC_TextureContents *LIBMATTI_MC_TextureContents_Load(
    const LIBMATTI_MC_MultiPackResourceManager *resourceManager, const LIBMATTI_MC_Identifier *resourceId);
// Java: public static TextureContents createMissing()
LIBMATTI_MC_TextureContents *LIBMATTI_MC_TextureContents_CreateMissing(void);
// Java: public boolean blur() / clamp()
int LIBMATTI_MC_TextureContents_Blur(const LIBMATTI_MC_TextureContents *contents);
int LIBMATTI_MC_TextureContents_Clamp(const LIBMATTI_MC_TextureContents *contents);
void LIBMATTI_MC_TextureContents_Free(LIBMATTI_MC_TextureContents *contents);

// ---------------------------------------------------------------------------
// ReloadableTexture / SimpleTexture / DynamicTexture constructors
// ---------------------------------------------------------------------------

// Java: public SimpleTexture(Identifier resourceId)
LIBMATTI_MC_AbstractTexture *LIBMATTI_MC_SimpleTexture_New(const LIBMATTI_MC_Identifier *resourceId);
// Java: public DynamicTexture(Supplier<String>, NativeImage) - takes ownership
LIBMATTI_MC_AbstractTexture *LIBMATTI_MC_DynamicTexture_New(LIBMATTI_B3D_NativeImage *pixels);

// Java: ReloadableTexture.apply(TextureContents) - sampler switch + upload
void LIBMATTI_MC_ReloadableTexture_Apply(LIBMATTI_MC_AbstractTexture *texture,
                                         LIBMATTI_MC_TextureContents *contents);
// Java: protected void doLoad(NativeImage)
void LIBMATTI_MC_ReloadableTexture_DoLoad(LIBMATTI_MC_AbstractTexture *texture, LIBMATTI_B3D_NativeImage *image);
// Java: protected TextureContents loadContents(ResourceManager)
LIBMATTI_MC_TextureContents *LIBMATTI_MC_ReloadableTexture_LoadContents(
    LIBMATTI_MC_AbstractTexture *texture, const LIBMATTI_MC_MultiPackResourceManager *resourceManager);

// Java: DynamicTexture.upload()
void LIBMATTI_MC_DynamicTexture_Upload(LIBMATTI_MC_AbstractTexture *texture);

// ---------------------------------------------------------------------------
// MissingTextureAtlasSprite
// ---------------------------------------------------------------------------

// Java: public static NativeImage generateMissingImage() - the 16x16 checkerboard
LIBMATTI_B3D_NativeImage *LIBMATTI_MC_MissingTextureAtlasSprite_GenerateMissingImage(void);
// Java: public static Identifier getLocation() - "minecraft:missingno"
LIBMATTI_MC_Identifier *LIBMATTI_MC_MissingTextureAtlasSprite_GetLocation(void);

// ---------------------------------------------------------------------------
// The GL upload the textures share (TextureUtil/CommandEncoder writeToTexture)
// ---------------------------------------------------------------------------

// Java: GlCommandEncoder.writeToTexture - uploads the RGBA raster to the
// texture through GlStateManager (pixel store, TexImage2D + TexSubImage2D).
void LIBMATTI_MC_Texture_WriteToTexture(unsigned int glTexture, const LIBMATTI_B3D_NativeImage *image);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_ABSTRACTTEXTURE_H
