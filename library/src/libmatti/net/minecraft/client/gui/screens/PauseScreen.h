// Port of net.minecraft.client.gui.screens.PauseScreen - the ESC pause menu.
// Java 1.21.11's layout: the "Game Menu" title, the two top buttons
// (Back to Game / Options...) split around the centre, the bottom row
// (Options... full width for the port's slice - the guide/link buttons of
// the real game stay out) and "Save and Quit to Title" (the port's slice:
// the quit flag the client polls). isPauseScreen returns true - the client
// freezes the tick loop over this screen.

#ifndef MATTICRAFT_MC_CLIENT_GUI_SCREENS_PAUSESCREEN_H
#define MATTICRAFT_MC_CLIENT_GUI_SCREENS_PAUSESCREEN_H

#include "libmatti/net/minecraft/client/gui/screens/Screen.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public static final Component SCREEN_TITLE = Component.translatable("menu.paused")
#define LIBMATTI_MC_PauseScreen_TITLE "Game Menu"
// Java: COLUMNS_ORIGINAL = 2 / LARGE_WIDTH = 200 (the OptionButton row) /
// DEFAULT_WIDTH = 150 (the game-menu buttons)
#define LIBMATTI_MC_PauseScreen_BUTTON_WIDTH 150
#define LIBMATTI_MC_PauseScreen_BUTTON_HEIGHT 20
#define LIBMATTI_MC_PauseScreen_BUTTON_SPACING 4

typedef struct LIBMATTI_MC_PauseScreen
{
    LIBMATTI_MC_Screen base;
    // Java: private final boolean showPauseMenu - the port pins true (the
    // in-game ESC path only opens this screen in-game)
    bool showPauseMenu;
    // the client contract: the poll answers "close me" / "quit to title"
    bool backToGame;
    bool quitToTitle;
    bool openOptions;
} LIBMATTI_MC_PauseScreen;

// Java: public PauseScreen() - the vtable fills (init/tick/isPauseScreen)
LIBMATTI_MC_PauseScreen *LIBMATTI_MC_PauseScreen_New(struct LIBMATTI_MC_Minecraft *minecraft);
void LIBMATTI_MC_PauseScreen_Free(LIBMATTI_MC_PauseScreen *screen);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_GUI_SCREENS_PAUSESCREEN_H
