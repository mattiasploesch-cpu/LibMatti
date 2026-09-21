// Port of net.neoforged.neoforgespi.transformation.ClassProcessorProvider.

#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessorProvider.h"

void LIBMATTI_NEOFORGESPI_ClassProcessorProvider_CreateProcessors(
    LIBMATTI_NEOFORGESPI_ClassProcessorProvider *provider,
    const LIBMATTI_NEOFORGESPI_ClassProcessorProvider_Context *context,
    LIBMATTI_NEOFORGESPI_ClassProcessorProvider_Collector collector, void *collectorUserdata)
{
    provider->createProcessors(provider, context, collector, collectorUserdata);
}
