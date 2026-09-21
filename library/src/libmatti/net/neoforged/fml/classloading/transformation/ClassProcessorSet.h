// Port of net.neoforged.fml.classloading.transformation.ClassProcessorSet.

#ifndef MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_CLASSPROCESSORSET_H
#define MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_CLASSPROCESSORSET_H

#include "libmatti/net/neoforged/neoforgespi/transformation/BytecodeProvider.h"
#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessor.h"
#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessorProvider.h"
#include "libmatti/org/objectweb/asm/Type.h"

#include <stddef.h>

// Java: public final class ClassProcessorSet
typedef struct LIBMATTI_FML_ClassProcessorSet
{
    // Java: private final List<ClassProcessor> sortedProcessors
    LIBMATTI_NEOFORGESPI_ClassProcessor **sortedProcessors;
    size_t sortedCount;
    // Java: private final Set<ProcessorName> markerProcessors
    const LIBMATTI_NEOFORGESPI_ProcessorName **markerProcessors;
    size_t markerCount;
    // Java: private final Set<String> generatedPackages
    const char **generatedPackages;
    size_t generatedPackageCount;
    // Java: private final SequencedMap<ProcessorName, ClassProcessor> processors
    const LIBMATTI_NEOFORGESPI_ProcessorName **processorNames;
    LIBMATTI_NEOFORGESPI_ClassProcessor **processors;
    size_t processorCount;
    // Java: private final Set<ProcessorName> allowedToRecomputeFrames
    const LIBMATTI_NEOFORGESPI_ProcessorName **allowedToRecomputeFrames;
    size_t allowedToRecomputeFramesCount;
    // Java: private boolean linked
    int linked;
    // Java: the BytecodeProvider lambdas created by link(bytecodeProviderLookup) are handed to the
    //       processors, which keep them for the lifetime of the set.
    LIBMATTI_NEOFORGESPI_BytecodeProvider **bytecodeProviders;
    size_t bytecodeProviderCount;
} LIBMATTI_FML_ClassProcessorSet;

// Java: private ClassProcessorSet(List<ClassProcessor>, Set<ProcessorName>, Set<String>, Set<ProcessorName>)
LIBMATTI_FML_ClassProcessorSet *LIBMATTI_FML_ClassProcessorSet_New(
    LIBMATTI_NEOFORGESPI_ClassProcessor **sortedProcessors, size_t sortedCount,
    const LIBMATTI_NEOFORGESPI_ProcessorName **markers, size_t markerCount,
    const char **generatedPackages, size_t generatedPackageCount,
    const LIBMATTI_NEOFORGESPI_ProcessorName **allowedToRecomputeFrames, size_t allowedToRecomputeFramesCount);
void LIBMATTI_FML_ClassProcessorSet_Free(LIBMATTI_FML_ClassProcessorSet *set);

// Java: public static ClassProcessorSet of(ClassProcessor... processors)
LIBMATTI_FML_ClassProcessorSet *LIBMATTI_FML_ClassProcessorSet_Of(LIBMATTI_NEOFORGESPI_ClassProcessor **processors,
                                                                 size_t processorCount);

// Java: boolean canRecomputeFrames(ProcessorName name)
int LIBMATTI_FML_ClassProcessorSet_CanRecomputeFrames(const LIBMATTI_FML_ClassProcessorSet *set,
                                                     const LIBMATTI_NEOFORGESPI_ProcessorName *name);
// Java: public boolean isMarker(ClassProcessor processor)
int LIBMATTI_FML_ClassProcessorSet_IsMarker(const LIBMATTI_FML_ClassProcessorSet *set,
                                           LIBMATTI_NEOFORGESPI_ClassProcessor *processor);
// Java: public Set<String> getGeneratedPackages()
const char **LIBMATTI_FML_ClassProcessorSet_GetGeneratedPackages(const LIBMATTI_FML_ClassProcessorSet *set,
                                                               size_t *count);
// Java: List<ClassProcessor> getSortedProcessors()
LIBMATTI_NEOFORGESPI_ClassProcessor **LIBMATTI_FML_ClassProcessorSet_GetSortedProcessors(
    const LIBMATTI_FML_ClassProcessorSet *set, size_t *count);
// Java: public List<ClassProcessor> transformersFor(Type classDesc, boolean isEmpty, ProcessorName upToTransformer)
LIBMATTI_NEOFORGESPI_ClassProcessor **LIBMATTI_FML_ClassProcessorSet_TransformersFor(
    const LIBMATTI_FML_ClassProcessorSet *set, const LIBMATTI_ASM_Type *classDesc, int isEmpty,
    const LIBMATTI_NEOFORGESPI_ProcessorName *upToTransformer, size_t *count);
// Java: public void link(Function<ProcessorName, BytecodeProvider> bytecodeProviderLookup)
// Java throws IllegalStateException when already linked; the C port reports that as 0.
int LIBMATTI_FML_ClassProcessorSet_Link(LIBMATTI_FML_ClassProcessorSet *set,
                                        LIBMATTI_NEOFORGESPI_BytecodeProvider *(*bytecodeProviderLookup)(
                                            const LIBMATTI_NEOFORGESPI_ProcessorName *name, void *userdata),
                                        void *userdata);

// Java: public static Builder builder()
typedef struct LIBMATTI_FML_ClassProcessorSet_Builder LIBMATTI_FML_ClassProcessorSet_Builder;
LIBMATTI_FML_ClassProcessorSet_Builder *LIBMATTI_FML_ClassProcessorSet_Builder_New(void);
void LIBMATTI_FML_ClassProcessorSet_Builder_Free(LIBMATTI_FML_ClassProcessorSet_Builder *builder);

// Java: public Builder markMarker(ProcessorName name)
void LIBMATTI_FML_ClassProcessorSet_Builder_MarkMarker(LIBMATTI_FML_ClassProcessorSet_Builder *builder,
                                                      const LIBMATTI_NEOFORGESPI_ProcessorName *name);
// Java: public Builder addProcessor(ClassProcessor toAdd)
void LIBMATTI_FML_ClassProcessorSet_Builder_AddProcessor(LIBMATTI_FML_ClassProcessorSet_Builder *builder,
                                                        LIBMATTI_NEOFORGESPI_ClassProcessor *toAdd);
// Java: public Builder addProcessors(Collection<ClassProcessor> toAdd)
void LIBMATTI_FML_ClassProcessorSet_Builder_AddProcessors(LIBMATTI_FML_ClassProcessorSet_Builder *builder,
                                                         LIBMATTI_NEOFORGESPI_ClassProcessor **toAdd, size_t count);
// Java: public Builder addProcessorProviders(Collection<ClassProcessorProvider> providers)
void LIBMATTI_FML_ClassProcessorSet_Builder_AddProcessorProviders(
    LIBMATTI_FML_ClassProcessorSet_Builder *builder, LIBMATTI_NEOFORGESPI_ClassProcessorProvider **providers,
    size_t count);
// Java: public ClassProcessorSet build(); NULL when Java would throw IllegalStateException
LIBMATTI_FML_ClassProcessorSet *LIBMATTI_FML_ClassProcessorSet_Builder_Build(
    LIBMATTI_FML_ClassProcessorSet_Builder *builder);

#endif //MATTICRAFT_FML_CLASSLOADING_TRANSFORMATION_CLASSPROCESSORSET_H
