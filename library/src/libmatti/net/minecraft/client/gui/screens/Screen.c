// Port of net.minecraft.client.gui.screens.Screen over the widget list. The
// lifecycle follows Java: init(width,height) clears + rebuilds through the
// vtable, tick/render/removed dispatch, keyPressed handles the ESC branch.

#include "libmatti/net/minecraft/client/gui/screens/Screen.h"
#include "libmatti/net/minecraft/client/gui/components/AbstractWidget.h"

#include <stdlib.h>
#include <string.h>

// Java: protected Screen(Minecraft, Font, Component)
void LIBMATTI_MC_Screen_Init(LIBMATTI_MC_Screen *screen, struct LIBMATTI_MC_Minecraft *minecraft,
                             const char *title)
{
    if (screen == NULL)
        return;
    memset(screen, 0, sizeof(*screen));
    screen->minecraft = minecraft;
    screen->title = title != NULL ? strdup(title) : NULL;
    screen->canBeClosed = true; // Java: shouldCloseOnEsc() -> true
}

void LIBMATTI_MC_Screen_Free(LIBMATTI_MC_Screen *screen)
{
    if (screen == NULL)
        return;
    if (screen->children != NULL)
    {
        for (int i = 0; i < screen->childCount; i++)
            LIBMATTI_MC_AbstractWidget_Free(screen->children[i]);
        free(screen->children);
    }
    free(screen->title);
    free(screen);
}

// Java: private final void clearWidgets / repositionElements ride the resize
void LIBMATTI_MC_Screen_Resize(LIBMATTI_MC_Screen *screen, int width, int height)
{
    if (screen == NULL)
        return;
    screen->width = width;
    screen->height = height;
    if (screen->children != NULL)
    {
        for (int i = 0; i < screen->childCount; i++)
            LIBMATTI_MC_AbstractWidget_Free(screen->children[i]);
        free(screen->children);
        screen->children = NULL;
        screen->childCount = 0;
        screen->childCapacity = 0;
    }
    if (screen->init != NULL)
        screen->init(screen, width, height);
    screen->initialized = true; // Java: private boolean initialized
}

void LIBMATTI_MC_Screen_Tick(LIBMATTI_MC_Screen *screen)
{
    if (screen == NULL)
        return;
    if (screen->tick != NULL)
        screen->tick(screen);
}

void LIBMATTI_MC_Screen_Render(LIBMATTI_MC_Screen *screen, int mouseX, int mouseY, float partialTick)
{
    if (screen == NULL)
        return;
    if (screen->render != NULL)
    {
        screen->render(screen, mouseX, mouseY, partialTick);
        return;
    }
    // Java: the base render walks the renderables (the port's children)
    for (int i = 0; i < screen->childCount; i++)
        LIBMATTI_MC_AbstractWidget_Render(screen->children[i], mouseX, mouseY, partialTick);
}

void LIBMATTI_MC_Screen_Removed(LIBMATTI_MC_Screen *screen)
{
    if (screen == NULL)
        return;
    if (screen->removed != NULL)
        screen->removed(screen);
}

bool LIBMATTI_MC_Screen_ShouldCloseOnEsc(const LIBMATTI_MC_Screen *screen)
{
    return screen != NULL ? screen->canBeClosed : false;
}

void LIBMATTI_MC_Screen_OnClose(LIBMATTI_MC_Screen *screen)
{
    // Java: this.minecraft.setScreen(null) - the client owns the pointer;
    // the port answers through the flag the client polls (the setScreen
    // tail lands with the client wiring).
    (void) screen;
}

// Java: public boolean keyPressed(KeyEvent) - the ESC branch first
bool LIBMATTI_MC_Screen_KeyPress(LIBMATTI_MC_Screen *screen, int keyCode, int scanCode, int modifiers)
{
    if (screen == NULL)
        return false;
    // Java: GLFW_KEY_ESCAPE = 256
    if (keyCode == 256 && LIBMATTI_MC_Screen_ShouldCloseOnEsc(screen))
    {
        LIBMATTI_MC_Screen_OnClose(screen);
        return true;
    }
    if (screen->keyPressed != NULL)
        return screen->keyPressed(screen, keyCode, scanCode, modifiers);
    return false;
}

struct LIBMATTI_MC_AbstractWidget *LIBMATTI_MC_Screen_AddRenderableWidget(LIBMATTI_MC_Screen *screen,
                                                                          struct LIBMATTI_MC_AbstractWidget *widget)
{
    if (screen == NULL || widget == NULL)
        return widget;
    if (screen->childCount >= screen->childCapacity)
    {
        int newCapacity = screen->childCapacity == 0 ? 8 : screen->childCapacity * 2;
        struct LIBMATTI_MC_AbstractWidget **grown =
            realloc(screen->children, (size_t) newCapacity * sizeof(struct LIBMATTI_MC_AbstractWidget *));
        if (grown == NULL)
            return widget;
        screen->children = grown;
        screen->childCapacity = newCapacity;
    }
    screen->children[screen->childCount++] = widget;
    return widget;
}

const char *LIBMATTI_MC_Screen_GetTitle(const LIBMATTI_MC_Screen *screen)
{
    return screen != NULL ? screen->title : NULL;
}
