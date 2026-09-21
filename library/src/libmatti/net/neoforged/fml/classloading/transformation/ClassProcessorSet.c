// Port of net.neoforged.fml.classloading.transformation.ClassProcessorSet.

#include "libmatti/net/neoforged/fml/classloading/transformation/ClassProcessorSet.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/CrashReportCallables.h"
#include "libmatti/net/neoforged/fml/classloading/transformation/ClassTransformStatistics.h"
#include "libmatti/net/neoforged/fml/loading/toposort/Graph.h"
#include "libmatti/net/neoforged/fml/loading/toposort/TopologicalSort.h"
#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessorIds.h"
#include "libmatti/org/objectweb/asm/tree/ClassNode.h"

#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

static int processor_name_equals(const LIBMATTI_NEOFORGESPI_ProcessorName *a,
                                 const LIBMATTI_NEOFORGESPI_ProcessorName *b)
{
    return LIBMATTI_NEOFORGESPI_ProcessorName_CompareTo(a, b) == 0;
}

// Java: CrashReportCallables.registerCrashCallable("Class Processors", () -> ClassTransformStatistics.computeCrashReportEntry(this))
static char *compute_crash_report_entry(void *userdata)
{
    return LIBMATTI_FML_ClassTransformStatistics_ComputeCrashReportEntry(
        (const LIBMATTI_FML_ClassProcessorSet *) userdata);
}

LIBMATTI_FML_ClassProcessorSet *LIBMATTI_FML_ClassProcessorSet_New(
    LIBMATTI_NEOFORGESPI_ClassProcessor **sortedProcessors, size_t sortedCount,
    const LIBMATTI_NEOFORGESPI_ProcessorName **markers, size_t markerCount,
    const char **generatedPackages, size_t generatedPackageCount,
    const LIBMATTI_NEOFORGESPI_ProcessorName **allowedToRecomputeFrames, size_t allowedToRecomputeFramesCount)
{
    LIBMATTI_FML_ClassProcessorSet *set = calloc(1, sizeof(LIBMATTI_FML_ClassProcessorSet));

    set->sortedProcessors = malloc(sizeof(*set->sortedProcessors) * (sortedCount > 0 ? sortedCount : 1));
    memcpy(set->sortedProcessors, sortedProcessors, sizeof(*set->sortedProcessors) * sortedCount);
    set->sortedCount = sortedCount;

    set->markerProcessors = malloc(sizeof(*set->markerProcessors) * (markerCount > 0 ? markerCount : 1));
    memcpy(set->markerProcessors, markers, sizeof(*set->markerProcessors) * markerCount);
    set->markerCount = markerCount;

    set->generatedPackages = malloc(sizeof(*set->generatedPackages) * (generatedPackageCount > 0 ? generatedPackageCount : 1));
    memcpy(set->generatedPackages, generatedPackages, sizeof(*set->generatedPackages) * generatedPackageCount);
    set->generatedPackageCount = generatedPackageCount;

    // Java: var processors = LinkedHashMap.newLinkedHashMap(...); for (var processor : sortedProcessors) processors.put(processor.name(), processor)
    set->processorNames = malloc(sizeof(*set->processorNames) * (sortedCount > 0 ? sortedCount : 1));
    set->processors = malloc(sizeof(*set->processors) * (sortedCount > 0 ? sortedCount : 1));
    set->processorCount = sortedCount;
    for (size_t i = 0; i < sortedCount; i++)
    {
        set->processorNames[i] = LIBMATTI_NEOFORGESPI_ClassProcessor_Name(sortedProcessors[i]);
        set->processors[i] = sortedProcessors[i];
    }

    set->allowedToRecomputeFrames =
        malloc(sizeof(*set->allowedToRecomputeFrames) * (allowedToRecomputeFramesCount > 0 ? allowedToRecomputeFramesCount : 1));
    memcpy(set->allowedToRecomputeFrames, allowedToRecomputeFrames,
           sizeof(*set->allowedToRecomputeFrames) * allowedToRecomputeFramesCount);
    set->allowedToRecomputeFramesCount = allowedToRecomputeFramesCount;

    // Java: CrashReportCallables.registerCrashCallable(...)
    LIBMATTI_FML_CrashReportCallables_RegisterCrashCallableSupplier("Class Processors", compute_crash_report_entry,
                                                                   set);
    return set;
}

