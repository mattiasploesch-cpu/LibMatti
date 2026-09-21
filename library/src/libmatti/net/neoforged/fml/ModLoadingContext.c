// Port of net.neoforged.fml.ModLoadingContext.

#include "libmatti/net/neoforged/fml/ModLoadingContext.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/ModList.h"

#include <stdlib.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: private static final ThreadLocal<ModLoadingContext> context = ThreadLocal.withInitial(ModLoadingContext::new);
static _Thread_local LIBMATTI_FML_ModLoadingContext *threadContext = NULL;

// Java: public static ModLoadingContext get()
LIBMATTI_FML_ModLoadingContext *LIBMATTI_FML_ModLoadingContext_Get(void)
{
    if (threadContext == NULL) threadContext = calloc(1, sizeof(LIBMATTI_FML_ModLoadingContext));
    return threadContext;
}

// Java: public void setActiveContainer(ModContainer container)
void LIBMATTI_FML_ModLoadingContext_SetActiveContainer(LIBMATTI_FML_ModLoadingContext *context,
                                                       LIBMATTI_FML_ModContainer *container)
{
    context->activeContainer = container;
}

// Java: activeContainer == null
//         ? ModList.get().getModContainerById("minecraft").orElseThrow(() -> new RuntimeException("Where is minecraft???!"))
//         : activeContainer
LIBMATTI_FML_ModContainer *LIBMATTI_FML_ModLoadingContext_GetActiveContainer(LIBMATTI_FML_ModLoadingContext *context)
{
    if (context->activeContainer != NULL) return context->activeContainer;

    LIBMATTI_FML_ModList *modList = LIBMATTI_FML_ModList_Get();
    LIBMATTI_FML_ModContainer *container =
        modList != NULL ? LIBMATTI_FML_ModList_GetModContainerById(modList, "minecraft") : NULL;

    if (container == NULL)
        // Java: orElseThrow(() -> new RuntimeException("Where is minecraft???!"))
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Where is minecraft?!");

    return container;
}

// Java: activeContainer == null ? "minecraft" : activeContainer.getNamespace()
const char *LIBMATTI_FML_ModLoadingContext_GetActiveNamespace(const LIBMATTI_FML_ModLoadingContext *context)
{
    return context->activeContainer == NULL ? "minecraft" : context->activeContainer->namespace;
}

// Java: public <T extends IExtensionPoint> void registerExtensionPoint(Class<T> point, Supplier<T> extension)
void LIBMATTI_FML_ModLoadingContext_RegisterExtensionPoint(LIBMATTI_FML_ModLoadingContext *context,
                                                          LIBMATTI_FML_IExtensionPoint point,
                                                          LIBMATTI_JU_Supplier extension)
{
    LIBMATTI_FML_ModContainer *container = LIBMATTI_FML_ModLoadingContext_GetActiveContainer(context);
    if (container == NULL) return;

    LIBMATTI_FML_ModContainer_RegisterExtensionPointSupplier(container, point, extension);
}
