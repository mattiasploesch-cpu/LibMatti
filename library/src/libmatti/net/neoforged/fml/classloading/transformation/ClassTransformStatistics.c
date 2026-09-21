// Port of net.neoforged.fml.classloading.transformation.ClassTransformStatistics.

#include "libmatti/net/neoforged/fml/classloading/transformation/ClassTransformStatistics.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/classloading/transformation/ClassProcessorSet.h"
#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessorIds.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final Logger LOGGER = LogManager.getLogger()
static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: Map<ProcessorName, Integer> TRANSFORMS_BY_PROCESSOR / POTENTIAL_BY_PROCESSOR
typedef struct
{
    const LIBMATTI_NEOFORGESPI_ProcessorName *name;
    int count;
} ProcessorCounter;

static ProcessorCounter *transformsByProcessor = NULL;
static size_t transformsCount = 0;
static ProcessorCounter *potentialByProcessor = NULL;
static size_t potentialCount = 0;

// Java: AtomicInteger LOADED_CLASS_COUNT / TRANSFORMED_CLASS_COUNT / MIXIN_PARSED_CLASS_COUNT
static int loadedClassCount = 0;
static int transformedClassCount = 0;
static int mixinParsedClassCount = 0;

// Java: map.compute(name, (k, v) -> v == null ? 1 : v + 1)
static void increment_counter(ProcessorCounter **counters, size_t *count,
                              const LIBMATTI_NEOFORGESPI_ProcessorName *name)
{
    for (size_t i = 0; i < *count; i++)
    {
        if (LIBMATTI_NEOFORGESPI_ProcessorName_CompareTo((*counters)[i].name, name) != 0) continue;
        (*counters)[i].count++;
        return;
    }
    *counters = realloc(*counters, sizeof(**counters) * (*count + 1));
    (*counters)[*count].name = name;
    (*counters)[*count].count = 1;
    (*count)++;
}

static int counter_for(ProcessorCounter *counters, size_t count, const LIBMATTI_NEOFORGESPI_ProcessorName *name)
{
    for (size_t i = 0; i < count; i++)
    {
        if (LIBMATTI_NEOFORGESPI_ProcessorName_CompareTo(counters[i].name, name) == 0) return counters[i].count;
    }
    return 0;
}

void LIBMATTI_FML_ClassTransformStatistics_IncrementMixinParsedClasses(void)
{
    mixinParsedClassCount++;
}

void LIBMATTI_FML_ClassTransformStatistics_IncrementAskedForTransform(LIBMATTI_NEOFORGESPI_ClassProcessor *processor)
{
    // Java: if (!processor.name().equals(ClassProcessorIds.COMPUTING_FRAMES)) POTENTIAL_BY_PROCESSOR.compute(...)
    LIBMATTI_NEOFORGESPI_ProcessorName *name = LIBMATTI_NEOFORGESPI_ClassProcessor_Name(processor);
    if (LIBMATTI_NEOFORGESPI_ProcessorName_CompareTo(
            name, LIBMATTI_NEOFORGESPI_ClassProcessorIds_COMPUTING_FRAMES()) != 0)
        increment_counter(&potentialByProcessor, &potentialCount, name);
}

void LIBMATTI_FML_ClassTransformStatistics_IncrementTransforms(LIBMATTI_NEOFORGESPI_ClassProcessor *processor)
{
    LIBMATTI_NEOFORGESPI_ProcessorName *name = LIBMATTI_NEOFORGESPI_ClassProcessor_Name(processor);
    if (LIBMATTI_NEOFORGESPI_ProcessorName_CompareTo(
            name, LIBMATTI_NEOFORGESPI_ClassProcessorIds_COMPUTING_FRAMES()) != 0)
        increment_counter(&transformsByProcessor, &transformsCount, name);
}

void LIBMATTI_FML_ClassTransformStatistics_IncrementLoadedClasses(void)
{
    loadedClassCount++;
}

void LIBMATTI_FML_ClassTransformStatistics_IncrementTransformedClasses(void)
{
    transformedClassCount++;
}

char *LIBMATTI_FML_ClassTransformStatistics_GetTransformationSummary(void)
{
    // Java: String.format("%s/%s (%.2f%%)", transformed, loaded, ratio)
    double ratio = loadedClassCount == 0 ? 0.0 : ((double) transformedClassCount) / loadedClassCount * 100;
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%d/%d (%.2f%%)", transformedClassCount, loadedClassCount, ratio);
    return strdup(buffer);
}