void LIBMATTI_FML_ClassProcessorSet_Free(LIBMATTI_FML_ClassProcessorSet *set)
{
    if (set == NULL) return;
    free(set->sortedProcessors);
    free(set->markerProcessors);
    free(set->generatedPackages);
    free(set->processorNames);
    free(set->processors);
    free(set->allowedToRecomputeFrames);
    free(set->bytecodeProviders);
    free(set);
}

LIBMATTI_FML_ClassProcessorSet *LIBMATTI_FML_ClassProcessorSet_Of(LIBMATTI_NEOFORGESPI_ClassProcessor **processors,
                                                                 size_t processorCount)
{
    // Java: return ClassProcessorSet.builder().addProcessors(Arrays.asList(processors)).build()
    LIBMATTI_FML_ClassProcessorSet_Builder *builder = LIBMATTI_FML_ClassProcessorSet_Builder_New();
    LIBMATTI_FML_ClassProcessorSet_Builder_AddProcessors(builder, processors, processorCount);
    LIBMATTI_FML_ClassProcessorSet *set = LIBMATTI_FML_ClassProcessorSet_Builder_Build(builder);
    LIBMATTI_FML_ClassProcessorSet_Builder_Free(builder);
    return set;
}

int LIBMATTI_FML_ClassProcessorSet_CanRecomputeFrames(const LIBMATTI_FML_ClassProcessorSet *set,
                                                     const LIBMATTI_NEOFORGESPI_ProcessorName *name)
{
    // Java: return allowedToRecomputeFrames.contains(name)
    for (size_t i = 0; i < set->allowedToRecomputeFramesCount; i++)
    {
        if (processor_name_equals(set->allowedToRecomputeFrames[i], name)) return 1;
    }
    return 0;
}

int LIBMATTI_FML_ClassProcessorSet_IsMarker(const LIBMATTI_FML_ClassProcessorSet *set,
                                           LIBMATTI_NEOFORGESPI_ClassProcessor *processor)
{
    // Java: return markerProcessors.contains(processor.name())
    LIBMATTI_NEOFORGESPI_ProcessorName *name = LIBMATTI_NEOFORGESPI_ClassProcessor_Name(processor);
    for (size_t i = 0; i < set->markerCount; i++)
    {
        if (processor_name_equals(set->markerProcessors[i], name)) return 1;
    }
    return 0;
}

const char **LIBMATTI_FML_ClassProcessorSet_GetGeneratedPackages(const LIBMATTI_FML_ClassProcessorSet *set,
                                                               size_t *count)
{
    *count = set->generatedPackageCount;
    return set->generatedPackages;
}

LIBMATTI_NEOFORGESPI_ClassProcessor **LIBMATTI_FML_ClassProcessorSet_GetSortedProcessors(
    const LIBMATTI_FML_ClassProcessorSet *set, size_t *count)
{
    *count = set->sortedCount;
    return set->sortedProcessors;
}

LIBMATTI_NEOFORGESPI_ClassProcessor **LIBMATTI_FML_ClassProcessorSet_TransformersFor(
    const LIBMATTI_FML_ClassProcessorSet *set, const LIBMATTI_ASM_Type *classDesc, int isEmpty,
    const LIBMATTI_NEOFORGESPI_ProcessorName *upToTransformer, size_t *count)
{
    LIBMATTI_NEOFORGESPI_ClassProcessor **out = NULL;
    size_t outCount = 0;
    int includesComputingFrames = 0;

    for (size_t i = 0; i < set->sortedCount; i++)
    {
        LIBMATTI_NEOFORGESPI_ClassProcessor *transformer = set->sortedProcessors[i];
        LIBMATTI_NEOFORGESPI_ProcessorName *name = LIBMATTI_NEOFORGESPI_ClassProcessor_Name(transformer);

        if (upToTransformer != NULL && processor_name_equals(upToTransformer, name))
        {
            break;
        }
        if (processor_name_equals(LIBMATTI_NEOFORGESPI_ClassProcessorIds_COMPUTING_FRAMES(), name))
        {
            includesComputingFrames = 1;
            out = realloc(out, sizeof(*out) * (outCount + 1));
            out[outCount++] = transformer;
        }
        else
        {
            LIBMATTI_FML_ClassTransformStatistics_IncrementAskedForTransform(transformer);

            LIBMATTI_NEOFORGESPI_ClassProcessor_SelectionContext context = {classDesc, isEmpty};
            if (LIBMATTI_NEOFORGESPI_ClassProcessor_HandlesClass(transformer, &context))
            {
                LIBMATTI_FML_ClassTransformStatistics_IncrementTransforms(transformer);
                out = realloc(out, sizeof(*out) * (outCount + 1));
                out[outCount++] = transformer;
            }
        }
    }

    if (outCount == 1 && includesComputingFrames)
    {
        // Java: the class does not actually require any transformation, as the only transformer present is
        //       the special no-op marker for where class hierarchy computation in frame computation goes up to
        free(out);
        *count = 0;
        return NULL;
    }

    *count = outCount;
    return out;
}

