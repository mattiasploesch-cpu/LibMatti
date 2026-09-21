// Port of net.neoforged.fml.classloading.transformation.ClassTransformer.

#ifndef MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_CLASSTRANSFORMER_H
#define MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_CLASSTRANSFORMER_H

#include "libmatti/net/neoforged/fml/classloading/transformation/ClassHierarchyRecomputationContext.h"
#include "libmatti/net/neoforged/fml/classloading/transformation/ClassProcessorAuditLog.h"
#include "libmatti/net/neoforged/fml/classloading/transformation/ClassProcessorSet.h"

#include <stddef.h>

// Java: public class ClassTransformer
typedef struct LIBMATTI_FML_ClassTransformer
{
    LIBMATTI_FML_ClassProcessorSet *processors;
    LIBMATTI_FML_ClassProcessorAuditLog *auditTrail;
} LIBMATTI_FML_ClassTransformer;

// Java: public ClassTransformer(ClassProcessorSet processors, ClassProcessorAuditLog auditTrail)
LIBMATTI_FML_ClassTransformer *LIBMATTI_FML_ClassTransformer_New(LIBMATTI_FML_ClassProcessorSet *processors,
                                                               LIBMATTI_FML_ClassProcessorAuditLog *auditTrail);
void LIBMATTI_FML_ClassTransformer_Free(LIBMATTI_FML_ClassTransformer *transformer);

// Java: public byte[] transform(byte[] inputClass, String className, ProcessorName upToTransformer, ClassHierarchyRecomputationContext locator)
unsigned char *LIBMATTI_FML_ClassTransformer_Transform(LIBMATTI_FML_ClassTransformer *transformer,
                                                       const unsigned char *inputClass, size_t inputLength,
                                                       const char *className,
                                                       const LIBMATTI_NEOFORGESPI_ProcessorName *upToTransformer,
                                                       LIBMATTI_FML_ClassHierarchyRecomputationContext *locator,
                                                       size_t *outLength);

#endif //MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_CLASSTRANSFORMER_H
