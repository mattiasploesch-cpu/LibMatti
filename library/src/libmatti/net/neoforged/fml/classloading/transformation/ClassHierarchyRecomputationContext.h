// Port of net.neoforged.fml.classloading.transformation.ClassHierarchyRecomputationContext.
// Every class name passed to these methods is in dot-separated form.

#ifndef MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_CLASSHIERARCHYRECOMPUTATIONCONTEXT_H
#define MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_CLASSHIERARCHYRECOMPUTATIONCONTEXT_H

#include <stddef.h>

// Java: public interface ClassHierarchyRecomputationContext
typedef struct LIBMATTI_FML_ClassHierarchyRecomputationContext
{
    // Java: @Nullable Class<?> findLoadedClass(String className); NULL when not loaded
    void *(*findLoadedClass)(struct LIBMATTI_FML_ClassHierarchyRecomputationContext *self, const char *className);
    // Java: byte[] upToFrames(String className) throws ClassNotFoundException; caller frees, NULL = not found
    unsigned char *(*upToFrames)(struct LIBMATTI_FML_ClassHierarchyRecomputationContext *self, const char *className,
                                 size_t *length);
    // Java: Class<?> locateParentClass(String className) throws ClassNotFoundException; NULL = not found
    void *(*locateParentClass)(struct LIBMATTI_FML_ClassHierarchyRecomputationContext *self, const char *className);
} LIBMATTI_FML_ClassHierarchyRecomputationContext;

// Java: the interface methods, dispatched to the implementing vtable
void *LIBMATTI_FML_ClassHierarchyRecomputationContext_FindLoadedClass(
    LIBMATTI_FML_ClassHierarchyRecomputationContext *context, const char *className);
unsigned char *LIBMATTI_FML_ClassHierarchyRecomputationContext_UpToFrames(
    LIBMATTI_FML_ClassHierarchyRecomputationContext *context, const char *className, size_t *length);
void *LIBMATTI_FML_ClassHierarchyRecomputationContext_LocateParentClass(
    LIBMATTI_FML_ClassHierarchyRecomputationContext *context, const char *className);

#endif //MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_CLASSHIERARCHYRECOMPUTATIONCONTEXT_H
