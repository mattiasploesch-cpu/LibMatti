// Texture harness: the P3.6 texture pipeline without a GL context - the PNG
// decode through the PngReader port (fixtures generated in-code), the metadata
// parse, the Stitcher layout and the SpriteLoader/TextureAtlas wiring over a
// PathPackResources-backed resource manager.

#include "libmatti/com/mojang/blaze3d/platform/NativeImage.h"
#include "libmatti/com/mojang/blaze3d/platform/TextureUtil.h"
#include "libmatti/net/minecraft/client/renderer/texture/AbstractTexture.h"
#include "libmatti/net/minecraft/client/renderer/texture/MissingTextureAtlasSprite.h"
#include "libmatti/net/minecraft/client/renderer/texture/SpriteLoader.h"
#include "libmatti/net/minecraft/client/renderer/texture/Stitcher.h"
#include "libmatti/net/minecraft/client/renderer/texture/TextureAtlas.h"
#include "libmatti/net/minecraft/client/renderer/texture/TextureManager.h"
#include "libmatti/net/minecraft/client/renderer/texture/atlas/SpriteSourceList.h"
#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/net/minecraft/server/packs/PathPackResources.h"
#include "libmatti/net/minecraft/server/packs/resources/MultiPackResourceManager.h"
#include "libmatti/net/minecraft/util/ARGB.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks = 0;
static int failures = 0;

#define CHECK(cond)                                            \
    do                                                         \
    {                                                          \
        checks++;                                              \
        if (!(cond))                                           \
        {                                                      \
            failures++;                                        \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        }                                                      \
    } while (0)

// ---------------------------------------------------------------------------
// Minimal PNG writer (zlib stored blocks) for the fixtures
// ---------------------------------------------------------------------------

#include <zlib.h>

static unsigned long crc32_of(const unsigned char *data, size_t length)
{
    return crc32(0L, data, (unsigned int) length);
}

static void write_chunk(FILE *file, const char *type, const unsigned char *data, size_t length)
{
    unsigned char header[8];
    header[0] = (unsigned char) (length >> 24);
    header[1] = (unsigned char) (length >> 16);
    header[2] = (unsigned char) (length >> 8);
    header[3] = (unsigned char) length;
    memcpy(header + 4, type, 4);
    fwrite(header, 1, 8, file);
    if (length > 0)
        fwrite(data, 1, length, file);
    unsigned long crc = crc32(0L, (const unsigned char *) type, 4);
    if (length > 0)
        crc = crc32(crc, data, (unsigned int) length);
    unsigned char crcBytes[4] = {(unsigned char) (crc >> 24), (unsigned char) (crc >> 16),
                                 (unsigned char) (crc >> 8), (unsigned char) crc};
    fwrite(crcBytes, 1, 4, file);
}

