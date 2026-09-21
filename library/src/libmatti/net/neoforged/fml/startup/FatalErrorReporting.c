#include "libmatti/net/neoforged/fml/startup/FatalErrorReporting.h"

#include "libmatti/java/awt/GraphicsEnvironment.h"
#include "libmatti/java/awt/Image.h"
#include "libmatti/java/awt/image/BaseMultiResolutionImage.h"
#include "libmatti/java/lang/Class.h"
#include "libmatti/java/lang/StringBuilder.h"
#include "libmatti/java/lang/System.h"
#include "libmatti/java/lang/reflect/InvocationTargetException.h"
#include "libmatti/java/net/URL.h"
#include "libmatti/java/nio/file/Path.h"
#include "libmatti/javax/imageio/ImageIO.h"
#include "libmatti/javax/swing/ImageIcon.h"
#include "libmatti/javax/swing/JOptionPane.h"
#include "libmatti/javax/swing/UIManager.h"
#include "libmatti/net/neoforged/fml/ModLoadingException.h"
#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"
#include "libmatti/net/neoforged/fml/i18n/FMLTranslations.h"
#include "libmatti/net/neoforged/fml/loading/ImmediateWindowHandler.h"
#include "libmatti/net/neoforged/fml/startup/FatalStartupException.h"
#include "libmatti/org/lwjgl/util/tinyfd/TinyFileDialogs.h"

#include <stdlib.h>
#include <string.h>

#define FML_FATAL_STARTUP_EXCEPTION_NAME "net.neoforged.fml.startup.FatalStartupException"
#define FML_FATAL_ERROR_REPORTING_NAME "net.neoforged.fml.startup.FatalErrorReporting"

// Java: private static Throwable unwrapException(Throwable t)
static LIBMATTI_JL_Throwable *unwrap_exception(LIBMATTI_JL_Throwable *t)
{
    if (t == NULL) return NULL;

    LIBMATTI_JL_Throwable *cause = LIBMATTI_JL_Throwable_GetCause(t);
    if (cause == NULL) return t;

    // Java: if (t instanceof InvocationTargetException) return cause;
    if (strcmp(LIBMATTI_JL_Throwable_GetName(t), LIBMATTI_JLR_InvocationTargetException_NAME) == 0) return cause;
    return t;
}

// Java: var issues = new ArrayList<ModLoadingIssue>(); - the port builds the list locally
static void add_issue(LIBMATTI_FML_ModLoadingIssue ***issues, size_t *issueCount, LIBMATTI_FML_ModLoadingIssue *issue)
{
    *issues = realloc(*issues, sizeof(LIBMATTI_FML_ModLoadingIssue *) * (*issueCount + 1));
    (*issues)[(*issueCount)++] = issue;
}

// Java: private static void collectModLoadingIssues(Throwable t, List<ModLoadingIssue> issues)
// The collected issues are borrowed from the exception's issue list - the caller owns
// the list, the issues stay owned by their ModLoadingException.
static void collect_mod_loading_issues(LIBMATTI_JL_Throwable *t, LIBMATTI_FML_ModLoadingIssue ***issues,
                                       size_t *issueCount)
{
    if (t == NULL) return;

    // Java: if (t instanceof ModLoadingException e) issues.addAll(e.getIssues());
    if (strcmp(LIBMATTI_JL_Throwable_GetName(t), "net.neoforged.fml.ModLoadingException") == 0)
    {
        size_t exceptionIssueCount = 0;
        LIBMATTI_FML_ModLoadingIssue **exceptionIssues = LIBMATTI_FML_ModLoadingException_GetIssues(
            (LIBMATTI_FML_ModLoadingException *) t, &exceptionIssueCount);
        for (size_t i = 0; i < exceptionIssueCount; i++)
            add_issue(issues, issueCount, exceptionIssues[i]);
    }

    // Java: for (var suppressed : t.getSuppressed()) collectModLoadingIssues(suppressed, issues);
    size_t suppressedCount = 0;
    LIBMATTI_JL_Throwable **suppressed = LIBMATTI_JL_Throwable_GetSuppressed(t, &suppressedCount);
    for (size_t i = 0; i < suppressedCount; i++)
        collect_mod_loading_issues(suppressed[i], issues, issueCount);

    // Java: if (t.getCause() != null) collectModLoadingIssues(t.getCause(), issues);
    collect_mod_loading_issues(LIBMATTI_JL_Throwable_GetCause(t), issues, issueCount);
}

