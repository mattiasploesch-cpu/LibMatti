#include "libmatti/matti/ui/bridge.h"

#include "libmatti/matti/ui/texture.h"
#include "libmatti/matti/Platform.h"

#include "libmatti/matti/native/Library.h"
#include "libmatti/matti/render/render.h"
#include "libmatti/net/neoforged/fml/earlydisplay/DisplayWindow.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stb_truetype.h"




#define UI_FONT_SECTION ".matti_font_monocraft"
#define UI_FONT_RESOURCE \
    MATTI_SOURCE_DIR "/vendor/FancyModLoader/earlydisplay/src/main/resources/net/neoforged/fml/earlydisplay/theme/Monocraft.ttf"

#define UI_FONT_PIXEL_HEIGHT 8
#define UI_FONT_MAX_SCALE 12
#define UI_FONT_FIRST_CODEPOINT 32
#define UI_FONT_LAST_CODEPOINT 126
#define UI_FONT_GLYPH_COUNT (UI_FONT_LAST_CODEPOINT - UI_FONT_FIRST_CODEPOINT + 1)

typedef struct UiGlyph
{
    int advance;
    int x0;
    int y0;
    int width;
    int height;
    unsigned char *bitmap;
} UiGlyph;

typedef struct UiFontSize
{
    int loaded;
    int attempted;
    float scale;
    int ascent;
    int descent;
    UiGlyph glyphs[UI_FONT_GLYPH_COUNT];
} UiFontSize;

typedef struct UiFont
{
    int loaded;
    int attempted;
    stbtt_fontinfo info;
    unsigned char *data;
    UiFontSize sizes[UI_FONT_MAX_SCALE + 1];
} UiFont;

static UiFont font;

static LIBMATTI_Renderer *renderer = NULL;
static int width = 0;
static int height = 0;
static int close_requested = 0;
static char backend[16] = "none";
static char requested_selection[64] = "auto:auto";
static char last_error[256] = "";






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

static void font_load(void)
{
    if (font.loaded || font.attempted)
        return;
    font.attempted = 1;

    size_t size = 0;
    unsigned char *data = NULL;
    const char *font_override = getenv("MATTI_THEME_FONT");
    if (font_override != NULL && font_override[0] != '\0')
        data = read_file(font_override, &size);
    if (data == NULL && !LIBMATTI_NATIVE_ReadEmbeddedSection(UI_FONT_SECTION, &data, &size))
        data = read_file(UI_FONT_RESOURCE, &size);
    if (data == NULL)
    {
        fprintf(stderr, "[ui] no Monocraft font available, text rendering disabled\n");
        return;
    }

    int offset = stbtt_GetFontOffsetForIndex(data, 0);
    if (offset < 0 || !stbtt_InitFont(&font.info, data, offset))
    {
        free(data);
        return;
    }

    font.data = data;
    font.loaded = 1;
}

/* Rasterize each requested text size at its real pixel height. Scaling the
 * 16 px bitmap with nearest-neighbour (the old path) produced the blocky text
 * the menu showed at larger GUI scales. */
static UiFontSize *font_size(int scale)
{
    font_load();
    if (!font.loaded)
        return NULL;
    if (scale < 1) scale = 1;
    if (scale > UI_FONT_MAX_SCALE) scale = UI_FONT_MAX_SCALE;

    UiFontSize *size = &font.sizes[scale];
    if (size->loaded || size->attempted)
        return size->loaded ? size : NULL;
    size->attempted = 1;
    size->scale = stbtt_ScaleForPixelHeight(&font.info, (float) UI_FONT_PIXEL_HEIGHT * (float) scale);

    int ascent = 0, descent = 0, line_gap = 0;
    stbtt_GetFontVMetrics(&font.info, &ascent, &descent, &line_gap);
    size->ascent = (int) (ascent * size->scale + 0.5f);
    size->descent = (int) (descent * size->scale - 0.5f);

    for (int i = 0; i < UI_FONT_GLYPH_COUNT; i++)
    {
        int codepoint = UI_FONT_FIRST_CODEPOINT + i;
        UiGlyph *glyph = &size->glyphs[i];

        int advance = 0, left_bearing = 0;
        stbtt_GetCodepointHMetrics(&font.info, codepoint, &advance, &left_bearing);
        glyph->advance = (int) (advance * size->scale + 0.5f);
        glyph->bitmap = stbtt_GetCodepointBitmap(&font.info, 0, size->scale, codepoint,
                                                 &glyph->width, &glyph->height, &glyph->x0,
                                                 &glyph->y0);
    }

    size->loaded = 1;
    return size;
}

