







#include "libmatti/matti/ui/menu.h"

#include "libmatti/matti/Platform.h"
#include "libmatti/matti/render/render.h"
#include "libmatti/matti/ui/bridge.h"
#include "libmatti/matti/ui/texture.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define GUI_BUTTON_WIDTH 200
#define GUI_BUTTON_HEIGHT 20
#define GUI_BUTTON_STEP 24
#define GUI_MAX_SCALE 6
#define GUI_TEXT_WHITE 0xFFFFFFFFu
#define GUI_TEXT_GRAY 0xFFA0A0A0u
#define GUI_TEXT_YELLOW 0xFFFFFF55u
#define GUI_TEXT_RED 0xFFFF5555u



#define WIDGETS_NORMAL_Y 66
#define WIDGETS_HOVER_Y 86
#define WIDGETS_DISABLED_Y 46
#define WIDGETS_BUTTON_WIDTH 200
#define WIDGETS_BUTTON_HEIGHT 20

static LIBMATTI_UI_Screen screen = LIBMATTI_UI_SCREEN_TITLE;
static LIBMATTI_UI_Screen options_return = LIBMATTI_UI_SCREEN_TITLE;
static int menu_scale = 0;
static int should_close = 0;
static int previous_mouse_down = 0;
static int previous_escape = 0;
static int mouse_gui_x = 0;
static int mouse_gui_y = 0;
static int restart_pending = 0;
static int restart_fullscreen = 0;
static char restart_selection[32] = "";

static LIBMATTI_UI_Texture *widgets_texture = NULL;
static LIBMATTI_UI_Texture *dirt_texture = NULL;
static LIBMATTI_UI_Texture *logo_texture = NULL;
static LIBMATTI_UI_Texture *edition_texture = NULL;
static LIBMATTI_UI_Texture *panorama[6];
static int menu_assets_loaded = 0;






int LIBMATTI_UI_GuiScale(void)
{
    int width = LIBMATTI_UI_WindowWidth();
    int height = LIBMATTI_UI_WindowHeight();
    if (menu_scale > 0)
        return menu_scale;
    int scale = 1;
    while (scale < GUI_MAX_SCALE && width / (scale + 1) >= 320
           && height / (scale + 1) >= 240)
        scale++;
    return scale;
}

void LIBMATTI_UI_SetGuiScale(int scale)
{
    menu_scale = scale < 0 ? 0 : scale > GUI_MAX_SCALE ? GUI_MAX_SCALE : scale;
}

int LIBMATTI_UI_GuiWidth(void)
{
    int scale = LIBMATTI_UI_GuiScale();
    return scale > 0 ? LIBMATTI_UI_WindowWidth() / scale : LIBMATTI_UI_WindowWidth();
}

int LIBMATTI_UI_GuiHeight(void)
{
    int scale = LIBMATTI_UI_GuiScale();
    return scale > 0 ? LIBMATTI_UI_WindowHeight() / scale : LIBMATTI_UI_WindowHeight();
}

static int button_x(int width)
{
    return (LIBMATTI_UI_GuiWidth() - width) / 2;
}

static int button_rect(int index, int count, int first_y, int *out_x, int *out_y, int *out_w, int *out_h)
{
    (void) count;
    int x = button_x(GUI_BUTTON_WIDTH);
    int y = first_y + index * GUI_BUTTON_STEP;
    if (out_x != NULL) *out_x = x;
    if (out_y != NULL) *out_y = y;
    if (out_w != NULL) *out_w = GUI_BUTTON_WIDTH;
    if (out_h != NULL) *out_h = GUI_BUTTON_HEIGHT;
    return 1;
}

static int in_rect(int px, int py, int x, int y, int width, int height)
{
    return px >= x && py >= y && px < x + width && py < y + height;
}