int LIBMATTI_FML_ClassProcessorSet_Link(LIBMATTI_FML_ClassProcessorSet *set,
                                        LIBMATTI_NEOFORGESPI_BytecodeProvider *(*bytecodeProviderLookup)(
                                            const LIBMATTI_NEOFORGESPI_ProcessorName *name, void *userdata),
                                        void *userdata)
{
    // Java: if (linked) throw new IllegalStateException("This set of class processors is already linked.")
    if (set->linked)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "IllegalStateException: This set of class processors is already linked.");
        return 0;
    }
    set->linked = 1;

    for (size_t i = 0; i < set->sortedCount; i++)
    {
        LIBMATTI_NEOFORGESPI_ClassProcessor *processor = set->sortedProcessors[i];

        // Java: the provider lives for as long as the processor holds it; the port keeps it on the set too
        LIBMATTI_NEOFORGESPI_BytecodeProvider *provider =
            bytecodeProviderLookup(LIBMATTI_NEOFORGESPI_ClassProcessor_Name(processor), userdata);
        set->bytecodeProviders = realloc(set->bytecodeProviders,
                                         sizeof(*set->bytecodeProviders) * (set->bytecodeProviderCount + 1));
        set->bytecodeProviders[set->bytecodeProviderCount++] = provider;

        LIBMATTI_NEOFORGESPI_ClassProcessor_LinkContext context = {
            set->processorNames,
            set->processors,
            set->processorCount,
            provider,
        };
        LIBMATTI_NEOFORGESPI_ClassProcessor_Link(processor, &context);
    }
    return 1;
}

// Java: public static final class Builder
struct LIBMATTI_FML_ClassProcessorSet_Builder
{
    LIBMATTI_NEOFORGESPI_ClassProcessor **processors;
    size_t processorCount;
    const LIBMATTI_NEOFORGESPI_ProcessorName **markers;
    size_t markerCount;
};

LIBMATTI_FML_ClassProcessorSet_Builder *LIBMATTI_FML_ClassProcessorSet_Builder_New(void)
{
    return calloc(1, sizeof(LIBMATTI_FML_ClassProcessorSet_Builder));
}

void LIBMATTI_FML_ClassProcessorSet_Builder_Free(LIBMATTI_FML_ClassProcessorSet_Builder *builder)
{
    if (builder == NULL) return;
    free(builder->processors);
    free(builder->markers);
    free(builder);
}

void LIBMATTI_FML_ClassProcessorSet_Builder_MarkMarker(LIBMATTI_FML_ClassProcessorSet_Builder *builder,
                                                      const LIBMATTI_NEOFORGESPI_ProcessorName *name)
{
    builder->markers = realloc(builder->markers, sizeof(*builder->markers) * (builder->markerCount + 1));
    builder->markers[builder->markerCount++] = name;
}

void LIBMATTI_FML_ClassProcessorSet_Builder_AddProcessor(LIBMATTI_FML_ClassProcessorSet_Builder *builder,
                                                        LIBMATTI_NEOFORGESPI_ClassProcessor *toAdd)
{
    builder->processors = realloc(builder->processors, sizeof(*builder->processors) * (builder->processorCount + 1));
    builder->processors[builder->processorCount++] = toAdd;
}

void LIBMATTI_FML_ClassProcessorSet_Builder_AddProcessors(LIBMATTI_FML_ClassProcessorSet_Builder *builder,
                                                         LIBMATTI_NEOFORGESPI_ClassProcessor **toAdd, size_t count)
{
    builder->processors = realloc(builder->processors, sizeof(*builder->processors) * (builder->processorCount + count));
    memcpy(builder->processors + builder->processorCount, toAdd, sizeof(*toAdd) * count);
    builder->processorCount += count;
}

