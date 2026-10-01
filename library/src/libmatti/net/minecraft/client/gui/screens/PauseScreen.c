// Port of net.minecraft.client.gui.screens.PauseScreen over the Screen base
// (the vanilla 1.21.11 widget set). Java builds the menu through the
// GridLayout(2) + FrameLayout.alignInRectangle(0.5, 0.25): Return to Game
// spans both columns, then the Advancements/Statistics, Send Feedback/Report
// Bugs and Options.../Share to LAN half pairs, and the Save and Quit to
// Title span. The port folds the grid arrange into straight bounds (the
// CellInhabitant setX/setY the Java pass writes): x = width/2 - 106 for the
// left column, + 106 for the right, the rows over gridY + 4 + row * 24
// (the 4px cell padding rides the stride). The buttons carry the SAME flag
// contract the client polls (the C onPress has no userdata, the file-local
// active screen answers); the inactive entries (Advancements, Statistics,
// Send Feedback, Report Bugs, Share to LAN) ride active=false like Java's
// screens-that-do-not-exist-yet would - they render disabled and eat the
// press (Java: Button.active gates the press before the sound).

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

// Java: the grid columns - the half cells sit at width/2 - 102 (the 98 cell
// centred in the left 102 half) and width/2 + 4 (the right half); the full
// spans ride width/2 - 102 over 204.
static void pause_init(LIBMATTI_MC_Screen *screen, int width, int height)
{
    active_pause_screen = (LIBMATTI_MC_PauseScreen *) screen;

    // Java: FrameLayout.alignInRectangle(grid, 0, 0, width, height, 0.5F, 0.25F)
    // - the grid (204 x 170) centres horizontally, the vertical align rides
    // the 0.25 fraction of the leftover. The first row's 50px paddingTop
    // carries the title band (the StringWidget at y 40 sits in it).
    int gridX = width / 2 - LIBMATTI_MC_PauseScreen_GRID_WIDTH / 2;
    int gridY = (int) ((float) (height - LIBMATTI_MC_PauseScreen_GRID_HEIGHT) * 0.25f);

    int leftX = gridX + LIBMATTI_MC_PauseScreen_BUTTON_PADDING;
    int rightX = gridX + 106 + LIBMATTI_MC_PauseScreen_BUTTON_PADDING;

    // Java: addRenderableWidget(Button.builder(RETURN_TO_GAME, ...).width(204)
    //           .build(), 2, newCellSettings().paddingTop(50)) - the first row
    // rides the 50px top padding (the title band sits above).
    LIBMATTI_MC_Screen_AddRenderableWidget(screen, (LIBMATTI_MC_AbstractWidget *)
        LIBMATTI_MC_Button_New(gridX + LIBMATTI_MC_PauseScreen_BUTTON_PADDING, gridY + 50,
                               LIBMATTI_MC_PauseScreen_BUTTON_WIDTH_FULL,
                               LIBMATTI_MC_PauseScreen_BUTTON_HEIGHT, "Back to Game",
                               press_back_to_game));

    // the 2-column rows ride the default cell padding (the 24px stride)
    // Java: openScreenButton(ADVANCEMENTS, ... new AdvancementsScreen(...))
    LIBMATTI_MC_AbstractWidget *advancements = (LIBMATTI_MC_AbstractWidget *)
        LIBMATTI_MC_Button_New(leftX, gridY + 50 + LIBMATTI_MC_PauseScreen_ROW_STRIDE,
                               LIBMATTI_MC_PauseScreen_BUTTON_WIDTH_HALF,
                               LIBMATTI_MC_PauseScreen_BUTTON_HEIGHT, "Advancements", NULL);
    LIBMATTI_MC_Screen_AddRenderableWidget(screen, advancements);

    // Java: openScreenButton(STATS, ... new StatsScreen(...))
    LIBMATTI_MC_AbstractWidget *stats = (LIBMATTI_MC_AbstractWidget *)
        LIBMATTI_MC_Button_New(rightX, gridY + 50 + LIBMATTI_MC_PauseScreen_ROW_STRIDE,
                               LIBMATTI_MC_PauseScreen_BUTTON_WIDTH_HALF,
                               LIBMATTI_MC_PauseScreen_BUTTON_HEIGHT, "Statistics", NULL);
    LIBMATTI_MC_Screen_AddRenderableWidget(screen, stats);

    // Java: openLinkButton(SEND_FEEDBACK, CommonLinks.RELEASE_FEEDBACK)
    LIBMATTI_MC_AbstractWidget *feedback = (LIBMATTI_MC_AbstractWidget *)
        LIBMATTI_MC_Button_New(leftX, gridY + 50 + 2 * LIBMATTI_MC_PauseScreen_ROW_STRIDE,
                               LIBMATTI_MC_PauseScreen_BUTTON_WIDTH_HALF,
                               LIBMATTI_MC_PauseScreen_BUTTON_HEIGHT, "Send Feedback", NULL);
    LIBMATTI_MC_Screen_AddRenderableWidget(screen, feedback);

    // Java: openLinkButton(REPORT_BUGS, CommonLinks.SNAPSHOT_BUGS_FEEDBACK)
    LIBMATTI_MC_AbstractWidget *bugs = (LIBMATTI_MC_AbstractWidget *)
        LIBMATTI_MC_Button_New(rightX, gridY + 50 + 2 * LIBMATTI_MC_PauseScreen_ROW_STRIDE,
                               LIBMATTI_MC_PauseScreen_BUTTON_WIDTH_HALF,
                               LIBMATTI_MC_PauseScreen_BUTTON_HEIGHT, "Report Bugs", NULL);
    LIBMATTI_MC_Screen_AddRenderableWidget(screen, bugs);

    // Java: openScreenButton(OPTIONS, () -> new OptionsScreen(this, this.options))
    LIBMATTI_MC_Screen_AddRenderableWidget(screen, (LIBMATTI_MC_AbstractWidget *)
        LIBMATTI_MC_Button_New(leftX, gridY + 50 + 3 * LIBMATTI_MC_PauseScreen_ROW_STRIDE,
                               LIBMATTI_MC_PauseScreen_BUTTON_WIDTH_HALF,
                               LIBMATTI_MC_PauseScreen_BUTTON_HEIGHT, "Options...",
                               press_options));

    // Java: openScreenButton(SHARE_TO_LAN, () -> new ShareToLanScreen(this))
    // (the singleplayer branch - the port runs the integrated server)
    LIBMATTI_MC_AbstractWidget *shareLan = (LIBMATTI_MC_AbstractWidget *)
        LIBMATTI_MC_Button_New(rightX, gridY + 50 + 3 * LIBMATTI_MC_PauseScreen_ROW_STRIDE,
                               LIBMATTI_MC_PauseScreen_BUTTON_WIDTH_HALF,
                               LIBMATTI_MC_PauseScreen_BUTTON_HEIGHT, "Share to LAN", NULL);
    LIBMATTI_MC_Screen_AddRenderableWidget(screen, shareLan);

    // Java: Button.builder(disconnectButtonLabel(...), ...) -> the disconnect
    // flow (the port folds it into stop() through the quit flag)
    LIBMATTI_MC_Screen_AddRenderableWidget(screen, (LIBMATTI_MC_AbstractWidget *)
        LIBMATTI_MC_Button_New(gridX + LIBMATTI_MC_PauseScreen_BUTTON_PADDING,
                               gridY + 50 + 4 * LIBMATTI_MC_PauseScreen_ROW_STRIDE,
                               LIBMATTI_MC_PauseScreen_BUTTON_WIDTH_FULL,
                               LIBMATTI_MC_PauseScreen_BUTTON_HEIGHT,
                               "Save and Quit to Title", press_quit));

    // Java: the port's screens-that-do-not-exist-yet stay OUT of the input:
    // active=false renders the disabled sprite and eats the press before the
    // sound (AbstractButton.mouseClicked gates on this.active).
    LIBMATTI_MC_AbstractWidget_SetActive(advancements, false);
    LIBMATTI_MC_AbstractWidget_SetActive(stats, false);
    LIBMATTI_MC_AbstractWidget_SetActive(feedback, false);
    LIBMATTI_MC_AbstractWidget_SetActive(bugs, false);
    LIBMATTI_MC_AbstractWidget_SetActive(shareLan, false);
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
