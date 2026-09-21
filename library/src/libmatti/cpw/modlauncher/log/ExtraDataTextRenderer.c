#include "libmatti/cpw/modlauncher/log/ExtraDataTextRenderer.h"

#include "libmatti/cpw/modlauncher/Launcher.h"
#include "libmatti/cpw/modlauncher/api/IEnvironment.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int style_is(const char *styleName, const char *expected)
{
    return styleName != NULL && strcmp(styleName, expected) == 0;
}

void LIBMATTI_ML_LogTransformerContext_SetClassName(LIBMATTI_ML_LogTransformerContext *context, const char *className)
{
    free(context->className);
    context->className = className != NULL ? strdup(className) : NULL;
}

const char *LIBMATTI_ML_LogTransformerContext_GetClassName(const LIBMATTI_ML_LogTransformerContext *context)
{
    return context->className;
}

void LIBMATTI_ML_LogTransformerContext_SetMethodName(LIBMATTI_ML_LogTransformerContext *context,
                                                     const char *methodName)
{
    free(context->methodName);
    context->methodName = methodName != NULL ? strdup(methodName) : NULL;
}

const char *LIBMATTI_ML_LogTransformerContext_GetMethodName(const LIBMATTI_ML_LogTransformerContext *context)
{
    return context->methodName;
}

char *LIBMATTI_ML_LogTransformerContext_ToString(const LIBMATTI_ML_LogTransformerContext *context)
{
    const char *className = context->className != NULL ? context->className : "";
    const char *methodName = context->methodName != NULL ? context->methodName : "";

    size_t length = strlen(className) + strlen(methodName) + 2;
    char *result = malloc(length);
    snprintf(result, length, "%s.%s", className, methodName);
    return result;
}

static void context_free(LIBMATTI_ML_LogTransformerContext *context)
{
    if (context == NULL) return;
    free(context->className);
    free(context->methodName);
    free(context);
}

static void extra_render(LIBMATTI_L4J_TextRenderer *self, const char *input, LIBMATTI_JL_StringBuilder *output,
                         const char *styleName)
{
    LIBMATTI_ML_ExtraDataTextRenderer *renderer = (LIBMATTI_ML_ExtraDataTextRenderer *)self;

    if (style_is(styleName, "StackTraceElement.ClassName"))
    {
        context_free(renderer->currentClass);
        renderer->currentClass = calloc(1, sizeof(LIBMATTI_ML_LogTransformerContext));
        LIBMATTI_ML_LogTransformerContext_SetClassName(renderer->currentClass, input);
    }
    else if (style_is(styleName, "StackTraceElement.MethodName"))
    {
        if (renderer->currentClass != NULL)
            LIBMATTI_ML_LogTransformerContext_SetMethodName(renderer->currentClass, input);
    }
    else if (style_is(styleName, "Suffix"))
    {
        LIBMATTI_ML_LogTransformerContext *context = renderer->currentClass;
        renderer->currentClass = NULL;

        if (context != NULL)
        {
            char *auditLine = renderer->auditData != NULL
                                  ? LIBMATTI_MLA_ITransformerAuditTrail_GetAuditString(
                                        renderer->auditData, LIBMATTI_ML_LogTransformerContext_GetClassName(context))
                                  : NULL;

            const char *audit = auditLine != NULL ? auditLine : "";
            size_t length = strlen(audit) + 5;
            char *rendered = malloc(length);
            snprintf(rendered, length, " {%s}", audit);

            renderer->wrapped->render(renderer->wrapped, rendered, output, "StackTraceElement.Transformers");

            free(rendered);
            free(auditLine);
            context_free(context);
        }
        return;
    }

    renderer->wrapped->render(renderer->wrapped, input, output, styleName);
}

static void extra_render_buffer(LIBMATTI_L4J_TextRenderer *self, LIBMATTI_JL_StringBuilder *input,
                                LIBMATTI_JL_StringBuilder *output)
{
    LIBMATTI_ML_ExtraDataTextRenderer *renderer = (LIBMATTI_ML_ExtraDataTextRenderer *)self;
    renderer->wrapped->renderBuffer(renderer->wrapped, input, output);
}

LIBMATTI_ML_ExtraDataTextRenderer *LIBMATTI_ML_ExtraDataTextRenderer_New(LIBMATTI_L4J_TextRenderer *wrapped)
{
    LIBMATTI_ML_ExtraDataTextRenderer *renderer = calloc(1, sizeof(LIBMATTI_ML_ExtraDataTextRenderer));
    renderer->base.render = extra_render;
    renderer->base.renderBuffer = extra_render_buffer;
    renderer->wrapped = wrapped;

    if (LIBMATTI_ML_Launcher_INSTANCE != NULL)
        renderer->auditData = LIBMATTI_MLA_IEnvironment_GetProperty(
            (LIBMATTI_MLA_IEnvironment *)LIBMATTI_ML_Launcher_INSTANCE->environment,
            LIBMATTI_MLA_IEnvironment_Keys_AuditTrail());

    return renderer;
}

void LIBMATTI_ML_ExtraDataTextRenderer_Free(LIBMATTI_ML_ExtraDataTextRenderer *renderer)
{
    if (renderer == NULL) return;
    context_free(renderer->currentClass);
    free(renderer);
}
