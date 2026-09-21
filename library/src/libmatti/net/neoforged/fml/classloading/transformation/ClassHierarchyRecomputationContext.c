// Port of net.neoforged.fml.classloading.transformation.ClassHierarchyRecomputationContext.

#include "libmatti/net/neoforged/fml/classloading/transformation/ClassHierarchyRecomputationContext.h"

void *LIBMATTI_FML_ClassHierarchyRecomputationContext_FindLoadedClass(
    LIBMATTI_FML_ClassHierarchyRecomputationContext *context, const char *className)
{
    return context->findLoadedClass(context, className);
}

unsigned char *LIBMATTI_FML_ClassHierarchyRecomputationContext_UpToFrames(
    LIBMATTI_FML_ClassHierarchyRecomputationContext *context, const char *className, size_t *length)
{
    return context->upToFrames(context, className, length);
}

void *LIBMATTI_FML_ClassHierarchyRecomputationContext_LocateParentClass(
    LIBMATTI_FML_ClassHierarchyRecomputationContext *context, const char *className)
{
    return context->locateParentClass(context, className);
}
