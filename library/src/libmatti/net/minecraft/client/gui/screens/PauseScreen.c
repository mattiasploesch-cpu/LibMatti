// Port of net.minecraft.client.gui.screens.PauseScreen over the Screen base.
// The buttons ride the P6.2 Button widget (the press/release edges fire the
// flags the client polls); the geometry follows the vanilla pause menu: the
// 204-wide buttons at width/2 - 102 stack over height/4 + 8 (Back to Game),
// + 72 (Options...) and + 96 (Save and Quit to Title). The render stays a
// client-side concern (the flat fallback draws the widgets, the client adds
// title + background + the tick freeze through isPauseScreen).

#include "libmatti/net/minecraft/client/gui/screens/PauseScreen.h"

#include "libmatti/net/minecraft/client/gui/components/Button.h"

#include <stdlib.h>

// Java's Button.builder lambdas capture `this`; the C Button onPress carries
// no userdata, so the port keeps the ONE active pause screen (the client owns
// the single instance) the press hooks answer through.
static LIBMATTI_MC_PauseScreen *active_pause_screen = NULL;

static void press_back_to_game(LIBMATTI_MC_Button *button)
{
    (void) button;
    if (active_pause_screen != NULL)
        active_pause_screen->backToGame = true;
}

static void press_options(LIBMATTI_MC_Button *button)
{
    (void) button;
    if (active_pause_screen != NULL)
        active_pause_screen->openOptions = true;
}

static void press_quit(LIBMATTI_MC_Button *button)
{
    (void) button;
    if (active_pause_screen != NULL)
        active_pause_screen->quitToTitle = true;
}

// Java: public boolean isPauseScreen() -> true - the tick loop freezes
static bool pause_is_pause_screen(const LIBMATTI_MC_Screen *screen)
{
    (void) screen;
    return true;
}

// Java: protected void init() - the widget build (the resize re-runs it)
static void pause_init(LIBMATTI_MC_Screen *screen, int width, int height)
{
    active_pause_screen = (LIBMATTI_MC_PauseScreen *) screen;

    // Java: addRenderableWidget(Button.builder(BACK_TO_GAME,
    //           b -> this.minecraft.setScreen((Screen)null))
    //           .bounds(this.width / 2 - 102, this.height / 4 + 8, 204, 20).build())
    LIBMATTI_MC_Screen_AddRenderableWidget(screen, (LIBMATTI_MC_AbstractWidget *)
        LIBMATTI_MC_Button_New(width / 2 - 102, height / 4 + 8, 204,
                               LIBMATTI_MC_PauseScreen_BUTTON_HEIGHT, "Back to Game",
                               press_back_to_game));

    // Java: addRenderableWidget(this.optionsButton = Button.builder(OPTIONS,
    //           b -> this.minecraft.setScreen(new OptionsScreen(this, this.options)))
    //           .bounds(this.width / 2 - 102, this.height / 4 + 72, 204, 20).build())
    LIBMATTI_MC_Screen_AddRenderableWidget(screen, (LIBMATTI_MC_AbstractWidget *)
        LIBMATTI_MC_Button_New(width / 2 - 102, height / 4 + 72, 204,
                               LIBMATTI_MC_PauseScreen_BUTTON_HEIGHT, "Options...",
                               press_options));

    // Java: addRenderableWidget(Button.builder(SAVE_AND_QUIT,
    //           b -> this.minecraft.stop())
    //           .bounds(this.width / 2 - 102, this.height / 4 + 96, 204, 20).build())
    LIBMATTI_MC_Screen_AddRenderableWidget(screen, (LIBMATTI_MC_AbstractWidget *)
        LIBMATTI_MC_Button_New(width / 2 - 102, height / 4 + 96, 204,
                               LIBMATTI_MC_PauseScreen_BUTTON_HEIGHT, "Save and Quit to Title",
                               press_quit));
}

LIBMATTI_MC_PauseScreen *LIBMATTI_MC_PauseScreen_New(struct LIBMATTI_MC_Minecraft *minecraft)
{
    LIBMATTI_MC_PauseScreen *screen = calloc(1, sizeof(LIBMATTI_MC_PauseScreen));
    if (screen == NULL)
        return NULL;
    // Java: super(Component.translatable("menu.paused")) - the pause menu
    // carries the Game Menu title
    LIBMATTI_MC_Screen_Init(&screen->base, minecraft, LIBMATTI_MC_PauseScreen_TITLE);
    screen->base.init = pause_init;
    screen->base.isPauseScreen = pause_is_pause_screen;
    screen->showPauseMenu = true;
    screen->backToGame = false;
    screen->quitToTitle = false;
    screen->openOptions = false;
    active_pause_screen = screen;
    return screen;
}

void LIBMATTI_MC_PauseScreen_Free(LIBMATTI_MC_PauseScreen *screen)
{
    if (screen == NULL)
        return;
    if (active_pause_screen == screen)
        active_pause_screen = NULL;
    // the children die through the base cleanup; the struct rides the
    // caller's allocation like the other screens
    LIBMATTI_MC_Screen_Cleanup(&screen->base);
    free(screen);
}
