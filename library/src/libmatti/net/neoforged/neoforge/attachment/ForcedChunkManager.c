// Port of net.neoforged.neoforge.attachment.ForcedChunkManager.
// The TicketController objects and the chunk system belong to the game port; the port keeps the
// init lifecycle and the controller id registry (the duplicate check of the Java callback).

#include "libmatti/net/neoforged/neoforge/attachment/ForcedChunkManager.h"

#include "libmatti/cpw/modlauncher/LogManager.h"

#include <stdlib.h>
#include <string.h>

// Java: private static boolean initialised / Map<Identifier, TicketController> controllers
static int initialised = 0;

static char **controllerIds = NULL;
static size_t controllerCount = 0;
static size_t controllerCapacity = 0;

// Java: the RegisterTicketControllersEvent callback - "Attempted to register two controllers
// with the same ID ..." on duplicates
void LIBMATTI_NEOFORGE_ForcedChunkManager_RegisterController(const char *controllerId)
{
    for (size_t i = 0; i < controllerCount; i++)
    {
        if (strcmp(controllerIds[i], controllerId) == 0)
        {
            LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), NULL,
                                     "IllegalArgumentException: Attempted to register two controllers with the same ID {}",
                                     controllerId);
            return;
        }
    }

    if (controllerCount == controllerCapacity)
    {
        controllerCapacity = controllerCapacity > 0 ? controllerCapacity * 2 : 8;
        controllerIds = realloc(controllerIds, sizeof(char *) * controllerCapacity);
    }
    controllerIds[controllerCount++] = strdup(controllerId);
}

// Java: public static synchronized void init()
void LIBMATTI_NEOFORGE_ForcedChunkManager_Init(void)
{
    // Java: if (initialised) throw new UnsupportedOperationException("Cannot init ticket controllers multiple times!")
    if (initialised)
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), NULL,
                                 "UnsupportedOperationException: Cannot init ticket controllers multiple times!");
        return;
    }
    initialised = 1;

    // Java: ModLoader.postEvent(new RegisterTicketControllersEvent(...)) - the controllers the
    // mods register flow through LIBMATTI_NEOFORGE_ForcedChunkManager_RegisterController; the
    // event object itself lives with the game port (it carries the chunk-system types)
}
