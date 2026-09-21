// Port of net.neoforged.fml.common.asm.AccessTransformerService.

#include "libmatti/net/neoforged/fml/common/asm/AccessTransformerService.h"

#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessorIds.h"

#include <stddef.h>
#include <stdlib.h>

// Java: public ProcessorName name() { return ClassProcessorIds.ACCESS_TRANSFORMERS; }
static LIBMATTI_NEOFORGESPI_ProcessorName *name(LIBMATTI_NEOFORGESPI_ClassProcessor *self)
{
    (void) self;
    return (LIBMATTI_NEOFORGESPI_ProcessorName *) LIBMATTI_NEOFORGESPI_ClassProcessorIds_ACCESS_TRANSFORMERS();
}

// Java: public Set<ProcessorName> runsBefore() { return Set.of(ClassProcessorIds.MIXIN); }
static const LIBMATTI_NEOFORGESPI_ProcessorName **runs_before(LIBMATTI_NEOFORGESPI_ClassProcessor *self,
                                                             size_t *count)
{
    static const LIBMATTI_NEOFORGESPI_ProcessorName *names[1];
    (void) self;
    names[0] = LIBMATTI_NEOFORGESPI_ClassProcessorIds_MIXIN();
    *count = 1;
    return names;
}

// Java: public ComputeFlags processClass(TransformationContext context) { return engine.transform(context.node(), context.type()) ? SIMPLE_REWRITE : NO_REWRITE; }
static LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags process_class(
    LIBMATTI_NEOFORGESPI_ClassProcessor *self,
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context)
{
    LIBMATTI_FML_AccessTransformerService *service = (LIBMATTI_FML_AccessTransformerService *) self;
    // Java: context.type().getInternalName()
    char *internalName = LIBMATTI_ASM_Type_GetInternalName(context->type);
    int transformed = LIBMATTI_AT_AccessTransformerEngine_Transform(service->engine, context->node, internalName);
    free(internalName);
    return transformed ? LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_SIMPLE_REWRITE
                       : LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_NO_REWRITE;
}

// Java: public boolean handlesClass(SelectionContext context) { return !context.empty() && engine.getTargets().contains(context.type()); }
static int handles_class(LIBMATTI_NEOFORGESPI_ClassProcessor *self,
                         const LIBMATTI_NEOFORGESPI_ClassProcessor_SelectionContext *context)
{
    LIBMATTI_FML_AccessTransformerService *service = (LIBMATTI_FML_AccessTransformerService *) self;
    if (context->empty)
        return 0;
    char *internalName = LIBMATTI_ASM_Type_GetInternalName(context->type);
    int contains = LIBMATTI_AT_AccessTransformerEngine_ContainsClassTarget(service->engine, internalName);
    free(internalName);
    return contains;
}

LIBMATTI_FML_AccessTransformerService *LIBMATTI_FML_AccessTransformerService_New(
    LIBMATTI_AT_AccessTransformerEngine *engine)
{
    LIBMATTI_FML_AccessTransformerService *service = calloc(1, sizeof(LIBMATTI_FML_AccessTransformerService));
    service->processor.name = name;
    service->processor.runsBefore = runs_before;
    service->processor.processClass = process_class;
    service->processor.handlesClass = handles_class;
    service->engine = engine;
    return service;
}

void LIBMATTI_FML_AccessTransformerService_Free(LIBMATTI_FML_AccessTransformerService *service)
{
    free(service);
}