static void ensure_assets(void)
{
    if (menu_assets_loaded)
        return;
    menu_assets_loaded = 1;

    const char *assets = getenv("MATTI_ASSETS_DIR");
    if (assets != NULL && assets[0] != '\0')
        LIBMATTI_UI_Texture_SetAssetRoot(assets);

    widgets_texture = LIBMATTI_UI_Texture_Load("gui/widgets.png");
    dirt_texture = LIBMATTI_UI_Texture_Load("gui/options_background.png");
    logo_texture = LIBMATTI_UI_Texture_Load("gui/title/minecraft.png");
    edition_texture = LIBMATTI_UI_Texture_Load("gui/title/edition.png");
    for (int i = 0; i < 6; i++)
    {
        char name[64];
        snprintf(name, sizeof(name), "gui/title/background/panorama_%d.png", i);
        panorama[i] = LIBMATTI_UI_Texture_Load(name);
    }
}






void LIBMATTI_UI_DrawLabel(const char *text, int x, int y, uint32_t argb, int scale, int shadow)
{
    int gui = LIBMATTI_UI_GuiScale();
    int text_scale = (scale < 1 ? 1 : scale) * gui;
    LIBMATTI_UI_DrawText(text, x * gui, y * gui, argb, text_scale, shadow);
}

void LIBMATTI_UI_DrawLabelCentered(const char *text, int y, uint32_t argb, int scale, int shadow)
{
    int gui = LIBMATTI_UI_GuiScale();
    int text_scale = (scale < 1 ? 1 : scale) * gui;
    int width = LIBMATTI_UI_TextWidth(text, text_scale);
    LIBMATTI_UI_DrawText(text, (LIBMATTI_UI_GuiWidth() * gui - width) / 2, y * gui, argb, text_scale,
                         shadow);
}

void LIBMATTI_UI_DrawButton(int x, int y, int width, int height, const char *label, int hovered, int enabled)
{
    int gui = LIBMATTI_UI_GuiScale();
    ensure_assets();

    int sprite_y = !enabled ? WIDGETS_DISABLED_Y : hovered ? WIDGETS_HOVER_Y : WIDGETS_NORMAL_Y;
    if (widgets_texture != NULL)
    {
        LIBMATTI_UI_DrawTextureRegion(widgets_texture, x * gui, y * gui, width * gui, height * gui, 0, sprite_y,
                                      WIDGETS_BUTTON_WIDTH, WIDGETS_BUTTON_HEIGHT, 0xFFFFFFFFu);
    }
    else
    {
        LIBMATTI_UI_FillRect(x * gui, y * gui, width * gui, height * gui, enabled ? 0xFF6E6E6Eu : 0xFF3E3E3Eu);
    }

    int text_width = LIBMATTI_UI_TextWidth(label, gui);
    int text_height = LIBMATTI_UI_TextHeight(gui);
    LIBMATTI_UI_DrawText(label, x * gui + (width * gui - text_width) / 2,
                         y * gui + (height * gui - text_height) / 2, enabled ? GUI_TEXT_WHITE : GUI_TEXT_GRAY,
                         gui, 1);
}

void LIBMATTI_UI_DrawButtonCentered(int y, int width, const char *label, int hovered, int enabled)
{
    LIBMATTI_UI_DrawButton(button_x(width), y, width, GUI_BUTTON_HEIGHT, label, hovered, enabled);
}

void LIBMATTI_UI_DrawMenuBackground(void)
{
    ensure_assets();
    int gui = LIBMATTI_UI_GuiScale();
    int width = LIBMATTI_UI_WindowWidth();
    int height = LIBMATTI_UI_WindowHeight();
    (void) gui;

    if (dirt_texture != NULL)
    {
        LIBMATTI_UI_DrawTextureTiled(dirt_texture, 0, 0, width, height, 32 * gui);
    }
    else
    {
        LIBMATTI_UI_Clear(0xFF2B2119u);
    }
    for (int y = 0; y < height; y += 4)
    {
        unsigned int alpha = (unsigned int) (0x70 - 0x40 * y / (height > 0 ? height : 1));
        LIBMATTI_UI_FillRectBlend(0, y, width, 4, alpha << 24);
    }
}





