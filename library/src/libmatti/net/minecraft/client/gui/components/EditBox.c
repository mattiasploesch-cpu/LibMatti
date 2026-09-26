// Port of net.minecraft.client.gui.components.EditBox over the char buffer.
// The value mutates through setValue/insertText/deleteText with the filter +
// max length gates, the cursor/highlight follow Java's clamping (the anchor
// keeps the selection), the responder fires on every committed change.

#include "libmatti/net/minecraft/client/gui/components/EditBox.h"

#include <stdlib.h>
#include <string.h>

// Java: public EditBox(Font, int x, int y, int width, int height, Component)
LIBMATTI_MC_EditBox *LIBMATTI_MC_EditBox_New(int x, int y, int width, int height, const char *message)
{
    LIBMATTI_MC_EditBox *editBox = calloc(1, sizeof(LIBMATTI_MC_EditBox));
    if (editBox == NULL)
        return NULL;
    LIBMATTI_MC_AbstractWidget_Init(&editBox->base, x, y, width, height, message);
    editBox->base.keyPressed = LIBMATTI_MC_EditBox_KeyPress;
    editBox->base.charTyped = LIBMATTI_MC_EditBox_CharTyped;
    editBox->maxLength = 32;      // Java: private int maxLength = 32
    editBox->editable = true;     // Java: private boolean editable = true
    editBox->bordered = true;     // Java: private boolean bordered = true
    editBox->value = strdup("");
    return editBox;
}

void LIBMATTI_MC_EditBox_Free(LIBMATTI_MC_EditBox *editBox)
{
    if (editBox == NULL)
        return;
    free(editBox->value);
    free(editBox->hint);
    free(editBox);
}

// Java: private void onValueChange(String) - the responder tail
static void value_changed(LIBMATTI_MC_EditBox *editBox)
{
    if (editBox->responder != NULL)
        editBox->responder(editBox->value, editBox->responderUserData);
}

void LIBMATTI_MC_EditBox_SetValue(LIBMATTI_MC_EditBox *editBox, const char *value)
{
    if (editBox == NULL)
        return;
    if (value == NULL)
        value = "";
    // Java: if (this.filter.test(text)) text = this.filter.test(...) - the
    // filter gate rides the candidate
    if (editBox->filter != NULL && !editBox->filter(value, editBox->filterUserData))
        return;
    int max = editBox->maxLength;
    if ((int) strlen(value) > max)
    {
        char *clamped = strndup(value, (size_t) max);
        free(editBox->value);
        editBox->value = clamped;
    }
    else
    {
        char *copy = strdup(value);
        free(editBox->value);
        editBox->value = copy;
    }
    LIBMATTI_MC_EditBox_MoveCursorToEnd(editBox);
    value_changed(editBox);
}

const char *LIBMATTI_MC_EditBox_GetValue(const LIBMATTI_MC_EditBox *editBox)
{
    return editBox != NULL && editBox->value != NULL ? editBox->value : "";
}

void LIBMATTI_MC_EditBox_SetMaxLength(LIBMATTI_MC_EditBox *editBox, int maxLength)
{
    if (editBox == NULL || maxLength < 0)
        return;
    editBox->maxLength = maxLength;
    if ((int) strlen(editBox->value) > maxLength)
        editBox->value[maxLength] = '\0';
}

void LIBMATTI_MC_EditBox_SetHint(LIBMATTI_MC_EditBox *editBox, const char *hint)
{
    if (editBox == NULL)
        return;
    free(editBox->hint);
    editBox->hint = hint != NULL ? strdup(hint) : NULL;
}

const char *LIBMATTI_MC_EditBox_GetHint(const LIBMATTI_MC_EditBox *editBox)
{
    return editBox != NULL ? editBox->hint : NULL;
}

void LIBMATTI_MC_EditBox_SetResponder(LIBMATTI_MC_EditBox *editBox,
                                      void (*responder)(const char *newValue, void *userData), void *userData)
{
    if (editBox == NULL)
        return;
    editBox->responder = responder;
    editBox->responderUserData = userData;
}

