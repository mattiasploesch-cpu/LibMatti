// Port of net.neoforged.fml.classloading.transformation.ClassTransformStatistics.
// Java's ConcurrentHashMap/atomics map to plain arrays and counters: the ported launcher
// transforms on one thread (the JVM-side map is per-processor bookkeeping).

#ifndef MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_CLASSTRANSFORMSTATISTICS_H
#define MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_CLASSTRANSFORMSTATISTICS_H

#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessor.h"

// Java: public final class ClassProcessorSet (defined in ClassProcessorSet.h)
typedef struct LIBMATTI_FML_ClassProcessorSet LIBMATTI_FML_ClassProcessorSet;

// Java: public static void incrementMixinParsedClasses()
void LIBMATTI_FML_ClassTransformStatistics_IncrementMixinParsedClasses(void);
// Java: static void incrementAskedForTransform(ClassProcessor processor)
void LIBMATTI_FML_ClassTransformStatistics_IncrementAskedForTransform(LIBMATTI_NEOFORGESPI_ClassProcessor *processor);
// Java: static void incrementTransforms(ClassProcessor processor)
void LIBMATTI_FML_ClassTransformStatistics_IncrementTransforms(LIBMATTI_NEOFORGESPI_ClassProcessor *processor);
// Java: static void incrementLoadedClasses()
void LIBMATTI_FML_ClassTransformStatistics_IncrementLoadedClasses(void);
// Java: static void incrementTransformedClasses()
void LIBMATTI_FML_ClassTransformStatistics_IncrementTransformedClasses(void);

// Java: public static synchronized String getTransformationSummary(); caller frees
char *LIBMATTI_FML_ClassTransformStatistics_GetTransformationSummary(void);
// Java: public static String getMixinParsedClassesSummary(); caller frees
char *LIBMATTI_FML_ClassTransformStatistics_GetMixinParsedClassesSummary(void);
// Java: public static synchronized void logTransformationSummary()
void LIBMATTI_FML_ClassTransformStatistics_LogTransformationSummary(void);
// Java: public synchronized static void checkTransformationBehavior()
void LIBMATTI_FML_ClassTransformStatistics_CheckTransformationBehavior(void);
// Java: public static synchronized String computeCrashReportEntry(ClassProcessorSet classProcessorSet); caller frees
char *LIBMATTI_FML_ClassTransformStatistics_ComputeCrashReportEntry(const LIBMATTI_FML_ClassProcessorSet *set);

#endif //MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_CLASSTRANSFORMSTATISTICS_H
