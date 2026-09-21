// Port of net.neoforged.fml.common.asm.enumextension.EnumProxy.

#include "libmatti/net/neoforged/fml/common/asm/enumextension/EnumProxy.h"

#include <stdlib.h>
#include <string.h>

// Java: public EnumProxy(Class<T> enumClass, Object... parameters)
LIBMATTI_FML_EnumProxy *LIBMATTI_FML_EnumProxy_New(const char *enumClassName, void **parameters,
                                                   size_t parameterCount)
{
    LIBMATTI_FML_EnumProxy *proxy = calloc(1, sizeof(LIBMATTI_FML_EnumProxy));
    proxy->enumClassName = enumClassName != NULL ? strdup(enumClassName) : NULL;

    if (parameterCount > 0)
    {
        proxy->parameters = malloc(sizeof(void *) * parameterCount);
        for (size_t i = 0; i < parameterCount; i++) proxy->parameters[i] = parameters[i];
        proxy->parameterCount = parameterCount;
    }

    return proxy;
}

void LIBMATTI_FML_EnumProxy_Free(LIBMATTI_FML_EnumProxy *proxy)
{
    if (proxy == NULL) return;
    free(proxy->enumClassName);
    free(proxy->parameters);
    free(proxy);
}

// Java: @ApiStatus.Internal public Object getParameter(int idx) { return parameters.get(idx); }
void *LIBMATTI_FML_EnumProxy_GetParameter(const LIBMATTI_FML_EnumProxy *proxy, int index)
{
    if (proxy == NULL || index < 0 || (size_t)index >= proxy->parameterCount) return NULL;
    return proxy->parameters[index];
}

// Java: @ApiStatus.Internal public void setValue(T value) { this.enumValue = value; }
void LIBMATTI_FML_EnumProxy_SetValue(LIBMATTI_FML_EnumProxy *proxy, void *value)
{
    if (proxy == NULL) return;
    proxy->enumValue = value;
}
