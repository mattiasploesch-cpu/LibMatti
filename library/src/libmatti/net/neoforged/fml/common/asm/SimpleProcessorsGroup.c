// Port of net.neoforged.fml.common.asm.SimpleProcessorsGroup.

#include "libmatti/net/neoforged/fml/common/asm/SimpleProcessorsGroup.h"

#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessorIds.h"

#include <stddef.h>
#include <stdlib.h>

// Java: public ProcessorName name() { return ClassProcessorIds.SIMPLE_PROCESSORS_GROUP; }
static LIBMATTI_NEOFORGESPI_ProcessorName *name(LIBMATTI_NEOFORGESPI_ClassProcessor *self)
{
    (void) self;
    return (LIBMATTI_NEOFORGESPI_ProcessorName *) LIBMATTI_NEOFORGESPI_ClassProcessorIds_SIMPLE_PROCESSORS_GROUP();
}

// Java: public Set<ProcessorName> runsAfter() { return Set.of(ClassProcessorIds.COMPUTING_FRAMES, ClassProcessorIds.MIXIN); }
static const LIBMATTI_NEOFORGESPI_ProcessorName **runs_after(LIBMATTI_NEOFORGESPI_ClassProcessor *self, size_t *count)
{
    static const LIBMATTI_NEOFORGESPI_ProcessorName *names[2];
    (void) self;
    names[0] = LIBMATTI_NEOFORGESPI_ClassProcessorIds_COMPUTING_FRAMES();
    names[1] = LIBMATTI_NEOFORGESPI_ClassProcessorIds_MIXIN();
    *count = 2;
    return names;
}

// Java: public boolean handlesClass(SelectionContext context) { return false; }
static int handles_class(LIBMATTI_NEOFORGESPI_ClassProcessor *self,
                         const LIBMATTI_NEOFORGESPI_ClassProcessor_SelectionContext *context)
{
    (void) self;
    (void) context;
    return 0;
}

// Java: public ComputeFlags processClass(TransformationContext context) { return ComputeFlags.NO_REWRITE; }
static LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags process_class(
    LIBMATTI_NEOFORGESPI_ClassProcessor *self,
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context)
{
    (void) self;
    (void) context;
    return LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_NO_REWRITE;
}

LIBMATTI_FML_SimpleProcessorsGroup *LIBMATTI_FML_SimpleProcessorsGroup_New(void)
{
    LIBMATTI_FML_SimpleProcessorsGroup *group = calloc(1, sizeof(LIBMATTI_FML_SimpleProcessorsGroup));
    group->processor.name = name;
    group->processor.runsAfter = runs_after;
    group->processor.handlesClass = handles_class;
    group->processor.processClass = process_class;
    return group;
}

void LIBMATTI_FML_SimpleProcessorsGroup_Free(LIBMATTI_FML_SimpleProcessorsGroup *group)
{
    free(group);
}
