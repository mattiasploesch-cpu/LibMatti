// Port of net.neoforged.neoforgespi.transformation.SimpleMethodProcessor.
// A concrete C processor embeds this struct as its first member and wires the ClassProcessor
// vtable slots to the helpers below.

#ifndef MATTICRAFT_NEOFORGESPI_TRANSFORMATION_SIMPLEMETHODPROCESSOR_H
#define MATTICRAFT_NEOFORGESPI_TRANSFORMATION_SIMPLEMETHODPROCESSOR_H

#include "libmatti/net/neoforged/neoforgespi/transformation/BaseSimpleProcessor.h"
#include "libmatti/org/objectweb/asm/tree/MethodNode.h"

#include <stddef.h>

// Java: public record Target(String className, String methodName, String methodDescriptor)
typedef struct LIBMATTI_NEOFORGESPI_SimpleMethodProcessor_Target
{
    char *className;
    char *methodName;
    char *methodDescriptor;
} LIBMATTI_NEOFORGESPI_SimpleMethodProcessor_Target;

// Java: abstract non-sealed class SimpleMethodProcessor extends BaseSimpleProcessor
typedef struct LIBMATTI_NEOFORGESPI_SimpleMethodProcessor
{
    // Java: private final AtomicReference<Map<String, Set<String>>> targetsByClass - the flat
    // (className, methodName+descriptor) pairs stand in for the map
    LIBMATTI_NEOFORGESPI_SimpleMethodProcessor_Target *targetsByClass;
    size_t targetCount;
    // Java: the AtomicReference being non-null
    int targetsInitialised;
    // Java: public abstract Set<Target> targets(); caller frees the array
    LIBMATTI_NEOFORGESPI_SimpleMethodProcessor_Target *(*getTargets)(
        struct LIBMATTI_NEOFORGESPI_SimpleMethodProcessor *self, size_t *count);
    // Java: public abstract void transform(MethodNode input, SimpleTransformationContext context)
    void (*transform)(struct LIBMATTI_NEOFORGESPI_SimpleMethodProcessor *self, LIBMATTI_ASMT_MethodNode *input,
                      LIBMATTI_NEOFORGESPI_SimpleTransformationContext *context);
} LIBMATTI_NEOFORGESPI_SimpleMethodProcessor;

// Java: @Override public final boolean handlesClass(SelectionContext context)
int LIBMATTI_NEOFORGESPI_SimpleMethodProcessor_HandlesClass(
    LIBMATTI_NEOFORGESPI_SimpleMethodProcessor *self,
    const LIBMATTI_NEOFORGESPI_ClassProcessor_SelectionContext *context);
// Java: @Override public final ComputeFlags processClass(TransformationContext context)
LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags LIBMATTI_NEOFORGESPI_SimpleMethodProcessor_ProcessClass(
    LIBMATTI_NEOFORGESPI_SimpleMethodProcessor *self,
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context);

#endif //MATTICRAFT_NEOFORGESPI_TRANSFORMATION_SIMPLEMETHODPROCESSOR_H
