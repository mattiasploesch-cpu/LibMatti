// Port of net.neoforged.fml.earlydisplay.DisplayWindow (the "fmlearlywindow"
// ImmediateWindowProvider). Java renders the theme'd loading screen with a
// scheduled renderer thread; the port owns the same window lifecycle (init,
// hand-off, tick, crash) and reports progress through the StartupNotification
// meters until the game port takes the window over.

#ifndef MATTICRAFT_FML_EARLYDISPLAY_DISPLAYWINDOW_H
#define MATTICRAFT_FML_EARLYDISPLAY_DISPLAYWINDOW_H

#include "libmatti/net/neoforged/fml/loading/EarlyLoadingScreenController.h"
#include "libmatti/net/neoforged/fml/loading/ProgramArgs.h"
#include "libmatti/net/neoforged/neoforgespi/earlywindow/ImmediateWindowProvider.h"

// Java: public class DisplayWindow implements ImmediateWindowProvider
typedef struct LIBMATTI_FML_DisplayWindow LIBMATTI_FML_DisplayWindow;

// Java: public DisplayWindow()
LIBMATTI_FML_DisplayWindow *LIBMATTI_FML_DisplayWindow_New(void);
// Java: the provider instance is static state in Java; the port frees it explicitly
void LIBMATTI_FML_DisplayWindow_Free(LIBMATTI_FML_DisplayWindow *window);

// Java: the ImmediateWindowProvider vtable of DisplayWindow (for ServiceLoader-style registration)
LIBMATTI_NEOFORGESPI_ImmediateWindowProvider *LIBMATTI_FML_DisplayWindow_AsProvider(
    LIBMATTI_FML_DisplayWindow *window);

// Java: public void initWindow() (also called through initialize)
void LIBMATTI_FML_DisplayWindow_InitWindow(LIBMATTI_FML_DisplayWindow *window);
// Java: the resize callback captures the instance; the port registers the active one
void LIBMATTI_FML_DisplayWindow_SetCurrent(LIBMATTI_FML_DisplayWindow *window);
LIBMATTI_FML_DisplayWindow *LIBMATTI_FML_DisplayWindow_Current(void);
// Java: public long takeOverGlfwWindow()
long LIBMATTI_FML_DisplayWindow_TakeOverGlfwWindow(LIBMATTI_FML_DisplayWindow *window);
// Java: public void renderToFramebuffer() (no-op until the renderer port exists)
void LIBMATTI_FML_DisplayWindow_RenderToFramebuffer(LIBMATTI_FML_DisplayWindow *window);
// Java: public int getFramebufferTextureId()
int LIBMATTI_FML_DisplayWindow_GetFramebufferTextureId(LIBMATTI_FML_DisplayWindow *window);
// Java: public void close()
void LIBMATTI_FML_DisplayWindow_Close(LIBMATTI_FML_DisplayWindow *window);

#endif //MATTICRAFT_FML_EARLYDISPLAY_DISPLAYWINDOW_H
