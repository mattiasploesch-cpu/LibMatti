#include "libmatti/matti/ui/texture.h"

#include "libmatti/matti/native/Library.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stb_image.h"




#define ASSET_SECTION_PREFIX ".matti_asset_"
#define ASSET_SECTION_MAX 192

static char asset_root[512];
static int asset_root_set = 0;

typedef struct TextureCacheEntry
{
    char *name;
    LIBMATTI_UI_Texture *texture;
} TextureCacheEntry;

static TextureCacheEntry *cache = NULL;
static size_t cache_count = 0;

const char *LIBMATTI_UI_Texture_AssetRoot(void)
{
    return asset_root_set ? asset_root : NULL;
}

void LIBMATTI_UI_Texture_SetAssetRoot(const char *root)
{
    if (root == NULL)
    {
        asset_root_set = 0;
        asset_root[0] = '\0';
        return;
    }
    snprintf(asset_root, sizeof(asset_root), "%s", root);
    asset_root_set = 1;
}

static unsigned char *read_file(const char *path, size_t *out_size)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL)
        return NULL;
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (size <= 0)
    {
        fclose(file);
        return NULL;
    }
    unsigned char *data = malloc((size_t) size);
    if (data != NULL && fread(data, 1, (size_t) size, file) != (size_t) size)
    {
        free(data);
        data = NULL;
    }
    fclose(file);
    if (data != NULL)
        *out_size = (size_t) size;
    return data;
}



static void section_name(const char *name, char *out, size_t out_size)
{
    size_t written = (size_t) snprintf(out, out_size, "%s", ASSET_SECTION_PREFIX);
    for (const char *cursor = name; *cursor != '\0' && written + 1 < out_size; cursor++)
    {
        char c = *cursor;
        out[written++] = (c == '/' || c == '.' || c == '-') ? '_' : c;
    }
    out[written] = '\0';
}

static LIBMATTI_UI_Texture *decode(const unsigned char *bytes, size_t size)
{
    int width = 0, height = 0, channels = 0;
    unsigned char *rgba = stbi_load_from_memory(bytes, (int) size, &width, &height, &channels, 4);
    if (rgba == NULL || width <= 0 || height <= 0)
    {
        if (rgba != NULL)
            stbi_image_free(rgba);
        return NULL;
    }

    LIBMATTI_UI_Texture *texture = calloc(1, sizeof(LIBMATTI_UI_Texture));
    if (texture == NULL)
    {
        stbi_image_free(rgba);
        return NULL;
    }
    texture->width = width;
    texture->height = height;
    texture->pixels = malloc((size_t) width * (size_t) height * sizeof(uint32_t));
    if (texture->pixels == NULL)
    {
        free(texture);
        stbi_image_free(rgba);
        return NULL;
    }
    for (size_t i = 0; i < (size_t) width * (size_t) height; i++)
    {
        unsigned int r = rgba[i * 4 + 0];
        unsigned int g = rgba[i * 4 + 1];
        unsigned int b = rgba[i * 4 + 2];
        unsigned int a = rgba[i * 4 + 3];
        texture->pixels[i] = a << 24 | r << 16 | g << 8 | b;
    }
    stbi_image_free(rgba);
    return texture;
}





static LIBMATTI_UI_Texture *load_uncached(const char *name)
{
    char path[1024];
    size_t size = 0;
    unsigned char *bytes = NULL;

    if (asset_root_set)
    {
        snprintf(path, sizeof(path), "%s/minecraft/textures/%s", asset_root, name);
        bytes = read_file(path, &size);
    }
    if (bytes == NULL)
    {
        snprintf(path, sizeof(path), "assets/minecraft/textures/%s", name);
        bytes = read_file(path, &size);
    }
    if (bytes == NULL)
    {


        const char *ui_root = getenv("MATTI_UI_ASSETS");
        if (ui_root != NULL && ui_root[0] != '\0')
        {
            snprintf(path, sizeof(path), "%s/%s", ui_root, name);
            bytes = read_file(path, &size);
        }
    }
    if (bytes == NULL)
    {
        snprintf(path, sizeof(path), "%s/library/src/libmatti/matti/ui/assets/%s", MATTI_SOURCE_DIR, name);
        bytes = read_file(path, &size);
    }
    if (bytes == NULL)
    {
        char section[ASSET_SECTION_MAX];
        section_name(name, section, sizeof(section));
        unsigned char *embedded = NULL;
        size_t embedded_size = 0;
        if (LIBMATTI_NATIVE_ReadEmbeddedSection(section, &embedded, &embedded_size))
            bytes = embedded;
    }
    if (bytes == NULL)
        return NULL;

    LIBMATTI_UI_Texture *texture = decode(bytes, size);
    free(bytes);
    return texture;
}

LIBMATTI_UI_Texture *LIBMATTI_UI_Texture_Load(const char *name)
{
    if (name == NULL || name[0] == '\0')
        return NULL;
    for (size_t i = 0; i < cache_count; i++)
    {
        if (strcmp(cache[i].name, name) == 0)
            return cache[i].texture;
    }

    LIBMATTI_UI_Texture *texture = load_uncached(name);
    TextureCacheEntry *grown = realloc(cache, (cache_count + 1) * sizeof(TextureCacheEntry));
    if (grown == NULL)
        return texture;
    cache = grown;
    cache[cache_count].name = strdup(name);
    cache[cache_count].texture = texture;
    cache_count++;
    return texture;
}

int LIBMATTI_UI_Texture_Width(const LIBMATTI_UI_Texture *texture)
{
    return texture != NULL ? texture->width : 0;
}

int LIBMATTI_UI_Texture_Height(const LIBMATTI_UI_Texture *texture)
{
    return texture != NULL ? texture->height : 0;
}

const uint32_t *LIBMATTI_UI_Texture_Pixels(const LIBMATTI_UI_Texture *texture)
{
    return texture != NULL ? texture->pixels : NULL;
}
