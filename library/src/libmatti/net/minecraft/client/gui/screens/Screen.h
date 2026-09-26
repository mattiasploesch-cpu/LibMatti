// Port of net.minecraft.client.gui.screens.Screen - the base class the P6.2
// hierarchy builds on. Java's Screen collapses into the C struct + a small
// vtable: the lifecycle (init/resize/tick/removed), the child widget list,
// the ESC-close contract and the shouldCloseOnEsc gate. The narration/
// tooltip/focus-navigation tails stay out (no narrator, no tab order yet).

#ifndef MATTICRAFT_MC_CLIENT_GUI_SCREENS_SCREEN_H
#define MATTICRAFT_MC_CLIENT_GUI_SCREENS_SCREEN_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct LIBMATTI_MC_AbstractWidget;
struct LIBMATTI_MC_Minecraft;

// Java: protected static final float FADE_IN_TIME = 2000.0F
#define LIBMATTI_MC_Screen_FADE_IN_TIME 2000.0f

// Java: public abstract class Screen extends AbstractContainerEventHandler
// implements Renderable - the port's vtable carries the subclass hooks
// (init/tick/render/onClose + the event tails the screens override)
typedef struct LIBMATTI_MC_Screen
{
    // the subclass vtable
    // Java: protected void init() - the widget building
    void (*init)(struct LIBMATTI_MC_Screen *screen, int width, int height);
    // Java: public void tick() - the per-tick tail (EditBox cursor etc.)
    void (*tick)(struct LIBMATTI_MC_Screen *screen);
    // Java: public void render(GuiGraphics, int mouseX, int mouseY, float partialTick)
    void (*render)(struct LIBMATTI_MC_Screen *screen, int mouseX, int mouseY, float partialTick);
    // Java: public void removed() - the teardown tail
    void (*removed)(struct LIBMATTI_MC_Screen *screen);
    // Java: public boolean isPauseScreen() - the pause gate (default false
    // in the port; the pause screens override to true)
    bool (*isPauseScreen)(const struct LIBMATTI_MC_Screen *screen);
    // Java: public boolean keyPressed - the ESC/focus handling rides the base
    // (the subclass returns false for unhandled keys); NULL = base default
    bool (*keyPressed)(struct LIBMATTI_MC_Screen *screen, int keyCode, int scanCode, int modifiers);

    // Java: protected final Minecraft minecraft
    struct LIBMATTI_MC_Minecraft *minecraft;
    // Java: public int width / height (the gui-scaled size)
    int width;
    int height;
    // Java: protected final Component title
    char *title;
    // Java: private final List<GuiEventListener> children (the port keeps the
    // AbstractWidget list the render/mouse walks)
    struct LIBMATTI_MC_AbstractWidget **children;
    int childCount;
    int childCapacity;
    // Java: private boolean initialized
    bool initialized;
    // Java: public boolean canBeClosed = true (the ESC gate rides
    // shouldCloseOnEsc(); the port collapses it into the flag)
    bool canBeClosed;
} LIBMATTI_MC_Screen;

// Java: protected Screen(Minecraft, Font, Component) - the base init (the
// title copies, the vtable the subclass fills after)
void LIBMATTI_MC_Screen_Init(LIBMATTI_MC_Screen *screen, struct LIBMATTI_MC_Minecraft *minecraft,
                             const char *title);
// the typed free (frees the children + the title; the subclass struct rides
// the caller's allocation)
void LIBMATTI_MC_Screen_Free(LIBMATTI_MC_Screen *screen);

// Java: public final void init(int width, int height) - the resize entry
// (clears + re-adds the widgets through the vtable's init)
void LIBMATTI_MC_Screen_Resize(LIBMATTI_MC_Screen *screen, int width, int height);
// the vtable dispatches (NULL vtable entry = the no-op default)
void LIBMATTI_MC_Screen_Tick(LIBMATTI_MC_Screen *screen);
void LIBMATTI_MC_Screen_Render(LIBMATTI_MC_Screen *screen, int mouseX, int mouseY, float partialTick);
void LIBMATTI_MC_Screen_Removed(LIBMATTI_MC_Screen *screen);

// Java: public boolean keyPressed(KeyEvent) - the ESC branch closes through
// shouldCloseOnEsc/onClose; the subclass vtable handles the rest first
bool LIBMATTI_MC_Screen_KeyPress(LIBMATTI_MC_Screen *screen, int keyCode, int scanCode, int modifiers);

// Java: public boolean shouldCloseOnEsc() -> canBeClosed
bool LIBMATTI_MC_Screen_ShouldCloseOnEsc(const LIBMATTI_MC_Screen *screen);
// Java: public void onClose() - the port answers "close me" (the client
// clears its screen pointer; the vtable's onClose tail rides the subclass)
void LIBMATTI_MC_Screen_OnClose(LIBMATTI_MC_Screen *screen);

// Java: protected <T ...> T addRenderableWidget(T) - the screen takes the
// widget (the Free frees them)
struct LIBMATTI_MC_AbstractWidget *LIBMATTI_MC_Screen_AddRenderableWidget(LIBMATTI_MC_Screen *screen,
                                                                          struct LIBMATTI_MC_AbstractWidget *widget);
// Java: public Component getTitle()
const char *LIBMATTI_MC_Screen_GetTitle(const LIBMATTI_MC_Screen *screen);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_GUI_SCREENS_SCREEN_H
