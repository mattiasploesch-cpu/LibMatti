// Port of cpw.mods.modlauncher.ArgumentHandler.

#include "libmatti/cpw/modlauncher/ArgumentHandler.h"

#include "libmatti/joptsimple/PathConverter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: the anonymous ITransformationService.OptionResult returned by optionResults(...)
typedef struct
{
    LIBMATTI_MLA_ITransformationService_OptionResult base;
    const char *serviceName;
    LIBMATTI_JOPT_OptionSet *set;
} OptionResult;

static const char *option_result_value(LIBMATTI_MLA_ITransformationService_OptionResult *self,
                                       const LIBMATTI_JOPT_OptionSpec *option);
static char **option_result_values(LIBMATTI_MLA_ITransformationService_OptionResult *self,
                                   const LIBMATTI_JOPT_OptionSpec *option, size_t *count);

// Java: private <V> void checkOwnership(OptionSpec<V> option)
static int check_ownership(const OptionResult *result, const LIBMATTI_JOPT_OptionSpec *option)
{
    // Java: option.options().stream().allMatch(opt -> opt.startsWith(serviceName + ".") || !opt.contains("."))
    const char *name = LIBMATTI_JOPT_OptionSpec_Options(option);
    if (strchr(name, '.') == NULL) return 1;

    size_t serviceLength = strlen(result->serviceName);
    return strncmp(name, result->serviceName, serviceLength) == 0 && name[serviceLength] == '.';
}

static const char *option_result_value(LIBMATTI_MLA_ITransformationService_OptionResult *self,
                                       const LIBMATTI_JOPT_OptionSpec *option)
{
    OptionResult *result = (OptionResult *)self;
    if (!check_ownership(result, option)) return NULL;
    return LIBMATTI_JOPT_OptionSet_ValueOf(result->set, option);
}

static char **option_result_values(LIBMATTI_MLA_ITransformationService_OptionResult *self,
                                   const LIBMATTI_JOPT_OptionSpec *option, size_t *count)
{
    OptionResult *result = (OptionResult *)self;
    if (!check_ownership(result, option))
    {
        *count = 0;
        return NULL;
    }
    return LIBMATTI_JOPT_OptionSet_ValuesOf(result->set, option, count);
}

// Java: private ITransformationService.OptionResult optionResults(String serviceName, OptionSet set)
LIBMATTI_MLA_ITransformationService_OptionResult *LIBMATTI_ML_ArgumentHandler_OptionResults(
    const char *serviceName, LIBMATTI_JOPT_OptionSet *set, void *userdata)
{
    (void)userdata;
    OptionResult *result = calloc(1, sizeof(OptionResult));
    result->base.value = option_result_value;
    result->base.values = option_result_values;
    result->serviceName = serviceName;
    result->set = set;
    return &result->base;
}

void LIBMATTI_ML_ArgumentHandler_FreeOptionResults(LIBMATTI_MLA_ITransformationService_OptionResult *result)
{
    if (result == NULL) return;
    free(result);
}

LIBMATTI_ML_ArgumentHandler *LIBMATTI_ML_ArgumentHandler_New(void)
{
    return calloc(1, sizeof(LIBMATTI_ML_ArgumentHandler));
}

void LIBMATTI_ML_ArgumentHandler_Free(LIBMATTI_ML_ArgumentHandler *handler)
{
    if (handler == NULL) return;
    for (size_t i = 0; i < handler->argCount; i++) free(handler->args[i]);
    free(handler->args);
    free(handler);
}

