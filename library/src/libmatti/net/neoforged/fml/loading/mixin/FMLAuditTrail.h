// Port of net.neoforged.fml.loading.mixin.FMLAuditTrail.
// Java: "Audit trail adapter for ModLauncher"

#ifndef MATTICRAFT_FML_LOADING_MIXIN_FMLAUDITTRAIL_H
#define MATTICRAFT_FML_LOADING_MIXIN_FMLAUDITTRAIL_H

#include "libmatti/org/spongepowered/asm/service/IMixinService.h"

// Java: class FMLAuditTrail implements IMixinAuditTrail
LIBMATTI_SP_IMixinAuditTrail *LIBMATTI_FML_FMLAuditTrail_Instance(void);

#endif //MATTICRAFT_FML_LOADING_MIXIN_FMLAUDITTRAIL_H
