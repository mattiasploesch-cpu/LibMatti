// Port of org.spongepowered.asm.launch.platform.container.ContainerHandleVirtual.

#include "libmatti/org/spongepowered/asm/launch/platform/container/ContainerHandleVirtual.h"

#include <stdlib.h>
#include <string.h>

// Java: public String getId() { return this.name; }
static const char *get_id(void *self)
{
    LIBMATTI_SP_ContainerHandleVirtual *handle = self;
    return handle->name;
}

// Java: public String getDescription() { return this.toString(); }
static const char *get_description(void *self)
{
    LIBMATTI_SP_ContainerHandleVirtual *handle = self;
    return handle->name;
}

// Java: public String getAttribute(String name) { return this.attributes.get(name); }
static const char *get_attribute(void *self, const char *name)
{
    LIBMATTI_SP_ContainerHandleVirtual *handle = self;
    for (size_t i = 0; i < handle->attributeCount; i++)
        if (strcmp(handle->attributeKeys[i], name) == 0) return handle->attributeValues[i];

    return NULL;
}

// Java: public Collection<IContainerHandle> getNestedContainers()
static LIBMATTI_SP_ContainerHandle **get_nested_containers(void *self, size_t *count)
{
    LIBMATTI_SP_ContainerHandleVirtual *handle = self;
    *count = handle->nestedContainerCount;
    return handle->nestedContainers;
}

// Java: public ContainerHandleVirtual(String name)
LIBMATTI_SP_ContainerHandleVirtual *LIBMATTI_SP_ContainerHandleVirtual_New(const char *name)
{
    LIBMATTI_SP_ContainerHandleVirtual *handle = calloc(1, sizeof(*handle));
    handle->name = strdup(name);

    handle->base.self = handle;
    handle->base.getId = get_id;
    handle->base.getDescription = get_description;
    handle->base.getAttribute = get_attribute;
    handle->base.getNestedContainers = get_nested_containers;
    return handle;
}

// Java: public ContainerHandleVirtual setAttribute(String key, String value)
LIBMATTI_SP_ContainerHandleVirtual *LIBMATTI_SP_ContainerHandleVirtual_SetAttribute(
    LIBMATTI_SP_ContainerHandleVirtual *handle, const char *key, const char *value)
{
    handle->attributeKeys = realloc(handle->attributeKeys, sizeof(char *) * (handle->attributeCount + 1));
    handle->attributeValues = realloc(handle->attributeValues, sizeof(char *) * (handle->attributeCount + 1));
    handle->attributeKeys[handle->attributeCount] = strdup(key);
    handle->attributeValues[handle->attributeCount] = strdup(value);
    handle->attributeCount++;
    return handle;
}

// Java: public ContainerHandleVirtual add(IContainerHandle nested)
LIBMATTI_SP_ContainerHandleVirtual *LIBMATTI_SP_ContainerHandleVirtual_Add(
    LIBMATTI_SP_ContainerHandleVirtual *handle, LIBMATTI_SP_ContainerHandle *nested)
{
    // Java: a LinkedHashSet; the port skips duplicates by pointer
    for (size_t i = 0; i < handle->nestedContainerCount; i++)
        if (handle->nestedContainers[i] == nested) return handle;

    handle->nestedContainers =
        realloc(handle->nestedContainers, sizeof(LIBMATTI_SP_ContainerHandle *) * (handle->nestedContainerCount + 1));
    handle->nestedContainers[handle->nestedContainerCount++] = nested;
    return handle;
}

void LIBMATTI_SP_ContainerHandleVirtual_Free(LIBMATTI_SP_ContainerHandleVirtual *handle)
{
    if (handle == NULL) return;
    free(handle->name);
    for (size_t i = 0; i < handle->attributeCount; i++)
    {
        free(handle->attributeKeys[i]);
        free(handle->attributeValues[i]);
    }
    free(handle->attributeKeys);
    free(handle->attributeValues);
    free(handle->nestedContainers);
    free(handle);
}
