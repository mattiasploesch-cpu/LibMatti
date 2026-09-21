// Port of net.neoforged.fml.classloading.transformation.ClassProcessorAuditLog.

#include "libmatti/net/neoforged/fml/classloading/transformation/ClassProcessorAuditLog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void LIBMATTI_FML_TransformerActivity_Accept(LIBMATTI_FML_TransformerActivity *activity, const char *name,
                                             const char *const *context, size_t contextCount)
{
    // Java: activities.add(activity + (context.length == 0 ? "" : ":" + String.join(":", context)))
    size_t length = strlen(name) + 1;
    if (contextCount > 0)
    {
        length++;
        for (size_t i = 0; i < contextCount; i++) length += strlen(context[i]) + 1;
    }

    char *entry = malloc(length);
    strcpy(entry, name);
    for (size_t i = 0; i < contextCount; i++)
    {
        strcat(entry, ":");
        strcat(entry, context[i]);
    }

    activity->activities = realloc(activity->activities, sizeof(*activity->activities) * (activity->activityCount + 1));
    activity->activities[activity->activityCount++] = entry;
}

void LIBMATTI_FML_TransformerActivity_Rewrites(LIBMATTI_FML_TransformerActivity *activity)
{
    activity->include = 1;
}

static char *get_audit_string(LIBMATTI_FML_ClassProcessorAuditSource *self, const char *clazz);

LIBMATTI_FML_ClassProcessorAuditLog *LIBMATTI_FML_ClassProcessorAuditLog_New(void)
{
    LIBMATTI_FML_ClassProcessorAuditLog *log = calloc(1, sizeof(LIBMATTI_FML_ClassProcessorAuditLog));
    log->source.getAuditString = get_audit_string;
    return log;
}

void LIBMATTI_FML_ClassProcessorAuditLog_Clear(LIBMATTI_FML_ClassProcessorAuditLog *log)
{
    for (size_t i = 0; i < log->count; i++)
    {
        for (size_t a = 0; a < log->activityCounts[i]; a++)
        {
            LIBMATTI_FML_TransformerActivity *activity = log->activities[i][a];
            for (size_t x = 0; x < activity->activityCount; x++) free(activity->activities[x]);
            free(activity->activities);
            free(activity);
        }
        free(log->activities[i]);
        free(log->classes[i]);
    }
    log->count = 0;
}

// Java: private List<TransformerActivity> getTransformerActivities(String clazz)
//       { return audit.computeIfAbsent(clazz, k -> new ArrayList<>()); }
static size_t get_transformer_activities(LIBMATTI_FML_ClassProcessorAuditLog *log, const char *clazz)
{
    for (size_t i = 0; i < log->count; i++)
    {
        if (strcmp(log->classes[i], clazz) == 0) return i;
    }

    log->classes = realloc(log->classes, sizeof(*log->classes) * (log->count + 1));
    log->activities = realloc(log->activities, sizeof(*log->activities) * (log->count + 1));
    log->activityCounts = realloc(log->activityCounts, sizeof(*log->activityCounts) * (log->count + 1));
    log->classes[log->count] = strdup(clazz);
    log->activities[log->count] = NULL;
    log->activityCounts[log->count] = 0;
    return log->count++;
}

static size_t activity_count_for(LIBMATTI_FML_ClassProcessorAuditLog *log, const char *clazz)
{
    for (size_t i = 0; i < log->count; i++)
    {
        if (strcmp(log->classes[i], clazz) == 0) return log->activityCounts[i];
    }
    return 0;
}

LIBMATTI_FML_TransformerActivity *LIBMATTI_FML_ClassProcessorAuditLog_ForClassProcessor(
    LIBMATTI_FML_ClassProcessorAuditLog *log, const char *clazz,
    LIBMATTI_NEOFORGESPI_ClassProcessor *classProcessor)
{
    // Java: var activities = getTransformerActivities(clazz)
    //       var activity = new TransformerActivity(classProcessor.name()); activities.add(activity); return activity
    size_t index = get_transformer_activities(log, clazz);

    LIBMATTI_FML_TransformerActivity *activity = calloc(1, sizeof(LIBMATTI_FML_TransformerActivity));
    activity->processorName = LIBMATTI_NEOFORGESPI_ClassProcessor_Name(classProcessor);
    log->activities[index] = realloc(log->activities[index],
                                     sizeof(*log->activities[index]) * (log->activityCounts[index] + 1));
    log->activities[index][log->activityCounts[index]++] = activity;
    return activity;
}

// Java: TransformerActivity - shouldInclude() / getActivityString()
static char *activity_string(const LIBMATTI_FML_TransformerActivity *activity)
{
    // Java: processorName + (activities.isEmpty() ? "" : "[" + String.join(",", activities) + "]")
    char *name = LIBMATTI_NEOFORGESPI_ProcessorName_ToString(activity->processorName);
    if (activity->activityCount == 0) return name;

    size_t length = strlen(name) + 3;
    for (size_t i = 0; i < activity->activityCount; i++) length += strlen(activity->activities[i]) + 1;

    char *result = malloc(length);
    snprintf(result, length, "%s[", name);
    for (size_t i = 0; i < activity->activityCount; i++)
    {
        strcat(result, activity->activities[i]);
        if (i + 1 < activity->activityCount) strcat(result, ",");
    }
    strcat(result, "]");
    free(name);
    return result;
}

static char *get_audit_string(LIBMATTI_FML_ClassProcessorAuditSource *self, const char *clazz)
{
    // Java: audit.getOrDefault(clazz, List.of()).stream().filter(shouldInclude).map(getActivityString).collect(joining(","))
    LIBMATTI_FML_ClassProcessorAuditLog *log = (LIBMATTI_FML_ClassProcessorAuditLog *) self;

    size_t count = activity_count_for(log, clazz);
    LIBMATTI_FML_TransformerActivity **activities = NULL;
    for (size_t i = 0; i < log->count; i++)
    {
        if (strcmp(log->classes[i], clazz) != 0) continue;
        activities = log->activities[i];
        break;
    }

    size_t capacity = 16;
    char *result = malloc(capacity);
    result[0] = '\0';
    size_t length = 0;
    for (size_t i = 0; i < count; i++)
    {
        // Java: filter(TransformerActivity::shouldInclude)
        if (!activities[i]->include && activities[i]->activityCount == 0) continue;

        char *string = activity_string(activities[i]);
        size_t stringLength = strlen(string);
        while (length + stringLength + 2 > capacity)
        {
            capacity *= 2;
            result = realloc(result, capacity);
        }
        if (length > 0)
        {
            result[length++] = ',';
        }
        memcpy(result + length, string, stringLength);
        length += stringLength;
        result[length] = '\0';
        free(string);
    }
    return result;
}
