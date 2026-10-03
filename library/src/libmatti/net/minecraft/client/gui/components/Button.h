// Port of net.minecraft.client.gui.components.{AbstractButton,Button} - the
// press-button widget. Java splits the behaviour (AbstractButton: the click
// arming, the on/off state) from Button (the OnPress callback + the builder);
// the C port carries both over one struct: the OnPress function pointer rides
// the vtable's onClick, the render draws the flat vanilla sprite fallback
// (the widget texture sprites land with the texture port).

#ifndef MATTICRAFT_MC_CLIENT_GUI_COMPONENTS_BUTTON_H
#define MATTICRAFT_MC_CLIENT_GUI_COMPONENTS_BUTTON_H

#include "libmatti/net/minecraft/client/gui/components/AbstractWidget.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: Button.DEFAULT_WIDTH / DEFAULT_HEIGHT / SMALL_WIDTH / BIG_WIDTH
#define LIBMATTI_MC_Button_SMALL_WIDTH 120
#define LIBMATTI_MC_Button_DEFAULT_WIDTH 150
#define LIBMATTI_MC_Button_BIG_WIDTH 200
#define LIBMATTI_MC_Button_DEFAULT_HEIGHT 20
#define LIBMATTI_MC_Button_DEFAULT_SPACING 8

// Java: public abstract class Button extends AbstractButton
typedef struct LIBMATTI_MC_Button
{
    LIBMATTI_MC_AbstractWidget base;
    // Java: protected final Button.OnPress onPress
    void (*onPress)(struct LIBMATTI_MC_Button *button);
    // the button-internal state (Java: AbstractButton's private fields)
    bool isPressed; // the click arming (the press edge the release completes)
    // the render data the widget renderer needs (the port's flat fallback)
    int textColor;
} LIBMATTI_MC_Button;

// Java: Button.Builder.build() - (x, y, width, height, message, onPress);
// width/height default to 150/20 like the Builder defaults
LIBMATTI_MC_Button *LIBMATTI_MC_Button_New(int x, int y, int width, int height, const char *message,
                                           void (*onPress)(struct LIBMATTI_MC_Button *button));
// Java: Button.builder(...).bounds(x, y, w, h) shorthand with the defaults
LIBMATTI_MC_Button *LIBMATTI_MC_Button_NewDefault(int x, int y, const char *message,
                                                  void (*onPress)(struct LIBMATTI_MC_Button *button));
void LIBMATTI_MC_Button_Free(LIBMATTI_MC_Button *button);

// Java: public void onPress(InputWithModifiers) -> this.onPress.onPress(this)
void LIBMATTI_MC_Button_Press(LIBMATTI_MC_Button *button);
// Java: AbstractButton's pressed state (the render sprite picks it)
bool LIBMATTI_MC_Button_IsPressed(const LIBMATTI_MC_Button *button);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_GUI_COMPONENTS_BUTTON_H