// Java: Builder.this.processors::add
static void collect_processor(LIBMATTI_NEOFORGESPI_ClassProcessor *processor, void *userdata)
{
    LIBMATTI_FML_ClassProcessorSet_Builder_AddProcessor((LIBMATTI_FML_ClassProcessorSet_Builder *) userdata, processor);
}

// Java: Builder.addProcessorProviders(Collection<ClassProcessorProvider> providers):
//       try { provider.createProcessors(context, Builder.this.processors::add); }
//       catch (Exception e) { var sourceFile = ServiceLoaderUtil.identifySourcePath(provider);
//           ModLoader.addLoadingIssue(error("fml.modloadingissue.coremod_error",
//               provider.getClass().getName(), sourceFile).withCause(e)); }
// The C port cannot catch a provider failure here: the ported providers cannot throw, so the
// try/catch is unreachable exactly like Java's when no provider fails.
void LIBMATTI_FML_ClassProcessorSet_Builder_AddProcessorProviders(
    LIBMATTI_FML_ClassProcessorSet_Builder *builder, LIBMATTI_NEOFORGESPI_ClassProcessorProvider **providers,
    size_t count)
{
    LIBMATTI_NEOFORGESPI_ClassProcessorProvider_Context context = {0};
    for (size_t i = 0; i < count; i++)
    {
        LIBMATTI_NEOFORGESPI_ClassProcessorProvider_CreateProcessors(providers[i], &context, collect_processor,
                                                                    builder);
    }
}

// Java: Comparator.comparing(ClassProcessor::orderingHint).thenComparing(ClassProcessor::name)
static int compare_processors(void *a, void *b, void *userdata)
{
    (void) userdata;
    LIBMATTI_NEOFORGESPI_ClassProcessor_OrderingHint hintA =
        LIBMATTI_NEOFORGESPI_ClassProcessor_GetOrderingHint((LIBMATTI_NEOFORGESPI_ClassProcessor *) a);
    LIBMATTI_NEOFORGESPI_ClassProcessor_OrderingHint hintB =
        LIBMATTI_NEOFORGESPI_ClassProcessor_GetOrderingHint((LIBMATTI_NEOFORGESPI_ClassProcessor *) b);
    if (hintA != hintB) return hintA < hintB ? -1 : 1;
    return LIBMATTI_NEOFORGESPI_ProcessorName_CompareTo(
        LIBMATTI_NEOFORGESPI_ClassProcessor_Name((LIBMATTI_NEOFORGESPI_ClassProcessor *) a),
        LIBMATTI_NEOFORGESPI_ClassProcessor_Name((LIBMATTI_NEOFORGESPI_ClassProcessor *) b));
}

// Java: private static ClassProcessor createSpecialComputeFramesNode()
static LIBMATTI_NEOFORGESPI_ProcessorName *special_compute_frames_name(LIBMATTI_NEOFORGESPI_ClassProcessor *self)
{
    (void) self;
    return (LIBMATTI_NEOFORGESPI_ProcessorName *) LIBMATTI_NEOFORGESPI_ClassProcessorIds_COMPUTING_FRAMES();
}

static const LIBMATTI_NEOFORGESPI_ProcessorName **special_compute_frames_runs_after(
    LIBMATTI_NEOFORGESPI_ClassProcessor *self, size_t *count)
{
    // Java: return Set.of()
    (void) self;
    *count = 0;
    return NULL;
}

static LIBMATTI_NEOFORGESPI_ClassProcessor_OrderingHint special_compute_frames_ordering_hint(
    LIBMATTI_NEOFORGESPI_ClassProcessor *self)
{
    // Java: return OrderingHint.EARLY
    (void) self;
    return LIBMATTI_NEOFORGESPI_ClassProcessor_OrderingHint_EARLY;
}

static int special_compute_frames_handles_class(
    LIBMATTI_NEOFORGESPI_ClassProcessor *self,
    const LIBMATTI_NEOFORGESPI_ClassProcessor_SelectionContext *context)
{
    // Java: return false
    (void) self;
    (void) context;
    return 0;
}

static LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags special_compute_frames_process_class(
    LIBMATTI_NEOFORGESPI_ClassProcessor *self,
    LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context)
{
    // Java: return ComputeFlags.NO_REWRITE
    (void) self;
    (void) context;
    return LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_NO_REWRITE;
}

static LIBMATTI_NEOFORGESPI_ClassProcessor *create_special_compute_frames_node(void)
{
    LIBMATTI_NEOFORGESPI_ClassProcessor *processor = calloc(1, sizeof(LIBMATTI_NEOFORGESPI_ClassProcessor));
    processor->name = special_compute_frames_name;
    processor->runsAfter = special_compute_frames_runs_after;
    processor->orderingHint = special_compute_frames_ordering_hint;
    processor->handlesClass = special_compute_frames_handles_class;
    processor->processClass = special_compute_frames_process_class;
    return processor;
}

// Java: private static List<ClassProcessor> sortProcessors(List<ClassProcessor> allProcessors, Set<ProcessorName> allowedToRecomputeFrames)
static LIBMATTI_NEOFORGESPI_ClassProcessor **sort_processors(
    LIBMATTI_FML_ClassProcessorSet_Builder *builder, const LIBMATTI_NEOFORGESPI_ProcessorName ***allowedToRecomputeFrames,
    size_t *allowedCount, size_t *sortedCount)
{
    // Java: var transformers = new HashMap<ProcessorName, ClassProcessor>()
    LIBMATTI_NEOFORGESPI_ClassProcessor **transformers = NULL;
    size_t transformerCount = 0;
    LIBMATTI_FML_Graph *graph = LIBMATTI_FML_Graph_New(1, 0);

    LIBMATTI_NEOFORGESPI_ClassProcessor *specialComputeFramesNode = create_special_compute_frames_node();
    LIBMATTI_FML_Graph_AddNode(graph, specialComputeFramesNode);
    transformers = realloc(transformers, sizeof(*transformers) * (transformerCount + 1));
    transformers[transformerCount++] = specialComputeFramesNode;

    for (size_t i = 0; i < builder->processorCount; i++)
    {
        LIBMATTI_NEOFORGESPI_ClassProcessor *transformer = builder->processors[i];
        LIBMATTI_NEOFORGESPI_ProcessorName *name = LIBMATTI_NEOFORGESPI_ClassProcessor_Name(transformer);

        // Java: if (transformers.containsKey(transformer.name())) { LOGGER.error("Duplicate transformers with name {}, of types {} and {}", ...); throw new IllegalStateException(...) }
        // TODO: Java also logs the two getClass().getName() values; C processors carry no runtime class name.
        for (size_t t = 0; t < transformerCount; t++)
        {
            if (!processor_name_equals(LIBMATTI_NEOFORGESPI_ClassProcessor_Name(transformers[t]), name)) continue;
            char *nameString = LIBMATTI_NEOFORGESPI_ProcessorName_ToString(name);
            LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Duplicate transformers with name {}", nameString);
            LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "IllegalStateException: Duplicate transformers with name: {}",
                                     nameString);
            free(nameString);
            LIBMATTI_FML_Graph_Free(graph);
            free(transformers);
            return NULL;
        }

        LIBMATTI_FML_Graph_AddNode(graph, transformer);
        transformers = realloc(transformers, sizeof(*transformers) * (transformerCount + 1));
        transformers[transformerCount++] = transformer;
    }

    for (size_t i = 0; i < transformerCount; i++)
    {
        LIBMATTI_NEOFORGESPI_ClassProcessor *self = transformers[i];

        // Java: for (var targetName : self.runsBefore()) { var target = transformers.get(targetName); if (target == self) continue; if (target != null) graph.putEdge(self, target); }
        size_t runsBeforeCount = 0;
        const LIBMATTI_NEOFORGESPI_ProcessorName **runsBefore =
            LIBMATTI_NEOFORGESPI_ClassProcessor_RunsBefore(self, &runsBeforeCount);
        for (size_t r = 0; r < runsBeforeCount; r++)
        {
            for (size_t t = 0; t < transformerCount; t++)
            {
                if (!processor_name_equals(
                        LIBMATTI_NEOFORGESPI_ClassProcessor_Name(transformers[t]), runsBefore[r]))
                    continue;
                if (transformers[t] == self) break;
                LIBMATTI_FML_Graph_PutEdge(graph, self, transformers[t]);
                break;
            }
        }

        // Java: for (var targetName : self.runsAfter()) { var target = transformers.get(targetName); if (target == self) continue; if (target != null) graph.putEdge(target, self); }
        size_t runsAfterCount = 0;
        const LIBMATTI_NEOFORGESPI_ProcessorName **runsAfter =
            LIBMATTI_NEOFORGESPI_ClassProcessor_RunsAfter(self, &runsAfterCount);
        for (size_t r = 0; r < runsAfterCount; r++)
        {
            for (size_t t = 0; t < transformerCount; t++)
            {
                if (!processor_name_equals(
                        LIBMATTI_NEOFORGESPI_ClassProcessor_Name(transformers[t]), runsAfter[r]))
                    continue;
                if (transformers[t] == self) break;
                LIBMATTI_FML_Graph_PutEdge(graph, transformers[t], self);
                break;
            }
        }
    }

    // Java: var sorted = TopologicalSort.topologicalSort(graph, Comparator.comparing(ClassProcessor::orderingHint).thenComparing(ClassProcessor::name))
    LIBMATTI_FML_CyclePresentException *cycles = NULL;
    size_t count = 0;
    void **sorted = LIBMATTI_FML_TopologicalSort_TopologicalSort(graph, compare_processors, NULL, &count, &cycles);

    // Java: for (var node : Graphs.reachableNodes(graph, specialComputeFramesNode)) allowedToRecomputeFrames.add(node.name())
    size_t reachableCount = 0;
    void **reachable = LIBMATTI_FML_Graph_ReachableNodes(graph, specialComputeFramesNode, &reachableCount);
    *allowedToRecomputeFrames =
        malloc(sizeof(**allowedToRecomputeFrames) * (reachableCount > 0 ? reachableCount : 1));
    *allowedCount = reachableCount;
    for (size_t i = 0; i < reachableCount; i++)
    {
        (*allowedToRecomputeFrames)[i] = LIBMATTI_NEOFORGESPI_ClassProcessor_Name(
            (LIBMATTI_NEOFORGESPI_ClassProcessor *) reachable[i]);
    }

    free(reachable);
    free(transformers);
    LIBMATTI_FML_Graph_Free(graph);
    free(cycles);

    *sortedCount = count;
    return (LIBMATTI_NEOFORGESPI_ClassProcessor **) sorted;
}