char *LIBMATTI_FML_ClassTransformStatistics_GetMixinParsedClassesSummary(void)
{
    // Java: return String.valueOf(MIXIN_PARSED_CLASS_COUNT.get())
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d", mixinParsedClassCount);
    return strdup(buffer);
}

void LIBMATTI_FML_ClassTransformStatistics_LogTransformationSummary(void)
{
    char *summary = LIBMATTI_FML_ClassTransformStatistics_GetTransformationSummary();
    char *mixinSummary = LIBMATTI_FML_ClassTransformStatistics_GetMixinParsedClassesSummary();
    LIBMATTI_ML_Logger_Debug(LOGGER(), NULL, "Transformed/total loaded classes: {} and {} parsed for mixin",
                             summary, mixinSummary);
    free(summary);
    free(mixinSummary);
}

void LIBMATTI_FML_ClassTransformStatistics_CheckTransformationBehavior(void)
{
    // Java: keys.forEach(name -> { ratio = actual/potential; if (ratio > 0.25) LOGGER.error(...) })
    for (size_t i = 0; i < transformsCount; i++)
    {
        const LIBMATTI_NEOFORGESPI_ProcessorName *name = transformsByProcessor[i].name;
        double ratio = ((double) transformsByProcessor[i].count) /
                       counter_for(potentialByProcessor, potentialCount, name);
        if (ratio <= 0.25) continue;

        char percent[16];
        snprintf(percent, sizeof(percent), "%.2f", ratio * 100);
        char *nameString = LIBMATTI_NEOFORGESPI_ProcessorName_ToString(name);
        if (LIBMATTI_NEOFORGESPI_ProcessorName_CompareTo(
                name, LIBMATTI_NEOFORGESPI_ClassProcessorIds_MIXIN()) == 0)
            LIBMATTI_ML_Logger_Error(LOGGER(), NULL,
                                     "Class processor {} transformed {}% of loaded class which is suspiciously high; "
                                     "this could be due to a mixin coprocessor attempting mass-ASM",
                                     nameString, percent);
        else
            LIBMATTI_ML_Logger_Error(LOGGER(), NULL,
                                     "Class processor {} transformed {}% of loaded class which is suspiciously high; "
                                     "it may be attempting mass-ASM. Please report this to the mod author.",
                                     nameString, percent);
        free(nameString);
    }
}

char *LIBMATTI_FML_ClassTransformStatistics_ComputeCrashReportEntry(const LIBMATTI_FML_ClassProcessorSet *set)
{
    // Java: entries.stream().map(e -> String.format("%05.2f%%: %s", e.ratio, e.name))
    //           .collect(Collectors.joining("\n\t\t", "\n\t\t", ""))
    size_t count = 0;
    LIBMATTI_NEOFORGESPI_ClassProcessor **transforms = LIBMATTI_FML_ClassProcessorSet_GetSortedProcessors(set, &count);

    size_t capacity = 64;
    char *result = malloc(capacity);
    result[0] = '\0';
    size_t length = 0;

    for (size_t i = 0; i < count; i++)
    {
        LIBMATTI_NEOFORGESPI_ProcessorName *name = LIBMATTI_NEOFORGESPI_ClassProcessor_Name(transforms[i]);
        int actual = counter_for(transformsByProcessor, transformsCount, name);
        int potential = counter_for(potentialByProcessor, potentialCount, name);
        double ratio = (actual == 0 || potential == 0) ? 0.0 : 100.0 * ((double) actual) / potential;

        // Java: classProcessorSet.isMarker(transform) ? name + " (marker)" : name.toString()
        char *nameString = LIBMATTI_NEOFORGESPI_ProcessorName_ToString(name);
        char *entry = NULL;
        if (LIBMATTI_FML_ClassProcessorSet_IsMarker(set, transforms[i]))
        {
            size_t entryLength = strlen(nameString) + 64;
            entry = malloc(entryLength);
            snprintf(entry, entryLength, "%05.2f%%: %s (marker)", ratio, nameString);
        }
        else
        {
            size_t entryLength = strlen(nameString) + 64;
            entry = malloc(entryLength);
            snprintf(entry, entryLength, "%05.2f%%: %s", ratio, nameString);
        }
        free(nameString);

        size_t entryLength = strlen(entry);
        while (length + entryLength + 4 > capacity)
        {
            capacity *= 2;
            result = realloc(result, capacity);
        }
        memcpy(result + length, "\n\t\t", 3);
        length += 3;
        memcpy(result + length, entry, entryLength);
        length += entryLength;
        result[length] = '\0';
        free(entry);
    }

    free(transforms);
    return result;
}
