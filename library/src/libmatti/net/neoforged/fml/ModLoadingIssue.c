#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"

#include "libmatti/java/lang/StringBuilder.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int is_mod_loading_issue_key(const char *translationKey)
{
    return strncmp(translationKey, "fml.modloadingissue.", 20) == 0;
}

// Java: the compact constructor: if (translationKey.startsWith("fml.") && !translationKey.startsWith("fml.modloadingissue.")) throw new IllegalArgumentException(...)
int LIBMATTI_FML_ModLoadingIssue_ValidateTranslationKey(const char *translationKey)
{
    if (strncmp(translationKey, "fml.", 4) == 0 && !is_mod_loading_issue_key(translationKey)) return 0;
    return 1;
}

// Java: public ModLoadingIssue(Severity severity, String translationKey, List<Object> translationArgs, Throwable cause,
//         Path affectedPath, IModFile affectedModFile, IModInfo affectedMod)
LIBMATTI_FML_ModLoadingIssue *LIBMATTI_FML_ModLoadingIssue_New(LIBMATTI_FML_ModLoadingIssue_Severity severity,
                                                               const char *translationKey,
                                                               const char **translationArgs, size_t translationArgCount,
                                                               LIBMATTI_JL_Throwable *cause, const char *affectedPath,
                                                               void *affectedModFile, const void *affectedMod)
{
    LIBMATTI_FML_ModLoadingIssue *issue = calloc(1, sizeof(LIBMATTI_FML_ModLoadingIssue));

    issue->severity = severity;
    issue->translationKey = strdup(translationKey);
    issue->translationArgCount = translationArgCount;
    issue->translationArgs = malloc(sizeof(*issue->translationArgs) * (translationArgCount == 0 ? 1 : translationArgCount));
    for (size_t i = 0; i < translationArgCount; i++)
        issue->translationArgs[i] = translationArgs[i] != NULL ? strdup(translationArgs[i]) : NULL;

    issue->cause = cause;
    issue->affectedPath = affectedPath != NULL ? strdup(affectedPath) : NULL;
    issue->affectedModFile = affectedModFile;
    // the struct field is not const; the const parameter is the API contract only
    issue->affectedMod = (void *) (uintptr_t) affectedMod;

    return issue;
}

void LIBMATTI_FML_ModLoadingIssue_Free(LIBMATTI_FML_ModLoadingIssue *issue)
{
    if (issue == NULL) return;

    free(issue->translationKey);
    for (size_t i = 0; i < issue->translationArgCount; i++)
        free(issue->translationArgs[i]);
    free(issue->translationArgs);
    free(issue->affectedPath);
    free(issue);
}

// Java: public static ModLoadingIssue error(String translationKey, Object... args)
LIBMATTI_FML_ModLoadingIssue *LIBMATTI_FML_ModLoadingIssue_Error(const char *translationKey,
                                                                 const char **args, size_t argCount)
{
    return LIBMATTI_FML_ModLoadingIssue_New(LIBMATTI_FML_ModLoadingIssue_SEVERITY_ERROR, translationKey, args, argCount,
                                            NULL, NULL, NULL, NULL);
}

// Java: public static ModLoadingIssue warning(String translationKey, Object... args)
LIBMATTI_FML_ModLoadingIssue *LIBMATTI_FML_ModLoadingIssue_Warning(const char *translationKey,
                                                                   const char **args, size_t argCount)
{
    return LIBMATTI_FML_ModLoadingIssue_New(LIBMATTI_FML_ModLoadingIssue_SEVERITY_WARNING, translationKey, args,
                                            argCount, NULL, NULL, NULL, NULL);
}

// Java: public ModLoadingIssue withAffectedPath(Path affectedPath)
LIBMATTI_FML_ModLoadingIssue *LIBMATTI_FML_ModLoadingIssue_WithAffectedPath(const LIBMATTI_FML_ModLoadingIssue *issue,
                                                                            const char *affectedPath)
{
    return LIBMATTI_FML_ModLoadingIssue_New(issue->severity, issue->translationKey,
                                            (const char **) issue->translationArgs, issue->translationArgCount,
                                            issue->cause, affectedPath, NULL, NULL);
}

