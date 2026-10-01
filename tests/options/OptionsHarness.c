// Options harness: the P6.4 core - the Options store (the vanilla key:value
// options.txt round-trip over FMLPaths.GAMEDIR), the PauseScreen (the
// vanilla 3-button layout, isPauseScreen, the flag contract) and the
// OptionsScreen (the 10% step rows, the live setters, the Done contract).

#include "libmatti/net/minecraft/client/Options.h"
#include "libmatti/net/minecraft/client/gui/components/Button.h"
#include "libmatti/net/minecraft/client/gui/screens/PauseScreen.h"
#include "libmatti/net/minecraft/client/gui/screens/OptionsScreen.h"
#include "libmatti/net/neoforged/fml/loading/FMLPaths.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures;
static int checks;

static void check(int condition, const char *what)
{
    checks++;
    if (!condition)
    {
        failures++;
        printf("FAIL: %s\n", what);
    }
}

// ---------------------------------------------------------------------------
// Options store (Java: Options.load/save over the key:value options.txt)
// ---------------------------------------------------------------------------

static void test_options_roundtrip(const char *dir)
{
    LIBMATTI_FML_FMLPaths_LoadAbsolutePaths(dir);

    // Java: the defaults before any file exists
    LIBMATTI_MC_Options options;
    LIBMATTI_MC_Options_Init(&options);
    check(options.sensitivity == 0.5f, "default sensitivity 0.5");
    check(options.masterVolume == 1.0f, "default master volume 1.0");
    check(options.guiScale == 0, "default guiScale auto (0)");
    check(options.fullscreen == false, "default fullscreen off");

    // the missing options.txt keeps the defaults (Java: the first save creates it)
    LIBMATTI_MC_Options_Load(&options);
    check(options.sensitivity == 0.5f, "missing file keeps the sensitivity default");
    check(options.masterVolume == 1.0f, "missing file keeps the volume default");

    // Java: Options.save - the plain key:value lines
    LIBMATTI_MC_Options_SetSensitivity(&options, 0.3f);
    LIBMATTI_MC_Options_SetMasterVolume(&options, 0.2f);
    LIBMATTI_MC_Options_SetGuiScale(&options, 3);
    LIBMATTI_MC_Options_SetFullscreen(&options, true);
    LIBMATTI_MC_Options_Save(&options);

    char *path = LIBMATTI_MC_Options_Path();
    check(path != NULL, "the options path resolves in the GAMEDIR");
    FILE *in = fopen(path, "r");
    check(in != NULL, "save creates options.txt");
    if (in != NULL)
    {
        char line[128];
        int sawSensitivity = 0, sawVolume = 0, sawScale = 0, sawFullscreen = 0;
        while (fgets(line, sizeof(line), in) != NULL)
        {
            if (strncmp(line, "sensitivity:", 12) == 0)
            {
                sawSensitivity = 1;
                check(strncmp(line, "sensitivity:0.3", 15) == 0, "sensitivity writes the 0..1 value");
            }
            else if (strncmp(line, "masterVolume:", 13) == 0)
            {
                sawVolume = 1;
                check(strncmp(line, "masterVolume:0.2", 16) == 0, "masterVolume writes the 0..1 value");
            }
            else if (strncmp(line, "guiScale:", 9) == 0)
            {
                sawScale = 1;
                check(strncmp(line, "guiScale:3", 10) == 0, "guiScale writes the pinned factor");
            }
            else if (strncmp(line, "fullscreen:", 11) == 0)
            {
                sawFullscreen = 1;
                check(strncmp(line, "fullscreen:true", 15) == 0, "fullscreen writes true/false");
            }
        }
        fclose(in);
        check(sawSensitivity && sawVolume && sawScale && sawFullscreen, "save carries the full option set");
    }
    free(path);

    // Java: Options.load - the round-trip restores the values
    LIBMATTI_MC_Options reloaded;
    LIBMATTI_MC_Options_Init(&reloaded);
    LIBMATTI_MC_Options_Load(&reloaded);
    check(reloaded.sensitivity == 0.3f, "load restores the sensitivity");
    check(reloaded.masterVolume == 0.2f, "load restores the master volume");
    check(reloaded.guiScale == 3, "load restores the gui scale");
    check(reloaded.fullscreen == true, "load restores the fullscreen flag");

    // Java: the clamps (the OptionInstance slider ranges)
    LIBMATTI_MC_Options_SetSensitivity(&reloaded, 5.0f);
    check(reloaded.sensitivity == 1.0f, "sensitivity clamps to 1");
    LIBMATTI_MC_Options_SetSensitivity(&reloaded, -1.0f);
    check(reloaded.sensitivity == 0.0f, "sensitivity clamps to 0");
    LIBMATTI_MC_Options_SetMasterVolume(&reloaded, 42.0f);
    check(reloaded.masterVolume == 1.0f, "master volume clamps to 1");
    check(LIBMATTI_MC_Options_SensitivityPercent(&reloaded) == 0, "the percent label follows the value");

    LIBMATTI_MC_Options_Free(&reloaded);
    LIBMATTI_MC_Options_Free(&options);
}

