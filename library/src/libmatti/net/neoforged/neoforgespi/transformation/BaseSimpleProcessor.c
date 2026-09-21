// Port of net.neoforged.neoforgespi.transformation.BaseSimpleProcessor.

#include "libmatti/net/neoforged/neoforgespi/transformation/BaseSimpleProcessor.h"

#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessorIds.h"

void LIBMATTI_NEOFORGESPI_BaseSimpleProcessor_Link(const LIBMATTI_NEOFORGESPI_ClassProcessor_LinkContext *context)
{
    // Java: ClassProcessor.super.link(context) - the ClassProcessor default is empty
    (void) context;
}

void LIBMATTI_NEOFORGESPI_BaseSimpleProcessor_AfterProcessing(
    const LIBMATTI_NEOFORGESPI_ClassProcessor_AfterProcessingContext *context)
{
    // Java: ClassProcessor.super.afterProcessing(context) - the ClassProcessor default is empty
    (void) context;
}

const LIBMATTI_NEOFORGESPI_ProcessorName **LIBMATTI_NEOFORGESPI_BaseSimpleProcessor_RunsAfter(size_t *count)
{
    static const LIBMATTI_NEOFORGESPI_ProcessorName *names[2];
    names[0] = LIBMATTI_NEOFORGESPI_ClassProcessorIds_SIMPLE_PROCESSORS_GROUP();
    names[1] = LIBMATTI_NEOFORGESPI_ClassProcessorIds_COMPUTING_FRAMES();
    *count = 2;
    return names;
}
