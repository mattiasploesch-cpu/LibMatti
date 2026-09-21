// Port of net.neoforged.fml.IBindingsProvider.

#include "libmatti/net/neoforged/fml/IBindingsProvider.h"

// Java: IEventBus getGameBus()
LIBMATTI_BUS_IEventBus *LIBMATTI_FML_IBindingsProvider_GetGameBus(const LIBMATTI_FML_IBindingsProvider *provider)
{
    return provider->getGameBus(provider->self);
}