void LIBMATTI_MC_EditBox_SetFilter(LIBMATTI_MC_EditBox *editBox,
                                   bool (*filter)(const char *candidate, void *userData), void *userData)
{
    if (editBox == NULL)
        return;
    editBox->filter = filter;
    editBox->filterUserData = userData;
}

void LIBMATTI_MC_EditBox_SetEditable(LIBMATTI_MC_EditBox *editBox, bool editable)
{
    if (editBox != NULL)
        editBox->editable = editable;
}

void LIBMATTI_MC_EditBox_SetFocused(LIBMATTI_MC_EditBox *editBox, bool focused)
{
    if (editBox != NULL)
        editBox->focused = focused;
}

bool LIBMATTI_MC_EditBox_IsFocused(const LIBMATTI_MC_EditBox *editBox)
{
    return editBox != NULL ? editBox->focused : false;
}

int LIBMATTI_MC_EditBox_GetCursorPosition(const LIBMATTI_MC_EditBox *editBox)
{
    return editBox != NULL ? editBox->cursorPos : 0;
}

// Java: setCursorPosition -> moveCursorTo(pos, false) - the clamps over the
// length AND the highlight reset (the no-shift cursor move clears the selection)
void LIBMATTI_MC_EditBox_SetCursorPosition(LIBMATTI_MC_EditBox *editBox, int position)
{
    if (editBox == NULL)
        return;
    int length = (int) strlen(editBox->value);
    if (position < 0)
        position = 0;
    if (position > length)
        position = length;
    editBox->cursorPos = position;
    editBox->highlightPos = position;
}

void LIBMATTI_MC_EditBox_MoveCursorToEnd(LIBMATTI_MC_EditBox *editBox)
{
    LIBMATTI_MC_EditBox_SetCursorPosition(editBox, editBox != NULL ? (int) strlen(editBox->value) : 0);
}

void LIBMATTI_MC_EditBox_MoveCursorTo(LIBMATTI_MC_EditBox *editBox, int position)
{
    LIBMATTI_MC_EditBox_SetCursorPosition(editBox, position);
}

void LIBMATTI_MC_EditBox_SetHighlightPos(LIBMATTI_MC_EditBox *editBox, int position)
{
    if (editBox == NULL)
        return;
    LIBMATTI_MC_EditBox_SetCursorPosition(editBox, position);
    editBox->highlightPos = editBox->cursorPos;
}

int LIBMATTI_MC_EditBox_GetHighlightPos(const LIBMATTI_MC_EditBox *editBox)
{
    return editBox != NULL ? editBox->highlightPos : 0;
}

// Java: public void insertText(String) - the filter + max length gate, the
// highlight range replaced (the port collapses the selection into the anchor)
void LIBMATTI_MC_EditBox_InsertText(LIBMATTI_MC_EditBox *editBox, const char *text)
{
    if (editBox == NULL || text == NULL || !editBox->editable)
        return;
    int start = editBox->cursorPos < editBox->highlightPos ? editBox->cursorPos : editBox->highlightPos;
    int end = editBox->cursorPos > editBox->highlightPos ? editBox->cursorPos : editBox->highlightPos;
    int length = (int) strlen(editBox->value);
    int insertLength = (int) strlen(text);
    if (end > length)
        end = length;

    // Java: the candidate = value[..start] + text + value[end..] clamped to
    // maxLength, the filter rides the whole candidate
    int tailLength = length - end;
    int available = editBox->maxLength - (start + tailLength);
    if (available <= 0)
        available = 0;
    if (insertLength > available)
        insertLength = available;

    char *candidate = malloc((size_t) start + (size_t) insertLength + (size_t) tailLength + 1);
    if (candidate == NULL)
        return;
    memcpy(candidate, editBox->value, (size_t) start);
    memcpy(candidate + start, text, (size_t) insertLength);
    memcpy(candidate + start + insertLength, editBox->value + end, (size_t) tailLength);
    candidate[start + insertLength + tailLength] = '\0';

    if (editBox->filter != NULL && !editBox->filter(candidate, editBox->filterUserData))
    {
        free(candidate);
        return;
    }

    free(editBox->value);
    editBox->value = candidate;
    editBox->cursorPos = start + insertLength;
    editBox->highlightPos = editBox->cursorPos;
    value_changed(editBox);
}

