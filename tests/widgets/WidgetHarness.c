// Widget harness: the P6.2 core - the Screen lifecycle (init/resize clears
// and rebuilds, ESC close contract), the AbstractWidget geometry/state gates
// and the Button/EditBox behaviour (the press/release click, the text value
// mutations over the filter/responder hooks).

#include "libmatti/net/minecraft/client/gui/components/Button.h"
#include "libmatti/net/minecraft/client/gui/components/EditBox.h"
#include "libmatti/net/minecraft/client/gui/screens/Screen.h"

#include <stdio.h>
#include <string.h>

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
// Screen lifecycle (Java: Screen.init/resize/removed/keyPressed)
// ---------------------------------------------------------------------------

static int g_buildCount;

typedef struct TestScreen
{
    LIBMATTI_MC_Screen base;
    LIBMATTI_MC_Button *button;
} TestScreen;

static void test_screen_init(LIBMATTI_MC_Screen *screen, int width, int height)
{
    TestScreen *test = (TestScreen *) screen;
    g_buildCount++;
    test->button = LIBMATTI_MC_Button_NewDefault(width / 2 - 75, height / 2, "Click",
                                                 NULL);
    LIBMATTI_MC_Screen_AddRenderableWidget(screen, &test->button->base);
}

static void test_screen(void)
{
    TestScreen screen;
    memset(&screen, 0, sizeof(screen));
    LIBMATTI_MC_Screen_Init(&screen.base, NULL, "Test");
    screen.base.init = test_screen_init;

    // Java: the first init(width, height) builds the widgets
    LIBMATTI_MC_Screen_Resize(&screen.base, 320, 240);
    check(g_buildCount == 1, "init builds once");
    check(screen.base.childCount == 1, "one widget added");
    check(screen.base.width == 320 && screen.base.height == 240, "size stores");
    check(screen.base.initialized, "initialized flag rides");

    // Java: the resize clears + rebuilds (no leaked duplicates)
    LIBMATTI_MC_Screen_Resize(&screen.base, 640, 480);
    check(g_buildCount == 2, "resize rebuilds");
    check(screen.base.childCount == 1, "children cleared before the rebuild");
    check(LIBMATTI_MC_AbstractWidget_GetX(screen.base.children[0]) == 245, "the widget re-added at the new size");

    // Java: getTitle / the title copy
    check(strcmp(LIBMATTI_MC_Screen_GetTitle(&screen.base), "Test") == 0, "title copies");

    // Java: keyPressed ESC -> onClose (canBeClosed gates it)
    check(LIBMATTI_MC_Screen_ShouldCloseOnEsc(&screen.base), "ESC close allowed by default");
    check(LIBMATTI_MC_Screen_KeyPress(&screen.base, 256, 0, 0), "ESC consumed by the base");
    screen.base.canBeClosed = false;
    check(!LIBMATTI_MC_Screen_KeyPress(&screen.base, 256, 0, 0), "the ESC gate refuses when closed-forbidden");
    check(!LIBMATTI_MC_Screen_KeyPress(&screen.base, 87, 0, 0), "other keys pass through (unhandled)");

    LIBMATTI_MC_Screen_Cleanup(&screen.base);
}

// ---------------------------------------------------------------------------
// AbstractWidget geometry + state gates
// ---------------------------------------------------------------------------

