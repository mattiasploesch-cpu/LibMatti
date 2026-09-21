// Implementation of the AbstractTexture family. The upload runs through
// GlStateManager exactly like Java's GlCommandEncoder.writeToTexture -> the
// GlStateManager texture path.

#include "libmatti/net/minecraft/client/renderer/texture/AbstractTexture.h"

#include "libmatti/com/google/gson/Gson.h"
#include "libmatti/com/google/gson/JsonElement.h"
#include "libmatti/com/mojang/blaze3d/opengl/GlStateManager.h"
#include "libmatti/org/lwjgl/opengl/Constants.h"
#include "libmatti/org/lwjgl/opengl/GL.h"

#include <stdlib.h>
#include <string.h>

// Java: protected GpuSampler sampler = RenderSystem.getSamplerCache().getSampler(
//       REPEAT, REPEAT, NEAREST, LINEAR, false) - the repeat/nearest default.
static void init_sampler(LIBMATTI_MC_AbstractTexture *texture)
{
    texture->addressModeClamp = 0;
    texture->filterLinear = 0;
}

// Java: public void close()
void LIBMATTI_MC_AbstractTexture_Close(LIBMATTI_MC_AbstractTexture *texture)
{
    if (texture == NULL)
        return;
    // Java: if (this.texture != null) { this.texture.close(); this.texture = null; }
    if (texture->texture != 0)
    {
        LIBMATTI_B3D_GlStateManager_DeleteTexture(texture->texture);
        texture->texture = 0;
    }
    texture->textureView = 0;
}

// Java: public GpuTexture getTexture()
unsigned int LIBMATTI_MC_AbstractTexture_GetTexture(const LIBMATTI_MC_AbstractTexture *texture)
{
    return texture->texture;
}

// ---------------------------------------------------------------------------
// TextureContents
// ---------------------------------------------------------------------------

// Java: resource.metadata().getSection(TextureMetadataSection.TYPE) - the
// port parses the .mcmeta bytes as JSON and picks the "texture" object.
static LIBMATTI_MC_TextureMetadataSection *parse_texture_metadata(const LIBMATTI_MC_Resource *resource)
{
    size_t metadataLength = 0;
    const unsigned char *metadataBytes = LIBMATTI_MC_Resource_Metadata(resource, &metadataLength);
    if (metadataBytes == NULL || metadataLength == 0)
        return NULL;

    LIBMATTI_GSON_Gson *gson = LIBMATTI_GSON_Gson_New();
    LIBMATTI_GSON_JsonElement *root = LIBMATTI_GSON_Gson_FromJson(gson, (const char *) metadataBytes, metadataLength);
    LIBMATTI_MC_TextureMetadataSection *section = NULL;
    if (root != NULL && LIBMATTI_GSON_JsonElement_IsJsonObject(root))
    {
        LIBMATTI_GSON_JsonElement *textureObject = LIBMATTI_GSON_JsonElement_GetMember(root, "texture");
        if (textureObject != NULL)
            section = LIBMATTI_MC_TextureMetadataSection_Parse(textureObject, NULL);
    }
    LIBMATTI_GSON_JsonElement_Free(root);
    LIBMATTI_GSON_Gson_Free(gson);
    return section;
}

// Java: public static TextureContents load(ResourceManager, Identifier)
LIBMATTI_MC_TextureContents *LIBMATTI_MC_TextureContents_Load(
    const LIBMATTI_MC_MultiPackResourceManager *resourceManager, const LIBMATTI_MC_Identifier *resourceId)
{
    // Java: Resource resource = resourceManager.getResourceOrThrow(location)
    LIBMATTI_MC_Resource *resource =
        LIBMATTI_MC_MultiPackResourceManager_GetResource(resourceManager, resourceId->namespace, resourceId->path);
    if (resource == NULL)
        return NULL;

    // Java: NativeImage.read(resource.open()) - Resource_Open hands out the
    // owning pointer (no copy), so the bytes stay with the resource here.
    size_t length = 0;
    unsigned char *bytes = LIBMATTI_MC_Resource_Open(resource, &length);
    if (bytes == NULL)
    {
        LIBMATTI_MC_Resource_Free(resource);
        return NULL;
    }

    LIBMATTI_B3D_NativeImage *image = LIBMATTI_B3D_NativeImage_Read(bytes, length);
    if (image == NULL)
    {
        LIBMATTI_MC_Resource_Free(resource);
        return NULL;
    }

    LIBMATTI_MC_TextureContents *contents = calloc(1, sizeof(LIBMATTI_MC_TextureContents));
    contents->image = image;
    contents->metadata = parse_texture_metadata(resource);
    LIBMATTI_MC_Resource_Free(resource);
    return contents;
}

