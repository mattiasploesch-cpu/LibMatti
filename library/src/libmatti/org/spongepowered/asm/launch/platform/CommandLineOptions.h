// Port of org.spongepowered.asm.launch.platform.CommandLineOptions.
// "Options passed in to Mixin via the command line"

#ifndef MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_COMMANDLINEOPTIONS_H
#define MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_COMMANDLINEOPTIONS_H

#include <stddef.h>

// Java: public final class CommandLineOptions
typedef struct LIBMATTI_SP_CommandLineOptions LIBMATTI_SP_CommandLineOptions;

struct LIBMATTI_SP_CommandLineOptions
{
    // Java: private List<String> configs
    char **configs;
    size_t configCount;
};

// Java: public static CommandLineOptions defaultArgs() - reads the sun.java.command property
LIBMATTI_SP_CommandLineOptions *LIBMATTI_SP_CommandLineOptions_DefaultArgs(void);
// Java: public static CommandLineOptions ofArgs(List<String> args) - NULL args = defaultArgs
LIBMATTI_SP_CommandLineOptions *LIBMATTI_SP_CommandLineOptions_OfArgs(const char *const *args, size_t count);
// Java: public static CommandLineOptions of(List<String> configs)
LIBMATTI_SP_CommandLineOptions *LIBMATTI_SP_CommandLineOptions_Of(const char *const *configs, size_t count);

void LIBMATTI_SP_CommandLineOptions_Free(LIBMATTI_SP_CommandLineOptions *options);

// Java: public List<String> getConfigs()
const char *const *LIBMATTI_SP_CommandLineOptions_GetConfigs(const LIBMATTI_SP_CommandLineOptions *options,
                                                             size_t *count);

#endif //MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_COMMANDLINEOPTIONS_H
