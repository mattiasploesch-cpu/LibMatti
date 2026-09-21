// Port of cpw.mods.modlauncher.api.ITransformerActivity.

#ifndef MATTICRAFT_MODLAUNCHER_ITRANSFORMERACTIVITY_H
#define MATTICRAFT_MODLAUNCHER_ITRANSFORMERACTIVITY_H

#include <stddef.h>

// Java: String COMPUTING_FRAMES_REASON / CLASSLOADING_REASON
#define LIBMATTI_MLA_ITransformerActivity_COMPUTING_FRAMES_REASON "computing_frames"
#define LIBMATTI_MLA_ITransformerActivity_CLASSLOADING_REASON "classloading"

// Java: enum Type { PLUGIN("pl"), TRANSFORMER("xf"), REASON("re") }
typedef enum
{
    LIBMATTI_MLA_ACTIVITY_PLUGIN = 0,
    LIBMATTI_MLA_ACTIVITY_TRANSFORMER,
    LIBMATTI_MLA_ACTIVITY_REASON
} LIBMATTI_MLA_ITransformerActivity_Type;

// Java: interface ITransformerActivity
typedef struct LIBMATTI_MLA_ITransformerActivity
{
    LIBMATTI_MLA_ITransformerActivity_Type type;
    char **context;
    size_t contextCount;
} LIBMATTI_MLA_ITransformerActivity;

// Java: Type.getLabel()
const char *LIBMATTI_MLA_ITransformerActivity_GetLabel(LIBMATTI_MLA_ITransformerActivity_Type type);
// Java: String[] getContext()
char **LIBMATTI_MLA_ITransformerActivity_GetContext(const LIBMATTI_MLA_ITransformerActivity *activity,
                                                    size_t *count);
// Java: Type getType()
LIBMATTI_MLA_ITransformerActivity_Type LIBMATTI_MLA_ITransformerActivity_GetType(
    const LIBMATTI_MLA_ITransformerActivity *activity);
// Java: String getActivityString(); caller frees
char *LIBMATTI_MLA_ITransformerActivity_GetActivityString(const LIBMATTI_MLA_ITransformerActivity *activity);

#endif //MATTICRAFT_MODLAUNCHER_ITRANSFORMERACTIVITY_H
