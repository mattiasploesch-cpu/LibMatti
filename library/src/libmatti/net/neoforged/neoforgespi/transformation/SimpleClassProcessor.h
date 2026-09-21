// Port of net.neoforged.neoforgespi.transformation.SimpleClassProcessor.
// A concrete C processor embeds this struct as its first member and wires the ClassProcessor
// vtable slots to the helpers below.

#ifndef MATTICRAFT_NEOFORGESPI_TRANSFORMATION_SIMPLECLASSPROCESSOR_H
#define MATTICRAFT_NEOFORGESPI_TRANSFORMATION_SIMPLECLASSPROCESSOR_H

#include "libmatti/net/neoforged/neoforgespi/transformation/BaseSimpleProcessor.h"

#include <stddef.h>

// Java: public record Target(String className)
typedef struct LIBMATTI_NEOFORGESPI_SimpleClassProcessor_Target
{
    char *className;
} LIBMATTI_NEOFORGESPI_SimpleClassProcessor_Target;

// Java: abstract non-sealed class SimpleClassProcessor extends BaseSimpleProcessor
typedef struct LIBMATTI_NEOFORGESPI_SimpleClassProcessor
{
    // Java: private final AtomicReference<Set<String>> targets
    char **targets;
    size_t targetCount;
    // Java: the AtomicReference being non-null
    int targetsInitialised;
    // Java: public abstract Set<Target> targets(); caller frees the array
    LIBMATTI_NEOFORGESPI_SimpleClassProcessor_Target *(*getTargets)(
        struct LIBMATTI_NEOFORGESPI_SimpleClassProcessor *self, size_t *count);
    // Java: public abstract void transform(ClassNode input, SimpleTransformationContext context)
    void (*transform)(struct LIBMATTI_NEOFORGESPI_SimpleClassProcessor *self, LIBMATTI_ASMT_ClassNode *input,
                      LIBMATTI_NEOFORGESPI_SimpleTransformationContext *context);
} LIBMATTI_NEOFORGESPI_SimpleClassProcessor;

// Java: @Override public final boolean handlesClass(SelectionContext context)
int LIBMATTI_NEOFORGESPI_SimpleClassProcessor_HandlesClass(
    LIBMATTI_NEOFORGESPI_SimpleClassProcessor *self,
    const LIBMATTI_NEOFORGESPI_ClassProcessor_SelectionContext *context);
// Java: @Override public final ComputeFlags processClass(TransformationContext context)
LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags LIBMATTI_NEOFORGESPI_SimpleClassProcessor_ProcessClass(
    LIBMATTI_NEOFORGESPI_SimpleClassProcessor *self,
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context);

#endif //MATTICRAFT_NEOFORGESPI_TRANSFORMATION_SIMPLECLASSPROCESSOR_H
