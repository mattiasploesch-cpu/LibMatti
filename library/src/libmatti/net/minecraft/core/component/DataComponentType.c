// Port of net.minecraft.core.component.DataComponentType.

#include "libmatti/net/minecraft/core/component/DataComponentType.h"

#include "libmatti/cpw/modlauncher/LogManager.h"

#include <stdlib.h>
#include <string.h>

// Java: DataComponentType.builder().persistent(codec).build() / .networkSynchronized(...)
LIBMATTI_MC_DataComponentType *LIBMATTI_MC_DataComponentType_Create(const char *name, void *codec)
{
    LIBMATTI_MC_DataComponentType *type = calloc(1, sizeof(LIBMATTI_MC_DataComponentType));
    type->name = strdup(name);
    type->codec = codec;
    return type;
}

// Java: the builder without .persistent(...) - a transient (network-only) component
LIBMATTI_MC_DataComponentType *LIBMATTI_MC_DataComponentType_CreateTransient(const char *name)
{
    return LIBMATTI_MC_DataComponentType_Create(name, NULL);
}

// Java: default boolean isTransient()
bool LIBMATTI_MC_DataComponentType_IsTransient(const LIBMATTI_MC_DataComponentType *type)
{
    return type->codec == NULL;
}

// Java: default Codec<T> codecOrThrow() - IllegalStateException on transient
void *LIBMATTI_MC_DataComponentType_CodecOrThrow(const LIBMATTI_MC_DataComponentType *type)
{
    if (type->codec == NULL)
    {
        LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
        LIBMATTI_ML_Logger_Error(logger, NULL, "{} is not a persistent component", type->name);
        exit(1);
    }
    return type->codec;
}
