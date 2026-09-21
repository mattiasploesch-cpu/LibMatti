// Port of net.neoforged.fml.common.asm.enumextension.IExtensibleEnum.
// Java: an empty marker interface implemented by vanilla enums that the RuntimeEnumExtender
// extends. In the port the interface only exists as the type descriptor the transformer matches
// on, because the ported launcher writes the class files itself and does not define the interface.

#ifndef MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_IEXTENSIBLEENUM_H
#define MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_IEXTENSIBLEENUM_H

// Java: Type.getType(IExtensibleEnum.class).getInternalName()
#define LIBMATTI_FML_IExtensibleEnum_INTERNAL_NAME "net/neoforged/fml/common/asm/enumextension/IExtensibleEnum"

// Java: Type.getType(IExtensibleEnum.class).getDescriptor()
#define LIBMATTI_FML_IExtensibleEnum_DESCRIPTOR "Lnet/neoforged/fml/common/asm/enumextension/IExtensibleEnum;"

#endif //MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_IEXTENSIBLEENUM_H
