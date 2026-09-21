// Port of net.neoforged.fml.loading.mixin.MixinFacade.
// TODO: org.spongepowered.asm.launch.MixinBootstrap (external)
// TODO: org.spongepowered.asm.mixin.MixinEnvironment / Mixins / Config (external)
// Java's FMLMixinClassProcessor / FMLMixinGeneratingClassProcessor weave Java bytecode (external);
// the port's mixin backend is the native hook table (libmatti/matti/mixin/MixinHookTable.h) plus the
// GOT patcher, so the two processors below are the table's collectors instead.

#ifndef MATTICRAFT_FML_LOADING_MIXIN_MIXINFACADE_H
#define MATTICRAFT_FML_LOADING_MIXIN_MIXINFACADE_H

#include "libmatti/net/neoforged/fml/classloading/transformation/TransformingClassLoader.h"
#include "libmatti/net/neoforged/fml/loading/LoadingModList.h"
#include "libmatti/net/neoforged/neoforgespi/language/MavenVersionAdapter.h"

#include <stddef.h>

// Java: public static final int DEFAULT_BEHAVIOUR_VERSION = FabricUtil.COMPATIBILITY_0_17_1
// FabricUtil encodes the version as major * 1000 * 1000 + minor * 1000 + patch, so
// COMPATIBILITY_0_17_1 is 17001.
extern int LIBMATTI_FML_MixinFacade_DEFAULT_BEHAVIOUR_VERSION;
// Java: HIGHEST_MIXIN_VERSION = FabricUtil.class.getModule().getDescriptor().version()
//       (the "Implementation-Version" of the running mixin is the fallback)
// The port has no module system, so the version of the mixin the launcher runs against is set here.
#define LIBMATTI_FML_MixinFacade_MIXIN_VERSION "0.17.3+mixin.0.8.7"
// Java: public static final ArtifactVersion HIGHEST_MIXIN_VERSION
extern LIBMATTI_NEOFORGESPI_ArtifactVersion *LIBMATTI_FML_MixinFacade_HIGHEST_MIXIN_VERSION;
// Java: public static final ArtifactVersion LOWEST_MIXIN_VERSION
extern LIBMATTI_NEOFORGESPI_ArtifactVersion *LIBMATTI_FML_MixinFacade_LOWEST_MIXIN_VERSION;

// Java: the Mixin config the facade accepted; FMLMixinService.addMixinConfigContent carries
// the same two pieces for Mixin, and the decorators are Config.decorate's KEY_MOD_ID and
// KEY_COMPATIBILITY.
typedef struct
{
    char *name;
    unsigned char *content;
    size_t contentLength;
    char *modId;
    int behaviorVersion;
} LIBMATTI_FML_MixinFacade_MixinConfig;

typedef struct LIBMATTI_FML_MixinFacade LIBMATTI_FML_MixinFacade;

struct LIBMATTI_FML_MixinFacade
{
    // Java's processors weave bytecode (external); the port collects hook-table descriptors.
    void *classProcessor;
    void *generatingClassProcessor;
    // TODO: net.neoforged.fml.loading.mixin.FMLMixinService
    void *service;
    // Java: Mixins.getConfigs()
    LIBMATTI_FML_MixinFacade_MixinConfig *configs;
    size_t configCount;
    // The native mixin backend: the target/hook chain of the process
    struct LIBMATTI_MIXIN_HookTable *hookTable;
};

// Java: public MixinFacade()
LIBMATTI_FML_MixinFacade *LIBMATTI_FML_MixinFacade_New(void);

// Java: public FMLMixinClassProcessor getClassProcessor()
void *LIBMATTI_FML_MixinFacade_GetClassProcessor(const LIBMATTI_FML_MixinFacade *facade);
// Java: public FMLMixinGeneratingClassProcessor getGeneratingClassProcessor()
void *LIBMATTI_FML_MixinFacade_GetGeneratingClassProcessor(const LIBMATTI_FML_MixinFacade *facade);
// Java: Mixins.getConfigs().stream().collect(toMap(Config::getName, Config::getConfig))
LIBMATTI_FML_MixinFacade_MixinConfig *LIBMATTI_FML_MixinFacade_GetConfig(const LIBMATTI_FML_MixinFacade *facade,
                                                                         const char *name);

// Java: public void finishInitialization(LoadingModList loadingModList, TransformingClassLoader classLoader)
void LIBMATTI_FML_MixinFacade_FinishInitialization(LIBMATTI_FML_MixinFacade *facade,
                                                   LIBMATTI_FML_LoadingModList *loadingModList,
                                                   LIBMATTI_FML_TransformingClassLoader *classLoader);

// Java: private static int calculateBehaviorVersion(ArtifactVersion behaviorVersion)
int LIBMATTI_FML_MixinFacade_CalculateBehaviorVersion(const LIBMATTI_NEOFORGESPI_ArtifactVersion *behaviorVersion);

// The native mixin backend the facade collects into (M4 hook table + M2 GOT patches)
struct LIBMATTI_MIXIN_HookTable *LIBMATTI_FML_MixinFacade_GetHookTable(const LIBMATTI_FML_MixinFacade *facade);

// Java: public void close()
void LIBMATTI_FML_MixinFacade_Close(LIBMATTI_FML_MixinFacade *facade);
void LIBMATTI_FML_MixinFacade_Free(LIBMATTI_FML_MixinFacade *facade);

#endif //MATTICRAFT_FML_LOADING_MIXIN_MIXINFACADE_H
