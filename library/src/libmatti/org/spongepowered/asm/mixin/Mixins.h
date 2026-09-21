// Port of org.spongepowered.asm.mixin.Mixins (sponge-mixin 0.17.3+mixin.0.8.7).
// "Entry point for registering global mixin resources."

#ifndef MATTICRAFT_SP_ASM_MIXIN_MIXINS_H
#define MATTICRAFT_SP_ASM_MIXIN_MIXINS_H

#include "libmatti/org/spongepowered/asm/mixin/MixinEnvironment.h"
#include "libmatti/org/spongepowered/asm/mixin/transformer/Config.h"

#include <stddef.h>

// Java: public final class Mixins
// Java: public static void addConfigurations(String... configFiles)
void LIBMATTI_SP_Mixins_AddConfigurations(const char *const *configFiles, size_t count);
// Java: public static void addConfigurations(String[] configFiles, IMixinConfigSource source)
void LIBMATTI_SP_Mixins_AddConfigurationsWithSource(const char *const *configFiles, size_t count,
                                                    LIBMATTI_SP_ContainerHandle *source);
// Java: public static void addConfiguration(String configFile)
void LIBMATTI_SP_Mixins_AddConfiguration(const char *configFile);
// Java: public static void addConfiguration(String configFile, IMixinConfigSource source)
void LIBMATTI_SP_Mixins_AddConfigurationWithSource(const char *configFile, LIBMATTI_SP_ContainerHandle *source);
// Java: @Deprecated static void addConfiguration(String configFile, MixinEnvironment fallback)
void LIBMATTI_SP_Mixins_AddConfigurationWithFallback(const char *configFile, void *fallback);

// Java: public static int getUnvisitedCount()
int LIBMATTI_SP_Mixins_GetUnvisitedCount(void);
// Java: public static Set<Config> getConfigs() - the port hands back the growable array
// Java: public static Set<Config> getConfigs() - the port hands back the growable item array;
// the array is owned by the blackboard and must not be freed by the caller
LIBMATTI_SP_Config **LIBMATTI_SP_Mixins_GetConfigs(size_t *count);
// Java: public static void registerErrorHandlerClass(String handlerName)
void LIBMATTI_SP_Mixins_RegisterErrorHandlerClass(const char *handlerName);
// Java: public static Set<String> getErrorHandlerClasses()
const char *const *LIBMATTI_SP_Mixins_GetErrorHandlerClasses(size_t *count);

#endif //MATTICRAFT_SP_ASM_MIXIN_MIXINS_H
