// Port of net.neoforged.neoforgespi.transformation.SimpleFieldProcessor.
// A concrete C processor embeds this struct as its first member and wires the ClassProcessor
// vtable slots to the helpers below.

#ifndef MATTICRAFT_NEOFORGESPI_TRANSFORMATION_SIMPLEFIELDPROCESSOR_H
#define MATTICRAFT_NEOFORGESPI_TRANSFORMATION_SIMPLEFIELDPROCESSOR_H

#include "libmatti/net/neoforged/neoforgespi/transformation/BaseSimpleProcessor.h"
#include "libmatti/org/objectweb/asm/tree/FieldNode.h"

#include <stddef.h>

// Java: public record Target(String className, String fieldName)
typedef struct LIBMATTI_NEOFORGESPI_SimpleFieldProcessor_Target
{
    char *className;
    char *fieldName;
} LIBMATTI_NEOFORGESPI_SimpleFieldProcessor_Target;

// Java: abstract non-sealed class SimpleFieldProcessor extends BaseSimpleProcessor
typedef struct LIBMATTI_NEOFORGESPI_SimpleFieldProcessor
{
    // Java: private final AtomicReference<Map<String, Set<String>>> targetsByClass - the flat
    // (className, fieldName) pairs stand in for the map
    LIBMATTI_NEOFORGESPI_SimpleFieldProcessor_Target *targetsByClass;
    size_t targetCount;
    // Java: the AtomicReference being non-null
    int targetsInitialised;
    // Java: public abstract Set<Target> targets(); caller frees the array
    LIBMATTI_NEOFORGESPI_SimpleFieldProcessor_Target *(*getTargets)(
        struct LIBMATTI_NEOFORGESPI_SimpleFieldProcessor *self, size_t *count);
    // Java: public abstract void transform(FieldNode input, SimpleTransformationContext context)
    void (*transform)(struct LIBMATTI_NEOFORGESPI_SimpleFieldProcessor *self, LIBMATTI_ASMT_FieldNode *input,
                      LIBMATTI_NEOFORGESPI_SimpleTransformationContext *context);
} LIBMATTI_NEOFORGESPI_SimpleFieldProcessor;

// Java: @Override public final boolean handlesClass(SelectionContext context)
int LIBMATTI_NEOFORGESPI_SimpleFieldProcessor_HandlesClass(
    LIBMATTI_NEOFORGESPI_SimpleFieldProcessor *self,
    const LIBMATTI_NEOFORGESPI_ClassProcessor_SelectionContext *context);
// Java: @Override public final ComputeFlags processClass(TransformationContext context)
LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags LIBMATTI_NEOFORGESPI_SimpleFieldProcessor_ProcessClass(
    LIBMATTI_NEOFORGESPI_SimpleFieldProcessor *self,
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context);

#endif //MATTICRAFT_NEOFORGESPI_TRANSFORMATION_SIMPLEFIELDPROCESSOR_H