// Java: Throwable.toString()
static void append_throwable_to_string(LIBMATTI_JL_StringBuilder *builder, LIBMATTI_JL_Throwable *t)
{
    LIBMATTI_JL_StringBuilder_Append(builder, LIBMATTI_JL_Throwable_GetName(t));
    const char *message = LIBMATTI_JL_Throwable_GetMessage(t);
    if (message != NULL)
    {
        LIBMATTI_JL_StringBuilder_Append(builder, ": ");
        LIBMATTI_JL_StringBuilder_Append(builder, message);
    }
}

// Java: private static void appendAbbreviatedExceptionChain(Throwable t, StringBuilder exceptionText)
static void append_abbreviated_exception_chain(LIBMATTI_JL_Throwable *t, LIBMATTI_JL_StringBuilder *exceptionText)
{
    int first = 1;
    while (t != NULL)
    {
        LIBMATTI_JL_StringBuilder_Append(exceptionText, first ? "  " : " \xe2\x86\xb3");
        append_throwable_to_string(exceptionText, t);
        LIBMATTI_JL_StringBuilder_Append(exceptionText, "\n");
        first = 0;
        t = unwrap_exception(LIBMATTI_JL_Throwable_GetCause(t));
    }
}

// Java: String.replace (all occurrences)
static char *replace_all(const char *value, const char *target, const char *replacement)
{
    size_t targetLength = strlen(target);
    if (targetLength == 0) return strdup(value);

    LIBMATTI_JL_StringBuilder *builder = LIBMATTI_JL_StringBuilder_New();
    for (size_t i = 0; value[i] != '\0';)
    {
        if (strncmp(value + i, target, targetLength) == 0)
        {
            LIBMATTI_JL_StringBuilder_Append(builder, replacement);
            i += targetLength;
        }
        else
        {
            LIBMATTI_JL_StringBuilder_AppendChar(builder, value[i]);
            i++;
        }
    }

    char *result = strdup(LIBMATTI_JL_StringBuilder_ToString(builder));
    LIBMATTI_JL_StringBuilder_Free(builder);
    return result;
}

// Java: private static String escapeHtmlContent(String content)
static char *escape_html_content(const char *content)
{
    char *result = replace_all(content, "&", "&amp;");
    char *next = replace_all(result, "<", "&lt;");
    free(result);
    result = next;

    next = replace_all(result, ">", "&gt;");
    free(result);
    result = next;

    next = replace_all(result, "\"", "&quot;");
    free(result);
    result = next;

    next = replace_all(result, "'", "&#x27;");
    free(result);
    return next;
}

// Java: List<Image>
typedef struct
{
    LIBMATTI_JAWT_Image **items;
    size_t count;
} ImageList;

// Java: private static void readImage(String filename, List<Image> images)
static void read_image(const char *filename, ImageList *images)
{
    // Java: FatalErrorReporting.class.getResource(filename)
    LIBMATTI_JL_Class *owner = LIBMATTI_JL_Class_Register(FML_FATAL_ERROR_REPORTING_NAME, NULL);
    LIBMATTI_JN_URL *resource = LIBMATTI_JL_Class_GetResource(owner, filename);

    // Java: var image = ImageIO.read(FatalErrorReporting.class.getResource(filename));
    LIBMATTI_JAWT_Image *image = LIBMATTI_JXI_ImageIO_Read(resource);
    LIBMATTI_JN_URL_Free(resource);

    images->items = realloc(images->items, sizeof(*images->items) * (images->count + 1));
    images->items[images->count++] = image;
}

// Java: private static BaseMultiResolutionImage createMultiResImage()
static LIBMATTI_JAWT_IM_BaseMultiResolutionImage *create_multi_res_image(void)
{
    ImageList images = {NULL, 0};
    read_image("crash-32x32.png", &images);
    read_image("crash-64x64.png", &images);
    read_image("crash-128x128.png", &images);

    // Java: new BaseMultiResolutionImage(images.toArray(Image[]::new))
    return LIBMATTI_JAWT_IM_BaseMultiResolutionImage_New(images.items, images.count);
}