// Java: public ModLoadingIssue withAffectedModFile(IModFile affectedModFile)
LIBMATTI_FML_ModLoadingIssue *LIBMATTI_FML_ModLoadingIssue_WithAffectedModFile(
    const LIBMATTI_FML_ModLoadingIssue *issue, void *affectedModFile, const char *filePath)
{
    return LIBMATTI_FML_ModLoadingIssue_New(issue->severity, issue->translationKey,
                                            (const char **) issue->translationArgs, issue->translationArgCount,
                                            issue->cause, filePath, affectedModFile, NULL);
}

// Java: public ModLoadingIssue withAffectedMod(IModInfo affectedMod)
LIBMATTI_FML_ModLoadingIssue *LIBMATTI_FML_ModLoadingIssue_WithAffectedMod(const LIBMATTI_FML_ModLoadingIssue *issue,
                                                                           const void *affectedMod, void *affectedModFile,
                                                                           const char *filePath)
{
    return LIBMATTI_FML_ModLoadingIssue_New(issue->severity, issue->translationKey,
                                            (const char **) issue->translationArgs, issue->translationArgCount,
                                            issue->cause, filePath, affectedModFile, affectedMod);
}

// Java: public ModLoadingIssue withCause(Throwable cause)
LIBMATTI_FML_ModLoadingIssue *LIBMATTI_FML_ModLoadingIssue_WithCause(const LIBMATTI_FML_ModLoadingIssue *issue,
                                                                     LIBMATTI_JL_Throwable *cause)
{
    return LIBMATTI_FML_ModLoadingIssue_New(issue->severity, issue->translationKey,
                                            (const char **) issue->translationArgs, issue->translationArgCount,
                                            cause, issue->affectedPath, issue->affectedModFile, issue->affectedMod);
}

// Java: public ModLoadingIssue withSeverity(Severity severity)
LIBMATTI_FML_ModLoadingIssue *LIBMATTI_FML_ModLoadingIssue_WithSeverity(const LIBMATTI_FML_ModLoadingIssue *issue,
                                                                       LIBMATTI_FML_ModLoadingIssue_Severity severity)
{
    return LIBMATTI_FML_ModLoadingIssue_New(severity, issue->translationKey,
                                            (const char **) issue->translationArgs, issue->translationArgCount,
                                            issue->cause, issue->affectedPath, issue->affectedModFile,
                                            issue->affectedMod);
}

// Java: @Override public String toString()
char *LIBMATTI_FML_ModLoadingIssue_ToString(const LIBMATTI_FML_ModLoadingIssue *issue)
{
    // Java: var result = new StringBuilder(severity + ": " + translationKey);
    LIBMATTI_JL_StringBuilder *result = LIBMATTI_JL_StringBuilder_NewFromString(
        issue->severity == LIBMATTI_FML_ModLoadingIssue_SEVERITY_ERROR ? "ERROR" : "WARNING");
    LIBMATTI_JL_StringBuilder_Append(result, ": ");
    LIBMATTI_JL_StringBuilder_Append(result, issue->translationKey);

    // Java: if (!translationArgs.isEmpty()) { result.append(" ["); ... result.append("]"); }
    if (issue->translationArgCount > 0)
    {
        LIBMATTI_JL_StringBuilder_Append(result, " [");
        for (size_t i = 0; i < issue->translationArgCount; i++)
        {
            if (i > 0) LIBMATTI_JL_StringBuilder_Append(result, "; ");
            LIBMATTI_JL_StringBuilder_Append(result, issue->translationArgs[i]);
        }
        LIBMATTI_JL_StringBuilder_Append(result, "]");
    }

    // Java: if (cause != null) { result.append(" caused by ").append(cause); }
    if (issue->cause != NULL)
    {
        LIBMATTI_JL_StringBuilder_Append(result, " caused by ");
        LIBMATTI_JL_StringBuilder_Append(result, LIBMATTI_JL_Throwable_GetName(issue->cause));
    }

    char *text = strdup(LIBMATTI_JL_StringBuilder_ToString(result));
    LIBMATTI_JL_StringBuilder_Free(result);
    return text;
}
