// Port of net.minecraft.client.gui.screens.OptionsScreen - the options
// screen Java opens from the pause menu (new OptionsScreen(this, options)).
// The port's slice carries the two settings the client applies live:
// the mouse sensitivity and the master volume, each as the vanilla
// 150-wide option row at the LEFT/RIGHT column positions, plus the Done
// button that returns to the pause screen. The bigger option catalogue
// (video/sound/controls sub-screens) stays with the later phases.

#ifndef MATTICRAFT_MC_CLIENT_GUI_SCREENS_OPTIONSSCREEN_H
#define MATTICRAFT_MC_CLIENT_GUI_SCREENS_OPTIONSSCREEN_H

#include "libmatti/net/minecraft/client/gui/screens/Screen.h"
#include "libmatti/net/minecraft/client/Options.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public static final Component TITLE = Component.translatable("options.online")
// (the shared options title) - the port prints it client-side
#define LIBMATTI_MC_OptionsScreen_TITLE "Options"

typedef struct LIBMATTI_MC_OptionsScreen
{
    LIBMATTI_MC_Screen base;
    // Java: this.options - the game settings the rows drive
    LIBMATTI_MC_Options *options;
    // the client contract: the poll answers "back to the pause screen"
    bool done;
} LIBMATTI_MC_OptionsScreen;

// Java: public OptionsScreen(Screen lastScreen, Options options) - the
// options struct rides the Minecraft allocation (the screen does not own it)
LIBMATTI_MC_OptionsScreen *LIBMATTI_MC_OptionsScreen_New(struct LIBMATTI_MC_Minecraft *minecraft,
                                                         LIBMATTI_MC_Options *options);
void LIBMATTI_MC_OptionsScreen_Free(LIBMATTI_MC_OptionsScreen *screen);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_GUI_SCREENS_OPTIONSSCREEN_H
