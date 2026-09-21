// Port of net.neoforged.fml.common.asm.enumextension.EnumProxy.
// Java: public final class EnumProxy<T extends Enum<T> & IExtensibleEnum> holds the constructor
// parameters of a modded enum constant and the constant once the enum initialised it.
// The ported launcher emits calls to getParameter/setValue from the transformed enum, so the
// descriptor constants below are the part the transformer needs; the parameter storage mirrors the
// Java field layout for callers inside the port.

#ifndef MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_ENUMPROXY_H
#define MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_ENUMPROXY_H

#include <stddef.h>

// Java: Type.getType(EnumProxy.class).getInternalName()
#define LIBMATTI_FML_EnumProxy_INTERNAL_NAME "net/neoforged/fml/common/asm/enumextension/EnumProxy"

// Java: Type.getType(EnumProxy.class).getDescriptor()
#define LIBMATTI_FML_EnumProxy_DESCRIPTOR "Lnet/neoforged/fml/common/asm/enumextension/EnumProxy;"

// Java: EnumProxy.getParameter(I)Ljava/lang/Object;
#define LIBMATTI_FML_EnumProxy_GET_PARAMETER_NAME "getParameter"
#define LIBMATTI_FML_EnumProxy_GET_PARAMETER_DESCRIPTOR "(I)Ljava/lang/Object;"

// Java: EnumProxy.setValue(Ljava/lang/Enum;)V
#define LIBMATTI_FML_EnumProxy_SET_VALUE_NAME "setValue"
#define LIBMATTI_FML_EnumProxy_SET_VALUE_DESCRIPTOR "(Ljava/lang/Enum;)V"

// Java: public final class EnumProxy<T extends Enum<T> & IExtensibleEnum>
typedef struct LIBMATTI_FML_EnumProxy
{
    // Java: private final Class<T> enumClass
    char *enumClassName;
    // Java: private final List<Object> parameters - the port cannot hold arbitrary Java objects
    void **parameters;
    size_t parameterCount;
    // Java: private volatile T enumValue
    void *enumValue;
} LIBMATTI_FML_EnumProxy;

// Java: public EnumProxy(Class<T> enumClass, Object... parameters)
LIBMATTI_FML_EnumProxy *LIBMATTI_FML_EnumProxy_New(const char *enumClassName, void **parameters,
                                                   size_t parameterCount);
void LIBMATTI_FML_EnumProxy_Free(LIBMATTI_FML_EnumProxy *proxy);

// Java: @ApiStatus.Internal public Object getParameter(int idx)
void *LIBMATTI_FML_EnumProxy_GetParameter(const LIBMATTI_FML_EnumProxy *proxy, int index);
// Java: @ApiStatus.Internal public void setValue(T value)
void LIBMATTI_FML_EnumProxy_SetValue(LIBMATTI_FML_EnumProxy *proxy, void *value);

#endif //MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_ENUMPROXY_H
