// Port of net.neoforged.fml.loading.ImmediateWindowHandler.

#ifndef MATTICRAFT_FML_IMMEDIATEWINDOWHANDLER_H
#define MATTICRAFT_FML_IMMEDIATEWINDOWHANDLER_H

#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"
#include "libmatti/net/neoforged/fml/loading/ProgramArgs.h"
#include "libmatti/net/neoforged/neoforgespi/ILaunchContext.h"

#include <stddef.h>

// Java: public static void load(ILaunchContext context, boolean headless, ProgramArgs arguments)
void LIBMATTI_FML_ImmediateWindowHandler_Load(LIBMATTI_NEOFORGESPI_ILaunchContext *context, int headless,
                                              LIBMATTI_FML_ProgramArgs *arguments);

// Java: static ImmediateWindowProvider provider - TODO: net.neoforged.neoforgespi.earlywindow.ImmediateWindowProvider
// NULL = no provider (Java: provider == null)
void *LIBMATTI_FML_ImmediateWindowHandler_GetProvider(void);

// Java: public static void setNeoForgeVersion(String version)
void LIBMATTI_FML_ImmediateWindowHandler_SetNeoForgeVersion(const char *version);
// Java: public static void setMinecraftVersion(String version)
void LIBMATTI_FML_ImmediateWindowHandler_SetMinecraftVersion(const char *version);
// Java: public static void renderTick()
void LIBMATTI_FML_ImmediateWindowHandler_RenderTick(void);
// Java: public static void updateProgress(String message)
void LIBMATTI_FML_ImmediateWindowHandler_UpdateProgress(const char *message);
// Java: public static void crash(String message)
void LIBMATTI_FML_ImmediateWindowHandler_Crash(const char *message);
// Java: public static void displayFatalErrorAndExit(List<ModLoadingIssue> issues, Path modsFolder, Path logFile, Path crashReportFile)
void LIBMATTI_FML_ImmediateWindowHandler_DisplayFatalErrorAndExit(LIBMATTI_FML_ModLoadingIssue **issues, size_t issueCount,
                                                                  const char *modsFolder, const char *logFile,
                                                                  const char *crashReportFile);

#endif //MATTICRAFT_FML_IMMEDIATEWINDOWHANDLER_H