static void test_widget(void)
{
    LIBMATTI_MC_Button *button = LIBMATTI_MC_Button_New(10, 20, 150, 20, "Hi", NULL);
    LIBMATTI_MC_AbstractWidget *widget = &button->base;
    if (widget == NULL)
    {
        printf("FAIL: widget allocates\n");
        failures++;
        return;
    }
    check(LIBMATTI_MC_AbstractWidget_GetX(widget) == 10, "x stores");
    check(LIBMATTI_MC_AbstractWidget_GetWidth(widget) == 150, "width stores");
    check(LIBMATTI_MC_AbstractWidget_GetHeight(widget) == 20, "height 20 (DEFAULT_HEIGHT)");
    check(strcmp(LIBMATTI_MC_AbstractWidget_GetMessage(widget), "Hi") == 0, "message copies");

    // Java: areCoordinatesInRectangle - the half-open rect
    check(LIBMATTI_MC_AbstractWidget_ContainsPoint(widget, 10.0, 20.0), "top-left inside");
    check(LIBMATTI_MC_AbstractWidget_ContainsPoint(widget, 159.0, 39.0), "bottom-right inside");
    check(!LIBMATTI_MC_AbstractWidget_ContainsPoint(widget, 160.0, 40.0), "past-end outside");
    check(!LIBMATTI_MC_AbstractWidget_ContainsPoint(widget, 9.0, 20.0), "left outside");

    LIBMATTI_MC_AbstractWidget_SetPosition(widget, 0, 0);
    LIBMATTI_MC_AbstractWidget_SetWidth(widget, 100);
    LIBMATTI_MC_AbstractWidget_SetHeight(widget, 10);
    check(LIBMATTI_MC_AbstractWidget_ContainsPoint(widget, 99.9, 9.9), "resized rect");

    LIBMATTI_MC_AbstractWidget_SetActive(widget, false);
    check(!LIBMATTI_MC_AbstractWidget_MouseClicked(widget, 50, 5, 0, true), "inactive ignores clicks");
    LIBMATTI_MC_AbstractWidget_SetActive(widget, true);
    LIBMATTI_MC_AbstractWidget_SetVisible(widget, false);
    check(!LIBMATTI_MC_AbstractWidget_MouseClicked(widget, 50, 5, 0, true), "invisible ignores clicks");
    LIBMATTI_MC_AbstractWidget_SetVisible(widget, true);

    LIBMATTI_MC_Button_Free(button);
}

// ---------------------------------------------------------------------------
// Button (Java: AbstractButton click arming + Button.OnPress)
// ---------------------------------------------------------------------------

static int g_pressCount;
static void count_press(LIBMATTI_MC_Button *button)
{
    (void) button;
    g_pressCount++;
}

static void test_button(void)
{
    LIBMATTI_MC_Button *button = LIBMATTI_MC_Button_NewDefault(0, 0, "Press", count_press);
    LIBMATTI_MC_AbstractWidget *widget = &button->base;
    g_pressCount = 0;

    // Java: the click rides press -> release (the release fires onClick)
    LIBMATTI_MC_AbstractWidget_MouseClicked(widget, 10, 10, 0, false);
    check(LIBMATTI_MC_Button_IsPressed(button), "the press edge arms");
    check(g_pressCount == 0, "no fire on the press edge");
    LIBMATTI_MC_AbstractWidget_MouseClicked(widget, 10, 10, 0, true);
    check(!LIBMATTI_MC_Button_IsPressed(button), "the release disarms");
    check(g_pressCount == 1, "the release fires OnPress");

    // the secondary button never fires
    LIBMATTI_MC_AbstractWidget_MouseClicked(widget, 10, 10, 1, true);
    check(g_pressCount == 1, "the secondary button ignores");

    // the outside click never fires
    LIBMATTI_MC_AbstractWidget_MouseClicked(widget, 500, 500, 0, true);
    check(g_pressCount == 1, "the outside release never fires");

    LIBMATTI_MC_Button_Free(button);
}

// ---------------------------------------------------------------------------
// EditBox (Java: EditBox value/cursor/filter/responder)
// ---------------------------------------------------------------------------

static char g_lastValue[64];
static int g_responderCalls;
static void record_responder(const char *value, void *userData)
{
    (void) userData;
    g_responderCalls++;
    snprintf(g_lastValue, sizeof(g_lastValue), "%s", value);
}

static bool reject_digits(const char *value, void *userData)
{
    (void) userData;
    for (; *value != '\0'; value++)
        if (*value >= '0' && *value <= '9')
            return false;
    return true;
}

