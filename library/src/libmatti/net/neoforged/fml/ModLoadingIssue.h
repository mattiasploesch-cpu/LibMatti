// Port of net.neoforged.fml.ModLoadingIssue.

#ifndef MATTICRAFT_FML_MODLOADINGISSUE_H
#define MATTICRAFT_FML_MODLOADINGISSUE_H

#include "libmatti/java/lang/Throwable.h"

#include <stddef.h>

// Java: public enum Severity { WARNING, ERROR }
typedef enum
{
    LIBMATTI_FML_ModLoadingIssue_SEVERITY_WARNING,
    LIBMATTI_FML_ModLoadingIssue_SEVERITY_ERROR
} LIBMATTI_FML_ModLoadingIssue_Severity;

// Java: public record ModLoadingIssue(Severity severity, String translationKey, List<Object> translationArgs,
//         @Nullable Throwable cause, @Nullable Path affectedPath, @Nullable IModFile affectedModFile,
//         @Nullable IModInfo affectedMod)
typedef struct LIBMATTI_FML_ModLoadingIssue
{
    LIBMATTI_FML_ModLoadingIssue_Severity severity;
    char *translationKey;
    // Java: List<Object> translationArgs - the C port keeps the arguments as strings
    char **translationArgs;
    size_t translationArgCount;
    LIBMATTI_JL_Throwable *cause;
    char *affectedPath; // Java: Path affectedPath
    // TODO: net.neoforged.neoforgespi.locating.IModFile
    void *affectedModFile;
    // TODO: net.neoforged.neoforgespi.language.IModInfo
    void *affectedMod;
} LIBMATTI_FML_ModLoadingIssue;

// Java: the compact constructor rejects fml.* keys outside fml.modloadingissue.* (0 = rejected, like Java's throw)
int LIBMATTI_FML_ModLoadingIssue_ValidateTranslationKey(const char *translationKey);

// Java: public ModLoadingIssue(Severity severity, String translationKey, List<Object> translationArgs, Throwable cause, Path affectedPath, IModFile affectedModFile, IModInfo affectedMod)
LIBMATTI_FML_ModLoadingIssue *LIBMATTI_FML_ModLoadingIssue_New(LIBMATTI_FML_ModLoadingIssue_Severity severity,
                                                               const char *translationKey,
                                                               const char **translationArgs, size_t translationArgCount,
                                                               LIBMATTI_JL_Throwable *cause, const char *affectedPath,
                                                               void *affectedModFile, const void *affectedMod);
void LIBMATTI_FML_ModLoadingIssue_Free(LIBMATTI_FML_ModLoadingIssue *issue);

// Java: public static ModLoadingIssue error(String translationKey, Object... args)
LIBMATTI_FML_ModLoadingIssue *LIBMATTI_FML_ModLoadingIssue_Error(const char *translationKey,
                                                                 const char **args, size_t argCount);
// Java: public static ModLoadingIssue warning(String translationKey, Object... args)
LIBMATTI_FML_ModLoadingIssue *LIBMATTI_FML_ModLoadingIssue_Warning(const char *translationKey,
                                                                   const char **args, size_t argCount);

// Java: public ModLoadingIssue withAffectedPath(Path affectedPath)
LIBMATTI_FML_ModLoadingIssue *LIBMATTI_FML_ModLoadingIssue_WithAffectedPath(const LIBMATTI_FML_ModLoadingIssue *issue,
                                                                            const char *affectedPath);
// Java: public ModLoadingIssue withAffectedModFile(IModFile affectedModFile)
// 'filePath' is affectedModFile.getFilePath() (the port cannot derive it from the unported IModFile)
LIBMATTI_FML_ModLoadingIssue *LIBMATTI_FML_ModLoadingIssue_WithAffectedModFile(
    const LIBMATTI_FML_ModLoadingIssue *issue, void *affectedModFile, const char *filePath);
// Java: public ModLoadingIssue withAffectedMod(IModInfo affectedMod)
// 'affectedModFile' / 'filePath' are affectedMod.getOwningFile().getFile() and its getFilePath()
LIBMATTI_FML_ModLoadingIssue *LIBMATTI_FML_ModLoadingIssue_WithAffectedMod(const LIBMATTI_FML_ModLoadingIssue *issue,
                                                                           const void *affectedMod, void *affectedModFile,
                                                                           const char *filePath);
// Java: public ModLoadingIssue withCause(Throwable cause)
LIBMATTI_FML_ModLoadingIssue *LIBMATTI_FML_ModLoadingIssue_WithCause(const LIBMATTI_FML_ModLoadingIssue *issue,
                                                                     LIBMATTI_JL_Throwable *cause);
// Java: public ModLoadingIssue withSeverity(Severity severity)
LIBMATTI_FML_ModLoadingIssue *LIBMATTI_FML_ModLoadingIssue_WithSeverity(const LIBMATTI_FML_ModLoadingIssue *issue,
                                                                       LIBMATTI_FML_ModLoadingIssue_Severity severity);

// Java: @Override public String toString()
char *LIBMATTI_FML_ModLoadingIssue_ToString(const LIBMATTI_FML_ModLoadingIssue *issue);

#endif //MATTICRAFT_FML_MODLOADINGISSUE_H
