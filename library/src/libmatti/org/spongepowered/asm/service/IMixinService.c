// The org.spongepowered.asm.service interfaces have no behaviour of their own:
// MixinService.c carries the singleton, and the FML implementations live in
// libmatti/net/neoforged/fml/loading/mixin/FMLMixinService.c (Java: the classes
// implementing the interfaces live in the loader, next to MixinFacade).
//
// This translation unit exists so the include of the interface header is tied to the
// library like every other ported package.

typedef int libmatti_sp_asm_service_translation_unit;
