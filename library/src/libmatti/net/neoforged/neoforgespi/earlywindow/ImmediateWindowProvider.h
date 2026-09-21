// Port of net.neoforged.neoforgespi.earlywindow.ImmediateWindowProvider.

#ifndef MATTICRAFT_NEOFORGESPI_EARLYWINDOW_IMMEDIATEWINDOWPROVIDER_H
#define MATTICRAFT_NEOFORGESPI_EARLYWINDOW_IMMEDIATEWINDOWPROVIDER_H

#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"
#include "libmatti/net/neoforged/fml/loading/EarlyLoadingScreenController.h"
#include "libmatti/net/neoforged/fml/loading/ProgramArgs.h"

#include <stddef.h>

// Java: public interface ImmediateWindowProvider extends EarlyLoadingScreenController
typedef struct LIBMATTI_NEOFORGESPI_ImmediateWindowProvider LIBMATTI_NEOFORGESPI_ImmediateWindowProvider;

struct LIBMATTI_NEOFORGESPI_ImmediateWindowProvider
{
    // Java: extends EarlyLoadingScreenController
    LIBMATTI_FML_EarlyLoadingScreenController controller;

    // Java: String name()
    const char *(*name)(void *self);
    // Java: void initialize(ProgramArgs args)
    void (*initialize)(void *self, LIBMATTI_FML_ProgramArgs *args);
    // Java: void setMinecraftVersion(String version)
    void (*setMinecraftVersion)(void *self, const char *version);
    // Java: void setNeoForgeVersion(String version)
    void (*setNeoForgeVersion)(void *self, const char *version);
    // Java: void crash(String message)
    void (*crash)(void *self, const char *message);
    // Java: void displayFatalErrorAndExit(List<ModLoadingIssue> issues, Path modsFolder, Path logFile, Path crashReportFile)
    void (*displayFatalErrorAndExit)(void *self, LIBMATTI_FML_ModLoadingIssue **issues, size_t issueCount,
                                     const char *modsFolder, const char *logFile, const char *crashReportFile);
};

// Java: public String name()
const char *LIBMATTI_NEOFORGESPI_ImmediateWindowProvider_Name(
    const LIBMATTI_NEOFORGESPI_ImmediateWindowProvider *provider);
// Java: public void initialize(ProgramArgs args)
void LIBMATTI_NEOFORGESPI_ImmediateWindowProvider_Initialize(
    const LIBMATTI_NEOFORGESPI_ImmediateWindowProvider *provider, LIBMATTI_FML_ProgramArgs *args);
// Java: public void setMinecraftVersion(String version)
void LIBMATTI_NEOFORGESPI_ImmediateWindowProvider_SetMinecraftVersion(
    const LIBMATTI_NEOFORGESPI_ImmediateWindowProvider *provider, const char *version);
// Java: public void setNeoForgeVersion(String version)
void LIBMATTI_NEOFORGESPI_ImmediateWindowProvider_SetNeoForgeVersion(
    const LIBMATTI_NEOFORGESPI_ImmediateWindowProvider *provider, const char *version);
// Java: public void crash(String message)
void LIBMATTI_NEOFORGESPI_ImmediateWindowProvider_Crash(
    const LIBMATTI_NEOFORGESPI_ImmediateWindowProvider *provider, const char *message);
// Java: public void displayFatalErrorAndExit(List<ModLoadingIssue> issues, Path modsFolder, Path logFile, Path crashReportFile)
void LIBMATTI_NEOFORGESPI_ImmediateWindowProvider_DisplayFatalErrorAndExit(
    const LIBMATTI_NEOFORGESPI_ImmediateWindowProvider *provider, LIBMATTI_FML_ModLoadingIssue **issues,
    size_t issueCount, const char *modsFolder, const char *logFile, const char *crashReportFile);

#endif //MATTICRAFT_NEOFORGESPI_EARLYWINDOW_IMMEDIATEWINDOWPROVIDER_H
