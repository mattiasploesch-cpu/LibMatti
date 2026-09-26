// Port of net.minecraft.client.gui.components.AbstractWidget - the geometry,
// the state gates and the mouse/keyboard routing. The hover flag rides the
// render pass (Java: render sets isHovered from the cursor), the click fires
// on the release edge like AbstractButton.mouseClicked.

#include "libmatti/net/minecraft/client/gui/components/AbstractWidget.h"

#include <stdlib.h>
#include <string.h>

void LIBMATTI_MC_AbstractWidget_Init(LIBMATTI_MC_AbstractWidget *widget, int x, int y, int width, int height,
                                     const char *message)
{
    if (widget == NULL)
        return;
    memset(widget, 0, sizeof(*widget));
    widget->x = x;
    widget->y = y;
    widget->width = width;
    widget->height = height;
    widget->active = true;  // Java: public boolean active = true
    widget->visible = true; // Java: public boolean visible = true
    widget->alpha = 1.0f;   // Java: protected float alpha = 1.0F
    widget->message = message != NULL ? strdup(message) : NULL;
}

void LIBMATTI_MC_AbstractWidget_Free(LIBMATTI_MC_AbstractWidget *widget)
{
    if (widget == NULL)
        return;
    // Java: the subclass tail rides the vtable (the typed extra state dies)
    if (widget->free != NULL)
        widget->free(widget);
    free(widget->message);
    free(widget);
}

int LIBMATTI_MC_AbstractWidget_GetX(const LIBMATTI_MC_AbstractWidget *widget)
{
    return widget != NULL ? widget->x : 0;
}

int LIBMATTI_MC_AbstractWidget_GetY(const LIBMATTI_MC_AbstractWidget *widget)
{
    return widget != NULL ? widget->y : 0;
}

int LIBMATTI_MC_AbstractWidget_GetWidth(const LIBMATTI_MC_AbstractWidget *widget)
{
    return widget != NULL ? widget->width : 0;
}

int LIBMATTI_MC_AbstractWidget_GetHeight(const LIBMATTI_MC_AbstractWidget *widget)
{
    return widget != NULL ? widget->height : 0;
}

void LIBMATTI_MC_AbstractWidget_SetPosition(LIBMATTI_MC_AbstractWidget *widget, int x, int y)
{
    if (widget == NULL)
        return;
    widget->x = x;
    widget->y = y;
}

void LIBMATTI_MC_AbstractWidget_SetWidth(LIBMATTI_MC_AbstractWidget *widget, int width)
{
    if (widget != NULL)
        widget->width = width;
}

void LIBMATTI_MC_AbstractWidget_SetHeight(LIBMATTI_MC_AbstractWidget *widget, int height)
{
    if (widget != NULL)
        widget->height = height;
}

void LIBMATTI_MC_AbstractWidget_SetMessage(LIBMATTI_MC_AbstractWidget *widget, const char *message)
{
    if (widget == NULL)
        return;
    free(widget->message);
    widget->message = message != NULL ? strdup(message) : NULL;
}

const char *LIBMATTI_MC_AbstractWidget_GetMessage(const LIBMATTI_MC_AbstractWidget *widget)
{
    return widget != NULL ? widget->message : NULL;
}

void LIBMATTI_MC_AbstractWidget_SetActive(LIBMATTI_MC_AbstractWidget *widget, bool active)
{
    if (widget != NULL)
        widget->active = active;
}

void LIBMATTI_MC_AbstractWidget_SetVisible(LIBMATTI_MC_AbstractWidget *widget, bool visible)
{
    if (widget != NULL)
        widget->visible = visible;
}

void LIBMATTI_MC_AbstractWidget_SetAlpha(LIBMATTI_MC_AbstractWidget *widget, float alpha)
{
    if (widget == NULL)
        return;
    // Java: Math clamp 0..1 (the setter clamps in the callers; the port clamps here)
    widget->alpha = alpha < 0.0f ? 0.0f : (alpha > 1.0f ? 1.0f : alpha);
}

bool LIBMATTI_MC_AbstractWidget_IsHovered(const LIBMATTI_MC_AbstractWidget *widget)
{
    return widget != NULL ? widget->isHovered : false;
}

// Java: private boolean areCoordinatesInRectangle(int, int)
bool LIBMATTI_MC_AbstractWidget_ContainsPoint(const LIBMATTI_MC_AbstractWidget *widget, double x, double y)
{
    if (widget == NULL)
        return false;
    return x >= (double) widget->x && x < (double) (widget->x + widget->width)
           && y >= (double) widget->y && y < (double) (widget->y + widget->height);
}

void LIBMATTI_MC_AbstractWidget_Render(LIBMATTI_MC_AbstractWidget *widget, int mouseX, int mouseY, float partialTick)
{
    if (widget == NULL || !widget->visible)
        return; // Java: if (this.visible) guards the whole pass
    // Java: this.isHovered = guiGraphics.containsPointInScissor(...) && in-rect
    widget->isHovered = LIBMATTI_MC_AbstractWidget_ContainsPoint(widget, mouseX, mouseY);
    if (widget->renderWidget != NULL)
        widget->renderWidget(widget, mouseX, mouseY, partialTick);
}

// Java: public boolean mouseClicked(MouseButtonEvent, boolean) over
// AbstractButton: the in-rect + active gate, onClick fires on the RELEASE
bool LIBMATTI_MC_AbstractWidget_MouseClicked(LIBMATTI_MC_AbstractWidget *widget, double mouseX, double mouseY,
                                             int button, bool isRelease)
{
    if (widget == NULL || !widget->active || !widget->visible)
        return false;
    if (!LIBMATTI_MC_AbstractWidget_ContainsPoint(widget, mouseX, mouseY))
        return false;
    // Java: if (this.clicked && isRelease) { this.playDownSound ... onClick }
    // - the port's release edge fires the vtable (the press edge arms the
    // hover state; the click-count tail rides the caller)
    if (isRelease && widget->onClick != NULL)
        widget->onClick(widget, mouseX, mouseY, button);
    return true;
}

bool LIBMATTI_MC_AbstractWidget_KeyPress(LIBMATTI_MC_AbstractWidget *widget, int keyCode, int scanCode, int modifiers)
{
    if (widget == NULL || !widget->active)
        return false;
    return widget->keyPressed != NULL ? widget->keyPressed(widget, keyCode, scanCode, modifiers) : false;
}

bool LIBMATTI_MC_AbstractWidget_CharTyped(LIBMATTI_MC_AbstractWidget *widget, unsigned int codePoint, int modifiers)
{
    if (widget == NULL || !widget->active)
        return false;
    return widget->charTyped != NULL ? widget->charTyped(widget, codePoint, modifiers) : false;
}
