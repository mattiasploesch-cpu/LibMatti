// Port of net.neoforged.neoforgespi.transformation.ClassProcessorProvider.

#ifndef MATTICRAFT_NEOFORGESPI_TRANSFORMATION_CLASSPROCESSORPROVIDER_H
#define MATTICRAFT_NEOFORGESPI_TRANSFORMATION_CLASSPROCESSORPROVIDER_H

#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessor.h"

// Java: record Context() - the record has no components
typedef struct LIBMATTI_NEOFORGESPI_ClassProcessorProvider_Context
{
    // Java: the record has no components
    char empty;
} LIBMATTI_NEOFORGESPI_ClassProcessorProvider_Context;

// Java: @ApiStatus.NonExtendable interface Collector { void add(ClassProcessor processor); }
// A single-method interface; the C port passes the receiver as a function pointer plus userdata.
typedef void (*LIBMATTI_NEOFORGESPI_ClassProcessorProvider_Collector)(
    LIBMATTI_NEOFORGESPI_ClassProcessor *processor, void *userdata);

// Java: public interface ClassProcessorProvider
typedef struct LIBMATTI_NEOFORGESPI_ClassProcessorProvider
{
    // Java: void createProcessors(Context context, Collector collector)
    void (*createProcessors)(struct LIBMATTI_NEOFORGESPI_ClassProcessorProvider *self,
                             const LIBMATTI_NEOFORGESPI_ClassProcessorProvider_Context *context,
                             LIBMATTI_NEOFORGESPI_ClassProcessorProvider_Collector collector, void *collectorUserdata);
} LIBMATTI_NEOFORGESPI_ClassProcessorProvider;

// Java: the interface method, dispatched to the implementing vtable
void LIBMATTI_NEOFORGESPI_ClassProcessorProvider_CreateProcessors(
    LIBMATTI_NEOFORGESPI_ClassProcessorProvider *provider,
    const LIBMATTI_NEOFORGESPI_ClassProcessorProvider_Context *context,
    LIBMATTI_NEOFORGESPI_ClassProcessorProvider_Collector collector, void *collectorUserdata);

#endif //MATTICRAFT_NEOFORGESPI_TRANSFORMATION_CLASSPROCESSORPROVIDER_H
