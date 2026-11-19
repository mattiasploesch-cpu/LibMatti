










#ifndef MATTICRAFT_MATTI_UI_BRIDGE_H
#define MATTICRAFT_MATTI_UI_BRIDGE_H

#include <stdint.h>

#include "libmatti/matti/ui/texture.h"

#ifdef __cplusplus
extern "C"
{
#endif



#define LIBMATTI_UI_KEY_ESCAPE 256
#define LIBMATTI_UI_KEY_ENTER 257
#define LIBMATTI_UI_KEY_TAB 258
#define LIBMATTI_UI_KEY_RIGHT 262
#define LIBMATTI_UI_KEY_LEFT 263
#define LIBMATTI_UI_KEY_DOWN 264
#define LIBMATTI_UI_KEY_UP 265



#define LIBMATTI_UI_MOUSE_LEFT 0
#define LIBMATTI_UI_MOUSE_RIGHT 1

typedef struct LIBMATTI_UI_WindowConfig
{
    int width;
    int height;
    int fullscreen;
    int vsync;



    const char *selection;
    const char *title;
} LIBMATTI_UI_WindowConfig;




int LIBMATTI_UI_WindowOpen(const LIBMATTI_UI_WindowConfig *config);
void LIBMATTI_UI_WindowClose(void);

int LIBMATTI_UI_WindowWidth(void);
int LIBMATTI_UI_WindowHeight(void);


const char *LIBMATTI_UI_BackendName(void);


const char *LIBMATTI_UI_RequestedSelection(void);


const char *LIBMATTI_UI_RequestedRenderer(void);
const char *LIBMATTI_UI_LastError(void);

void LIBMATTI_UI_PollEvents(void);
int LIBMATTI_UI_ShouldClose(void);
void LIBMATTI_UI_RequestClose(void);
void LIBMATTI_UI_SetTitle(const char *title);
double LIBMATTI_UI_Time(void);



void LIBMATTI_UI_Clear(uint32_t argb);
void LIBMATTI_UI_FillRect(int x, int y, int width, int height, uint32_t argb);


void LIBMATTI_UI_FillRectBlend(int x, int y, int width, int height, uint32_t argb);
void LIBMATTI_UI_DrawPixel(int x, int y, uint32_t argb);
void LIBMATTI_UI_Present(void);



int LIBMATTI_UI_SaveScreenshot(const char *path);







void LIBMATTI_UI_DrawTexture(const LIBMATTI_UI_Texture *texture, int x, int y, int width, int height);
void LIBMATTI_UI_DrawTextureRegion(const LIBMATTI_UI_Texture *texture, int x, int y, int width, int height,
                                   int source_x, int source_y, int source_width, int source_height,
                                   uint32_t tint);
void LIBMATTI_UI_DrawTextureTiled(const LIBMATTI_UI_Texture *texture, int x, int y, int width, int height,
                                  int tile);



int LIBMATTI_UI_TextHeight(int scale);
int LIBMATTI_UI_TextWidth(const char *text, int scale);


void LIBMATTI_UI_DrawText(const char *text, int x, int y, uint32_t argb, int scale, int shadow);



int LIBMATTI_UI_KeyDown(int key);
int LIBMATTI_UI_MouseX(void);
int LIBMATTI_UI_MouseY(void);
int LIBMATTI_UI_MouseDown(int button);

#ifdef __cplusplus
}
#endif

#endif