static int write_rgba_png(const char *path, int width, int height, const unsigned char *rgba)
{
    FILE *file = fopen(path, "wb");
    if (file == NULL)
        return 0;

    static const unsigned char signature[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
    fwrite(signature, 1, 8, file);

    // IHDR: bit depth 8, colour type 6 (RGBA)
    unsigned char ihdr[13];
    ihdr[0] = (unsigned char) (width >> 24); ihdr[1] = (unsigned char) (width >> 16);
    ihdr[2] = (unsigned char) (width >> 8); ihdr[3] = (unsigned char) width;
    ihdr[4] = (unsigned char) (height >> 24); ihdr[5] = (unsigned char) (height >> 16);
    ihdr[6] = (unsigned char) (height >> 8); ihdr[7] = (unsigned char) height;
    ihdr[8] = 8; ihdr[9] = 6; ihdr[10] = 0; ihdr[11] = 0; ihdr[12] = 0;
    write_chunk(file, "IHDR", ihdr, 13);

    // IDAT: filter byte 0 per scanline, zlib "stored" deflate blocks
    size_t rawSize = (size_t) height * (1 + (size_t) width * 4);
    unsigned char *raw = malloc(rawSize);
    for (int y = 0; y < height; y++)
    {
        raw[y * (1 + width * 4)] = 0; // filter none
        memcpy(raw + y * (1 + width * 4) + 1, rgba + (size_t) y * width * 4, (size_t) width * 4);
    }

    uLongf compressedSize = compressBound((uLong) rawSize);
    unsigned char *compressed = malloc(compressedSize);
    compress2(compressed, &compressedSize, raw, (uLong) rawSize, Z_DEFAULT_COMPRESSION);

    unsigned char lengthBytes[4] = {(unsigned char) (compressedSize >> 24), (unsigned char) (compressedSize >> 16),
                                    (unsigned char) (compressedSize >> 8), (unsigned char) compressedSize};
    fwrite(lengthBytes, 1, 4, file);
    fwrite("IDAT", 1, 4, file);
    fwrite(compressed, 1, compressedSize, file);
    unsigned long crc = crc32(0L, (const unsigned char *) "IDAT", 4);
    crc = crc32(crc, compressed, (unsigned int) compressedSize);
    unsigned char crcBytes[4] = {(unsigned char) (crc >> 24), (unsigned char) (crc >> 16),
                                 (unsigned char) (crc >> 8), (unsigned char) crc};
    fwrite(crcBytes, 1, 4, file);

    free(raw);
    free(compressed);
    write_chunk(file, "IEND", NULL, 0);
    fclose(file);
    return 1;
}

// ---------------------------------------------------------------------------
// The fixture pack: assets/textureharness/textures/block/*.png
// ---------------------------------------------------------------------------

static void make_fixture_pack(const char *root)
{
    char path[512];
    snprintf(path, sizeof(path), "%s/assets/textureharness/textures/block", root);
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "mkdir -p '%s'", path);
    if (system(cmd) != 0) {}

    // A 16x16 red block texture
    unsigned char *rgba = malloc(16 * 16 * 4);
    for (int i = 0; i < 16 * 16; i++)
    {
        rgba[i * 4 + 0] = 220;
        rgba[i * 4 + 1] = 40;
        rgba[i * 4 + 2] = 40;
        rgba[i * 4 + 3] = 255;
    }
    snprintf(path, sizeof(path), "%s/assets/textureharness/textures/block/red.png", root);
    if (!write_rgba_png(path, 16, 16, rgba)) {}

    // A 16x16 texture with a transparent corner (mip CUTOUT path)
    for (int y = 0; y < 16; y++)
    {
        for (int x = 0; x < 16; x++)
        {
            int alpha = (x < 4 && y < 4) ? 0 : 255;
            rgba[(y * 16 + x) * 4 + 0] = 40;
            rgba[(y * 16 + x) * 4 + 1] = 200;
            rgba[(y * 16 + x) * 4 + 2] = 60;
            rgba[(y * 16 + x) * 4 + 3] = (unsigned char) alpha;
        }
    }
    snprintf(path, sizeof(path), "%s/assets/textureharness/textures/block/cutout.png", root);
    if (!write_rgba_png(path, 16, 16, rgba)) {}
    free(rgba);

    // The atlas definition: the directory source lists textures/block
    snprintf(path, sizeof(path), "%s/assets/textureharness/atlases", root);
    snprintf(cmd, sizeof(cmd), "mkdir -p '%s'", path);
    if (system(cmd) != 0) {}
    snprintf(path, sizeof(path), "%s/assets/textureharness/atlases/blocks.json", root);
    FILE *json = fopen(path, "w");
    if (json != NULL)
    {
        fprintf(json,
                "{\"sources\":[{\"type\":\"minecraft:directory\",\"source\":\"block\",\"prefix\":\"block/\"}]}");
        fclose(json);
    }
}

// ---------------------------------------------------------------------------
// Checks
// ---------------------------------------------------------------------------