static const UiGlyph *font_glyph(const UiFontSize *size, int codepoint)
{
    if (size == NULL || codepoint < UI_FONT_FIRST_CODEPOINT
        || codepoint > UI_FONT_LAST_CODEPOINT)
        return NULL;
    return &size->glyphs[codepoint - UI_FONT_FIRST_CODEPOINT];
}

int LIBMATTI_UI_TextHeight(int scale)
{
    const UiFontSize *size = font_size(scale);
    if (size == NULL)
        return UI_FONT_PIXEL_HEIGHT * (scale < 1 ? 1 : scale);
    return size->ascent - size->descent;
}

int LIBMATTI_UI_TextWidth(const char *text, int scale)
{
    const UiFontSize *size = font_size(scale);
    if (text == NULL || size == NULL)
        return 0;

    int width_total = 0;
    for (const unsigned char *cursor = (const unsigned char *) text; *cursor != '\0'; cursor++)
    {
        const UiGlyph *glyph = font_glyph(size, *cursor);
        if (glyph != NULL)
            width_total += glyph->advance;
    }
    return width_total;
}




static void blend_pixel(int x, int y, uint32_t argb, unsigned int coverage)
{
    if (renderer == NULL || x < 0 || y < 0 || x >= width || y >= height)
        return;
    unsigned int alpha = coverage * ((argb >> 24) & 0xFFu) / 255u;
    if (alpha == 0)
        return;
    if (alpha >= 255)
    {
        LIBMATTI_Render_DrawPixel(renderer, x, y, argb | 0xFF000000u);
        return;
    }

    uint32_t destination = LIBMATTI_Render_GetPixel(renderer, x, y);
    unsigned int inverse = 255u - alpha;
    unsigned int red = (((argb >> 16) & 0xFFu) * alpha + ((destination >> 16) & 0xFFu) * inverse) / 255u;
    unsigned int green = (((argb >> 8) & 0xFFu) * alpha + ((destination >> 8) & 0xFFu) * inverse) / 255u;
    unsigned int blue = ((argb & 0xFFu) * alpha + (destination & 0xFFu) * inverse) / 255u;
    LIBMATTI_Render_DrawPixel(renderer, x, y, 0xFF000000u | red << 16 | green << 8 | blue);
}

void LIBMATTI_UI_DrawText(const char *text, int x, int y, uint32_t argb, int scale, int shadow)
{
    const UiFontSize *size = font_size(scale);
    if (renderer == NULL || text == NULL || size == NULL)
        return;
    if (scale < 1) scale = 1;

    int origin_x = x;
    int baseline = y + size->ascent;
    int line_height = size->ascent - size->descent;
    int shadow_offset = scale > 0 ? scale : 1;

    for (const unsigned char *cursor = (const unsigned char *) text; *cursor != '\0'; cursor++)
    {
        int codepoint = *cursor;
        if (codepoint == '\n')
        {
            x = origin_x;
            baseline += line_height;
            continue;
        }

        const UiGlyph *glyph = font_glyph(size, codepoint);
        if (glyph == NULL)
            continue;

        if (glyph->bitmap != NULL)
        {
            int glyph_x = x + glyph->x0;
            int glyph_y = baseline + glyph->y0;
            for (int row = 0; row < glyph->height; row++)
            {
                for (int column = 0; column < glyph->width; column++)
                {
                    unsigned char coverage = glyph->bitmap[row * glyph->width + column];
                    if (coverage == 0)
                        continue;
                    if (shadow)
                        blend_pixel(glyph_x + column + shadow_offset,
                                    glyph_y + row + shadow_offset, 0xFF000000u, coverage / 2u);
                    blend_pixel(glyph_x + column, glyph_y + row, argb, coverage);
                }
            }
        }

        x += glyph->advance;
    }
}






static void set_error(const char *message)
{
    snprintf(last_error, sizeof(last_error), "%s", message != NULL ? message : "unknown error");
}

