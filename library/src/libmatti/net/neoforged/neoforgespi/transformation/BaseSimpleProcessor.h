// Port of net.neoforged.neoforgespi.transformation.BaseSimpleProcessor (package-private in Java).
// The Java class's final methods are helpers that the concrete simple processors wire into their
// ClassProcessor vtable slots.

#ifndef MATTICRAFT_NEOFORGESPI_TRANSFORMATION_BASESIMPLEPROCESSOR_H
#define MATTICRAFT_NEOFORGESPI_TRANSFORMATION_BASESIMPLEPROCESSOR_H

#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessor.h"

// Java: @Override public final void link(LinkContext context) { ClassProcessor.super.link(context); }
void LIBMATTI_NEOFORGESPI_BaseSimpleProcessor_Link(const LIBMATTI_NEOFORGESPI_ClassProcessor_LinkContext *context);
// Java: @Override public final void afterProcessing(AfterProcessingContext context) { ClassProcessor.super.afterProcessing(context); }
void LIBMATTI_NEOFORGESPI_BaseSimpleProcessor_AfterProcessing(
    const LIBMATTI_NEOFORGESPI_ClassProcessor_AfterProcessingContext *context);
// Java: @Override public Set<ProcessorName> runsAfter() { return Set.of(SIMPLE_PROCESSORS_GROUP, COMPUTING_FRAMES); }
const LIBMATTI_NEOFORGESPI_ProcessorName **LIBMATTI_NEOFORGESPI_BaseSimpleProcessor_RunsAfter(size_t *count);

#endif //MATTICRAFT_NEOFORGESPI_TRANSFORMATION_BASESIMPLEPROCESSOR_H