static void test_editbox(void)
{
    LIBMATTI_MC_EditBox *editBox = LIBMATTI_MC_EditBox_New(0, 0, 200, 20, "");
    if (editBox == NULL)
    {
        printf("FAIL: editbox allocates\n");
        failures++;
        return;
    }
    LIBMATTI_MC_AbstractWidget *widget = &editBox->base;

    // Java: setValue + the cursor rides the end
    LIBMATTI_MC_EditBox_SetValue(editBox, "hello");
    check(strcmp(LIBMATTI_MC_EditBox_GetValue(editBox), "hello") == 0, "value stores");
    check(LIBMATTI_MC_EditBox_GetCursorPosition(editBox) == 5, "cursor rides the end");

    // Java: the max length clamps
    LIBMATTI_MC_EditBox_SetMaxLength(editBox, 8);
    LIBMATTI_MC_EditBox_SetValue(editBox, "0123456789abcdef");
    check(strlen(LIBMATTI_MC_EditBox_GetValue(editBox)) == 8, "maxLength clamps the value");

    // Java: charTyped inserts at the cursor (the printable gate)
    LIBMATTI_MC_EditBox_SetValue(editBox, "ab");
    LIBMATTI_MC_EditBox_SetCursorPosition(editBox, 1);
    LIBMATTI_MC_AbstractWidget_CharTyped(widget, 'X', 0);
    check(strcmp(LIBMATTI_MC_EditBox_GetValue(editBox), "aXb") == 0, "charTyped inserts at the cursor");
    check(LIBMATTI_MC_EditBox_GetCursorPosition(editBox) == 2, "the cursor rides the insert");

    // the control characters never insert
    LIBMATTI_MC_AbstractWidget_CharTyped(widget, '\n', 0);
    check(strcmp(LIBMATTI_MC_EditBox_GetValue(editBox), "aXb") == 0, "the control char refuses");

    // Java: backspace deletes before the cursor
    LIBMATTI_MC_AbstractWidget_KeyPress(widget, 259, 0, 0);
    check(strcmp(LIBMATTI_MC_EditBox_GetValue(editBox), "ab") == 0, "backspace deletes");

    // Java: delete removes after the cursor
    LIBMATTI_MC_EditBox_SetCursorPosition(editBox, 0);
    LIBMATTI_MC_AbstractWidget_KeyPress(widget, 261, 0, 0);
    check(strcmp(LIBMATTI_MC_EditBox_GetValue(editBox), "b") == 0, "delete removes after the cursor");

    // the left/right arrows walk the cursor
    LIBMATTI_MC_EditBox_SetValue(editBox, "abc");
    LIBMATTI_MC_AbstractWidget_KeyPress(widget, 263, 0, 0);
    LIBMATTI_MC_AbstractWidget_KeyPress(widget, 263, 0, 0);
    check(LIBMATTI_MC_EditBox_GetCursorPosition(editBox) == 1, "left walks the cursor");
    LIBMATTI_MC_AbstractWidget_KeyPress(widget, 262, 0, 0);
    check(LIBMATTI_MC_EditBox_GetCursorPosition(editBox) == 2, "right walks the cursor");

    // Java: the responder fires on every committed change
    g_responderCalls = 0;
    LIBMATTI_MC_EditBox_SetResponder(editBox, record_responder, NULL);
    LIBMATTI_MC_EditBox_SetValue(editBox, "abc");
    LIBMATTI_MC_AbstractWidget_CharTyped(widget, 'd', 0);
    check(g_responderCalls >= 2, "responder fires on setValue + insert");
    check(strcmp(g_lastValue, "abcd") == 0, "the responder sees the new value");

    // Java: the filter refuses the whole candidate
    LIBMATTI_MC_EditBox_SetFilter(editBox, reject_digits, NULL);
    LIBMATTI_MC_EditBox_SetValue(editBox, "abc123");
    check(strcmp(LIBMATTI_MC_EditBox_GetValue(editBox), "abcd") == 0, "the filter refuses the digits candidate");
    LIBMATTI_MC_AbstractWidget_CharTyped(widget, '5', 0);
    check(strcmp(LIBMATTI_MC_EditBox_GetValue(editBox), "abcd") == 0, "the filter refuses the digit insert");

    // Java: setEditable(false) freezes the mutations
    LIBMATTI_MC_EditBox_SetEditable(editBox, false);
    LIBMATTI_MC_AbstractWidget_CharTyped(widget, 'z', 0);
    check(strcmp(LIBMATTI_MC_EditBox_GetValue(editBox), "abcd") == 0, "the read-only box refuses inserts");

    LIBMATTI_MC_EditBox_Free(editBox);
}

int main(void)
{
    test_screen();
    test_widget();
    test_button();
    test_editbox();
    printf("widgets: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}