int LIBMATTI_UI_WindowOpen(const LIBMATTI_UI_WindowConfig *config)
{
    LIBMATTI_UI_WindowClose();
    close_requested = 0;

    LIBMATTI_RenderConfig render_config = LIBMATTI_Render_DefaultConfig();
    render_config.width = config != NULL && config->width > 0 ? config->width : render_config.width;
    render_config.height = config != NULL && config->height > 0 ? config->height : render_config.height;
    render_config.title = config != NULL && config->title != NULL ? config->title : "Matticraft";
    render_config.fullscreen = config != NULL ? config->fullscreen : 0;
    render_config.vsync = config != NULL ? config->vsync : 1;

    const char *requested = config != NULL ? config->selection : NULL;
    if (requested == NULL || requested[0] == '\0')
        requested = getenv("MATTI_RENDERER");
    if (requested == NULL || requested[0] == '\0')
        requested = LIBMATTI_Platform_DefaultRenderer();
    snprintf(requested_selection, sizeof(requested_selection), "%s", requested);




    LIBMATTI_FML_DisplayWindow *early = LIBMATTI_FML_DisplayWindow_Current();
    if (early != NULL)
    {
        LIBMATTI_FML_DisplayWindow_Close(early);
        LIBMATTI_FML_DisplayWindow_SetCurrent(NULL);
    }

    renderer = LIBMATTI_Render_InitByName(requested, &render_config);
    if (renderer == NULL)
    {
        set_error(LIBMATTI_Render_GetLastError());
        return 0;
    }

    LIBMATTI_Render_GetSize(renderer, &width, &height);
    snprintf(backend, sizeof(backend), "%s", LIBMATTI_Render_GetBackendName(renderer));

    fprintf(stderr, "[ui] platform=%s arch=%s bits=%d backend=%s framebuffer=%dx%d\n",
            LIBMATTI_Platform_Name(), LIBMATTI_Platform_Arch(), LIBMATTI_Platform_Bits(), backend, width,
            height);
    return 1;
}

void LIBMATTI_UI_WindowClose(void)
{
    if (renderer != NULL)
    {
        LIBMATTI_Render_Destroy(renderer);
        renderer = NULL;
    }
    width = 0;
    height = 0;
    snprintf(backend, sizeof(backend), "%s", "none");
}

int LIBMATTI_UI_WindowWidth(void)
{
    return width;
}

int LIBMATTI_UI_WindowHeight(void)
{
    return height;
}

const char *LIBMATTI_UI_BackendName(void)
{
    return backend;
}

const char *LIBMATTI_UI_RequestedSelection(void)
{
    return requested_selection;
}

const char *LIBMATTI_UI_RequestedRenderer(void)
{
    return LIBMATTI_UI_RequestedSelection();
}

const char *LIBMATTI_UI_LastError(void)
{
    return last_error;
}

void LIBMATTI_UI_PollEvents(void)
{
    if (renderer != NULL)
        LIBMATTI_Render_PollEvents(renderer);
}

int LIBMATTI_UI_ShouldClose(void)
{
    if (close_requested)
        return 1;
    return renderer == NULL ? 1 : LIBMATTI_Render_ShouldClose(renderer);
}

void LIBMATTI_UI_RequestClose(void)
{
    close_requested = 1;
    if (renderer != NULL)
        LIBMATTI_Render_RequestClose(renderer);
}

void LIBMATTI_UI_SetTitle(const char *title)
{
    if (renderer != NULL && title != NULL)
        LIBMATTI_Render_SetTitle(renderer, title);
}

double LIBMATTI_UI_Time(void)
{
    return renderer != NULL ? LIBMATTI_Render_GetTime(renderer) : 0.0;
}






void LIBMATTI_UI_Clear(uint32_t argb)
{
    if (renderer != NULL)
        LIBMATTI_Render_Clear(renderer, argb);
}

void LIBMATTI_UI_FillRect(int x, int y, int rect_width, int rect_height, uint32_t argb)
{
    if (renderer != NULL)
        LIBMATTI_Render_FillRect(renderer, x, y, rect_width, rect_height, argb);
}