void LIBMATTI_MC_EditBox_DeleteText(LIBMATTI_MC_EditBox *editBox, int direction)
{
    if (editBox == NULL || !editBox->editable)
        return;
    // Java: deleteText(-1) deletes the char before the cursor (backspace),
    // deleteText(1) the char after (delete) - over the highlight range first
    int start = editBox->cursorPos < editBox->highlightPos ? editBox->cursorPos : editBox->highlightPos;
    int end = editBox->cursorPos > editBox->highlightPos ? editBox->cursorPos : editBox->highlightPos;
    int length = (int) strlen(editBox->value);
    if (start == end)
    {
        if (direction < 0)
        {
            if (start <= 0)
                return;
            start--;
        }
        else
        {
            if (end >= length)
                return;
            end++;
        }
    }
    char *candidate = malloc((size_t) length - (size_t) (end - start) + 1);
    if (candidate == NULL)
        return;
    memcpy(candidate, editBox->value, (size_t) start);
    memcpy(candidate + start, editBox->value + end, (size_t) (length - end));
    candidate[start + (length - end)] = '\0';

    if (editBox->filter != NULL && !editBox->filter(candidate, editBox->filterUserData))
    {
        free(candidate);
        return;
    }

    free(editBox->value);
    editBox->value = candidate;
    editBox->cursorPos = start;
    editBox->highlightPos = start;
    value_changed(editBox);
}

bool LIBMATTI_MC_EditBox_KeyPress(LIBMATTI_MC_AbstractWidget *widget, int keyCode, int scanCode, int modifiers)
{
    (void) scanCode;
    (void) modifiers;
    LIBMATTI_MC_EditBox *editBox = (LIBMATTI_MC_EditBox *) widget;
    if (editBox == NULL || !editBox->editable)
        return false;
    int length = (int) strlen(editBox->value);
    switch (keyCode)
    {
    case 259: // GLFW_KEY_BACKSPACE
        LIBMATTI_MC_EditBox_DeleteText(editBox, -1);
        return true;
    case 261: // GLFW_KEY_DELETE
        LIBMATTI_MC_EditBox_DeleteText(editBox, 1);
        return true;
    case 262: // GLFW_KEY_RIGHT
        if (editBox->cursorPos < length)
            LIBMATTI_MC_EditBox_SetHighlightPos(editBox, editBox->cursorPos + 1);
        return true;
    case 263: // GLFW_KEY_LEFT
        if (editBox->cursorPos > 0)
            LIBMATTI_MC_EditBox_SetHighlightPos(editBox, editBox->cursorPos - 1);
        return true;
    case 268: // GLFW_KEY_HOME
        LIBMATTI_MC_EditBox_SetHighlightPos(editBox, 0);
        return true;
    case 269: // GLFW_KEY_END
        LIBMATTI_MC_EditBox_SetHighlightPos(editBox, length);
        return true;
    default:
        return false;
    }
}

bool LIBMATTI_MC_EditBox_CharTyped(LIBMATTI_MC_AbstractWidget *widget, unsigned int codePoint, int modifiers)
{
    (void) modifiers;
    LIBMATTI_MC_EditBox *editBox = (LIBMATTI_MC_EditBox *) widget;
    if (editBox == NULL || !editBox->editable)
        return false;
    // Java: SharedConstants.isAllowedChatCharacter - the printable ASCII gate
    // (the UTF-8 tails land with the text port)
    if (codePoint < 32 || codePoint > 126)
        return false;
    char text[2] = {(char) codePoint, '\0'};
    LIBMATTI_MC_EditBox_InsertText(editBox, text);
    return true;
}

void LIBMATTI_MC_EditBox_Tick(LIBMATTI_MC_EditBox *editBox)
{
    // Java: tickCount++ (the cursor blink phase the render reads)
    if (editBox != NULL)
        editBox->blinkTick++;
}