// Java: private static void showErrorUsingSwing(String message)
static void show_error_using_swing(const char *message)
{
    // Java: try { UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName()); } catch (Exception ignored) {}
    LIBMATTI_JS_UIManager_SetLookAndFeel(LIBMATTI_JS_UIManager_GetSystemLookAndFeelClassName());

    // Java: String html = "<html><body width='400'><strong>Fatal Startup Error</strong>" + "<p><pre>";
    LIBMATTI_JL_StringBuilder *html = LIBMATTI_JL_StringBuilder_NewFromString(
        "<html><body width='400'><strong>Fatal Startup Error</strong><p><pre>");

    char *escaped = escape_html_content(message);
    LIBMATTI_JL_StringBuilder_Append(html, escaped);
    free(escaped);

    // Java: var icon = new ImageIcon(createMultiResImage());
    LIBMATTI_JAWT_IM_BaseMultiResolutionImage *multiResImage = create_multi_res_image();
    LIBMATTI_JS_ImageIcon *icon = LIBMATTI_JS_ImageIcon_New((LIBMATTI_JAWT_Image *)multiResImage);

    // Java: JOptionPane.showMessageDialog(null, html, "Fatal Error", JOptionPane.ERROR_MESSAGE, icon);
    LIBMATTI_JS_JOptionPane_ShowMessageDialog(NULL, LIBMATTI_JL_StringBuilder_ToString(html), "Fatal Error",
                                              LIBMATTI_JS_JOptionPane_ERROR_MESSAGE, icon);

    LIBMATTI_JS_ImageIcon_Free(icon);
    LIBMATTI_JAWT_IM_BaseMultiResolutionImage_Free(multiResImage);
    LIBMATTI_JL_StringBuilder_Free(html);
}

// Java: public static void reportFatalError(Throwable t)
void LIBMATTI_FML_FatalErrorReporting_ReportFatalError(LIBMATTI_JL_Throwable *t)
{
    // Java: Path gameDir = t instanceof FatalStartupException ? fatalStartupException.getStartupArgs().gameDirectory() : null
    const char *gameDir = NULL;
    if (t != NULL && strcmp(LIBMATTI_JL_Throwable_GetName(t), FML_FATAL_STARTUP_EXCEPTION_NAME) == 0)
    {
        LIBMATTI_FML_StartupArgs *startupArgs =
            LIBMATTI_FML_FatalStartupException_GetStartupArgs((LIBMATTI_FML_FatalStartupException *)t);
        gameDir = startupArgs->gameDirectory;
    }
    LIBMATTI_FML_FatalErrorReporting_ReportFatalErrorWithPaths(t, gameDir, NULL, NULL);
}

