// Port of org.spongepowered.asm.util.Constants (the parts the FML layer uses).

#ifndef MATTICRAFT_SP_ASM_UTIL_CONSTANTS_H
#define MATTICRAFT_SP_ASM_UTIL_CONSTANTS_H

// Java: public static final String SIDE_CLIENT = "CLIENT"
#define LIBMATTI_SP_Constants_SIDE_CLIENT "CLIENT"
// Java: public static final String SIDE_SERVER = "SERVER"
#define LIBMATTI_SP_Constants_SIDE_SERVER "SERVER"
// Java: public static final String SIDE_DEDICATEDSERVER = "DEDICATEDSERVER"
#define LIBMATTI_SP_Constants_SIDE_DEDICATEDSERVER "DEDICATEDSERVER"
// Java: public static final String SIDE_UNKNOWN = "UNKNOWN"
#define LIBMATTI_SP_Constants_SIDE_UNKNOWN "UNKNOWN"

// Java: public abstract static class ManifestAttributes
#define LIBMATTI_SP_Constants_ATTR_TWEAKER "TweakClass"
#define LIBMATTI_SP_Constants_ATTR_MAINCLASS "Main-Class"
#define LIBMATTI_SP_Constants_ATTR_MIXINCONFIGS "MixinConfigs"
#define LIBMATTI_SP_Constants_ATTR_TOKENPROVIDERS "MixinTokenProviders"
#define LIBMATTI_SP_Constants_ATTR_MIXINCONNECTOR "MixinConnector"
#define LIBMATTI_SP_Constants_ATTR_COMPATIBILITY "MixinCompatibilityLevel"

#endif //MATTICRAFT_SP_ASM_UTIL_CONSTANTS_H
