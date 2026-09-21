// Port of net.neoforged.fml.common.asm.enumextension.ExtensionInfo.

#ifndef MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_EXTENSIONINFO_H
#define MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_EXTENSIONINFO_H

#include "libmatti/net/neoforged/fml/common/asm/enumextension/NetworkedEnum.h"

// Java: Type.getType(ExtensionInfo.class).getInternalName()
#define LIBMATTI_FML_ExtensionInfo_INTERNAL_NAME "net/neoforged/fml/common/asm/enumextension/ExtensionInfo"

// Java: Type.getType(ExtensionInfo.class).getDescriptor()
#define LIBMATTI_FML_ExtensionInfo_DESCRIPTOR "Lnet/neoforged/fml/common/asm/enumextension/ExtensionInfo;"

// Java: Type.getMethodDescriptor(EXT_INFO) - the getExtensionInfo() getter descriptor
#define LIBMATTI_FML_ExtensionInfo_GETTER_DESCRIPTOR "()Lnet/neoforged/fml/common/asm/enumextension/ExtensionInfo;"

// Java: Type.getMethodDescriptor(VOID_TYPE, BOOLEAN_TYPE, INT_TYPE, INT_TYPE, NET_CHECK) - the record constructor
#define LIBMATTI_FML_ExtensionInfo_CONSTRUCTOR_DESCRIPTOR \
    "(ZIILnet/neoforged/fml/common/asm/enumextension/NetworkedEnum$NetworkCheck;)V"

// Java: public record ExtensionInfo(boolean extended, int vanillaCount, int totalCount, @Nullable NetworkedEnum.NetworkCheck netCheck)
typedef struct LIBMATTI_FML_ExtensionInfo
{
    int extended;
    int vanillaCount;
    int totalCount;
    // Java: @Nullable NetworkedEnum.NetworkCheck netCheck
    LIBMATTI_FML_NetworkedEnum_NetworkCheck *netCheck;
} LIBMATTI_FML_ExtensionInfo;

// Java: public ExtensionInfo(boolean extended, int vanillaCount, int totalCount, NetworkCheck netCheck)
LIBMATTI_FML_ExtensionInfo *LIBMATTI_FML_ExtensionInfo_New(int extended, int vanillaCount, int totalCount,
                                                           LIBMATTI_FML_NetworkedEnum_NetworkCheck *netCheck);
// Java: public static <T> ExtensionInfo nonExtended(Class<T> enumClass)
LIBMATTI_FML_ExtensionInfo *LIBMATTI_FML_ExtensionInfo_NonExtended(
    LIBMATTI_FML_NetworkedEnum_NetworkCheck *netCheck);
void LIBMATTI_FML_ExtensionInfo_Free(LIBMATTI_FML_ExtensionInfo *info);

#endif //MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_EXTENSIONINFO_H