LIBMATTI_FML_ClassProcessorSet *LIBMATTI_FML_ClassProcessorSet_Builder_Build(
    LIBMATTI_FML_ClassProcessorSet_Builder *builder)
{
    const LIBMATTI_NEOFORGESPI_ProcessorName **allowedToRecomputeFrames = NULL;
    size_t allowedToRecomputeFramesCount = 0;
    size_t sortedCount = 0;
    LIBMATTI_NEOFORGESPI_ClassProcessor **sortedProcessors =
        sort_processors(builder, &allowedToRecomputeFrames, &allowedToRecomputeFramesCount, &sortedCount);
    if (sortedProcessors == NULL)
    {
        free(allowedToRecomputeFrames);
        return NULL;
    }

    // Java: var packageNames = new HashSet<String>(); for (var factory : processors) packageNames.addAll(factory.generatesPackages())
    const char **packageNames = NULL;
    size_t packageCount = 0;
    for (size_t i = 0; i < builder->processorCount; i++)
    {
        size_t count = 0;
        const char **generated = LIBMATTI_NEOFORGESPI_ClassProcessor_GeneratesPackages(builder->processors[i], &count);
        for (size_t p = 0; p < count; p++)
        {
            int seen = 0;
            for (size_t e = 0; e < packageCount; e++)
                if (strcmp(packageNames[e], generated[p]) == 0)
                {
                    seen = 1;
                    break;
                }
            if (seen) continue;
            packageNames = realloc(packageNames, sizeof(*packageNames) * (packageCount + 1));
            packageNames[packageCount++] = generated[p];
        }
    }

    LIBMATTI_FML_ClassProcessorSet *set = LIBMATTI_FML_ClassProcessorSet_New(
        sortedProcessors, sortedCount, builder->markers, builder->markerCount, packageNames, packageCount,
        allowedToRecomputeFrames, allowedToRecomputeFramesCount);

    free(sortedProcessors);
    free(allowedToRecomputeFrames);
    free(packageNames);
    return set;
}
