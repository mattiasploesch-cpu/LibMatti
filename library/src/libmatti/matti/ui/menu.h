















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



int LIBMATTI_UI_GuiScale(void);
void LIBMATTI_UI_SetGuiScale(int scale);
int LIBMATTI_UI_GuiWidth(void);
int LIBMATTI_UI_GuiHeight(void);



void LIBMATTI_UI_MainWindow_Reset(void);
void LIBMATTI_UI_MainWindow_SetScreen(LIBMATTI_UI_Screen screen);
void LIBMATTI_UI_MainWindow_SetScreenByName(const char *name);
LIBMATTI_UI_Screen LIBMATTI_UI_MainWindow_CurrentScreen(void);
const char *LIBMATTI_UI_MainWindow_ScreenName(void);
int LIBMATTI_UI_MainWindow_ShouldClose(void);



void LIBMATTI_UI_MainWindow_Update(void);
void LIBMATTI_UI_MainWindow_Draw(double time);




int LIBMATTI_UI_MainWindow_TakeRestart(char *selection, int selection_size, int *fullscreen);



void LIBMATTI_UI_DrawMenuBackground(void);
void LIBMATTI_UI_DrawPanorama(double time);
void LIBMATTI_UI_DrawButton(int x, int y, int width, int height, const char *label, int hovered, int enabled);
void LIBMATTI_UI_DrawButtonCentered(int y, int width, const char *label, int hovered, int enabled);
void LIBMATTI_UI_DrawLabelCentered(const char *text, int y, uint32_t argb, int scale, int shadow);
void LIBMATTI_UI_DrawLabel(const char *text, int x, int y, uint32_t argb, int scale, int shadow);

#ifdef __cplusplus
}
#endif

#endif
