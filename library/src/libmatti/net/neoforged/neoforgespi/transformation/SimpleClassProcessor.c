// Port of net.neoforged.neoforgespi.transformation.SimpleClassProcessor.

#include "libmatti/net/neoforged/neoforgespi/transformation/SimpleClassProcessor.h"

#include "libmatti/org/objectweb/asm/Type.h"

#include <stdlib.h>
#include <string.h>

// Java: targets.updateAndGet(set -> set != null ? set : targets().stream().map(Target::className).collect(toSet()))
static void initialise_targets(LIBMATTI_NEOFORGESPI_SimpleClassProcessor *self)
{
    if (self->targetsInitialised) return;

    size_t count = 0;
    LIBMATTI_NEOFORGESPI_SimpleClassProcessor_Target *targets = self->getTargets(self, &count);
    self->targets = malloc(sizeof(*self->targets) * (count > 0 ? count : 1));
    self->targetCount = count;
    for (size_t i = 0; i < count; i++) self->targets[i] = strdup(targets[i].className);
    for (size_t i = 0; i < count; i++) free(targets[i].className);
    free(targets);
    self->targetsInitialised = 1;
}

int LIBMATTI_NEOFORGESPI_SimpleClassProcessor_HandlesClass(
    LIBMATTI_NEOFORGESPI_SimpleClassProcessor *self,
    const LIBMATTI_NEOFORGESPI_ClassProcessor_SelectionContext *context)
{
    // Java: return targets.contains(context.type().getClassName())
    initialise_targets(self);

    char *className = LIBMATTI_ASM_Type_GetClassName(context->type);
    int contains = 0;
    for (size_t i = 0; i < self->targetCount; i++)
    {
        if (strcmp(self->targets[i], className) != 0) continue;
        contains = 1;
        break;
    }
    free(className);
    return contains;
}

LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags LIBMATTI_NEOFORGESPI_SimpleClassProcessor_ProcessClass(
    LIBMATTI_NEOFORGESPI_SimpleClassProcessor *self,
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context)
{
    // Java: transform(context.node(), context); return ComputeFlags.COMPUTE_FRAMES
    self->transform(self, LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_Node(context),
                    &context->simple);
    return LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_COMPUTE_FRAMES;
}
