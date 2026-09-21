// Port of net.neoforged.fml.classloading.transformation.ClassProcessorAuditSource.

#ifndef MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_CLASSPROCESSORAUDITSOURCE_H
#define MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_CLASSPROCESSORAUDITSOURCE_H

// Java: public interface ClassProcessorAuditSource
typedef struct LIBMATTI_FML_ClassProcessorAuditSource
{
    // Java: String getAuditString(String clazz); caller frees
    char *(*getAuditString)(struct LIBMATTI_FML_ClassProcessorAuditSource *self, const char *clazz);
} LIBMATTI_FML_ClassProcessorAuditSource;

// Java: the interface method, dispatched to the implementing vtable
char *LIBMATTI_FML_ClassProcessorAuditSource_GetAuditString(LIBMATTI_FML_ClassProcessorAuditSource *source,
                                                           const char *clazz);

#endif //MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_CLASSPROCESSORAUDITSOURCE_H
