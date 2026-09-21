// Port of net.neoforged.fml.mclanguageprovider.MinecraftModContainer.

#include "libmatti/net/neoforged/fml/mclanguageprovider/MinecraftModContainer.h"

#include <stdlib.h>

// Java: public MinecraftModContainer(IModInfo info) - the event bus stays null (@Nullable getEventBus())
LIBMATTI_FML_MinecraftModContainer *LIBMATTI_FML_MinecraftModContainer_New(LIBMATTI_NEOFORGESPI_IModInfo *info)
{
    LIBMATTI_FML_MinecraftModContainer *container = calloc(1, sizeof(LIBMATTI_FML_MinecraftModContainer));
    LIBMATTI_FML_ModContainer_Init(&container->base, info);
    // Java: the subclass adds no fields ModContainer does not already release
    container->base.free = NULL;
    return container;
}
