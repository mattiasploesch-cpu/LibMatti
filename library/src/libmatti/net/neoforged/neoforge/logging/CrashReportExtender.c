// Port of net.neoforged.neoforge.logging.CrashReportExtender.
// Java builds a CrashReport with one category per issue and saves it to
// crash-reports/crash-<timestamp>-fml.txt; the port writes the same categories into the file
// (the CrashReport object model is the game port's part, the report text is the same layout).

#include "libmatti/net/neoforged/neoforge/logging/CrashReportExtender.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/nio/file/Files.h"
#include "libmatti/net/neoforged/fml/i18n/FMLTranslations.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Java: category.setDetail(name, value)
static void write_detail(FILE *file, const char *name, const char *value)
{
    // Java: the report indents the details with two tabs
    fprintf(file, "\t\t%s: %s\n", name, value != NULL ? value : "<null>");
}

// Java: public static File dumpModLoadingCrashReport(final Logger logger, final List<ModLoadingIssue> issues, final File topLevelDir)
char *LIBMATTI_NEOFORGE_CrashReportExtender_DumpModLoadingCrashReport(const LIBMATTI_ML_Logger *logger,
                                                                     LIBMATTI_FML_ModLoadingIssue **issues,
                                                                     size_t issueCount, const char *topLevelDir)
{
    // Java: new File(topLevelDir, "crash-reports") + crashReport.saveToFile creates the parents
    char reportsDir[512];
    snprintf(reportsDir, sizeof(reportsDir), "%s/crash-reports", topLevelDir);
    LIBMATTI_JNF_Files_CreateDirectories(reportsDir);

    // Java: the file name carries the timestamp
    char timestamp[32];
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H.%M.%S", local);

    char filePath[768];
    snprintf(filePath, sizeof(filePath), "%s/crash-%s-fml.txt", reportsDir, timestamp);

    FILE *file = fopen(filePath, "w");
    if (file == NULL)
    {
        LIBMATTI_ML_Logger_Fatal(logger, NULL, "Failed to save crash report");
        return NULL;
    }

    // Java: CrashReport.forThrowable(new ModLoadingCrashException("Mod loading has failed"), "Mod loading failures have occurred; ...")
    fprintf(file, "---- Minecraft Crash Report ----\n");
    fprintf(file, "// Mod loading failures have occurred; consult the issue messages for more details\n\n");
    fprintf(file, "Time: %s\n", timestamp);
    fprintf(file, "Description: Mod loading failures have occurred\n");

    for (size_t i = 0; i < issueCount; i++)
    {
        const LIBMATTI_FML_ModLoadingIssue *issue = issues[i];

        // Java: "Mod loading issue for: <modid>" or "Mod loading issue"
        fprintf(file, "\n-- Mod loading issue --\n");

        // Java: the failure message is the translated issue, control codes stripped
        char *translated = LIBMATTI_FML_FMLTranslations_TranslateIssueEnglish(issue);
        char *stripped = LIBMATTI_FML_FMLTranslations_StripControlCodes(translated != NULL ? translated : issue->translationKey);
        write_detail(file, "Failure message", stripped);
        free(translated);
        free(stripped);

        // Java: Failure message arg N on the translation fallback
        for (size_t a = 0; a < issue->translationArgCount; a++)
        {
            char name[32];
            snprintf(name, sizeof(name), "Failure message arg %zu", a + 1);
            write_detail(file, name, issue->translationArgs[a]);
        }

        // Java: Mod file / Mod version / Mod issues URL / Exception message
        if (issue->affectedPath != NULL)
            write_detail(file, "Mod file", issue->affectedPath);
        else
            write_detail(file, "Mod file", "<No mod information provided>");
        write_detail(file, "Mod version", "<No mod information provided>");
        write_detail(file, "Mod issues URL", "<No issues URL found>");
        write_detail(file, "Exception message", issue->cause != NULL ? "see the log for the stack trace" : "<No associated exception found>");
    }

    fclose(file);

    LIBMATTI_ML_Logger_Fatal(logger, NULL, "Crash report saved to {}", filePath);

    // Java: Bootstrap.realStdoutPrintln(crashReport.getFriendlyReport(ReportType.CRASH))
    char *contents = LIBMATTI_JNF_Files_ReadString(filePath);
    if (contents != NULL)
    {
        fprintf(stdout, "%s\n", contents);
        fflush(stdout);
        free(contents);
    }

    // Java: return file2
    return strdup(filePath);
}
