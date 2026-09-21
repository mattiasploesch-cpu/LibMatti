// Port of net.neoforged.fml.loading.mixin.FMLMixinService.
// Java: public class FMLMixinService implements IMixinService

#ifndef MATTICRAFT_FML_LOADING_MIXIN_FMLMIXINSERVICE_H
#define MATTICRAFT_FML_LOADING_MIXIN_FMLMIXINSERVICE_H

#include "libmatti/org/spongepowered/asm/launch/platform/container/IContainerHandle.h"
#include "libmatti/org/spongepowered/asm/service/IMixinService.h"

#include <stddef.h>

typedef struct LIBMATTI_NEOFORGESPI_IModFile LIBMATTI_NEOFORGESPI_IModFile;

// Java: class FMLMixinService implements IMixinService (the port's singleton)
LIBMATTI_SP_IMixinService *LIBMATTI_FML_FMLMixinService_Instance(void);

// Java: public void addMixinConfigContent(String config, byte[] resource)
void LIBMATTI_FML_FMLMixinService_AddMixinConfigContent(const char *config, const unsigned char *resource,
                                                        size_t length);
// Java: public void addMixinContainer(IContainerHandle handle)
void LIBMATTI_FML_FMLMixinService_AddMixinContainer(LIBMATTI_SP_ContainerHandle *handle);
// Java: @VisibleForTesting public void clearMixinContainers()
void LIBMATTI_FML_FMLMixinService_ClearMixinContainers(void);

// Java: public IMixinTransformer getMixinTransformer()
void *LIBMATTI_FML_FMLMixinService_GetMixinTransformer(void);
// Java: public void setBytecodeProvider(@Nullable IClassBytecodeProvider bytecodeProvider)
void LIBMATTI_FML_FMLMixinService_SetBytecodeProvider(void *provider);

#endif //MATTICRAFT_FML_LOADING_MIXIN_FMLMIXINSERVICE_H