void LIBMATTI_UI_DrawPanorama(double time)
{
    ensure_assets();
    int width = LIBMATTI_UI_WindowWidth();
    int height = LIBMATTI_UI_WindowHeight();
    if (width <= 0 || height <= 0)
        return;

    int have_faces = 1;
    for (int i = 0; i < 6; i++)
    {
        if (panorama[i] == NULL)
            have_faces = 0;
    }
    if (!have_faces)
    {
        LIBMATTI_UI_DrawMenuBackground();
        return;
    }

    const float fov = 85.0f * (float) M_PI / 180.0f;
    const float pitch = 10.0f * (float) M_PI / 180.0f;
    const float yaw = (float) (time * 2.0 * M_PI / 180.0);
    const float tan_half = tanf(fov * 0.5f);
    const float aspect = (float) width / (float) height;
    const float cp = cosf(pitch), sp = sinf(pitch);
    const float cy = cosf(yaw), sy = sinf(yaw);

    for (int y = 0; y < height; y++)
    {
        float ndy = (1.0f - 2.0f * ((float) y + 0.5f) / (float) height) * tan_half;
        for (int x = 0; x < width; x++)
        {
            float ndx = (2.0f * ((float) x + 0.5f) / (float) width - 1.0f) * tan_half * aspect;



            float dx = ndx, dy = ndy, dz = -1.0f;
            float dy1 = dy * cp - dz * sp;
            float dz1 = dy * sp + dz * cp;
            float dx2 = dx * cy + dz1 * sy;
            float dz2 = -dx * sy + dz1 * cy;

            float ax = fabsf(dx2), ay = fabsf(dy1), az = fabsf(dz2);
            int face;
            float u, v;
            if (ay >= ax && ay >= az)
            {
                face = dy1 > 0.0f ? 4 : 5;
                u = dx2 / ay;
                v = dz2 / ay;
            }
            else if (ax >= az)
            {
                face = dx2 > 0.0f ? 1 : 3;
                u = dz2 / ax;
                v = dy1 / ax;
            }
            else
            {
                face = dz2 > 0.0f ? 0 : 2;
                u = dx2 / az;
                v = dy1 / az;
            }

            u = (u + 1.0f) * 0.5f;
            v = (v + 1.0f) * 0.5f;
            if (face == 1 || face == 2 || face == 4)
                u = 1.0f - u;
            if (face == 2 || face == 3 || face == 5)
                v = 1.0f - v;
            if (u < 0.0f) u = 0.0f;
            if (u > 0.999999f) u = 0.999999f;
            if (v < 0.0f) v = 0.0f;
            if (v > 0.999999f) v = 0.999999f;

            const LIBMATTI_UI_Texture *texture = panorama[face];
            int tx = (int) (u * (float) texture->width);
            int ty = (int) (v * (float) texture->height);
            LIBMATTI_UI_DrawPixel(x, y, texture->pixels[ty * texture->width + tx] | 0xFF000000u);
        }
    }
}






static void draw_title(double time)
{
    LIBMATTI_UI_DrawPanorama(time);
    int gui = LIBMATTI_UI_GuiScale();
    int width = LIBMATTI_UI_GuiWidth();
    int height = LIBMATTI_UI_GuiHeight();




    if (logo_texture != NULL)
    {
        int logo_w = 256 * gui;
        int logo_h = 44 * gui;
        LIBMATTI_UI_DrawTextureRegion(logo_texture, (LIBMATTI_UI_WindowWidth() - logo_w) / 2, 30 * gui, logo_w, logo_h,
                                      0, 0, logo_texture->width, logo_texture->height * 44 / 64, 0xFFFFFFFFu);
    }
    if (edition_texture != NULL)
    {
        int edition_w = 128 * gui;
        int edition_h = 16 * gui;
        LIBMATTI_UI_DrawTextureRegion(edition_texture, (LIBMATTI_UI_WindowWidth() - edition_w) / 2, (30 + 44 - 7) * gui,
                                      edition_w, edition_h, 0, 0, edition_texture->width, edition_texture->height,
                                      0xFFFFFFFFu);
    }

    int splash_width = LIBMATTI_UI_TextWidth("Also try Matticraft!", gui);
    LIBMATTI_UI_DrawText("Also try Matticraft!", width / 2 * gui + 104 * gui - splash_width / 2, 74 * gui,
                         GUI_TEXT_YELLOW, gui, 1);

    const char *labels[4] = {"Singleplayer", "Multiplayer", "Options...", "Quit Game"};
    int first_y = height / 4 + 48;
    for (int i = 0; i < 4; i++)
    {
        int x, y, w, h;
        button_rect(i, 4, first_y, &x, &y, &w, &h);
        int hovered = in_rect(mouse_gui_x, mouse_gui_y, x, y, w, h);
        LIBMATTI_UI_DrawButton(x, y, w, h, labels[i], hovered, 1);
    }

    LIBMATTI_UI_DrawText("Minecraft 1.21.11", 2 * gui, LIBMATTI_UI_WindowHeight() - 10 * gui,
                         GUI_TEXT_WHITE, gui, 1);
    const char *copyright = "Copyright Mojang AB. Do not distribute!";
    int copy_width = LIBMATTI_UI_TextWidth(copyright, gui);
    LIBMATTI_UI_DrawText(copyright, LIBMATTI_UI_WindowWidth() - copy_width - 2 * gui,
                         LIBMATTI_UI_WindowHeight() - 10 * gui, GUI_TEXT_WHITE, gui, 1);
}