// Java: DiscoveryData setArgs(String[] args)
LIBMATTI_ML_ArgumentHandler_DiscoveryData LIBMATTI_ML_ArgumentHandler_SetArgs(LIBMATTI_ML_ArgumentHandler *handler,
                                                                             int argc, char *argv[])
{
    handler->args = malloc(sizeof(*handler->args) * (size_t)(argc > 0 ? argc : 1));
    handler->argCount = (size_t)argc;
    for (int i = 0; i < argc; i++) handler->args[i] = strdup(argv[i]);

    LIBMATTI_JOPT_OptionParser *parser = LIBMATTI_JOPT_OptionParser_New();
    LIBMATTI_JOPT_PathConverter *converter = LIBMATTI_JOPT_PathConverter_New(
        LIBMATTI_JOPT_PATH_DIRECTORY_EXISTING);

    LIBMATTI_JOPT_OptionSpec *gameDir = LIBMATTI_JOPT_OptionSpec_DefaultsTo(
        LIBMATTI_JOPT_OptionSpec_WithValuesConvertedBy(
            LIBMATTI_JOPT_OptionSpec_WithRequiredArg(
                LIBMATTI_JOPT_OptionParser_Accepts(parser, "gameDir", "Alternative game directory")),
            converter),
        ".");
    LIBMATTI_JOPT_OptionSpec *launchTarget = LIBMATTI_JOPT_OptionSpec_WithRequiredArg(
        LIBMATTI_JOPT_OptionParser_Accepts(parser, "launchTarget", "LauncherService target to launch"));

    LIBMATTI_JOPT_OptionParser_AllowsUnrecognizedOptions(parser);
    LIBMATTI_JOPT_OptionSet *optionSet = LIBMATTI_JOPT_OptionParser_Parse(parser, argc, argv);

    LIBMATTI_ML_ArgumentHandler_DiscoveryData data;
    data.gameDir = strdup(LIBMATTI_JOPT_OptionSet_ValueOf(optionSet, gameDir));
    const char *target = LIBMATTI_JOPT_OptionSet_ValueOf(optionSet, launchTarget);
    data.launchTarget = target != NULL ? strdup(target) : NULL;
    data.arguments = argv;
    data.argumentCount = (size_t)argc;

    LIBMATTI_JOPT_OptionSet_Free(optionSet);
    LIBMATTI_JOPT_OptionParser_Free(parser);
    LIBMATTI_JOPT_PathConverter_Free(converter);

    return data;
}

// the env.computePropertyIfAbsent value functions
typedef struct
{
    const LIBMATTI_JOPT_OptionSpec *option;
    LIBMATTI_JOPT_OptionSet *set;
} OptionValueSupplier;

static void *supply_option_value(LIBMATTI_MLA_Key *key, void *userdata)
{
    (void)key;
    OptionValueSupplier *supplier = userdata;
    return (void *)LIBMATTI_JOPT_OptionSet_ValueOf(supplier->set, supplier->option);
}

void LIBMATTI_ML_ArgumentHandler_ProcessArguments(
    LIBMATTI_ML_ArgumentHandler *handler, LIBMATTI_ML_Environment *env,
    void (*parserConsumer)(LIBMATTI_JOPT_OptionParser *parser, void *userdata), void *parserUserdata,
    void (*resultConsumer)(LIBMATTI_JOPT_OptionSet *optionSet,
                           LIBMATTI_MLA_ITransformationService_OptionResult *(*resultHandler)(
                               const char *serviceName, LIBMATTI_JOPT_OptionSet *set, void *userdata),
                           void *userdata),
    void *resultUserdata)
{
    LIBMATTI_JOPT_OptionParser *parser = LIBMATTI_JOPT_OptionParser_New();
    LIBMATTI_JOPT_OptionParser_AllowsUnrecognizedOptions(parser);

    handler->profileOption = LIBMATTI_JOPT_OptionSpec_WithRequiredArg(
        LIBMATTI_JOPT_OptionParser_Accepts(parser, "version", "The version we launched with"));
    handler->gameDirOption = LIBMATTI_JOPT_OptionSpec_DefaultsTo(
        LIBMATTI_JOPT_OptionSpec_WithValuesConvertedBy(
            LIBMATTI_JOPT_OptionSpec_WithRequiredArg(
                LIBMATTI_JOPT_OptionParser_Accepts(parser, "gameDir", "Alternative game directory")),
            LIBMATTI_JOPT_PathConverter_New(LIBMATTI_JOPT_PATH_DIRECTORY_EXISTING)),
        ".");
    handler->assetsDirOption = LIBMATTI_JOPT_OptionSpec_WithValuesConvertedBy(
        LIBMATTI_JOPT_OptionSpec_WithRequiredArg(
            LIBMATTI_JOPT_OptionParser_Accepts(parser, "assetsDir", "Assets directory")),
        LIBMATTI_JOPT_PathConverter_New(LIBMATTI_JOPT_PATH_DIRECTORY_EXISTING));
    handler->uuidOption = LIBMATTI_JOPT_OptionSpec_WithRequiredArg(
        LIBMATTI_JOPT_OptionParser_Accepts(parser, "uuid", "The UUID of the logging in player"));
    handler->launchTarget = LIBMATTI_JOPT_OptionSpec_WithRequiredArg(
        LIBMATTI_JOPT_OptionParser_Accepts(parser, "launchTarget", "LauncherService target to launch"));

    // Java: parserConsumer.accept(parser)
    if (parserConsumer != NULL) parserConsumer(parser, parserUserdata);

    handler->nonOption = LIBMATTI_JOPT_OptionParser_NonOptions(parser);
    handler->optionSet = LIBMATTI_JOPT_OptionParser_Parse(parser, (int)handler->argCount, handler->args);

    OptionValueSupplier version = {handler->profileOption, handler->optionSet};
    OptionValueSupplier gameDir = {handler->gameDirOption, handler->optionSet};
    OptionValueSupplier assetsDir = {handler->assetsDirOption, handler->optionSet};
    OptionValueSupplier launchTarget = {handler->launchTarget, handler->optionSet};
    OptionValueSupplier uuid = {handler->uuidOption, handler->optionSet};

    LIBMATTI_ML_Environment_ComputePropertyIfAbsent(env, LIBMATTI_MLA_IEnvironment_Keys_Version(), supply_option_value,
                                                    &version);
    LIBMATTI_ML_Environment_ComputePropertyIfAbsent(env, LIBMATTI_MLA_IEnvironment_Keys_GameDir(), supply_option_value,
                                                    &gameDir);
    LIBMATTI_ML_Environment_ComputePropertyIfAbsent(env, LIBMATTI_MLA_IEnvironment_Keys_AssetsDir(),
                                                    supply_option_value, &assetsDir);
    LIBMATTI_ML_Environment_ComputePropertyIfAbsent(env, LIBMATTI_MLA_IEnvironment_Keys_LaunchTarget(),
                                                    supply_option_value, &launchTarget);
    LIBMATTI_ML_Environment_ComputePropertyIfAbsent(env, LIBMATTI_MLA_IEnvironment_Keys_Uuid(), supply_option_value,
                                                    &uuid);

    // Java: resultConsumer.accept(this.optionSet, this::optionResults)
    if (resultConsumer != NULL) resultConsumer(handler->optionSet, LIBMATTI_ML_ArgumentHandler_OptionResults,
                                               resultUserdata);
}

