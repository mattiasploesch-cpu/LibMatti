// Port of net.minecraft.client.gui.components.AbstractWidget - the widget
// base (x/y/width/height, the active/visible/alpha gates, the hover state
// and the mouse click routing). Java's GuiEventListener/event tails collapse
// into the vtable the subclasses fill (Button/EditBox).

#ifndef MATTICRAFT_MC_CLIENT_GUI_COMPONENTS_ABSTRACTWIDGET_H
#define MATTICRAFT_MC_CLIENT_GUI_COMPONENTS_ABSTRACTWIDGET_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public abstract class AbstractWidget implements Renderable,
// GuiEventListener, NarratableEntry, TabOrderedElement
typedef struct LIBMATTI_MC_AbstractWidget
{
    // the subclass vtable
    // Java: protected abstract void updateWidgetNarration (the port drops the
    // narration) - the subclasses carry renderWidget/onClick instead
    void (*renderWidget)(struct LIBMATTI_MC_AbstractWidget *widget, int mouseX, int mouseY, float partialTick);
    // Java: public void onClick(MouseButtonEvent, boolean) - default no-op
    void (*onClick)(struct LIBMATTI_MC_AbstractWidget *widget, double mouseX, double mouseY, int button);
    // Java: public boolean mouseClicked(MouseButtonEvent, boolean) - the
    // subclass override (AbstractButton arms the press); NULL = the base
    bool (*mouseClicked)(struct LIBMATTI_MC_AbstractWidget *widget, double mouseX, double mouseY,
                         int button, bool isRelease);
    // Java: public boolean keyPressed(KeyEvent) - EditBox consumes, default false
    bool (*keyPressed)(struct LIBMATTI_MC_AbstractWidget *widget, int keyCode, int scanCode, int modifiers);
    // Java: public boolean charTyped(CharacterEvent) - EditBox consumes
    bool (*charTyped)(struct LIBMATTI_MC_AbstractWidget *widget, unsigned int codePoint, int modifiers);
    // the typed free (the subclass frees its extra state)
    void (*free)(struct LIBMATTI_MC_AbstractWidget *widget);

    // Java: private int x / y / width / height
    int x;
    int y;
    int width;
    int height;
    // Java: protected boolean isHovered
    bool isHovered;
    // Java: public boolean active = true / visible = true
    bool active;
    bool visible;
    // Java: protected float alpha = 1.0F
    float alpha;
    // Java: protected Component message
    char *message;
} LIBMATTI_MC_AbstractWidget;

// Java: protected AbstractWidget(int x, int y, int width, int height, Component)
void LIBMATTI_MC_AbstractWidget_Init(LIBMATTI_MC_AbstractWidget *widget, int x, int y, int width, int height,
                                     const char *message);
// the base free (the message dies; subclasses chain through the vtable free)
void LIBMATTI_MC_AbstractWidget_Free(LIBMATTI_MC_AbstractWidget *widget);

// Java: public int getX() / getY() / getWidth() / getHeight()
int LIBMATTI_MC_AbstractWidget_GetX(const LIBMATTI_MC_AbstractWidget *widget);
int LIBMATTI_MC_AbstractWidget_GetY(const LIBMATTI_MC_AbstractWidget *widget);
int LIBMATTI_MC_AbstractWidget_GetWidth(const LIBMATTI_MC_AbstractWidget *widget);
int LIBMATTI_MC_AbstractWidget_GetHeight(const LIBMATTI_MC_AbstractWidget *widget);
// Java: public void setPosition(int, int) / setWidth(int) / setHeight(int)
void LIBMATTI_MC_AbstractWidget_SetPosition(LIBMATTI_MC_AbstractWidget *widget, int x, int y);
void LIBMATTI_MC_AbstractWidget_SetWidth(LIBMATTI_MC_AbstractWidget *widget, int width);
void LIBMATTI_MC_AbstractWidget_SetHeight(LIBMATTI_MC_AbstractWidget *widget, int height);
// Java: public void setMessage(Component)
void LIBMATTI_MC_AbstractWidget_SetMessage(LIBMATTI_MC_AbstractWidget *widget, const char *message);
const char *LIBMATTI_MC_AbstractWidget_GetMessage(const LIBMATTI_MC_AbstractWidget *widget);
// Java: public void setActive(boolean) / setVisible(boolean) / setAlpha(float)
void LIBMATTI_MC_AbstractWidget_SetActive(LIBMATTI_MC_AbstractWidget *widget, bool active);
void LIBMATTI_MC_AbstractWidget_SetVisible(LIBMATTI_MC_AbstractWidget *widget, bool visible);
void LIBMATTI_MC_AbstractWidget_SetAlpha(LIBMATTI_MC_AbstractWidget *widget, float alpha);

// Java: public boolean isHovered() / isHoveredOrFocused()
bool LIBMATTI_MC_AbstractWidget_IsHovered(const LIBMATTI_MC_AbstractWidget *widget);
// Java: private boolean areCoordinatesInRectangle(int, int) -> the hit test
bool LIBMATTI_MC_AbstractWidget_ContainsPoint(const LIBMATTI_MC_AbstractWidget *widget, double x, double y);

// Java: public final void render(...) - the hover gate + the vtable dispatch
// (invisible widgets render nothing, inactive widgets ride the alpha)
void LIBMATTI_MC_AbstractWidget_Render(LIBMATTI_MC_AbstractWidget *widget, int mouseX, int mouseY, float partialTick);

// Java: public boolean mouseClicked(MouseButtonEvent, boolean) - the vtable
// override first (AbstractButton), else the base: hit test + active gate,
// the vtable's onClick fires on release
bool LIBMATTI_MC_AbstractWidget_MouseClicked(LIBMATTI_MC_AbstractWidget *widget, double mouseX, double mouseY,
                                             int button, bool isRelease);
// the base dispatch the vtable override chains through (the hit test + the
// onClick release firing without the override)
bool LIBMATTI_MC_AbstractWidget_MouseClickedBase(LIBMATTI_MC_AbstractWidget *widget, double mouseX, double mouseY,
                                                 int button, bool isRelease);
// Java: public boolean keyPressed / charTyped - the vtable pass
bool LIBMATTI_MC_AbstractWidget_KeyPress(LIBMATTI_MC_AbstractWidget *widget, int keyCode, int scanCode, int modifiers);
bool LIBMATTI_MC_AbstractWidget_CharTyped(LIBMATTI_MC_AbstractWidget *widget, unsigned int codePoint, int modifiers);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_GUI_COMPONENTS_ABSTRACTWIDGET_H
