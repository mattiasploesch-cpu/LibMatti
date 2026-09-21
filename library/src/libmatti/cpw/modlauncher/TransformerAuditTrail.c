// Port of cpw.mods.modlauncher.TransformerAuditTrail.

#include "libmatti/cpw/modlauncher/TransformerAuditTrail.h"

#include "libmatti/cpw/modlauncher/TransformerHolder.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_ML_TransformerAuditTrail *LIBMATTI_ML_TransformerAuditTrail_New(void)
{
    return calloc(1, sizeof(LIBMATTI_ML_TransformerAuditTrail));
}

// Java: private List<ITransformerActivity> getTransformerActivities(String clazz)
//           { return audit.computeIfAbsent(clazz, k -> new ArrayList<>()); }
static LIBMATTI_ML_TransformerAuditTrailEntry *activities_for(LIBMATTI_ML_TransformerAuditTrail *trail,
                                                              const char *clazz)
{
    for (size_t i = 0; i < trail->count; i++)
        if (strcmp(trail->entries[i].className, clazz) == 0) return &trail->entries[i];

    trail->entries = realloc(trail->entries, sizeof(*trail->entries) * (trail->count + 1));
    LIBMATTI_ML_TransformerAuditTrailEntry *entry = &trail->entries[trail->count++];
    entry->className = strdup(clazz);
    entry->activities = NULL;
    entry->count = 0;
    return entry;
}

static void add_activity(LIBMATTI_ML_TransformerAuditTrailEntry *entry, LIBMATTI_MLA_ITransformerActivity_Type type,
                         char **context, size_t contextCount)
{
    entry->activities = realloc(entry->activities, sizeof(*entry->activities) * (entry->count + 1));
    LIBMATTI_MLA_ITransformerActivity *activity = &entry->activities[entry->count++];
    activity->type = type;
    activity->context = malloc(sizeof(*activity->context) * (contextCount > 0 ? contextCount : 1));
    activity->contextCount = contextCount;
    for (size_t i = 0; i < contextCount; i++) activity->context[i] = strdup(context[i]);
}

// Java: private String[] concat(String first, String[] rest)
static char **concat(const char *first, char **rest, size_t restCount)
{
    char **result = malloc(sizeof(*result) * (restCount + 1));
    result[0] = strdup(first);
    for (size_t i = 0; i < restCount; i++) result[i + 1] = strdup(rest[i]);
    return result;
}

void LIBMATTI_ML_TransformerAuditTrail_AddReason(LIBMATTI_ML_TransformerAuditTrail *trail, const char *clazz,
                                                 const char *reason)
{
    char *context[1];
    context[0] = (char *)reason;
    add_activity(activities_for(trail, clazz), LIBMATTI_MLA_ACTIVITY_REASON, context, 1);
}

void LIBMATTI_ML_TransformerAuditTrail_AddPluginCustomAuditTrail(LIBMATTI_ML_TransformerAuditTrail *trail,
                                                                 const char *clazz,
                                                                 LIBMATTI_MLS_ILaunchPluginService *plugin,
                                                                 char **data, size_t dataCount)
{
    char **context = concat(LIBMATTI_MLS_ILaunchPluginService_Name(plugin), data, dataCount);
    add_activity(activities_for(trail, clazz), LIBMATTI_MLA_ACTIVITY_PLUGIN, context, dataCount + 1);

    for (size_t i = 0; i <= dataCount; i++) free(context[i]);
    free(context);
}

void LIBMATTI_ML_TransformerAuditTrail_AddPluginAuditTrail(LIBMATTI_ML_TransformerAuditTrail *trail, const char *clazz,
                                                           LIBMATTI_MLS_ILaunchPluginService *plugin,
                                                           LIBMATTI_MLS_Phase phase)
{
    // Java: new TransformerActivity(PLUGIN, plugin.name(), phase.name().substring(0, 1))
    char *context[2];
    context[0] = (char *)LIBMATTI_MLS_ILaunchPluginService_Name(plugin);
    context[1] = phase == LIBMATTI_MLS_PHASE_BEFORE ? "B" : "A";
    add_activity(activities_for(trail, clazz), LIBMATTI_MLA_ACTIVITY_PLUGIN, context, 2);
}

void LIBMATTI_ML_TransformerAuditTrail_AddTransformerAuditTrail(LIBMATTI_ML_TransformerAuditTrail *trail,
                                                               const char *clazz,
                                                               LIBMATTI_MLA_ITransformationService *transformService,
                                                               LIBMATTI_MLA_ITransformer *transformer)
{
    size_t labelCount = 0;
    char **labels = LIBMATTI_MLA_ITransformer_Labels(transformer, &labelCount);

    char **context = concat(LIBMATTI_MLA_ITransformationService_Name(transformService), labels, labelCount);
    add_activity(activities_for(trail, clazz), LIBMATTI_MLA_ACTIVITY_TRANSFORMER, context, labelCount + 1);

    for (size_t i = 0; i <= labelCount; i++) free(context[i]);
    free(context);
}

// Java: public List<ITransformerActivity> getActivityFor(String className)
LIBMATTI_MLA_ITransformerActivity *LIBMATTI_MLA_ITransformerAuditTrail_GetActivityFor(
    LIBMATTI_MLA_ITransformerAuditTrail *trail, const char *className, size_t *count)
{
    for (size_t i = 0; i < trail->count; i++)
    {
        if (strcmp(trail->entries[i].className, className) != 0) continue;
        *count = trail->entries[i].count;
        return trail->entries[i].activities;
    }

    *count = 0;
    return NULL;
}

// Java: public String getAuditString(String clazz)
char *LIBMATTI_MLA_ITransformerAuditTrail_GetAuditString(LIBMATTI_MLA_ITransformerAuditTrail *trail,
                                                         const char *clazz)
{
    size_t count = 0;
    LIBMATTI_MLA_ITransformerActivity *activities =
        LIBMATTI_MLA_ITransformerAuditTrail_GetActivityFor(trail, clazz, &count);

    if (count == 0) return strdup("");

    size_t length = 1;
    char **parts = malloc(sizeof(*parts) * count);
    for (size_t i = 0; i < count; i++)
    {
        parts[i] = LIBMATTI_MLA_ITransformerActivity_GetActivityString(&activities[i]);
        length += strlen(parts[i]) + 1;
    }

    // Java: Collectors.joining(",")
    char *result = malloc(length);
    result[0] = '\0';
    for (size_t i = 0; i < count; i++)
    {
        if (i > 0) strcat(result, ",");
        strcat(result, parts[i]);
        free(parts[i]);
    }
    free(parts);

    return result;
}

void LIBMATTI_ML_TransformerAuditTrail_Free(LIBMATTI_ML_TransformerAuditTrail *trail)
{
    if (trail == NULL) return;

    for (size_t i = 0; i < trail->count; i++)
    {
        free(trail->entries[i].className);
        for (size_t a = 0; a < trail->entries[i].count; a++)
        {
            for (size_t c = 0; c < trail->entries[i].activities[a].contextCount; c++)
                free(trail->entries[i].activities[a].context[c]);
            free(trail->entries[i].activities[a].context);
        }
        free(trail->entries[i].activities);
    }
    free(trail->entries);
    free(trail);
}