const char *LIBMATTI_ML_ArgumentHandler_GetLaunchTarget(const LIBMATTI_ML_ArgumentHandler *handler)
{
    return LIBMATTI_JOPT_OptionSet_ValueOf(handler->optionSet, handler->launchTarget);
}

// Java: private void addOptionToString(OptionSpec<?> option, OptionSet optionSet, List<String> appendTo)
static void add_option_to_string(const LIBMATTI_JOPT_OptionSpec *option, const LIBMATTI_JOPT_OptionSet *optionSet,
                                 char ***appendTo, size_t *count)
{
    if (!LIBMATTI_JOPT_OptionSet_Has(optionSet, option)) return;

    const char *value = LIBMATTI_JOPT_OptionSet_ValueOf(optionSet, option);
    if (value == NULL) return;

    char *name = malloc(strlen(LIBMATTI_JOPT_OptionSpec_Options(option)) + 3);
    sprintf(name, "--%s", LIBMATTI_JOPT_OptionSpec_Options(option));

    *appendTo = realloc(*appendTo, sizeof(**appendTo) * (*count + 2));
    (*appendTo)[(*count)++] = name;
    (*appendTo)[(*count)++] = strdup(value);
}

char **LIBMATTI_ML_ArgumentHandler_BuildArgumentList(const LIBMATTI_ML_ArgumentHandler *handler, size_t *count)
{
    char **args = NULL;
    size_t argCount = 0;

    add_option_to_string(handler->profileOption, handler->optionSet, &args, &argCount);
    add_option_to_string(handler->gameDirOption, handler->optionSet, &args, &argCount);
    add_option_to_string(handler->assetsDirOption, handler->optionSet, &args, &argCount);
    add_option_to_string(handler->uuidOption, handler->optionSet, &args, &argCount);

    // Java: this.optionSet.nonOptionArguments().stream().map(Object::toString).forEach(args::add)
    size_t nonOptionCount = 0;
    char **nonOptions = LIBMATTI_JOPT_OptionSet_NonOptionArguments(handler->optionSet, &nonOptionCount);

    for (size_t i = 0; i < nonOptionCount; i++)
    {
        args = realloc(args, sizeof(*args) * (argCount + 1));
        args[argCount++] = strdup(nonOptions[i]);
    }

    *count = argCount;
    return args;
}
