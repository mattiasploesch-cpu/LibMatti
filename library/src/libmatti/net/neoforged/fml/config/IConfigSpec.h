// Port of net.neoforged.fml.config.IConfigSpec.
// Java's interface becomes the virtual-dispatch struct the C port uses for its interfaces
// (the same convention as IModFile / JarResource): 'self' plus one function pointer per method.
// NeoForge provides ModConfigSpec for the common cases; that class is the game port's part,
// so callers bring their own spec implementation built with LIBMATTI_FML_IConfigSpec_Of.

#ifndef MATTICRAFT_FML_CONFIG_ICONFIGSPEC_H
#define MATTICRAFT_FML_CONFIG_ICONFIGSPEC_H

#include "libmatti/com/electronwill/nightconfig/core/Config.h"

#include <stddef.h>

// Java: interface IConfigSpec.ILoadedConfig (permits LoadedConfig)
typedef struct LIBMATTI_FML_ILoadedConfig LIBMATTI_FML_ILoadedConfig;

struct LIBMATTI_FML_ILoadedConfig
{
    void *self;
    // Java: CommentedConfig config()
    LIBMATTI_NC_Config *(*config)(void *self);
    // Java: void save()
    void (*save)(void *self);
};

// Java: public interface IConfigSpec
typedef struct LIBMATTI_FML_IConfigSpec LIBMATTI_FML_IConfigSpec;

struct LIBMATTI_FML_IConfigSpec
{
    void *self;
    // Java: boolean isEmpty()
    int (*isEmpty)(void *self);
    // Java: void validateSpec(ModConfig config)
    void (*validateSpec)(void *self, void *config);
    // Java: boolean isCorrect(UnmodifiableCommentedConfig config)
    int (*isCorrect)(void *self, const LIBMATTI_NC_Config *config);
    // Java: void correct(CommentedConfig config)
    void (*correct)(void *self, LIBMATTI_NC_Config *config);
    // Java: void acceptConfig(@Nullable ILoadedConfig config)
    void (*acceptConfig)(void *self, LIBMATTI_FML_ILoadedConfig *config);
};

// Builds a spec interface from an implementation (the port's virtual dispatch, like
// LIBMATTI_NEOFORGESPI_IModFile_New in neoforgespi).
LIBMATTI_FML_IConfigSpec LIBMATTI_FML_IConfigSpec_Of(void *self,
                                                     int (*isEmpty)(void *self),
                                                     void (*validateSpec)(void *self, void *config),
                                                     int (*isCorrect)(void *self, const LIBMATTI_NC_Config *config),
                                                     void (*correct)(void *self, LIBMATTI_NC_Config *config),
                                                     void (*acceptConfig)(void *self,
                                                                          LIBMATTI_FML_ILoadedConfig *config));

#endif //MATTICRAFT_FML_CONFIG_ICONFIGSPEC_H