static void draw_options(void)
{
    LIBMATTI_UI_DrawMenuBackground();
    int width = LIBMATTI_UI_GuiWidth();
    int height = LIBMATTI_UI_GuiHeight();
    LIBMATTI_UI_DrawLabelCentered("Options", height / 5, GUI_TEXT_WHITE, 2, 1);

    char scale_label[48];
    if (menu_scale == 0)
        snprintf(scale_label, sizeof(scale_label), "GUI Scale: Auto (%d)", LIBMATTI_UI_GuiScale());
    else
        snprintf(scale_label, sizeof(scale_label), "GUI Scale: %d", LIBMATTI_UI_GuiScale());
    char renderer_label[80];
    snprintf(renderer_label, sizeof(renderer_label), "Renderer: %s", LIBMATTI_UI_RequestedSelection());

    const char *labels[4];
    labels[0] = "Back";
    labels[1] = scale_label;
    labels[2] = renderer_label;
    labels[3] = restart_fullscreen ? "Fullscreen: ON" : "Fullscreen: OFF";

    int first_y = height / 4 + 20;
    for (int i = 0; i < 4; i++)
    {
        int x, y, w, h;
        button_rect(i, 4, first_y, &x, &y, &w, &h);
        int hovered = in_rect(mouse_gui_x, mouse_gui_y, x, y, w, h);
        LIBMATTI_UI_DrawButton(x, y, w, h, labels[i], hovered, 1);
    }
    (void) width;
}

static void draw_pause(void)
{
    LIBMATTI_UI_DrawMenuBackground();
    int height = LIBMATTI_UI_GuiHeight();
    LIBMATTI_UI_DrawLabelCentered("Game Menu", height / 5, GUI_TEXT_WHITE, 2, 1);

    const char *labels[3] = {"Back to Game", "Options...", "Save and Quit to Title"};
    int first_y = height / 4 + 40;
    for (int i = 0; i < 3; i++)
    {
        int x, y, w, h;
        button_rect(i, 3, first_y, &x, &y, &w, &h);
        int hovered = in_rect(mouse_gui_x, mouse_gui_y, x, y, w, h);
        LIBMATTI_UI_DrawButton(x, y, w, h, labels[i], hovered, 1);
    }
}

static void draw_world(void)
{
    int width = LIBMATTI_UI_WindowWidth();
    int height = LIBMATTI_UI_WindowHeight();
    int horizon = height * 2 / 3;
    const int bands = 24;
    for (int band = 0; band < bands; band++)
    {
        int y = band * horizon / bands;
        int next = (band + 1) * horizon / bands;
        float t = (float) band / (float) (bands - 1);
        unsigned int r = (unsigned int) (0x6F + (0xBF - 0x6F) * t);
        unsigned int g = (unsigned int) (0xA8 + (0xD9 - 0xA8) * t);
        unsigned int b = (unsigned int) (0xDC + (0xF2 - 0xDC) * t);
        LIBMATTI_UI_FillRect(0, y, width, next - y + 1, 0xFF000000u | r << 16 | g << 8 | b);
    }
    LIBMATTI_UI_FillRect(0, horizon, width, 8, 0xFF5E9E3Eu);
    LIBMATTI_UI_FillRect(0, horizon + 8, width, height - horizon - 8, 0xFF7A5A3Au);

    LIBMATTI_UI_DrawLabelCentered("World rendering is ported next", LIBMATTI_UI_GuiHeight() / 2 - 20, GUI_TEXT_WHITE, 2, 1);
    LIBMATTI_UI_DrawLabelCentered("ESC opens the game menu", LIBMATTI_UI_GuiHeight() / 2 + 10, GUI_TEXT_WHITE, 1, 1);
}






