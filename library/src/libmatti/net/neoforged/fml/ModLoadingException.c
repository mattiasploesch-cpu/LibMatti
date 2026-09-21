#include "libmatti/net/neoforged/fml/ModLoadingException.h"

#include "libmatti/java/lang/StringBuilder.h"
#include "libmatti/net/neoforged/fml/i18n/FMLTranslations.h"

#include <stdlib.h>
#include <string.h>

// Java: translation.replace("\n", "\n\t  ")
static char *replace_newlines(const char *text)
{
    size_t length = strlen(text);
    char *result = malloc(length * 4 + 1);
    size_t written = 0;

    for (size_t i = 0; i < length; i++)
    {
        if (text[i] != '\n')
        {
            result[written++] = text[i];
            continue;
        }

        memcpy(result + written, "\n\t  ", 4);
        written += 4;
    }

    result[written] = '\0';
    return result;
}

// Java: private void appendIssue(ModLoadingIssue issue, StringBuilder result)
static void append_issue(const LIBMATTI_FML_ModLoadingIssue *issue, LIBMATTI_JL_StringBuilder *result)
{
    // Java: try { translation = FMLTranslations.stripControlCodes(FMLTranslations.translateIssueEnglish(issue)); }
    //       catch (Exception e) { translation = issue.toString(); } // Fall back to *something* readable in case the translation fails
    char *translation = LIBMATTI_FML_FMLTranslations_TranslateIssueEnglish(issue);
    if (translation == NULL)
    {
        translation = LIBMATTI_FML_ModLoadingIssue_ToString(issue);
    }
    else
    {
        char *stripped = LIBMATTI_FML_FMLTranslations_StripControlCodes(translation);
        free(translation);
        translation = stripped;
    }

    char *indented = replace_newlines(translation);
    free(translation);

    // Java: result.append("\t- ").append(translation).append("\n");
    LIBMATTI_JL_StringBuilder_Append(result, "\t- ");
    LIBMATTI_JL_StringBuilder_Append(result, indented);
    LIBMATTI_JL_StringBuilder_Append(result, "\n");

    free(indented);
}

// Java: @Override public String getMessage()
char *LIBMATTI_FML_ModLoadingException_GetMessage(const LIBMATTI_FML_ModLoadingException *exception)
{
    // Java: var result = new StringBuilder();
    LIBMATTI_JL_StringBuilder *result = LIBMATTI_JL_StringBuilder_New();

    // Java: var errors = issues.stream().filter(i -> i.severity() == ERROR).toList();
    //       if (!errors.isEmpty()) { result.append("Loading errors encountered:\n"); for (var error : errors) appendIssue(error, result); }
    int hasErrors = 0;
    for (size_t i = 0; i < exception->issueCount; i++)
    {
        if (exception->issues[i]->severity == LIBMATTI_FML_ModLoadingIssue_SEVERITY_ERROR) hasErrors = 1;
    }

    if (hasErrors)
    {
        LIBMATTI_JL_StringBuilder_Append(result, "Loading errors encountered:\n");
        for (size_t i = 0; i < exception->issueCount; i++)
        {
            if (exception->issues[i]->severity != LIBMATTI_FML_ModLoadingIssue_SEVERITY_ERROR) continue;
            append_issue(exception->issues[i], result);
        }
    }

    // Java: var warnings = issues.stream().filter(i -> i.severity() == WARNING).toList();
    //       if (!warnings.isEmpty()) { result.append("Loading warnings encountered:\n"); for (var warning : warnings) appendIssue(warning, result); }
    int hasWarnings = 0;
    for (size_t i = 0; i < exception->issueCount; i++)
    {
        if (exception->issues[i]->severity == LIBMATTI_FML_ModLoadingIssue_SEVERITY_WARNING) hasWarnings = 1;
    }

    if (hasWarnings)
    {
        LIBMATTI_JL_StringBuilder_Append(result, "Loading warnings encountered:\n");
        for (size_t i = 0; i < exception->issueCount; i++)
        {
            if (exception->issues[i]->severity != LIBMATTI_FML_ModLoadingIssue_SEVERITY_WARNING) continue;
            append_issue(exception->issues[i], result);
        }
    }

    char *message = strdup(LIBMATTI_JL_StringBuilder_ToString(result));
    LIBMATTI_JL_StringBuilder_Free(result);
    return message;
}

static LIBMATTI_FML_ModLoadingException *new_exception(LIBMATTI_FML_ModLoadingIssue **issues, size_t issueCount)
{
    LIBMATTI_FML_ModLoadingException *exception = calloc(1, sizeof(LIBMATTI_FML_ModLoadingException));
    exception->base.className = strdup("net.neoforged.fml.ModLoadingException");

    exception->issueCount = issueCount;
    exception->issues = malloc(sizeof(*exception->issues) * (issueCount == 0 ? 1 : issueCount));
    for (size_t i = 0; i < issueCount; i++)
        exception->issues[i] = issues[i];

    // Java computes getMessage() on demand; the C port stores it on the throwable as well so the
    // existing printStackTrace surface shows the same text.
    exception->base.message = LIBMATTI_FML_ModLoadingException_GetMessage(exception);
    return exception;
}

// Java: public ModLoadingException(ModLoadingIssue issue)
LIBMATTI_FML_ModLoadingException *LIBMATTI_FML_ModLoadingException_New(LIBMATTI_FML_ModLoadingIssue *issue)
{
    // Java: this(List.of(issue));
    LIBMATTI_FML_ModLoadingIssue *issues[1] = {issue};
    return new_exception(issues, 1);
}

// Java: public ModLoadingException(List<ModLoadingIssue> issues)
LIBMATTI_FML_ModLoadingException *LIBMATTI_FML_ModLoadingException_NewWithIssues(LIBMATTI_FML_ModLoadingIssue **issues,
                                                                                 size_t issueCount)
{
    return new_exception(issues, issueCount);
}

// Java: public List<ModLoadingIssue> getIssues()
LIBMATTI_FML_ModLoadingIssue **LIBMATTI_FML_ModLoadingException_GetIssues(
    const LIBMATTI_FML_ModLoadingException *exception, size_t *count)
{
    *count = exception->issueCount;
    return exception->issues;
}

void LIBMATTI_FML_ModLoadingException_Free(LIBMATTI_FML_ModLoadingException *exception)
{
    if (exception == NULL) return;

    free(exception->base.className);
    free(exception->base.message);
    free(exception->issues);
    free(exception);
}