static void test_options_garbage(const char *dir)
{
    // Java: unknown keys trace and stay ignored; malformed lines skip
    char *path = LIBMATTI_MC_Options_Path();
    FILE *out = fopen(path, "w");
    if (out != NULL)
    {
        fprintf(out, "thisLineHasNoColon\nunknownKey:7\nsensitivity:0.8\nmasterVolume:notANumber\n");
        fclose(out);
    }
    free(path);

    LIBMATTI_MC_Options options;
    LIBMATTI_MC_Options_Init(&options);
    LIBMATTI_MC_Options_Load(&options);
    check(options.sensitivity == 0.8f, "the known key parses");
    check(options.masterVolume == 1.0f, "the unparseable value keeps the default");
    check(options.guiScale == 0, "the unknown keys stay ignored");
    LIBMATTI_MC_Options_Free(&options);
}

// ---------------------------------------------------------------------------
// PauseScreen (Java: PauseScreen init + isPauseScreen + the flag contract)
// ---------------------------------------------------------------------------

static void test_pause_screen(void)
{
    LIBMATTI_MC_PauseScreen *pause = LIBMATTI_MC_PauseScreen_New(NULL);
    check(pause != NULL, "the pause screen allocates");
    if (pause == NULL)
        return;

    // Java: the resize builds the widgets (the first init)
    LIBMATTI_MC_Screen_Resize(&pause->base, 427, 240);
    check(pause->base.childCount == 8, "the vanilla 1.21.11 8-button layout");
    check(pause->base.isPauseScreen != NULL && pause->base.isPauseScreen(&pause->base),
          "isPauseScreen answers true");

    // Java: the grid bounds - the FrameLayout align (0.5, 0.25) over the
    // 204x170 grid: gridX = width/2 - 102, gridY = (height - 170) * 0.25, the
    // rows at gridY + 50 + row * 24 (the 4px cell padding rides the stride).
    // 427x240: gridX 111, gridY 17.
    int gridX = 427 / 2 - 102;
    int gridY = (int) ((float) (240 - 170) * 0.25f);
    check(LIBMATTI_MC_AbstractWidget_GetX(pause->base.children[0]) == gridX + 4,
          "Back to Game at gridX + 4");
    check(LIBMATTI_MC_AbstractWidget_GetWidth(pause->base.children[0]) == 204,
          "the full button width 204");
    check(LIBMATTI_MC_AbstractWidget_GetY(pause->base.children[0]) == gridY + 50,
          "Back to Game at gridY + 50 (the title band rides above)");
    check(LIBMATTI_MC_AbstractWidget_GetX(pause->base.children[2]) == gridX + 110,
          "the right column at gridX + 110");
    check(LIBMATTI_MC_AbstractWidget_GetY(pause->base.children[5]) == gridY + 50 + 3 * 24,
          "Options in the third half row");
    check(LIBMATTI_MC_AbstractWidget_GetY(pause->base.children[7]) == gridY + 50 + 4 * 24,
          "Quit in the last row");
    check(LIBMATTI_MC_AbstractWidget_GetWidth(pause->base.children[7]) == 204,
          "the quit span rides 204");
    check(strcmp(LIBMATTI_MC_AbstractWidget_GetMessage(pause->base.children[0]), "Back to Game") == 0,
          "the Back to Game label");
    check(strcmp(LIBMATTI_MC_AbstractWidget_GetMessage(pause->base.children[7]), "Save and Quit to Title") == 0,
          "the quit label");

    // Java: the screens-that-do-not-exist-yet stay inactive (the disabled
    // sprite + the eaten press) - only Back to Game / Options / Quit answer
    check(pause->base.children[0]->active && pause->base.children[5]->active
              && pause->base.children[7]->active,
          "the three wired buttons stay active");
    check(!pause->base.children[1]->active && !pause->base.children[2]->active
              && !pause->base.children[3]->active && !pause->base.children[4]->active
              && !pause->base.children[6]->active,
          "Advancements/Statistics/Feedback/Bugs/LAN render disabled");

    // Java: the resize rebuilds (the stale widgets die, the layout re-runs)
    LIBMATTI_MC_Screen_Resize(&pause->base, 320, 240);
    check(pause->base.childCount == 8, "the resize rebuilds exactly 8 widgets");
    check(LIBMATTI_MC_AbstractWidget_GetX(pause->base.children[0]) == 320 / 2 - 102 + 4,
          "the resize re-centres the grid");

    // Java: the button presses set the flags (the onClick release edge)
    LIBMATTI_MC_Button *back = (LIBMATTI_MC_Button *) pause->base.children[0];
    LIBMATTI_MC_Button *options = (LIBMATTI_MC_Button *) pause->base.children[5];
    LIBMATTI_MC_Button *quit = (LIBMATTI_MC_Button *) pause->base.children[7];
    check(!pause->backToGame && !pause->openOptions && !pause->quitToTitle, "the flags start false");
    LIBMATTI_MC_Button_Press(back);
    check(pause->backToGame, "Back to Game sets the flag");
    pause->backToGame = false;
    LIBMATTI_MC_Button_Press(options);
    check(pause->openOptions, "Options sets the flag");
    LIBMATTI_MC_Button_Press(quit);
    check(pause->quitToTitle, "Quit sets the flag");

    // Java: the ESC contract - the pause closes through the base (the client
    // polls the close like the inventory screen)
    check(LIBMATTI_MC_Screen_ShouldCloseOnEsc(&pause->base), "the pause closes on ESC");

    LIBMATTI_MC_PauseScreen_Free(pause);
}

