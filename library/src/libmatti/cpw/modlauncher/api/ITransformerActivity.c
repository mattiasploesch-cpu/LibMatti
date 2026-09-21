// Port of cpw.mods.modlauncher.api.ITransformerActivity.

#include "libmatti/cpw/modlauncher/api/ITransformerActivity.h"

#include <stdlib.h>
#include <string.h>

const char *LIBMATTI_MLA_ITransformerActivity_GetLabel(LIBMATTI_MLA_ITransformerActivity_Type type)
{
    switch (type)
    {
    case LIBMATTI_MLA_ACTIVITY_PLUGIN: return "pl";
    case LIBMATTI_MLA_ACTIVITY_TRANSFORMER: return "xf";
    case LIBMATTI_MLA_ACTIVITY_REASON: return "re";
    default: return "";
    }
}

char **LIBMATTI_MLA_ITransformerActivity_GetContext(const LIBMATTI_MLA_ITransformerActivity *activity, size_t *count)
{
    *count = activity->contextCount;
    return activity->context;
}

LIBMATTI_MLA_ITransformerActivity_Type LIBMATTI_MLA_ITransformerActivity_GetType(
    const LIBMATTI_MLA_ITransformerActivity *activity)
{
    return activity->type;
}

char *LIBMATTI_MLA_ITransformerActivity_GetActivityString(const LIBMATTI_MLA_ITransformerActivity *activity)
{
    // Java: type.getLabel() + ":" + String.join(":", context)
    size_t length = strlen(LIBMATTI_MLA_ITransformerActivity_GetLabel(activity->type)) + 1;
    for (size_t i = 0; i < activity->contextCount; i++) length += strlen(activity->context[i]) + 1;

    char *result = malloc(length);
    strcpy(result, LIBMATTI_MLA_ITransformerActivity_GetLabel(activity->type));

    for (size_t i = 0; i < activity->contextCount; i++)
    {
        strcat(result, ":");
        strcat(result, activity->context[i]);
    }

    return result;
}