void LIBMATTI_UI_MainWindow_Reset(void)
{
    screen = LIBMATTI_UI_SCREEN_TITLE;
    options_return = LIBMATTI_UI_SCREEN_TITLE;
    should_close = 0;
    previous_mouse_down = 0;
    previous_escape = 0;
    restart_pending = 0;
    restart_fullscreen = 0;
    mouse_gui_x = 0;
    mouse_gui_y = 0;
    menu_assets_loaded = 0;
}

void LIBMATTI_UI_MainWindow_SetScreen(LIBMATTI_UI_Screen new_screen)
{
    screen = new_screen;
}

void LIBMATTI_UI_MainWindow_SetScreenByName(const char *name)
{
    if (name == NULL)
        return;
    if (strcmp(name, "options") == 0)
        screen = LIBMATTI_UI_SCREEN_OPTIONS;
    else if (strcmp(name, "pause") == 0)
        screen = LIBMATTI_UI_SCREEN_PAUSE;
    else if (strcmp(name, "world") == 0)
        screen = LIBMATTI_UI_SCREEN_WORLD;
    else
        screen = LIBMATTI_UI_SCREEN_TITLE;
}

LIBMATTI_UI_Screen LIBMATTI_UI_MainWindow_CurrentScreen(void)
{
    return screen;
}

const char *LIBMATTI_UI_MainWindow_ScreenName(void)
{
    switch (screen)
    {
        case LIBMATTI_UI_SCREEN_OPTIONS: return "options";
        case LIBMATTI_UI_SCREEN_PAUSE: return "pause";
        case LIBMATTI_UI_SCREEN_WORLD: return "world";
        default: return "title";
    }
}

int LIBMATTI_UI_MainWindow_ShouldClose(void)
{
    return should_close;
}

int LIBMATTI_UI_MainWindow_TakeRestart(char *selection, int selection_size, int *fullscreen)
{
    if (!restart_pending)
        return 0;
    restart_pending = 0;
    if (selection != NULL && selection_size > 0)
        snprintf(selection, (size_t) selection_size, "%s", restart_selection);
    if (fullscreen != NULL)
        *fullscreen = restart_fullscreen;
    return 1;
}

static void click_title(int x, int y)
{
    int first_y = LIBMATTI_UI_GuiHeight() / 4 + 48;
    for (int i = 0; i < 4; i++)
    {
        int bx, by, bw, bh;
        button_rect(i, 4, first_y, &bx, &by, &bw, &bh);
        if (!in_rect(x, y, bx, by, bw, bh))
            continue;
        switch (i)
        {
            case 0: screen = LIBMATTI_UI_SCREEN_WORLD; break;
            case 1: screen = LIBMATTI_UI_SCREEN_WORLD; break;
            case 2:
                options_return = LIBMATTI_UI_SCREEN_TITLE;
                screen = LIBMATTI_UI_SCREEN_OPTIONS;
                break;
            default: should_close = 1; break;
        }
        return;
    }
}

