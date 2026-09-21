// Port of net.neoforged.fml.loading.mixin.FMLMixinLogger.
// Java: public class FMLMixinLogger implements ILogger - maps the Mixin Level ordinals to the
// launcher's log levels. The port's LogManager logger replaces log4j2 directly.

#ifndef MATTICRAFT_FML_LOADING_MIXIN_FMLMIXINLOGGER_H
#define MATTICRAFT_FML_LOADING_MIXIN_FMLMIXINLOGGER_H

#include "libmatti/org/spongepowered/asm/logging/ILogger.h"

// Java: public FMLMixinLogger(String name)
LIBMATTI_SP_ILogger *LIBMATTI_FML_FMLMixinLogger_Instance(const char *name);

#endif //MATTICRAFT_FML_LOADING_MIXIN_FMLMIXINLOGGER_H