// ---------------------------------------------------------------------------
// OptionsScreen (Java: OptionsScreen init + the option rows + the Done)
// ---------------------------------------------------------------------------

static void test_options_screen(void)
{
    LIBMATTI_MC_Options options;
    LIBMATTI_MC_Options_Init(&options);
    LIBMATTI_MC_Options_SetSensitivity(&options, 0.5f);
    LIBMATTI_MC_Options_SetMasterVolume(&options, 1.0f);

    LIBMATTI_MC_OptionsScreen *screen = LIBMATTI_MC_OptionsScreen_New(NULL, &options);
    check(screen != NULL, "the options screen allocates");
    if (screen == NULL)
    {
        LIBMATTI_MC_Options_Free(&options);
        return;
    }
    LIBMATTI_MC_Screen_Resize(&screen->base, 427, 240);
    check(screen->base.childCount == 3, "the two option rows + Done");
    check(screen->base.isPauseScreen != NULL && screen->base.isPauseScreen(&screen->base),
          "the options keep the pause gate");

    // Java: the row geometry - 150 wide at width/2 - 155 / +5, Done 200 at + 120
    LIBMATTI_MC_Button *sensitivity = (LIBMATTI_MC_Button *) screen->base.children[0];
    LIBMATTI_MC_Button *volume = (LIBMATTI_MC_Button *) screen->base.children[1];
    LIBMATTI_MC_Button *done = (LIBMATTI_MC_Button *) screen->base.children[2];
    check(LIBMATTI_MC_AbstractWidget_GetX(&sensitivity->base) == 427 / 2 - 155, "sensitivity rides the left column");
    check(LIBMATTI_MC_AbstractWidget_GetX(&volume->base) == 427 / 2 + 5, "master volume rides the right column");
    check(LIBMATTI_MC_AbstractWidget_GetWidth(&done->base) == 200, "Done rides BIG_WIDTH 200");
    check(LIBMATTI_MC_AbstractWidget_GetY(&done->base) == 240 / 4 + 120, "Done at height/4 + 120");

    // Java: the press applies the 10% step through the OptionInstance setter
    // (the sensitivity label rides the value*200% the vanilla slider shows)
    LIBMATTI_MC_Button_Press(sensitivity);
    check(options.sensitivity > 0.59f && options.sensitivity < 0.61f, "the sensitivity step is 10%");
    check(strstr(LIBMATTI_MC_AbstractWidget_GetMessage(&sensitivity->base), "120%") != NULL,
          "the sensitivity label follows the value*200% label");
    LIBMATTI_MC_Button_Press(volume);
    check(volume != NULL, "the volume press answers");
    check(strstr(LIBMATTI_MC_AbstractWidget_GetMessage(&volume->base), "0%") != NULL,
          "the volume wraps at the top and labels 0%");

    // Java: the wrap at the top of the range (0.6 -> five more 10% steps)
    LIBMATTI_MC_Button_Press(sensitivity);
    LIBMATTI_MC_Button_Press(sensitivity);
    LIBMATTI_MC_Button_Press(sensitivity);
    LIBMATTI_MC_Button_Press(sensitivity);
    LIBMATTI_MC_Button_Press(sensitivity);
    check(options.sensitivity == 0.0f, "the sensitivity wraps to 0 at the top");

    // Java: Done -> the flag (the client returns to the pause screen)
    check(!screen->done, "done starts false");
    LIBMATTI_MC_Button_Press(done);
    check(screen->done, "Done sets the flag");

    // the options struct rides the caller (the screen does not own it)
    LIBMATTI_MC_OptionsScreen_Free(screen);
    LIBMATTI_MC_Options_Free(&options);
}

int main(void)
{
    // the isolated game dir (FMLPaths points there for the harness)
    char dir[] = "/tmp/matti_options_XXXXXX";
    if (mkdtemp(dir) == NULL)
    {
        printf("FAIL: mkdtemp\n");
        return 1;
    }

    test_options_roundtrip(dir);
    test_options_garbage(dir);
    test_pause_screen();
    test_options_screen();

    // the harness leaves no trace outside the temp dir
    char *path = LIBMATTI_MC_Options_Path();
    if (path != NULL)
    {
        unlink(path);
        free(path);
    }
    rmdir(dir);

    printf("%s: %d checks, %d failures\n", "options", checks, failures);
    return failures != 0 ? 1 : 0;
}
