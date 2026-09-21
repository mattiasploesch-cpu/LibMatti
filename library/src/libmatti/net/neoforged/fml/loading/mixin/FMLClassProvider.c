// Ports of net.neoforged.fml.loading.mixin.FMLClassProvider,
// net.neoforged.fml.loading.mixin.FMLMixinFeatureValidator and
// net.neoforged.fml.loading.mixin.FMLMixinAdviceProvider.

#include "libmatti/net/neoforged/fml/loading/mixin/FMLClassProvider.h"

#include "libmatti/java/lang/Class.h"
#include "libmatti/net/neoforged/fml/loading/FMLLoader.h"
#include "libmatti/org/spongepowered/asm/service/IMixinService.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// FMLClassProvider - "Class provider for use under ModLauncher"
// ---------------------------------------------------------------------------

// Java: @Deprecated public URL[] getClassPath() { return new URL[0]; }
static void class_provider_get_class_path(void *self, size_t *count)
{
    (void) self;
    *count = 0;
}

// Java: public Class<?> findClass(String name, boolean initialize)
//       { return Class.forName(name, initialize, FMLLoader.getCurrent().getCurrentClassLoader()); }
static LIBMATTI_JL_Class *class_provider_find_class(void *self, const char *name, int initialize)
{
    (void) self;
    return LIBMATTI_JL_Class_ForName(name, initialize,
                                     LIBMATTI_FML_FMLLoader_GetCurrentClassLoader(LIBMATTI_FML_FMLLoader_GetCurrent()));
}

// Java: public Class<?> findAgentClass(String name, boolean initialize)
//       { return Class.forName(name, initialize, getClass().getClassLoader()); }
static LIBMATTI_JL_Class *class_provider_find_agent_class(void *self, const char *name, int initialize)
{
    (void) self;
    return LIBMATTI_JL_Class_ForName(name, initialize, NULL);
}

// Java: class FMLClassProvider implements IClassProvider
LIBMATTI_SP_IClassProvider *LIBMATTI_FML_FMLClassProvider_Instance(void)
{
    LIBMATTI_SP_IClassProvider *provider = calloc(1, sizeof(*provider));
    provider->self = provider;
    provider->getClassPath = class_provider_get_class_path;
    provider->findClass = class_provider_find_class;
    provider->findAgentClass = class_provider_find_agent_class;
    return provider;
}

// ---------------------------------------------------------------------------
// FMLMixinFeatureValidator
// ---------------------------------------------------------------------------

// Java: public void validateEnumExtension(IMixinInfo mixin, ClassInfo targetClass)
// The port has no mixin/target ClassInfo; the native backend never extends Java enums.
static void feature_validator_validate_enum_extension(void *self, void *mixin, void *targetClass)
{
    (void) self;
    (void) mixin;
    (void) targetClass;
}

// Java: class FMLMixinFeatureValidator implements IFeatureValidator
LIBMATTI_SP_IFeatureValidator *LIBMATTI_FML_FMLMixinFeatureValidator_Instance(void)
{
    LIBMATTI_SP_IFeatureValidator *validator = calloc(1, sizeof(*validator));
    validator->self = validator;
    validator->validateEnumExtension = feature_validator_validate_enum_extension;
    return validator;
}

// ---------------------------------------------------------------------------
// FMLMixinAdviceProvider
// ---------------------------------------------------------------------------

// Java: public String higherCompatibilityNeeded(int requiredCompatibility, String requiredCompatibilityString)
//       { return "Set `behaviorVersion = \"%s\"` or higher on your mixin config in your neoforge.mods.toml"
//                .formatted(requiredCompatibilityString); }
static char *advice_provider_higher_compatibility_needed(void *self, int requiredCompatibility,
                                                         const char *requiredCompatibilityString)
{
    (void) self;
    (void) requiredCompatibility;

    static const char FORMAT[] = "Set `behaviorVersion = \"%s\"` or higher on your mixin config in your neoforge.mods.toml";
    char *message = malloc(sizeof(FORMAT) + strlen(requiredCompatibilityString));
    sprintf(message, FORMAT, requiredCompatibilityString);
    return message;
}

// Java: class FMLMixinAdviceProvider implements IAdviceProvider
LIBMATTI_SP_IAdviceProvider *LIBMATTI_FML_FMLMixinAdviceProvider_Instance(void)
{
    LIBMATTI_SP_IAdviceProvider *provider = calloc(1, sizeof(*provider));
    provider->self = provider;
    provider->higherCompatibilityNeeded = advice_provider_higher_compatibility_needed;
    return provider;
}
