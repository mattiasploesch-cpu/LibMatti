// Port of org.spongepowered.asm.launch.MixinBootstrap (sponge-mixin 0.17.3+mixin.0.8.7).
// "Bootstraps the mixin subsystem. This class acts as a bridge between the mixin subsystem and
// the tweaker or coremod which is bootstrapping it."

#ifndef MATTICRAFT_SP_ASM_LAUNCH_MIXINBOOTSTRAP_H
#define MATTICRAFT_SP_ASM_LAUNCH_MIXINBOOTSTRAP_H

#include <stddef.h>

// Java: public static final String VERSION = "0.8.7";
#define LIBMATTI_SP_MixinBootstrap_VERSION "0.8.7"

typedef struct LIBMATTI_SP_CommandLineOptions LIBMATTI_SP_CommandLineOptions;
typedef struct LIBMATTI_SP_MixinPlatformManager LIBMATTI_SP_MixinPlatformManager;

// Java: public static void addProxy() (@Deprecated; delegates to beginPhase)
void LIBMATTI_SP_MixinBootstrap_AddProxy(void);
// Java: public static MixinPlatformManager getPlatform()
LIBMATTI_SP_MixinPlatformManager *LIBMATTI_SP_MixinBootstrap_GetPlatform(void);
// Java: public static void init()
void LIBMATTI_SP_MixinBootstrap_Init(void);
// Java: static boolean start() (package-private, phase 1)
int LIBMATTI_SP_MixinBootstrap_Start(void);
// Java: @Deprecated static void doInit(List<String> args) -> doInit(CommandLineOptions)
void LIBMATTI_SP_MixinBootstrap_DoInitArgs(char **args, size_t argCount);
// Java: static void doInit(CommandLineOptions args) (package-private, phase 2)
void LIBMATTI_SP_MixinBootstrap_DoInit(LIBMATTI_SP_CommandLineOptions *args);
// Java: static void inject() (package-private)
void LIBMATTI_SP_MixinBootstrap_Inject(void);

#endif //MATTICRAFT_SP_ASM_LAUNCH_MIXINBOOTSTRAP_H