static void click_options(int x, int y)
{
    int first_y = LIBMATTI_UI_GuiHeight() / 4 + 20;
    for (int i = 0; i < 4; i++)
    {
        int bx, by, bw, bh;
        button_rect(i, 4, first_y, &bx, &by, &bw, &bh);
        if (!in_rect(x, y, bx, by, bw, bh))
            continue;
        if (i == 0)
        {
            screen = options_return;
        }
        else if (i == 1)
        {
            int scale = menu_scale + 1;
            if (scale > GUI_MAX_SCALE)
                scale = 0;
            LIBMATTI_UI_SetGuiScale(scale);
        }
        else if (i == 2)
        {



            static const char *const choices[] = {
                "native:opengl", "native:vulkan", "sdl:opengl", "sdl:vulkan", "sdl:auto",
                "sdl:directx", "sdl:metal",
            };
            const int count = (int) (sizeof(choices) / sizeof(choices[0]));
            const char *current = LIBMATTI_UI_RequestedSelection();
            int index = -1;
            for (int c = 0; c < count; c++)
            {
                if (strcmp(choices[c], current) == 0)
                {
                    index = c;
                    break;
                }
            }
            do
            {
                index = (index + 1) % count;
            } while (!LIBMATTI_Platform_SupportsApi(strchr(choices[index], ':') + 1));
            snprintf(restart_selection, sizeof(restart_selection), "%s", choices[index]);
            restart_pending = 1;
        }
        else if (i == 3)
        {
            restart_fullscreen = !restart_fullscreen;
            restart_pending = 1;
            snprintf(restart_selection, sizeof(restart_selection), "%s", LIBMATTI_UI_RequestedSelection());
        }
        return;
    }
}

static void click_pause(int x, int y)
{
    int first_y = LIBMATTI_UI_GuiHeight() / 4 + 40;
    for (int i = 0; i < 3; i++)
    {
        int bx, by, bw, bh;
        button_rect(i, 3, first_y, &bx, &by, &bw, &bh);
        if (!in_rect(x, y, bx, by, bw, bh))
            continue;
        if (i == 0)
            screen = LIBMATTI_UI_SCREEN_WORLD;
        else if (i == 1)
        {
            options_return = LIBMATTI_UI_SCREEN_PAUSE;
            screen = LIBMATTI_UI_SCREEN_OPTIONS;
        }
        else
            screen = LIBMATTI_UI_SCREEN_TITLE;
        return;
    }
}

void LIBMATTI_UI_MainWindow_Update(void)
{
    ensure_assets();
    int scale = LIBMATTI_UI_GuiScale();
    mouse_gui_x = LIBMATTI_UI_MouseX() / (scale > 0 ? scale : 1);
    mouse_gui_y = LIBMATTI_UI_MouseY() / (scale > 0 ? scale : 1);

    int mouse_down = LIBMATTI_UI_MouseDown(LIBMATTI_UI_MOUSE_LEFT);
    int clicked = mouse_down && !previous_mouse_down;
    previous_mouse_down = mouse_down;

    int escape_down = LIBMATTI_UI_KeyDown(LIBMATTI_UI_KEY_ESCAPE);
    int escape = escape_down && !previous_escape;
    previous_escape = escape_down;

    if (escape)
    {
        switch (screen)
        {
            case LIBMATTI_UI_SCREEN_TITLE: should_close = 1; break;
            case LIBMATTI_UI_SCREEN_OPTIONS: screen = options_return; break;
            case LIBMATTI_UI_SCREEN_PAUSE: screen = LIBMATTI_UI_SCREEN_WORLD; break;
            default: screen = LIBMATTI_UI_SCREEN_PAUSE; break;
        }
        return;
    }

    if (!clicked)
        return;
    switch (screen)
    {
        case LIBMATTI_UI_SCREEN_TITLE: click_title(mouse_gui_x, mouse_gui_y); break;
        case LIBMATTI_UI_SCREEN_OPTIONS: click_options(mouse_gui_x, mouse_gui_y); break;
        case LIBMATTI_UI_SCREEN_PAUSE: click_pause(mouse_gui_x, mouse_gui_y); break;
        default: break;
    }
}

void LIBMATTI_UI_MainWindow_Draw(double time)
{
    ensure_assets();
    switch (screen)
    {
        case LIBMATTI_UI_SCREEN_OPTIONS: draw_options(); break;
        case LIBMATTI_UI_SCREEN_PAUSE: draw_pause(); break;
        case LIBMATTI_UI_SCREEN_WORLD: draw_world(); break;
        default: draw_title(time); break;
    }
}
