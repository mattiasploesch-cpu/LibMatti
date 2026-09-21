#include "libmatti/net/neoforged/neoforgespi/locating/IOrderedProvider.h"

// Java: public default int getPriority()
int LIBMATTI_NEOFORGESPI_IOrderedProvider_GetPriority(const LIBMATTI_NEOFORGESPI_IOrderedProvider *provider)
{
    return provider->getPriority(provider->self);
}
