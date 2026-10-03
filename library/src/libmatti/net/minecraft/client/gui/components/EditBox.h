// Port of net.minecraft.client.gui.components.EditBox - the text field.
// Java's EditBox carries the value buffer, the cursor/selection bookkeeping,
// the max length, the responder/filter hooks and the hint; the port keeps the
// same surface over a plain char buffer (the UTF-8/codepoint tails collapse
// into bytes, the IME trails stay out).

#ifndef MATTICRAFT_MC_CLIENT_GUI_COMPONENTS_EDITBOX_H
#define MATTICRAFT_MC_CLIENT_GUI_COMPONENTS_EDITBOX_H

#include "libmatti/net/minecraft/client/gui/components/AbstractWidget.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: EditBox.DEFAULT_TEXT_COLOR = 0xE0E0E0 / the bordered-background shade
#define LIBMATTI_MC_EditBox_DEFAULT_TEXT_COLOR 0xE0E0E0
#define LIBMATTI_MC_EditBox_BACK_COLOR 0xFF000000
#define LIBMATTI_MC_EditBox_BORDER_COLOR 0xFFA0A0A0

// Java: public class EditBox extends AbstractWidget
typedef struct LIBMATTI_MC_EditBox
{
    LIBMATTI_MC_AbstractWidget base;
    // Java: private String value = ""
    char *value;
    // Java: private int maxLength = 32
    int maxLength;
    // Java: private int cursorPos / displayPos (the port keeps one cursor;
    // the scroll display tail rides the renderer)
    int cursorPos;
    // Java: private int highlightPos (the selection anchor)
    int highlightPos;
    // Java: private boolean editable = true / bordered = true
    bool editable;
    bool bordered;
    // Java: private Consumer<String> responder
    void (*responder)(const char *newValue, void *userData);
    void *responderUserData;
    // Java: private Predicate<String> filter
    bool (*filter)(const char *candidate, void *userData);
    void *filterUserData;
    // Java: private Component hint
    char *hint;
    // Java: private boolean canLoseFocus / focused
    bool focused;
    // Java: private long focusedTime (the cursor blink phase)
    double focusedTime;
    // the port's blink bookkeeping (the renderer toggles the cursor line)
    int blinkTick;
} LIBMATTI_MC_EditBox;

// Java: public EditBox(Font, int x, int y, int width, int height, Component)
LIBMATTI_MC_EditBox *LIBMATTI_MC_EditBox_New(int x, int y, int width, int height, const char *message);
void LIBMATTI_MC_EditBox_Free(LIBMATTI_MC_EditBox *editBox);

// Java: public void setValue(String) / getValue()
void LIBMATTI_MC_EditBox_SetValue(LIBMATTI_MC_EditBox *editBox, const char *value);
const char *LIBMATTI_MC_EditBox_GetValue(const LIBMATTI_MC_EditBox *editBox);
// Java: public void setMaxLength(int)
void LIBMATTI_MC_EditBox_SetMaxLength(LIBMATTI_MC_EditBox *editBox, int maxLength);
// Java: public void setHint(Component) / getHint
void LIBMATTI_MC_EditBox_SetHint(LIBMATTI_MC_EditBox *editBox, const char *hint);
const char *LIBMATTI_MC_EditBox_GetHint(const LIBMATTI_MC_EditBox *editBox);
// Java: public void setResponder(Consumer<String>) (+ the user-data the port carries)
void LIBMATTI_MC_EditBox_SetResponder(LIBMATTI_MC_EditBox *editBox,
                                      void (*responder)(const char *newValue, void *userData), void *userData);
// Java: public void setFilter(Predicate<String>)
void LIBMATTI_MC_EditBox_SetFilter(LIBMATTI_MC_EditBox *editBox,
                                   bool (*filter)(const char *candidate, void *userData), void *userData);
// Java: public void setEditable(boolean) / isEditable
void LIBMATTI_MC_EditBox_SetEditable(LIBMATTI_MC_EditBox *editBox, bool editable);
// Java: public void setFocused(boolean) / isFocused (the cursor blink rides it)
void LIBMATTI_MC_EditBox_SetFocused(LIBMATTI_MC_EditBox *editBox, bool focused);
bool LIBMATTI_MC_EditBox_IsFocused(const LIBMATTI_MC_EditBox *editBox);

// Java: public int getCursorPosition() / setCursorPosition(int)
int LIBMATTI_MC_EditBox_GetCursorPosition(const LIBMATTI_MC_EditBox *editBox);
void LIBMATTI_MC_EditBox_SetCursorPosition(LIBMATTI_MC_EditBox *editBox, int position);
// Java: public void moveCursorTo(int, boolean) / moveCursor(int) - the
// highlight tracking collapses into the anchor reset; the no-arg form moves
// to the end (Java: moveCursorTo(stringLength))
void LIBMATTI_MC_EditBox_MoveCursorTo(LIBMATTI_MC_EditBox *editBox, int position);
void LIBMATTI_MC_EditBox_MoveCursorToEnd(LIBMATTI_MC_EditBox *editBox);
// Java: public void setHighlightPos(int) / getHighlightedText
void LIBMATTI_MC_EditBox_SetHighlightPos(LIBMATTI_MC_EditBox *editBox, int position);
int LIBMATTI_MC_EditBox_GetHighlightPos(const LIBMATTI_MC_EditBox *editBox);

// Java: public void insertText(String) - the filter + max length gate
void LIBMATTI_MC_EditBox_InsertText(LIBMATTI_MC_EditBox *editBox, const char *text);
// Java: public boolean keyPressed(KeyEvent) - the arrows/backspace/delete/
// home/end/selection keys (the keycodes ride GLFW)
bool LIBMATTI_MC_EditBox_KeyPress(LIBMATTI_MC_AbstractWidget *widget, int keyCode, int scanCode, int modifiers);
// Java: public boolean charTyped(CharacterEvent) - the printable insert
bool LIBMATTI_MC_EditBox_CharTyped(LIBMATTI_MC_AbstractWidget *widget, unsigned int codePoint, int modifiers);
// Java: public void deleteText(int) - the backspace/delete direction
void LIBMATTI_MC_EditBox_DeleteText(LIBMATTI_MC_EditBox *editBox, int direction);
// Java: private void setTextColor / the port's tick (the blink counter)
void LIBMATTI_MC_EditBox_Tick(LIBMATTI_MC_EditBox *editBox);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_GUI_COMPONENTS_EDITBOX_H
