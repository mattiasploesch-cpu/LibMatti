// Port of net.neoforged.fml.classloading.transformation.ClassProcessorAuditSource.

#include "libmatti/net/neoforged/fml/classloading/transformation/ClassProcessorAuditSource.h"

char *LIBMATTI_FML_ClassProcessorAuditSource_GetAuditString(LIBMATTI_FML_ClassProcessorAuditSource *source,
                                                           const char *clazz)
{
    return source->getAuditString(source, clazz);
}
