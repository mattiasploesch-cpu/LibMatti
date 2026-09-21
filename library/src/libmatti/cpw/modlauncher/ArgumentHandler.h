// Port of cpw.mods.modlauncher.ArgumentHandler.

#ifndef MATTICRAFT_MODLAUNCHER_ARGUMENTHANDLER_H
#define MATTICRAFT_MODLAUNCHER_ARGUMENTHANDLER_H

#include "libmatti/cpw/modlauncher/Environment.h"
#include "libmatti/cpw/modlauncher/api/ITransformationService.h"
#include "libmatti/joptsimple/OptionParser.h"
#include "libmatti/joptsimple/OptionSet.h"

#include <stddef.h>

// Java: record DiscoveryData(Path gameDir, String launchTarget, String[] arguments)
typedef struct
{
    char *gameDir;
    char *launchTarget;
    char **arguments;
    size_t argumentCount;
} LIBMATTI_ML_ArgumentHandler_DiscoveryData;

// Java: public class ArgumentHandler
typedef struct LIBMATTI_ML_ArgumentHandler
{
    // Java: private String[] args
    char **args;
    size_t argCount;
    // Java: private OptionSet optionSet
    LIBMATTI_JOPT_OptionSet *optionSet;
    LIBMATTI_JOPT_OptionSpec *profileOption;
    LIBMATTI_JOPT_OptionSpec *gameDirOption;
    LIBMATTI_JOPT_OptionSpec *assetsDirOption;
    LIBMATTI_JOPT_OptionSpec *nonOption;
    LIBMATTI_JOPT_OptionSpec *launchTarget;
    LIBMATTI_JOPT_OptionSpec *uuidOption;
} LIBMATTI_ML_ArgumentHandler;

// Java: ArgumentHandler()
LIBMATTI_ML_ArgumentHandler *LIBMATTI_ML_ArgumentHandler_New(void);
void LIBMATTI_ML_ArgumentHandler_Free(LIBMATTI_ML_ArgumentHandler *handler);

// Java: DiscoveryData setArgs(String[] args)
LIBMATTI_ML_ArgumentHandler_DiscoveryData LIBMATTI_ML_ArgumentHandler_SetArgs(LIBMATTI_ML_ArgumentHandler *handler,
                                                                             int argc, char *argv[]);

// Java: void processArguments(Environment env, Consumer<OptionParser> parserConsumer,
//                             BiConsumer<OptionSet, BiFunction<String, OptionSet, ITransformationService.OptionResult>> resultConsumer)
void LIBMATTI_ML_ArgumentHandler_ProcessArguments(
    LIBMATTI_ML_ArgumentHandler *handler, LIBMATTI_ML_Environment *env,
    void (*parserConsumer)(LIBMATTI_JOPT_OptionParser *parser, void *userdata), void *parserUserdata,
    void (*resultConsumer)(LIBMATTI_JOPT_OptionSet *optionSet,
                           LIBMATTI_MLA_ITransformationService_OptionResult *(*resultHandler)(
                               const char *serviceName, LIBMATTI_JOPT_OptionSet *set, void *userdata),
                           void *userdata),
    void *resultUserdata);

// Java: private ITransformationService.OptionResult optionResults(String serviceName, OptionSet set)
LIBMATTI_MLA_ITransformationService_OptionResult *LIBMATTI_ML_ArgumentHandler_OptionResults(
    const char *serviceName, LIBMATTI_JOPT_OptionSet *set, void *userdata);
// releases a result returned by OptionResults
void LIBMATTI_ML_ArgumentHandler_FreeOptionResults(LIBMATTI_MLA_ITransformationService_OptionResult *result);

// Java: String getLaunchTarget()
const char *LIBMATTI_ML_ArgumentHandler_GetLaunchTarget(const LIBMATTI_ML_ArgumentHandler *handler);
// Java: public String[] buildArgumentList()
char **LIBMATTI_ML_ArgumentHandler_BuildArgumentList(const LIBMATTI_ML_ArgumentHandler *handler, size_t *count);

#endif //MATTICRAFT_MODLAUNCHER_ARGUMENTHANDLER_H
