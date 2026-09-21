// Port of net.neoforged.fml.IBindingsProvider.

#ifndef MATTICRAFT_FML_IBINDINGSPROVIDER_H
#define MATTICRAFT_FML_IBINDINGSPROVIDER_H

#include "libmatti/net/neoforged/bus/api/IEventBus.h"

// Java: @ApiStatus.Internal public interface IBindingsProvider
typedef struct LIBMATTI_FML_IBindingsProvider LIBMATTI_FML_IBindingsProvider;

struct LIBMATTI_FML_IBindingsProvider
{
    void *self;

    // Java: IEventBus getGameBus()
    LIBMATTI_BUS_IEventBus *(*getGameBus)(void *self);
};

// Java: IEventBus getGameBus()
LIBMATTI_BUS_IEventBus *LIBMATTI_FML_IBindingsProvider_GetGameBus(const LIBMATTI_FML_IBindingsProvider *provider);

#endif //MATTICRAFT_FML_IBINDINGSPROVIDER_H
