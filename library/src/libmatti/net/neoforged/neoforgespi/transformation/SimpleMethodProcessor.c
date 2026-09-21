// Port of net.neoforged.neoforgespi.transformation.SimpleMethodProcessor.

#include "libmatti/net/neoforged/neoforgespi/transformation/SimpleMethodProcessor.h"

#include "libmatti/org/objectweb/asm/Type.h"

#include <stdlib.h>
#include <string.h>

// Java: targetsByClass.updateAndGet(map -> map != null ? map : targets().stream().collect(groupingBy(...)))
static void initialise_targets(LIBMATTI_NEOFORGESPI_SimpleMethodProcessor *self)
{
    if (self->targetsInitialised) return;

    size_t count = 0;
    LIBMATTI_NEOFORGESPI_SimpleMethodProcessor_Target *targets = self->getTargets(self, &count);
    self->targetsByClass = calloc(count > 0 ? count : 1, sizeof(*self->targetsByClass));
    self->targetCount = count;
    for (size_t i = 0; i < count; i++)
    {
        self->targetsByClass[i].className = strdup(targets[i].className);
        self->targetsByClass[i].methodName = strdup(targets[i].methodName);
        self->targetsByClass[i].methodDescriptor = strdup(targets[i].methodDescriptor);
        free(targets[i].className);
        free(targets[i].methodName);
        free(targets[i].methodDescriptor);
    }
    free(targets);
    self->targetsInitialised = 1;
}

static int targets_by_class_contains(LIBMATTI_NEOFORGESPI_SimpleMethodProcessor *self, const char *className)
{
    for (size_t i = 0; i < self->targetCount; i++)
    {
        if (strcmp(self->targetsByClass[i].className, className) == 0) return 1;
    }
    return 0;
}

int LIBMATTI_NEOFORGESPI_SimpleMethodProcessor_HandlesClass(
    LIBMATTI_NEOFORGESPI_SimpleMethodProcessor *self,
    const LIBMATTI_NEOFORGESPI_ClassProcessor_SelectionContext *context)
{
    // Java: return targetsByClass().containsKey(context.type().getClassName())
    initialise_targets(self);

    char *className = LIBMATTI_ASM_Type_GetClassName(context->type);
    int contains = targets_by_class_contains(self, className);
    free(className);
    return contains;
}

LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags LIBMATTI_NEOFORGESPI_SimpleMethodProcessor_ProcessClass(
    LIBMATTI_NEOFORGESPI_SimpleMethodProcessor *self,
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context)
{
    // Java: var targetMethods = this.targetsByClass().get(context.type().getClassName()); if (targetMethods == null) return NO_REWRITE
    char *className = LIBMATTI_ASM_Type_GetClassName(context->type);
    int hasTargetClass = targets_by_class_contains(self, className);
    free(className);
    if (!hasTargetClass) return LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_NO_REWRITE;

    // Java: for (var method : context.node().methods) if (targetMethods.contains(method.name + method.desc)) { transform(method, context); transformed = true; }
    char *nodeClassName = LIBMATTI_ASM_Type_GetClassName(context->type);
    LIBMATTI_ASMT_ClassNode *node = LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_Node(context);
    int transformed = 0;
    for (size_t i = 0; i < node->methodCount; i++)
    {
        LIBMATTI_ASMT_MethodNode *method = node->methods[i];
        int targeted = 0;
        for (size_t t = 0; t < self->targetCount; t++)
        {
            if (strcmp(self->targetsByClass[t].className, nodeClassName) != 0) continue;
            if (strcmp(self->targetsByClass[t].methodName, method->name) != 0) continue;
            if (strcmp(self->targetsByClass[t].methodDescriptor, method->desc) != 0) continue;
            targeted = 1;
            break;
        }
        if (!targeted) continue;
        self->transform(self, method, &context->simple);
        transformed = 1;
    }
    free(nodeClassName);

    // Java: return transformed ? ComputeFlags.COMPUTE_FRAMES : ComputeFlags.NO_REWRITE
    return transformed ? LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_COMPUTE_FRAMES
                       : LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_NO_REWRITE;
}
