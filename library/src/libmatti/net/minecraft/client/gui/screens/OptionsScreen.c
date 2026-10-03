// Port of net.minecraft.client.gui.screens.OptionsScreen over the Screen
// base. The rows ride the vanilla option-button geometry: 150 wide, the
// left column at width/2 - 155, the right at +5, the row height at
// height/4 - 24 + 8*row (Java's y base for the first small-option row).
// Each click applies the 10% step immediately (Java: the OptionInstance's
// setter runs through the widget press) - the client folds the sensitivity
// into the mouse-look and the volume into the sound engine live, and the
// close path flushes options.txt. The Done button returns to the pause
// screen through the flag the client polls.

#include "libmatti/net/minecraft/client/gui/screens/OptionsScreen.h"

#include "libmatti/net/minecraft/client/gui/components/Button.h"

#include <stdio.h>
#include <stdlib.h>

// Java's Button lambdas capture the screen; the C onPress carries no
// userdata, so the port keeps the ONE active options screen (the client
// owns the single instance) the press hooks answer through.
static LIBMATTI_MC_OptionsScreen *active_options_screen = NULL;

// Java: the OptionInstance.step for the small options - 10% of the range
#define STEP 0.1f

static void press_done(LIBMATTI_MC_Button *button)
{
    (void) button;
    if (active_options_screen != NULL)
        active_options_screen->done = true;
}

// Java: this.sensitivity = new OptionInstance(..., (button, value) -> options.sensitivity().set(value))
static void press_sensitivity(LIBMATTI_MC_Button *button)
{
    LIBMATTI_MC_OptionsScreen *screen = active_options_screen;
    if (screen == NULL || screen->options == NULL)
        return;
    float value = LIBMATTI_MC_Options_GetSensitivity(screen->options) + STEP;
    if (value > 1.0f + 1e-5f)
        value = 0.0f; // Java: the slider wraps at the top of the range
    LIBMATTI_MC_Options_SetSensitivity(screen->options, value);
    char label[64];
    snprintf(label, sizeof(label), "Sensitivity: %d%%",
             LIBMATTI_MC_Options_SensitivityPercent(screen->options));
    LIBMATTI_MC_AbstractWidget_SetMessage(&button->base, label);
}

static void press_master_volume(LIBMATTI_MC_Button *button)
{
    LIBMATTI_MC_OptionsScreen *screen = active_options_screen;
    if (screen == NULL || screen->options == NULL)
        return;
    float value = LIBMATTI_MC_Options_GetMasterVolume(screen->options) + STEP;
    if (value > 1.0f + 1e-5f)
        value = 0.0f;
    LIBMATTI_MC_Options_SetMasterVolume(screen->options, value);
    char label[64];
    snprintf(label, sizeof(label), "Master Volume: %d%%",
             (int) (LIBMATTI_MC_Options_GetMasterVolume(screen->options) * 100.0f + 0.5f));
    LIBMATTI_MC_AbstractWidget_SetMessage(&button->base, label);
}

// Java: public boolean isPauseScreen() -> true (the options over the pause
// screen keep the game frozen like the real game)
static bool options_is_pause_screen(const LIBMATTI_MC_Screen *screen)
{
    (void) screen;
    return true;
}

// Java: protected void init() - the widget build (the resize re-runs it)
static void options_init(LIBMATTI_MC_Screen *screen, int width, int height)
{
    LIBMATTI_MC_OptionsScreen *options = (LIBMATTI_MC_OptionsScreen *) screen;
    active_options_screen = options;

    int leftX = width / 2 - 155;
    int rightX = width / 2 + 5;
    int rowY = height / 4 - 24 + 8; // Java: the first small-option row base

    char label[64];

    // Java: the sensitivity row rides the LEFT column (the small option set)
    LIBMATTI_MC_Button *sensitivity = LIBMATTI_MC_Button_New(leftX, rowY, 150,
        LIBMATTI_MC_Button_DEFAULT_HEIGHT, "Sensitivity", press_sensitivity);
    snprintf(label, sizeof(label), "Sensitivity: %d%%",
             LIBMATTI_MC_Options_SensitivityPercent(options->options));
    LIBMATTI_MC_AbstractWidget_SetMessage(&sensitivity->base, label);
    LIBMATTI_MC_Screen_AddRenderableWidget(screen, &sensitivity->base);

    // Java: the master volume row rides the RIGHT column (SoundSource.MASTER)
    LIBMATTI_MC_Button *volume = LIBMATTI_MC_Button_New(rightX, rowY, 150,
        LIBMATTI_MC_Button_DEFAULT_HEIGHT, "Master Volume", press_master_volume);
    snprintf(label, sizeof(label), "Master Volume: %d%%",
             (int) (LIBMATTI_MC_Options_GetMasterVolume(options->options) * 100.0f + 0.5f));
    LIBMATTI_MC_AbstractWidget_SetMessage(&volume->base, label);
    LIBMATTI_MC_Screen_AddRenderableWidget(screen, &volume->base);

    // Java: addRenderableWidget(doneButton = Button.builder(GUI_DONE,
    //           b -> this.minecraft.setScreen(this.lastScreen))
    //           .bounds(this.width / 2 - 100, this.height / 4 + 120, 200, 20).build())
    LIBMATTI_MC_Screen_AddRenderableWidget(screen, (LIBMATTI_MC_AbstractWidget *)
        LIBMATTI_MC_Button_New(width / 2 - 100, height / 4 + 120,
                               LIBMATTI_MC_Button_BIG_WIDTH,
                               LIBMATTI_MC_Button_DEFAULT_HEIGHT, "Done", press_done));
}

LIBMATTI_MC_OptionsScreen *LIBMATTI_MC_OptionsScreen_New(struct LIBMATTI_MC_Minecraft *minecraft,
                                                         LIBMATTI_MC_Options *options)
{
    LIBMATTI_MC_OptionsScreen *screen = calloc(1, sizeof(LIBMATTI_MC_OptionsScreen));
    if (screen == NULL)
        return NULL;
    // Java: super(new GenericMessageScreen(TITLE)) - the shared options title
    LIBMATTI_MC_Screen_Init(&screen->base, minecraft, LIBMATTI_MC_OptionsScreen_TITLE);
    screen->base.init = options_init;
    screen->base.isPauseScreen = options_is_pause_screen;
    screen->options = options;
    screen->done = false;
    return screen;
}

void LIBMATTI_MC_OptionsScreen_Free(LIBMATTI_MC_OptionsScreen *screen)
{
    if (screen == NULL)
        return;
    if (active_options_screen == screen)
        active_options_screen = NULL;
    LIBMATTI_MC_Screen_Cleanup(&screen->base);
    free(screen);
}
