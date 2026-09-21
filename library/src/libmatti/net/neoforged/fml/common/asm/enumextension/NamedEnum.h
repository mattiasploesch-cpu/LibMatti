// Port of net.neoforged.fml.common.asm.enumextension.NamedEnum.
// Java: @Target(ElementType.TYPE) @interface NamedEnum { int value() default 0; }

#ifndef MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_NAMEDENUM_H
#define MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_NAMEDENUM_H

// Java: Type.getType(NamedEnum.class).getDescriptor()
#define LIBMATTI_FML_NAMEDENUM_DESCRIPTOR "Lnet/neoforged/fml/common/asm/enumextension/NamedEnum;"

// Java: int value() default 0
#define LIBMATTI_FML_NAMEDENUM_DEFAULT_VALUE 0

// Java: int value() - the element name of the annotation
#define LIBMATTI_FML_NAMEDENUM_VALUE "value"

#endif //MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_NAMEDENUM_H
