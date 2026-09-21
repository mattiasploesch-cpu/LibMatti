// Port of the org.spongepowered.asm.service interfaces the FML layer implements
// (sponge-mixin 0.17.3+mixin.0.8.7): IPropertyKey, IGlobalPropertyService, IMixinInternal,
// IMixinService, IMixinServiceBootstrap, IClassTracker, IMixinAuditTrail, IFeatureValidator,
// IAdviceProvider and IClassProvider.
// The IMixinService interface is the port's virtual-dispatch struct; the FML implementation
// (FMLMixinService) fills it. The transformer-facing interfaces (ITransformerProvider,
// IClassBytecodeProvider) stay out until the transformer is ported - the native backend
// needs none of them.

#ifndef MATTICRAFT_SP_ASM_SERVICE_IMIXINSERVICE_H
#define MATTICRAFT_SP_ASM_SERVICE_IMIXINSERVICE_H

#include "libmatti/org/spongepowered/asm/logging/ILogger.h"
#include "libmatti/org/spongepowered/asm/mixin/MixinEnvironment.h"

#include <stddef.h>

typedef struct LIBMATTI_JL_Throwable LIBMATTI_JL_Throwable;
typedef struct LIBMATTI_JL_Class LIBMATTI_JL_Class;
typedef struct LIBMATTI_SP_ILogger LIBMATTI_SP_ILogger;
typedef struct LIBMATTI_SP_ReEntranceLock LIBMATTI_SP_ReEntranceLock;
typedef struct LIBMATTI_SP_ContainerHandle LIBMATTI_SP_ContainerHandle;
typedef struct LIBMATTI_SP_MixinEnvironment_Legacy LIBMATTI_SP_MixinEnvironment_Legacy;

// Java: public interface IPropertyKey - the FML implementation keys by name
typedef struct
{
    const char *name;
} LIBMATTI_SP_IPropertyKey;

// Java: public interface IGlobalPropertyService ("blackboard")
typedef struct LIBMATTI_SP_IGlobalPropertyService LIBMATTI_SP_IGlobalPropertyService;

struct LIBMATTI_SP_IGlobalPropertyService
{
    void *self;

    // Java: IPropertyKey resolveKey(String name)
    LIBMATTI_SP_IPropertyKey (*resolveKey)(void *self, const char *name);
    // Java: <T> T getProperty(IPropertyKey key) - NULL when unset
    void *(*getProperty)(void *self, LIBMATTI_SP_IPropertyKey key);
    // Java: void setProperty(IPropertyKey key, Object value)
    void (*setProperty)(void *self, LIBMATTI_SP_IPropertyKey key, void *value);
    // Java: String getPropertyString(IPropertyKey key, String defaultValue)
    const char *(*getPropertyString)(void *self, LIBMATTI_SP_IPropertyKey key, const char *defaultValue);
};

// Java: public interface IMixinInternal (marker the transformer offers itself through)
typedef struct LIBMATTI_SP_IMixinInternal LIBMATTI_SP_IMixinInternal;

struct LIBMATTI_SP_IMixinInternal
{
    void *self;
    // Java: the transformer factory offer; the port's native backend offers nothing, so the
    // callback is what a future transformer port would hook
    void (*offer)(void *self);
};

// Java: public interface IMixinService
typedef struct LIBMATTI_SP_IMixinService LIBMATTI_SP_IMixinService;

struct LIBMATTI_SP_IMixinService
{
    void *self;

    // Java: void prepare()
    void (*prepare)(void *self);
    // Java: Phase getInitialPhase()
    LIBMATTI_SP_MixinEnvironment_Phase (*getInitialPhase)(void *self);
    // Java: void offer(IMixinInternal internal)
    void (*offer)(void *self, LIBMATTI_SP_IMixinInternal *internal);
    // Java: void init()
    void (*init)(void *self);
    // Java: void beginPhase()
    void (*beginPhase)(void *self);
    // Java: void checkEnv(Object bootSource)
    void (*checkEnv)(void *self, void *bootSource);
    // Java: ReEntranceLock getReEntranceLock()
    LIBMATTI_SP_ReEntranceLock *(*getReEntranceLock)(void *self);
    // Java: String getSideName()
    const char *(*getSideName)(void *self);
    // Java: String getName()
    const char *(*getName)(void *self);
    // Java: boolean isValid()
    int (*isValid)(void *self);
    // Java: CompatibilityLevel getMinCompatibilityLevel()
    LIBMATTI_SP_MixinEnvironment_CompatibilityLevel (*getMinCompatibilityLevel)(void *self);
    // Java: CompatibilityLevel getMaxCompatibilityLevel() - NULL-equivalent = NONE
    LIBMATTI_SP_MixinEnvironment_CompatibilityLevel (*getMaxCompatibilityLevel)(void *self);
    // Java: ILogger getLogger(String name)
    LIBMATTI_SP_ILogger *(*getLogger)(void *self, const char *name);
    // Java: Collection<String> getPlatformAgents()
    const char *const *(*getPlatformAgents)(void *self, size_t *count);
    // Java: IContainerHandle getPrimaryContainer()
    LIBMATTI_SP_ContainerHandle *(*getPrimaryContainer)(void *self);
    // Java: Collection<IContainerHandle> getMixinContainers()
    LIBMATTI_SP_ContainerHandle **(*getMixinContainers)(void *self, size_t *count);
    // Java: InputStream getResourceAsStream(String name) - the port hands back the bytes read
    unsigned char *(*getResourceAsStream)(void *self, const char *name, size_t *outLength);
    // Java: IClassTracker getClassTracker()
    void *(*getClassTracker)(void *self);
    // Java: IMixinAuditTrail getAuditTrail()
    void *(*getAuditTrail)(void *self);
    // Java: IFeatureValidator getFeatureValidator()
    void *(*getFeatureValidator)(void *self);
    // Java: IAdviceProvider getAdviceProvider()
    void *(*getAdviceProvider)(void *self);
    // Java: ITransformerProvider getTransformerProvider() - NULL in FML
    void *(*getTransformerProvider)(void *self);
    // Java: IClassProvider getClassProvider()
    void *(*getClassProvider)(void *self);
    // Java: IClassBytecodeProvider getBytecodeProvider() - throws when unset; NULL = unset
    void *(*getBytecodeProvider)(void *self);
};

