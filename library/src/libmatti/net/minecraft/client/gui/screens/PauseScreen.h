// Port of net.minecraft.client.gui.screens.PauseScreen over the Screen base
// (the vanilla 1.21.11 widget set). The buttons ride the P6.2 Button widget
// (the press/release edges fire the flags the client polls); the layout
// follows Java's GridLayout over the FrameLayout 0.5/0.25 align:
//   Return to Game (204, span 2) | Advancements | Statistics
//   Send Feedback | Report Bugs  | Options...  | Share to LAN
//   Save and Quit to Title (204, span 2)
// The grid is 212 wide (98 + 8 + 98), 166 tall; the client renders the title
// + the sprites + the blur backdrop (the render rides render_pause_overlay).

#ifndef MATTICRAFT_NET_MINECRAFT_CLIENT_GUI_SCREENS_PAUSESCREEN_H
#define MATTICRAFT_NET_MINECRAFT_CLIENT_GUI_SCREENS_PAUSESCREEN_H

#include "libmatti/net/minecraft/client/gui/screens/Screen.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public static final Component SCREEN_TITLE (menu.paused) - the
// vanilla 1.21.11 in-game pause carries "Game Menu"
#define LIBMATTI_MC_PauseScreen_TITLE "Game Menu"

// Java: PauseScreen's constants (the vanilla 1.21.11 source)
#define LIBMATTI_MC_PauseScreen_BUTTON_WIDTH_FULL 204
#define LIBMATTI_MC_PauseScreen_BUTTON_WIDTH_HALF 98
#define LIBMATTI_MC_PauseScreen_BUTTON_HEIGHT 20
#define LIBMATTI_MC_PauseScreen_MENU_PADDING_TOP 50
// Java: the GridLayout cell padding (the 4px gap the rows ride)
#define LIBMATTI_MC_PauseScreen_BUTTON_PADDING 4
// Java: the grid rows ride the 4px cell padding (20 + 4); the top band
// carries the 50px paddingTop (the title sits in it)
#define LIBMATTI_MC_PauseScreen_ROW_STRIDE 24
// Java: the FrameLayout align (0.5, 0.25) over the 204x170 grid (the 5 rows:
// 50 top band + 5x20 buttons + 4x4 gaps + 4 bottom pad)
#define LIBMATTI_MC_PauseScreen_GRID_WIDTH 204
#define LIBMATTI_MC_PauseScreen_GRID_HEIGHT 170
#define LIBMATTI_MC_PauseScreen_TITLE_Y 40

// Java: public class PauseScreen extends Screen - the flag contract the
// client polls (the C API has no setScreen; the press hooks answer through
// the one active instance)
typedef struct LIBMATTI_MC_PauseScreen
{
    LIBMATTI_MC_Screen base;
    bool showPauseMenu;
    // Java: the lambdas' effects (the client polls these per tick)
    bool backToGame;    // Return to Game -> setScreen(null)
    bool openOptions;   // Options... -> setScreen(new OptionsScreen(...))
    bool quitToTitle;   // Save and Quit to Title -> stop()
} LIBMATTI_MC_PauseScreen;

LIBMATTI_MC_PauseScreen *LIBMATTI_MC_PauseScreen_New(struct LIBMATTI_MC_Minecraft *minecraft);
void LIBMATTI_MC_PauseScreen_Free(LIBMATTI_MC_PauseScreen *screen);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_NET_MINECRAFT_CLIENT_GUI_SCREENS_PAUSESCREEN_H
