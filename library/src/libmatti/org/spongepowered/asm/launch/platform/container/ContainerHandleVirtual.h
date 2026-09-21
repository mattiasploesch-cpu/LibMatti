// Port of org.spongepowered.asm.launch.platform.container.ContainerHandleVirtual.
// "A virtual container, used to marshal other containers around"

#ifndef MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_CONTAINER_CONTAINERHANDLEVIRTUAL_H
#define MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_CONTAINER_CONTAINERHANDLEVIRTUAL_H

#include "libmatti/org/spongepowered/asm/launch/platform/container/IContainerHandle.h"

#include <stddef.h>

// Java: public class ContainerHandleVirtual implements IContainerHandle
typedef struct LIBMATTI_SP_ContainerHandleVirtual LIBMATTI_SP_ContainerHandleVirtual;

struct LIBMATTI_SP_ContainerHandleVirtual
{
    LIBMATTI_SP_ContainerHandle base;

    // Java: private final String name
    char *name;
    // Java: private final Map<String, String> attributes
    char **attributeKeys;
    char **attributeValues;
    size_t attributeCount;
    // Java: private final Set<IContainerHandle> nestedContainers
    LIBMATTI_SP_ContainerHandle **nestedContainers;
    size_t nestedContainerCount;
};

// Java: public ContainerHandleVirtual(String name)
LIBMATTI_SP_ContainerHandleVirtual *LIBMATTI_SP_ContainerHandleVirtual_New(const char *name);
void LIBMATTI_SP_ContainerHandleVirtual_Free(LIBMATTI_SP_ContainerHandleVirtual *handle);

// Java: public String getName()
const char *LIBMATTI_SP_ContainerHandleVirtual_GetName(const LIBMATTI_SP_ContainerHandleVirtual *handle);
// Java: public ContainerHandleVirtual setAttribute(String key, String value)
LIBMATTI_SP_ContainerHandleVirtual *LIBMATTI_SP_ContainerHandleVirtual_SetAttribute(
    LIBMATTI_SP_ContainerHandleVirtual *handle, const char *key, const char *value);
// Java: public ContainerHandleVirtual add(IContainerHandle nested)
LIBMATTI_SP_ContainerHandleVirtual *LIBMATTI_SP_ContainerHandleVirtual_Add(
    LIBMATTI_SP_ContainerHandleVirtual *handle, LIBMATTI_SP_ContainerHandle *nested);

#endif //MATTICRAFT_SP_ASM_LAUNCH_PLATFORM_CONTAINER_CONTAINERHANDLEVIRTUAL_H