// Java: public interface IMixinServiceBootstrap
typedef struct LIBMATTI_SP_IMixinServiceBootstrap LIBMATTI_SP_IMixinServiceBootstrap;

struct LIBMATTI_SP_IMixinServiceBootstrap
{
    void *self;
    // Java: String getName()
    const char *(*getName)(void *self);
    // Java: String getServiceClassName()
    const char *(*getServiceClassName)(void *self);
    // Java: void bootstrap()
    void (*bootstrap)(void *self);
};

// Java: IClassTracker (FMLClassTracker)
typedef struct LIBMATTI_SP_IClassTracker LIBMATTI_SP_IClassTracker;

struct LIBMATTI_SP_IClassTracker
{
    void *self;
    // Java: void registerInvalidClass(String className)
    void (*registerInvalidClass)(void *self, const char *className);
    // Java: boolean isClassLoaded(String className)
    int (*isClassLoaded)(void *self, const char *className);
    // Java: String getClassRestrictions(String className)
    const char *(*getClassRestrictions)(void *self, const char *className);
};

// Java: IMixinAuditTrail (FMLAuditTrail)
typedef struct LIBMATTI_SP_IMixinAuditTrail LIBMATTI_SP_IMixinAuditTrail;

struct LIBMATTI_SP_IMixinAuditTrail
{
    void *self;
    // Java: void onApply(String className, String mixinName)
    void (*onApply)(void *self, const char *className, const char *mixinName);
    // Java: void onPostProcess(String className)
    void (*onPostProcess)(void *self, const char *className);
    // Java: void onGenerate(String className, String generatorName)
    void (*onGenerate)(void *self, const char *className, const char *generatorName);
    // Java: void setConsumer(String className, BiConsumer<String, String[]> consumer)
    void (*setConsumer)(void *self, const char *className,
                        void (*consumer)(const char *activity, const char **context, size_t contextCount,
                                         void *userdata), void *userdata);
};

// Java: IFeatureValidator
typedef struct LIBMATTI_SP_IFeatureValidator LIBMATTI_SP_IFeatureValidator;

struct LIBMATTI_SP_IFeatureValidator
{
    void *self;
    // Java: void validateEnumExtension(IMixinInfo mixin, ClassInfo targetClass) - the port has no
    // mixin/target ClassInfo; the native backend never extends Java enums, so the check stays out
    void (*validateEnumExtension)(void *self, void *mixin, void *targetClass);
};

// Java: IAdviceProvider
typedef struct LIBMATTI_SP_IAdviceProvider LIBMATTI_SP_IAdviceProvider;

struct LIBMATTI_SP_IAdviceProvider
{
    void *self;
    // Java: String higherCompatibilityNeeded(int requiredCompatibility, String requiredCompatibilityString)
    char *(*higherCompatibilityNeeded)(void *self, int requiredCompatibility,
                                       const char *requiredCompatibilityString);
};

// Java: IClassProvider
typedef struct LIBMATTI_SP_IClassProvider LIBMATTI_SP_IClassProvider;

struct LIBMATTI_SP_IClassProvider
{
    void *self;
    // Java: URL[] getClassPath() (deprecated) - an empty array in FML
    void (*getClassPath)(void *self, size_t *count);
    // Java: Class<?> findClass(String name, boolean initialize) - NULL when absent
    LIBMATTI_JL_Class *(*findClass)(void *self, const char *name, int initialize);
    // Java: Class<?> findAgentClass(String name, boolean initialize)
    LIBMATTI_JL_Class *(*findAgentClass)(void *self, const char *name, int initialize);
};

// Java: public static void boot()
void LIBMATTI_SP_MixinService_Boot(void);
// Java: public static IMixinService getService()
LIBMATTI_SP_IMixinService *LIBMATTI_SP_MixinService_GetService(void);
// Java: public static IGlobalPropertyService getGlobalPropertyService()
LIBMATTI_SP_IGlobalPropertyService *LIBMATTI_SP_MixinService_GetGlobalPropertyService(void);

#endif //MATTICRAFT_SP_ASM_SERVICE_IMIXINSERVICE_H
