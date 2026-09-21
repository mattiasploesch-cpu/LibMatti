// Port of net.neoforged.fml.common.asm.enumextension.ExtensionInfo.

#include "libmatti/net/neoforged/fml/common/asm/enumextension/ExtensionInfo.h"

#include <stdlib.h>

// Java: public ExtensionInfo(boolean extended, int vanillaCount, int totalCount, NetworkCheck netCheck)
LIBMATTI_FML_ExtensionInfo *LIBMATTI_FML_ExtensionInfo_New(int extended, int vanillaCount, int totalCount,
                                                           LIBMATTI_FML_NetworkedEnum_NetworkCheck *netCheck)
{
    LIBMATTI_FML_ExtensionInfo *info = calloc(1, sizeof(LIBMATTI_FML_ExtensionInfo));
    info->extended = extended;
    info->vanillaCount = vanillaCount;
    info->totalCount = totalCount;
    info->netCheck = netCheck;
    return info;
}

// Java: public static <T> ExtensionInfo nonExtended(Class<T> enumClass)
// Java reads the @NetworkedEnum annotation off the class; the port is handed that annotation's value
// because the class lookup (Class.getDeclaredAnnotation) is JVM-backed.
LIBMATTI_FML_ExtensionInfo *LIBMATTI_FML_ExtensionInfo_NonExtended(
    LIBMATTI_FML_NetworkedEnum_NetworkCheck *netCheck)
{
    return LIBMATTI_FML_ExtensionInfo_New(0, 0, 0, netCheck);
}

void LIBMATTI_FML_ExtensionInfo_Free(LIBMATTI_FML_ExtensionInfo *info)
{
    free(info);
}
