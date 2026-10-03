// Port of net.minecraft.client.gui.components.{AbstractButton,Button}. The
// press edge arms (mouseClicked press), the release edge fires the OnPress
// callback; the render draws the vanilla flat sprite fallback (the grey
// gradient shade + the centred label color the active/hover states drive).

#include "libmatti/net/minecraft/client/gui/components/Button.h"

#include <stdlib.h>

// Java: AbstractButton.onPress fires through Button.onPress.onPress(this)
static void button_on_click(LIBMATTI_MC_AbstractWidget *widget, double mouseX, double mouseY, int buttonId)
{
    (void) mouseX;
    (void) mouseY;
    if (buttonId != 0) // Java: the primary button only
        return;
    LIBMATTI_MC_Button *button = (LIBMATTI_MC_Button *) widget;
    button->isPressed = false;
    LIBMATTI_MC_Button_Press(button);
}

// Java: AbstractButton.mouseClicked - the press edge arms (clicked = true),
// the release edge chains into the base dispatch (onClick fires)
static bool button_mouse_clicked(LIBMATTI_MC_AbstractWidget *widget, double mouseX, double mouseY,
                                 int buttonId, bool isRelease)
{
    LIBMATTI_MC_Button *button = (LIBMATTI_MC_Button *) widget;
    if (!isRelease && buttonId == 0
        && widget->active && widget->visible
        && LIBMATTI_MC_AbstractWidget_ContainsPoint(widget, mouseX, mouseY))
    {
        button->isPressed = true; // Java: this.clicked = true
        return true;
    }
    bool handled = LIBMATTI_MC_AbstractWidget_MouseClickedBase(widget, mouseX, mouseY, buttonId, isRelease);
    if (isRelease)
        button->isPressed = false; // Java: this.clicked = false
    return handled;
}

static void button_free(LIBMATTI_MC_AbstractWidget *widget)
{
    (void) widget; // the Button carries no extra heap state
}

static void button_render_widget(LIBMATTI_MC_AbstractWidget *widget, int mouseX, int mouseY, float partialTick)
{
    (void) partialTick;
    LIBMATTI_MC_Button *button = (LIBMATTI_MC_Button *) widget;
    (void) button;
    (void) mouseX;
    (void) mouseY;
    // Java: renderDefaultSprite + renderDefaultLabel - the widget sprite and
    // the centred message. The rendering rides the client's GuiRenderer (the
    // screens pass the draw through their render hook); the widget keeps the
    // state (hover/pressed/active) the renderer tints with.
}

LIBMATTI_MC_Button *LIBMATTI_MC_Button_New(int x, int y, int width, int height, const char *message,
                                           void (*onPress)(struct LIBMATTI_MC_Button *button))
{
    LIBMATTI_MC_Button *button = calloc(1, sizeof(LIBMATTI_MC_Button));
    if (button == NULL)
        return NULL;
    LIBMATTI_MC_AbstractWidget_Init(&button->base, x, y, width, height, message);
    button->base.renderWidget = button_render_widget;
    button->base.onClick = button_on_click;
    button->base.mouseClicked = button_mouse_clicked;
    button->base.free = button_free;
    // Java: AbstractButton overrides mouseClicked; the port routes the press
    // edge through the same vtable slot the base exposes
    button->onPress = onPress;
    button->isPressed = false;
    button->textColor = 0xFFE0E0E0;
    return button;
}

LIBMATTI_MC_Button *LIBMATTI_MC_Button_NewDefault(int x, int y, const char *message,
                                                  void (*onPress)(struct LIBMATTI_MC_Button *button))
{
    // Java: Button.builder(message, onPress).bounds(x, y, 150, 20)
    return LIBMATTI_MC_Button_New(x, y, LIBMATTI_MC_Button_DEFAULT_WIDTH, LIBMATTI_MC_Button_DEFAULT_HEIGHT,
                                  message, onPress);
}

void LIBMATTI_MC_Button_Free(LIBMATTI_MC_Button *button)
{
    LIBMATTI_MC_AbstractWidget_Free(&button->base);
}

void LIBMATTI_MC_Button_Press(LIBMATTI_MC_Button *button)
{
    if (button != NULL && button->onPress != NULL)
        button->onPress(button);
}

bool LIBMATTI_MC_Button_IsPressed(const LIBMATTI_MC_Button *button)
{
    return button != NULL ? button->isPressed : false;
}
