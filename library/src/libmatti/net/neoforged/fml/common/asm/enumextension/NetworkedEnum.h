// Port of net.neoforged.fml.common.asm.enumextension.NetworkedEnum.

#ifndef MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_NETWORKEDENUM_H
#define MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_NETWORKEDENUM_H

// Java: @Target(ElementType.TYPE) @Retention(RetentionPolicy.RUNTIME) @interface NetworkedEnum
#define LIBMATTI_FML_NETWORKEDENUM_DESCRIPTOR "Lnet/neoforged/fml/common/asm/enumextension/NetworkedEnum;"

// Java: NetworkCheck value() - the element name of the annotation
#define LIBMATTI_FML_NETWORKEDENUM_VALUE "value"

// Java: enum NetworkCheck { CLIENTBOUND, SERVERBOUND, BIDIRECTIONAL }
typedef enum
{
    LIBMATTI_FML_NETWORKEDENUM_CLIENTBOUND = 0,
    LIBMATTI_FML_NETWORKEDENUM_SERVERBOUND,
    LIBMATTI_FML_NETWORKEDENUM_BIDIRECTIONAL
} LIBMATTI_FML_NetworkedEnum_NetworkCheck;

// Java: Type.getType(NetworkedEnum.NetworkCheck.class).getInternalName()
#define LIBMATTI_FML_NETWORKEDENUM_NETWORKCHECK_INTERNAL_NAME \
    "net/neoforged/fml/common/asm/enumextension/NetworkedEnum$NetworkCheck"

// Java: Type.getType(NetworkedEnum.NetworkCheck.class).getDescriptor()
#define LIBMATTI_FML_NETWORKEDENUM_NETWORKCHECK_DESCRIPTOR \
    "Lnet/neoforged/fml/common/asm/enumextension/NetworkedEnum$NetworkCheck;"

// Java: NetworkCheck.name()
const char *LIBMATTI_FML_NetworkedEnum_NetworkCheck_Name(LIBMATTI_FML_NetworkedEnum_NetworkCheck check);

#endif //MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_NETWORKEDENUM_H
