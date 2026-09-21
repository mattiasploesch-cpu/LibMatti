#ifndef MATTICRAFT_MODLAUNCHER_EXTRADATATEXTRENDERER_H
#define MATTICRAFT_MODLAUNCHER_EXTRADATATEXTRENDERER_H

#include "libmatti/cpw/modlauncher/api/ITransformerAuditTrail.h"
#include "libmatti/org/apache/logging/log4j/core/pattern/TextRenderer.h"

typedef struct
{
    char *className;
    char *methodName;
} LIBMATTI_ML_LogTransformerContext;

typedef struct LIBMATTI_ML_ExtraDataTextRenderer
{
    LIBMATTI_L4J_TextRenderer base;
    LIBMATTI_L4J_TextRenderer *wrapped;
    LIBMATTI_MLA_ITransformerAuditTrail *auditData;
    LIBMATTI_ML_LogTransformerContext *currentClass;
} LIBMATTI_ML_ExtraDataTextRenderer;

LIBMATTI_ML_ExtraDataTextRenderer *LIBMATTI_ML_ExtraDataTextRenderer_New(LIBMATTI_L4J_TextRenderer *wrapped);
void LIBMATTI_ML_ExtraDataTextRenderer_Free(LIBMATTI_ML_ExtraDataTextRenderer *renderer);

void LIBMATTI_ML_LogTransformerContext_SetClassName(LIBMATTI_ML_LogTransformerContext *context, const char *className);
const char *LIBMATTI_ML_LogTransformerContext_GetClassName(const LIBMATTI_ML_LogTransformerContext *context);
void LIBMATTI_ML_LogTransformerContext_SetMethodName(LIBMATTI_ML_LogTransformerContext *context,
                                                     const char *methodName);
const char *LIBMATTI_ML_LogTransformerContext_GetMethodName(const LIBMATTI_ML_LogTransformerContext *context);
char *LIBMATTI_ML_LogTransformerContext_ToString(const LIBMATTI_ML_LogTransformerContext *context);

#endif //MATTICRAFT_MODLAUNCHER_EXTRADATATEXTRENDERER_H