// Java: public static TextureContents createMissing()
LIBMATTI_MC_TextureContents *LIBMATTI_MC_TextureContents_CreateMissing(void)
{
    LIBMATTI_MC_TextureContents *contents = calloc(1, sizeof(LIBMATTI_MC_TextureContents));
    contents->image = LIBMATTI_MC_MissingTextureAtlasSprite_GenerateMissingImage();
    contents->metadata = NULL;
    return contents;
}

// Java: public boolean blur()
int LIBMATTI_MC_TextureContents_Blur(const LIBMATTI_MC_TextureContents *contents)
{
    return contents->metadata != NULL ? contents->metadata->blur : 0;
}

// Java: public boolean clamp()
int LIBMATTI_MC_TextureContents_Clamp(const LIBMATTI_MC_TextureContents *contents)
{
    return contents->metadata != NULL ? contents->metadata->clamp : 0;
}

// Java: public void close() { this.image.close(); }
void LIBMATTI_MC_TextureContents_Free(LIBMATTI_MC_TextureContents *contents)
{
    if (contents == NULL)
        return;
    LIBMATTI_B3D_NativeImage_Free(contents->image);
    LIBMATTI_MC_TextureMetadataSection_Free(contents->metadata);
    free(contents);
}

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

// Java: public SimpleTexture(Identifier resourceId)
LIBMATTI_MC_AbstractTexture *LIBMATTI_MC_SimpleTexture_New(const LIBMATTI_MC_Identifier *resourceId)
{
    LIBMATTI_MC_AbstractTexture *texture = calloc(1, sizeof(LIBMATTI_MC_AbstractTexture));
    texture->kind = LIBMATTI_MC_TextureKind_SIMPLE;
    init_sampler(texture);
    texture->resourceId = LIBMATTI_MC_Identifier_FromNamespaceAndPath(resourceId->namespace, resourceId->path);
    return texture;
}

// Java: public DynamicTexture(Supplier<String> label, NativeImage pixels) -
// createTexture + upload in the constructor; the port uploads lazily through
// DynamicTexture_Upload because the GL context may not exist yet at creation.
LIBMATTI_MC_AbstractTexture *LIBMATTI_MC_DynamicTexture_New(LIBMATTI_B3D_NativeImage *pixels)
{
    LIBMATTI_MC_AbstractTexture *texture = calloc(1, sizeof(LIBMATTI_MC_AbstractTexture));
    texture->kind = LIBMATTI_MC_TextureKind_DYNAMIC;
    init_sampler(texture);
    texture->pixels = pixels;
    return texture;
}

// ---------------------------------------------------------------------------
// ReloadableTexture
// ---------------------------------------------------------------------------

// Java: protected void doLoad(NativeImage)
void LIBMATTI_MC_ReloadableTexture_DoLoad(LIBMATTI_MC_AbstractTexture *texture, LIBMATTI_B3D_NativeImage *image)
{
    // Java: GpuDevice.createTexture(resourceId::toString, 5, RGBA8, w, h, 1, 1)
    LIBMATTI_MC_AbstractTexture_Close(texture);
    texture->texture = LIBMATTI_B3D_GlStateManager_GenTexture();
    texture->textureView = texture->texture;
    LIBMATTI_MC_Texture_WriteToTexture(texture->texture, image);
}

