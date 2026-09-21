// Port of net.neoforged.fml.classloading.transformation.ClassProcessorAuditLog.
// Java's ConcurrentHashMap maps to plain arrays here: the ported launcher transforms on one thread.

#ifndef MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_CLASSPROCESSORAUDITLOG_H
#define MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_CLASSPROCESSORAUDITLOG_H

#include "libmatti/net/neoforged/fml/classloading/transformation/ClassProcessorAuditSource.h"
#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessor.h"

#include <stddef.h>

// Java: static final class TransformerActivity implements BiConsumer<String, String[]>
typedef struct LIBMATTI_FML_TransformerActivity
{
    LIBMATTI_NEOFORGESPI_ProcessorName *processorName;
    // Java: private final List<String> activities
    char **activities;
    size_t activityCount;
    // Java: private boolean include = false
    int include;
} LIBMATTI_FML_TransformerActivity;

// Java: public void accept(String activity, String... context)
void LIBMATTI_FML_TransformerActivity_Accept(LIBMATTI_FML_TransformerActivity *activity, const char *name,
                                             const char *const *context, size_t contextCount);
// Java: void rewrites() { include = true; }
void LIBMATTI_FML_TransformerActivity_Rewrites(LIBMATTI_FML_TransformerActivity *activity);

// Java: public class ClassProcessorAuditLog implements ClassProcessorAuditSource
typedef struct LIBMATTI_FML_ClassProcessorAuditLog
{
    // Java: implements ClassProcessorAuditSource
    LIBMATTI_FML_ClassProcessorAuditSource source;

    // Java: Map<String, List<TransformerActivity>> audit
    char **classes;
    LIBMATTI_FML_TransformerActivity ***activities;
    size_t *activityCounts;
    size_t count;
} LIBMATTI_FML_ClassProcessorAuditLog;

// Java: public ClassProcessorAuditLog()
LIBMATTI_FML_ClassProcessorAuditLog *LIBMATTI_FML_ClassProcessorAuditLog_New(void);

// Java: public void clear()
void LIBMATTI_FML_ClassProcessorAuditLog_Clear(LIBMATTI_FML_ClassProcessorAuditLog *log);

// Java: TransformerActivity forClassProcessor(String clazz, ClassProcessor classProcessor)
LIBMATTI_FML_TransformerActivity *LIBMATTI_FML_ClassProcessorAuditLog_ForClassProcessor(
    LIBMATTI_FML_ClassProcessorAuditLog *log, const char *clazz,
    LIBMATTI_NEOFORGESPI_ClassProcessor *classProcessor);
// Java: public String getAuditString(String clazz); caller frees
char *LIBMATTI_FML_ClassProcessorAuditLog_GetAuditString(LIBMATTI_FML_ClassProcessorAuditLog *log, const char *clazz);

#endif //MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_CLASSPROCESSORAUDITLOG_H
