// Port of net.neoforged.fml.ModLoadingContext.
// Java: private static final ThreadLocal<ModLoadingContext> context =
//         ThreadLocal.withInitial(ModLoadingContext::new) - the C port keeps one context per thread.

#ifndef MATTICRAFT_FML_MODLOADINGCONTEXT_H
#define MATTICRAFT_FML_MODLOADINGCONTEXT_H

#include "libmatti/java/util/function/Supplier.h"
#include "libmatti/net/neoforged/fml/IExtensionPoint.h"
#include "libmatti/net/neoforged/fml/ModContainer.h"

// Java: public class ModLoadingContext
typedef struct LIBMATTI_FML_ModLoadingContext LIBMATTI_FML_ModLoadingContext;

struct LIBMATTI_FML_ModLoadingContext
{
    // Java: private ModContainer activeContainer
    LIBMATTI_FML_ModContainer *activeContainer;
};

// Java: public static ModLoadingContext get()
LIBMATTI_FML_ModLoadingContext *LIBMATTI_FML_ModLoadingContext_Get(void);

// Java: public void setActiveContainer(ModContainer container)
void LIBMATTI_FML_ModLoadingContext_SetActiveContainer(LIBMATTI_FML_ModLoadingContext *context,
                                                       LIBMATTI_FML_ModContainer *container);
// Java: public ModContainer getActiveContainer() - NULL when the fallback lookup fails
LIBMATTI_FML_ModContainer *LIBMATTI_FML_ModLoadingContext_GetActiveContainer(LIBMATTI_FML_ModLoadingContext *context);
// Java: public String getActiveNamespace()
const char *LIBMATTI_FML_ModLoadingContext_GetActiveNamespace(const LIBMATTI_FML_ModLoadingContext *context);

// Java: public <T extends IExtensionPoint> void registerExtensionPoint(Class<T> point, Supplier<T> extension)
void LIBMATTI_FML_ModLoadingContext_RegisterExtensionPoint(LIBMATTI_FML_ModLoadingContext *context,
                                                          LIBMATTI_FML_IExtensionPoint point,
                                                          LIBMATTI_JU_Supplier extension);

#endif //MATTICRAFT_FML_MODLOADINGCONTEXT_H