// Java: ReloadableTexture.apply(TextureContents) - the sampler comes from the
// metadata; the metadata section carries blur/clamp.
void LIBMATTI_MC_ReloadableTexture_Apply(LIBMATTI_MC_AbstractTexture *texture,
                                         LIBMATTI_MC_TextureContents *contents)
{
    // Java: AddressMode mode = clamp ? CLAMP_TO_EDGE : REPEAT
    texture->addressModeClamp = LIBMATTI_MC_TextureContents_Clamp(contents);
    texture->filterLinear = LIBMATTI_MC_TextureContents_Blur(contents);

    // Java: try (NativeImage image = contents.image()) { this.doLoad(image); }
    LIBMATTI_MC_ReloadableTexture_DoLoad(texture, contents->image);
    LIBMATTI_B3D_NativeImage_Free(contents->image);
    contents->image = NULL;
}

// Java: SimpleTexture.loadContents(ResourceManager) = TextureContents.load(...)
LIBMATTI_MC_TextureContents *LIBMATTI_MC_ReloadableTexture_LoadContents(
    LIBMATTI_MC_AbstractTexture *texture, const LIBMATTI_MC_MultiPackResourceManager *resourceManager)
{
    return LIBMATTI_MC_TextureContents_Load(resourceManager, texture->resourceId);
}

// Java: DynamicTexture.upload()
void LIBMATTI_MC_DynamicTexture_Upload(LIBMATTI_MC_AbstractTexture *texture)
{
    if (texture->pixels == NULL || texture->texture == 0)
        return;
    LIBMATTI_MC_Texture_WriteToTexture(texture->texture, texture->pixels);
}

// ---------------------------------------------------------------------------
// MissingTextureAtlasSprite
// ---------------------------------------------------------------------------

// Java: public static NativeImage generateMissingImage(int width, int height)
static LIBMATTI_B3D_NativeImage *generate_missing_image(int width, int height)
{
    LIBMATTI_B3D_NativeImage *image = LIBMATTI_B3D_NativeImage_New(width, height, 0);
    if (image == NULL)
        return NULL;
    // Java: int i = -524040 - the magenta
    const int magenta = -524040;
    const int black = -16777216;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            // Java: if (j < height / 2 ^ k < width / 2) - the XOR checkerboard
            if ((y < height / 2) ^ (x < width / 2))
                LIBMATTI_B3D_NativeImage_SetPixel(image, x, y, magenta);
            else
                LIBMATTI_B3D_NativeImage_SetPixel(image, x, y, black);
        }
    }
    return image;
}

LIBMATTI_B3D_NativeImage *LIBMATTI_MC_MissingTextureAtlasSprite_GenerateMissingImage(void)
{
    return generate_missing_image(16, 16);
}

LIBMATTI_MC_Identifier *LIBMATTI_MC_MissingTextureAtlasSprite_GetLocation(void)
{
    // Java: Identifier.withDefaultNamespace("missingno")
    return LIBMATTI_MC_Identifier_WithDefaultNamespace("missingno");
}

// ---------------------------------------------------------------------------
// writeToTexture
// ---------------------------------------------------------------------------

// Java: GlCommandEncoder.writeToTexture(GpuTexture, NativeImage) - the GL path
// (GlTextureManager/GlCommandEncoder): pixel store, bind, TexImage2D.
void LIBMATTI_MC_Texture_WriteToTexture(unsigned int glTexture, const LIBMATTI_B3D_NativeImage *image)
{
    LIBMATTI_GL_glPixelStorei(LIBMATTI_GL_GL_UNPACK_ALIGNMENT, 1);
    LIBMATTI_B3D_GlStateManager_BindTexture(glTexture);
    LIBMATTI_GL_glTexImage2D(LIBMATTI_GL_GL_TEXTURE_2D, 0, LIBMATTI_GL_GL_RGBA8,
                             image->width, image->height, 0,
                             LIBMATTI_GL_GL_RGBA, LIBMATTI_GL_GL_UNSIGNED_BYTE, image->pixels);
}
