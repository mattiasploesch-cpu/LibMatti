// Port of net.neoforged.neoforgespi.transformation.SimpleFieldProcessor.

#include "libmatti/net/neoforged/neoforgespi/transformation/SimpleFieldProcessor.h"

#include "libmatti/org/objectweb/asm/Type.h"

#include <stdlib.h>
#include <string.h>

// Java: targetsByClass.updateAndGet(map -> map != null ? map : targets().stream().collect(groupingBy(...)))
static void initialise_targets(LIBMATTI_NEOFORGESPI_SimpleFieldProcessor *self)
{
    if (self->targetsInitialised) return;

    size_t count = 0;
    LIBMATTI_NEOFORGESPI_SimpleFieldProcessor_Target *targets = self->getTargets(self, &count);
    self->targetsByClass = calloc(count > 0 ? count : 1, sizeof(*self->targetsByClass));
    self->targetCount = count;
    for (size_t i = 0; i < count; i++)
    {
        self->targetsByClass[i].className = strdup(targets[i].className);
        self->targetsByClass[i].fieldName = strdup(targets[i].fieldName);
        free(targets[i].className);
        free(targets[i].fieldName);
    }
    free(targets);
    self->targetsInitialised = 1;
}

static int targets_by_class_contains(LIBMATTI_NEOFORGESPI_SimpleFieldProcessor *self, const char *className)
{
    for (size_t i = 0; i < self->targetCount; i++)
    {
        if (strcmp(self->targetsByClass[i].className, className) == 0) return 1;
    }
    return 0;
}

int LIBMATTI_NEOFORGESPI_SimpleFieldProcessor_HandlesClass(
    LIBMATTI_NEOFORGESPI_SimpleFieldProcessor *self,
    const LIBMATTI_NEOFORGESPI_ClassProcessor_SelectionContext *context)
{
    // Java: return targetsByClass().containsKey(context.type().getClassName())
    initialise_targets(self);

    char *className = LIBMATTI_ASM_Type_GetClassName(context->type);
    int contains = targets_by_class_contains(self, className);
    free(className);
    return contains;
}

LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags LIBMATTI_NEOFORGESPI_SimpleFieldProcessor_ProcessClass(
    LIBMATTI_NEOFORGESPI_SimpleFieldProcessor *self,
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context)
{
    // Java: var targetFields = this.targetsByClass().get(context.type().getClassName()); if (targetFields == null) return NO_REWRITE
    char *className = LIBMATTI_ASM_Type_GetClassName(context->type);
    int hasTargetClass = targets_by_class_contains(self, className);
    free(className);
    if (!hasTargetClass) return LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_NO_REWRITE;

    // Java: for (var field : context.node().fields) if (targetFields.contains(field.name)) { transform(field, context); transformed = true; }
    char *nodeClassName = LIBMATTI_ASM_Type_GetClassName(context->type);
    LIBMATTI_ASMT_ClassNode *node = LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_Node(context);
    int transformed = 0;
    for (size_t i = 0; i < node->fieldCount; i++)
    {
        LIBMATTI_ASMT_FieldNode *field = node->fields[i];
        int targeted = 0;
        for (size_t t = 0; t < self->targetCount; t++)
        {
            if (strcmp(self->targetsByClass[t].className, nodeClassName) != 0) continue;
            if (strcmp(self->targetsByClass[t].fieldName, field->name) != 0) continue;
            targeted = 1;
            break;
        }
        if (!targeted) continue;
        self->transform(self, field, &context->simple);
        transformed = 1;
    }
    free(nodeClassName);

    // Java: return transformed ? ComputeFlags.COMPUTE_FRAMES : ComputeFlags.NO_REWRITE
    return transformed ? LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_COMPUTE_FRAMES
                       : LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_NO_REWRITE;
}