// The one texture check that matters end to end: fixture pack -> PNG decode ->
// atlas JSON -> directory sprites -> stitch -> upload -> lookup.
static void check_sprite_pipeline(void)
{
    // Build the fixture pack under build/textureharness (the harness runs
    // with the tests/ directory as the working directory - see add_test).
    const char *root = "build/textureharness";
    make_fixture_pack(root);

    // A directory pack over the fixture root
    LIBMATTI_MC_PackLocationInfo *info = LIBMATTI_MC_PackLocationInfo_New("textureharness", "Texture Harness", NULL);
    LIBMATTI_MC_PathPackResources *pack = LIBMATTI_MC_PathPackResources_New(info, root, 1);    LIBMATTI_MC_PackResources *packs[1] = {(LIBMATTI_MC_PackResources *) pack};
    LIBMATTI_MC_MultiPackResourceManager *resources =
        LIBMATTI_MC_MultiPackResourceManager_New(LIBMATTI_MC_PackType_CLIENT_RESOURCES, packs, 1);

    // The atlas definition resolves and the directory source finds the two
    // block textures (+ the missing sprite appended by the loader).
    LIBMATTI_MC_Identifier *atlasId = LIBMATTI_MC_Identifier_WithDefaultNamespace("blocks");
    LIBMATTI_MC_SpriteLoader *loader = LIBMATTI_MC_SpriteLoader_Create(atlasId, 1024);
    LIBMATTI_MC_SpriteLoader_Preparations *preparations =
        LIBMATTI_MC_SpriteLoader_LoadAndStitch(loader, resources, atlasId, 0);

    CHECK(preparations != NULL);
    if (preparations != NULL)
    {
        CHECK(preparations->spriteCount == 3); // red + cutout + missingno
        CHECK(preparations->width >= 32);
        CHECK(preparations->missing != NULL || preparations->spriteCount == 3);

        // The atlas takes the sprites over and reports the stitched page.
        LIBMATTI_MC_TextureAtlas *atlas = LIBMATTI_MC_TextureAtlas_New(atlasId, 1024);
        CHECK(LIBMATTI_MC_TextureAtlas_Upload(atlas, preparations));

        LIBMATTI_MC_Identifier *redId = LIBMATTI_MC_Identifier_WithDefaultNamespace("block/red");
        LIBMATTI_MC_TextureAtlasSprite *red = LIBMATTI_MC_TextureAtlas_GetSprite(atlas, redId);
        CHECK(red != NULL);
        CHECK(red->contents->width == 16);
        CHECK(red->x >= 0 && red->x < preparations->width);
        LIBMATTI_MC_Identifier_Free(redId);

        // An unknown sprite falls back to the missing sprite
        LIBMATTI_MC_Identifier *unknownId = LIBMATTI_MC_Identifier_WithDefaultNamespace("block/unknown");
        LIBMATTI_MC_TextureAtlasSprite *unknown = LIBMATTI_MC_TextureAtlas_GetSprite(atlas, unknownId);
        CHECK(unknown == atlas->missingSprite);
        LIBMATTI_MC_Identifier_Free(unknownId);

        LIBMATTI_MC_TextureAtlas_Free(atlas);
        LIBMATTI_MC_SpriteLoader_Preparations_Free(preparations);
    }

    LIBMATTI_MC_SpriteLoader_Free(loader);
    LIBMATTI_MC_Identifier_Free(atlasId);
    LIBMATTI_MC_MultiPackResourceManager_Free(resources);
    LIBMATTI_MC_PathPackResources_Free(pack);
}

static void check_texture_manager(void)
{
    // The TextureManager without a GL context: the missing texture registers.
    LIBMATTI_MC_TextureManager *manager = LIBMATTI_MC_TextureManager_New(NULL);
    CHECK(manager != NULL);

    LIBMATTI_MC_Identifier *missingId = LIBMATTI_MC_MissingTextureAtlasSprite_GetLocation();
    LIBMATTI_MC_AbstractTexture *missing = LIBMATTI_MC_TextureManager_GetTexture(manager, missingId);
    CHECK(missing != NULL);
    LIBMATTI_MC_Identifier_Free(missingId);

    LIBMATTI_MC_TextureManager_Free(manager);
}

int main(void)
{
    check_sprite_pipeline();
    check_texture_manager();

    printf("texture harness: %d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
