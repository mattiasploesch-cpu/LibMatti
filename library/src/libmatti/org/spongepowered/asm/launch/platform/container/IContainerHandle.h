// Port of org.spongepowered.asm.launch.platform.container.IContainerHandle
// (sponge-mixin 0.17.3+mixin.0.8.7). The interface extends IMixinConfigSource; Java's interface
// becomes the virtual-dispatch struct the C port uses for its interfaces.

#ifndef MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_CONTAINER_ICONTAINERHANDLE_H
#define MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_CONTAINER_ICONTAINERHANDLE_H

#include <stddef.h>

// Java: public interface IContainerHandle extends IMixinConfigSource
typedef struct LIBMATTI_SP_ContainerHandle LIBMATTI_SP_ContainerHandle;

struct LIBMATTI_SP_ContainerHandle
{
    void *self;

    // Java (IMixinConfigSource): String getId()
    const char *(*getId)(void *self);
    // Java (IMixinConfigSource): String getDescription()
    const char *(*getDescription)(void *self);
    // Java: String getAttribute(String name) - NULL when not present
    const char *(*getAttribute)(void *self, const char *name);
    // Java: Collection<IContainerHandle> getNestedContainers() - the port fills a caller array
    LIBMATTI_SP_ContainerHandle **(*getNestedContainers)(void *self, size_t *count);
};

#endif //MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_CONTAINER_ICONTAINERHANDLE_H