void LIBMATTI_UI_FillRectBlend(int x, int y, int rect_width, int rect_height, uint32_t argb)
{
    if (renderer == NULL || rect_width <= 0 || rect_height <= 0)
        return;
    unsigned int alpha = (argb >> 24) & 0xFFu;
    for (int row = 0; row < rect_height; row++)
    {
        for (int column = 0; column < rect_width; column++)
            blend_pixel(x + column, y + row, argb | 0xFF000000u, alpha);
    }
}

void LIBMATTI_UI_DrawPixel(int x, int y, uint32_t argb)
{
    if (renderer != NULL)
        LIBMATTI_Render_DrawPixel(renderer, x, y, argb);
}

void LIBMATTI_UI_Present(void)
{
    if (renderer != NULL)
        LIBMATTI_Render_Present(renderer);
}






void LIBMATTI_UI_DrawTextureRegion(const LIBMATTI_UI_Texture *texture, int x, int y, int rect_width,
                                   int rect_height, int source_x, int source_y, int source_width,
                                   int source_height, uint32_t tint)
{
    if (renderer == NULL || texture == NULL || texture->pixels == NULL)
        return;
    if (rect_width <= 0 || rect_height <= 0 || source_width <= 0 || source_height <= 0)
        return;

    unsigned int tint_alpha = (tint >> 24) & 0xFFu;
    for (int row = 0; row < rect_height; row++)
    {
        int sample_y = source_y + row * source_height / rect_height;
        if (sample_y < 0 || sample_y >= texture->height)
            continue;
        for (int column = 0; column < rect_width; column++)
        {
            int sample_x = source_x + column * source_width / rect_width;
            if (sample_x < 0 || sample_x >= texture->width)
                continue;

            uint32_t pixel = texture->pixels[sample_y * texture->width + sample_x];
            unsigned int alpha = ((pixel >> 24) & 0xFFu) * tint_alpha / 255u;
            if (alpha == 0)
                continue;
            unsigned int red = (((pixel >> 16) & 0xFFu) * ((tint >> 16) & 0xFFu)) / 255u;
            unsigned int green = (((pixel >> 8) & 0xFFu) * ((tint >> 8) & 0xFFu)) / 255u;
            unsigned int blue = ((pixel & 0xFFu) * (tint & 0xFFu)) / 255u;
            blend_pixel(x + column, y + row, 0xFF000000u | red << 16 | green << 8 | blue, alpha);
        }
    }
}

void LIBMATTI_UI_DrawTexture(const LIBMATTI_UI_Texture *texture, int x, int y, int rect_width, int rect_height)
{
    if (texture == NULL)
        return;
    LIBMATTI_UI_DrawTextureRegion(texture, x, y, rect_width, rect_height, 0, 0, texture->width, texture->height,
                                  0xFFFFFFFFu);
}

void LIBMATTI_UI_DrawTextureTiled(const LIBMATTI_UI_Texture *texture, int x, int y, int rect_width,
                                  int rect_height, int tile)
{
    if (texture == NULL || tile <= 0)
        return;
    for (int tile_y = 0; tile_y < rect_height; tile_y += tile)
    {
        int height = tile;
        if (tile_y + height > rect_height)
            height = rect_height - tile_y;
        for (int tile_x = 0; tile_x < rect_width; tile_x += tile)
        {
            int width = tile;
            if (tile_x + width > rect_width)
                width = rect_width - tile_x;
            LIBMATTI_UI_DrawTextureRegion(texture, x + tile_x, y + tile_y, width, height, 0, 0, texture->width,
                                          texture->height, 0xFFFFFFFFu);
        }
    }
}

int LIBMATTI_UI_SaveScreenshot(const char *path)
{
    if (renderer == NULL || path == NULL)
        return 0;
    return LIBMATTI_Render_SavePPM(renderer, path);
}






int LIBMATTI_UI_MouseX(void)
{
    double x = 0.0, y = 0.0;
    LIBMATTI_Render_GetMousePosition(renderer, &x, &y);
    return (int) x;
}

int LIBMATTI_UI_MouseY(void)
{
    double x = 0.0, y = 0.0;
    LIBMATTI_Render_GetMousePosition(renderer, &x, &y);
    return (int) y;
}

int LIBMATTI_UI_KeyDown(int key)
{
    return LIBMATTI_Render_IsKeyDown(renderer, key);
}

int LIBMATTI_UI_MouseDown(int button)
{
    return LIBMATTI_Render_IsMouseDown(renderer, button);
}
