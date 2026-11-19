// The C main window: vanilla-styled screens drawn entirely in C.
//
// Rust owns only the actual window (the GLFW/SDL handle and the frame loop);
// every screen, the panorama, the dirt background, the widget sheet, the logo
// and the bitmap text are C. The C side reads input through the bridge, so a
// Rust loop is just:
//
//   LIBMATTI_UI_MainWindow_Update();
//   LIBMATTI_UI_MainWindow_Draw(LIBMATTI_UI_Time());
//   LIBMATTI_UI_Present();
//
// Coordinates in this API are logical GUI pixels; menu.c multiplies by the
// GUI scale. The texture and drawing primitives are public so Rust screens can
// build on the same widgets.

#ifndef MATTICRAFT_MATTI_UI_MENU_H
#define MATTICRAFT_MATTI_UI_MENU_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum LIBMATTI_UI_Screen
{
    LIBMATTI_UI_SCREEN_TITLE = 0,
    LIBMATTI_UI_SCREEN_OPTIONS,
    LIBMATTI_UI_SCREEN_PAUSE,
    LIBMATTI_UI_SCREEN_WORLD
} LIBMATTI_UI_Screen;

// GUI scale the menus use (vanilla "GUI Scale": 1..4, 0 = auto).
int LIBMATTI_UI_GuiScale(void);
void LIBMATTI_UI_SetGuiScale(int scale);
int LIBMATTI_UI_GuiWidth(void);
int LIBMATTI_UI_GuiHeight(void);

// Screen stack / state.
void LIBMATTI_UI_MainWindow_Reset(void);
void LIBMATTI_UI_MainWindow_SetScreen(LIBMATTI_UI_Screen screen);
void LIBMATTI_UI_MainWindow_SetScreenByName(const char *name);
LIBMATTI_UI_Screen LIBMATTI_UI_MainWindow_CurrentScreen(void);
const char *LIBMATTI_UI_MainWindow_ScreenName(void);
int LIBMATTI_UI_MainWindow_ShouldClose(void);

// One frame: input edges (mouse click, ESC) then drawing.
void LIBMATTI_UI_MainWindow_Update(void);
void LIBMATTI_UI_MainWindow_Draw(double time);

// A renderer-selection/fullscreen change requested from the options screen.
// Rust closes the window, applies the new `layer:api` selection and reopens it.
int LIBMATTI_UI_MainWindow_TakeRestart(char *selection, int selection_size, int *fullscreen);

// Drawing primitives shared by every screen.
void LIBMATTI_UI_DrawMenuBackground(void);
void LIBMATTI_UI_DrawPanorama(double time);
void LIBMATTI_UI_DrawButton(int x, int y, int width, int height, const char *label, int hovered, int enabled);
void LIBMATTI_UI_DrawButtonCentered(int y, int width, const char *label, int hovered, int enabled);
void LIBMATTI_UI_DrawLabelCentered(const char *text, int y, uint32_t argb, int scale, int shadow);
void LIBMATTI_UI_DrawLabel(const char *text, int x, int y, uint32_t argb, int scale, int shadow);

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_MATTI_UI_MENU_H
