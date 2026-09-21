#include "libmatti/net/neoforged/neoforgespi/earlywindow/ImmediateWindowProvider.h"

// Java: public String name()
const char *LIBMATTI_NEOFORGESPI_ImmediateWindowProvider_Name(
    const LIBMATTI_NEOFORGESPI_ImmediateWindowProvider *provider)
{
    return provider->name(provider->controller.self);
}

// Java: public void initialize(ProgramArgs args)
void LIBMATTI_NEOFORGESPI_ImmediateWindowProvider_Initialize(
    const LIBMATTI_NEOFORGESPI_ImmediateWindowProvider *provider, LIBMATTI_FML_ProgramArgs *args)
{
    provider->initialize(provider->controller.self, args);
}

// Java: public void setMinecraftVersion(String version)
void LIBMATTI_NEOFORGESPI_ImmediateWindowProvider_SetMinecraftVersion(
    const LIBMATTI_NEOFORGESPI_ImmediateWindowProvider *provider, const char *version)
{
    provider->setMinecraftVersion(provider->controller.self, version);
}

// Java: public void setNeoForgeVersion(String version)
void LIBMATTI_NEOFORGESPI_ImmediateWindowProvider_SetNeoForgeVersion(
    const LIBMATTI_NEOFORGESPI_ImmediateWindowProvider *provider, const char *version)
{
    provider->setNeoForgeVersion(provider->controller.self, version);
}

// Java: public void crash(String message)
void LIBMATTI_NEOFORGESPI_ImmediateWindowProvider_Crash(
    const LIBMATTI_NEOFORGESPI_ImmediateWindowProvider *provider, const char *message)
{
    provider->crash(provider->controller.self, message);
}

// Java: public void displayFatalErrorAndExit(List<ModLoadingIssue> issues, Path modsFolder, Path logFile, Path crashReportFile)
void LIBMATTI_NEOFORGESPI_ImmediateWindowProvider_DisplayFatalErrorAndExit(
    const LIBMATTI_NEOFORGESPI_ImmediateWindowProvider *provider, LIBMATTI_FML_ModLoadingIssue **issues,
    size_t issueCount, const char *modsFolder, const char *logFile, const char *crashReportFile)
{
    provider->displayFatalErrorAndExit(provider->controller.self, issues, issueCount, modsFolder, logFile,
                                       crashReportFile);
}
