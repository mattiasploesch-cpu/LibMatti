// Port of org.spongepowered.asm.launch.platform.CommandLineOptions.

#include "libmatti/org/spongepowered/asm/launch/platform/CommandLineOptions.h"

#include "libmatti/java/lang/System.h"

#include <stdlib.h>
#include <string.h>

// Java: private void parseArgs(List<String> args)
static void parse_args(LIBMATTI_SP_CommandLineOptions *options, const char *const *args, size_t count)
{
    int captureNext = 0;
    for (size_t i = 0; i < count; i++)
    {
        if (captureNext)
        {
            options->configs = realloc(options->configs, sizeof(char *) * (options->configCount + 1));
            options->configs[options->configCount++] = strdup(args[i]);
        }
        captureNext = strcmp("--mixin", args[i]) == 0 || strcmp("--mixin.config", args[i]) == 0;
    }
}

static LIBMATTI_SP_CommandLineOptions *options_new(void)
{
    return calloc(1, sizeof(LIBMATTI_SP_CommandLineOptions));
}

// Java: public static CommandLineOptions defaultArgs() { return CommandLineOptions.ofArgs(null); }
LIBMATTI_SP_CommandLineOptions *LIBMATTI_SP_CommandLineOptions_DefaultArgs(void)
{
    // Java: String argv = System.getProperty("sun.java.command"); if (argv != null) args = argv.split(" ");
    LIBMATTI_SP_CommandLineOptions *options = options_new();
    const char *argv = LIBMATTI_JL_System_GetProperty("sun.java.command");
    if (argv != NULL)
    {
        // Java: Arrays.asList(argv.split(" "))
        char *split = strdup(argv);
        const char **tokens = NULL;
        size_t tokenCount = 0;
        char *saveptr = NULL;
        for (char *token = strtok_r(split, " ", &saveptr); token != NULL; token = strtok_r(NULL, " ", &saveptr))
        {
            tokens = realloc(tokens, sizeof(char *) * (tokenCount + 1));
            tokens[tokenCount++] = token;
        }
        parse_args(options, tokens, tokenCount);
        free(tokens);
        free(split);
    }
    return options;
}

// Java: public static CommandLineOptions ofArgs(List<String> args)
LIBMATTI_SP_CommandLineOptions *LIBMATTI_SP_CommandLineOptions_OfArgs(const char *const *args, size_t count)
{
    if (args == NULL) return LIBMATTI_SP_CommandLineOptions_DefaultArgs();

    LIBMATTI_SP_CommandLineOptions *options = options_new();
    parse_args(options, args, count);
    return options;
}

// Java: public static CommandLineOptions of(List<String> configs)
LIBMATTI_SP_CommandLineOptions *LIBMATTI_SP_CommandLineOptions_Of(const char *const *configs, size_t count)
{
    LIBMATTI_SP_CommandLineOptions *options = options_new();
    for (size_t i = 0; i < count; i++)
    {
        options->configs = realloc(options->configs, sizeof(char *) * (options->configCount + 1));
        options->configs[options->configCount++] = strdup(configs[i]);
    }
    return options;
}

void LIBMATTI_SP_CommandLineOptions_Free(LIBMATTI_SP_CommandLineOptions *options)
{
    if (options == NULL) return;
    for (size_t i = 0; i < options->configCount; i++) free(options->configs[i]);
    free(options->configs);
    free(options);
}

// Java: public List<String> getConfigs()
const char *const *LIBMATTI_SP_CommandLineOptions_GetConfigs(const LIBMATTI_SP_CommandLineOptions *options,
                                                             size_t *count)
{
    *count = options->configCount;
    return (const char *const *) options->configs;
}