// Java: public static void reportFatalError(Throwable t, @Nullable Path gameDir, @Nullable Path logFile, @Nullable Path crashReport)
void LIBMATTI_FML_FatalErrorReporting_ReportFatalErrorWithPaths(LIBMATTI_JL_Throwable *t, const char *gameDir,
                                                               const char *logFile, const char *crashReport)
{
    // Java: var issues = new ArrayList<ModLoadingIssue>(); collectModLoadingIssues(t, issues);
    LIBMATTI_FML_ModLoadingIssue **issues = NULL;
    size_t issueCount = 0;
    collect_mod_loading_issues(t, &issues, &issueCount);

    // Java: if (issues.isEmpty()) { ... } - the port's ModLoadingException channel is
    // register-with-ModLoader, so the exception chain is inspected for the same issues.
    if (issueCount == 0)
    {
        LIBMATTI_JL_Throwable *exception = t;
        while (exception != NULL)
        {
            if (strcmp(LIBMATTI_JL_Throwable_GetName(exception), "net.neoforged.fml.ModLoadingException") == 0)
            {
                size_t exceptionIssueCount = 0;
                LIBMATTI_FML_ModLoadingIssue **exceptionIssues = LIBMATTI_FML_ModLoadingException_GetIssues(
                    (LIBMATTI_FML_ModLoadingException *) exception, &exceptionIssueCount);
                for (size_t i = 0; i < exceptionIssueCount; i++)
                    add_issue(&issues, &issueCount, exceptionIssues[i]);
            }
            exception = LIBMATTI_JL_Throwable_GetCause(exception);
        }
    }

    // Java: if (issues.isEmpty()) { t = unwrapException(t); ...
    if (issueCount == 0)
    {
        t = unwrap_exception(t);

        // Java: var errorText = new StringBuilder(e.getMessage()); ...
        //       else { var exceptionText = new StringBuilder(); appendAbbreviatedExceptionChain(t, exceptionText); }
        LIBMATTI_JL_StringBuilder *errorText;
        if (t != NULL && strcmp(LIBMATTI_JL_Throwable_GetName(t), FML_FATAL_STARTUP_EXCEPTION_NAME) == 0)
        {
            const char *message = LIBMATTI_JL_Throwable_GetMessage(t);
            errorText = LIBMATTI_JL_StringBuilder_NewFromString(message != NULL ? message : "");
            if (LIBMATTI_JL_Throwable_GetCause(t) != NULL)
            {
                LIBMATTI_JL_StringBuilder_Append(errorText, "\n\nTechnical Details:\n");
                append_abbreviated_exception_chain(unwrap_exception(LIBMATTI_JL_Throwable_GetCause(t)), errorText);
            }
        }
        else
        {
            errorText = LIBMATTI_JL_StringBuilder_New();
            append_abbreviated_exception_chain(t, errorText);
        }

        // Java: issues.add(ModLoadingIssue.error("fml.modloadingissue.technical_error", errorText).withCause(t));
        LIBMATTI_FML_ModLoadingIssue *technicalIssue =
            LIBMATTI_FML_ModLoadingIssue_Error("fml.modloadingissue.technical_error", NULL, 0);
        technicalIssue = LIBMATTI_FML_ModLoadingIssue_WithAffectedPath(technicalIssue,
                                                                       LIBMATTI_JL_StringBuilder_ToString(errorText));
        add_issue(&issues, &issueCount, technicalIssue);
        LIBMATTI_JL_StringBuilder_Free(errorText);
    }

    // Java: ImmediateWindowHandler.displayFatalErrorAndExit(issues,
    //           gameDir != null ? gameDir.resolve("mods") : null, logFile, crashReport);
    char *modsFolder = gameDir != NULL ? LIBMATTI_JNF_Path_Resolve(gameDir, "mods") : NULL;
    LIBMATTI_FML_ImmediateWindowHandler_DisplayFatalErrorAndExit(issues, issueCount, modsFolder, logFile, crashReport);
    free(modsFolder);

    // Java: for (var issue : issues) { errorReport.append(FMLTranslations.translateIssue(issue)); }
    //       reportFatalError(errorReport.toString());
    LIBMATTI_JL_StringBuilder *report = LIBMATTI_JL_StringBuilder_NewFromString("Fatal Startup Error\n\n");
    for (size_t i = 0; i < issueCount; i++)
    {
        char *translated = LIBMATTI_FML_FMLTranslations_TranslateIssue(issues[i]);
        LIBMATTI_JL_StringBuilder_Append(report, translated);
        LIBMATTI_JL_StringBuilder_Append(report, "\n");
        free(translated);
    }
    LIBMATTI_FML_FatalErrorReporting_ReportFatalErrorMessage(LIBMATTI_JL_StringBuilder_ToString(report));
    LIBMATTI_JL_StringBuilder_Free(report);

    // The issues array is locally owned; the issues themselves stay with their owners
    // (the ModLoadingException instances or the technical issue created above).
    for (size_t i = 0; i < issueCount; i++)
    {
        // Java: the technical issue is only referenced here - free the ones the port created
        if (strcmp(issues[i]->translationKey, "fml.modloadingissue.technical_error") == 0)
            LIBMATTI_FML_ModLoadingIssue_Free(issues[i]);
    }
    free(issues);
}

// Java: public static void reportFatalErrorOnConsole(Throwable t)
void LIBMATTI_FML_FatalErrorReporting_ReportFatalErrorOnConsole(LIBMATTI_JL_Throwable *t)
{
    t = unwrap_exception(t);
    // Java: t.printStackTrace();
    LIBMATTI_JL_Throwable_PrintStackTrace(t);
}

// Java: public static void reportFatalError(String message)
void LIBMATTI_FML_FatalErrorReporting_ReportFatalErrorMessage(const char *message)
{
    // Java: System.setProperty("java.awt.headless", "false"); // Overriding what MC set
    LIBMATTI_JL_System_SetProperty("java.awt.headless", "false");

    // Java: if (!GraphicsEnvironment.isHeadless()) { showErrorUsingSwing(message); }
    if (!LIBMATTI_JAWT_GraphicsEnvironment_IsHeadless())
    {
        show_error_using_swing(message);
    }
    else
    {
        // Java: TinyFD refuses to let us use quotes
        //       message = message.replace('"', '`'); message = message.replace('\'', '`');
        char *replaced = replace_all(message, "\"", "`");
        char *next = replace_all(replaced, "'", "`");
        free(replaced);

        // Java: TinyFileDialogs.tinyfd_messageBox("NeoForge - Fatal Startup Error", message, "ok", "error", 1);
        LIBMATTI_LWJGL_TINYFD_TinyFileDialogs_TinyfdMessageBox("NeoForge - Fatal Startup Error", next, "ok", "error",
                                                              1);
        free(next);
    }

    // Java: System.exit(1);
    LIBMATTI_JL_System_Exit(1);
}
