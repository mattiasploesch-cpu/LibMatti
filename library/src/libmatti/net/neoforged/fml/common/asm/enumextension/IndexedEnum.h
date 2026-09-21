// Port of net.neoforged.fml.common.asm.enumextension.IndexedEnum.
// Java: @Target(ElementType.TYPE) @interface IndexedEnum { int value() default 0; }
// The port keeps the annotation descriptor the RuntimeEnumExtender reads plus the default value.

#ifndef MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_INDEXEDENUM_H
#define MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_INDEXEDENUM_H

// Java: Type.getType(IndexedEnum.class).getDescriptor()
#define LIBMATTI_FML_INDEXEDENUM_DESCRIPTOR "Lnet/neoforged/fml/common/asm/enumextension/IndexedEnum;"

// Java: int value() default 0
#define LIBMATTI_FML_INDEXEDENUM_DEFAULT_VALUE 0

// Java: int value() - the element name of the annotation
#define LIBMATTI_FML_INDEXEDENUM_VALUE "value"

#endif //MATTICRAFT_FML_COMMON_ASM_ENUMEXTENSION_INDEXEDENUM_H
