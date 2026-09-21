// Port of org.spongepowered.asm.service.MixinService (the singleton front of the service layer).
// The service and the property service are implemented in libmatti/net/neoforged/fml/loading/mixin
// (FMLMixinService / FMLMixinGlobalProperties), the singleton here resolves them through the
// mixin.bootstrapService / mixin.service system properties exactly like Java.

#ifndef MATTICRAFT_SP_ASM_SERVICE_MIXINSERVICE_H
#define MATTICRAFT_SP_ASM_SERVICE_MIXINSERVICE_H

// The declarations live in IMixinService.h (Boot/GetService/GetGlobalPropertyService);
// this header exists for the include the Java import graph suggests (service -> MixinService).

#include "libmatti/org/spongepowered/asm/service/IMixinService.h"

#endif //MATTICRAFT_SP_ASM_SERVICE_MIXINSERVICE_H
