// Port of cpw.mods.modlauncher.api.ITransformerAuditTrail.

#ifndef MATTICRAFT_MODLAUNCHER_ITRANSFORMERAUDITTRAIL_H
#define MATTICRAFT_MODLAUNCHER_ITRANSFORMERAUDITTRAIL_H

#include "libmatti/cpw/modlauncher/api/ITransformerActivity.h"

#include <stddef.h>

// Java: interface ITransformerAuditTrail
typedef struct LIBMATTI_MLA_ITransformerAuditTrail LIBMATTI_MLA_ITransformerAuditTrail;

// Java: List<ITransformerActivity> getActivityFor(String className) - read only list
LIBMATTI_MLA_ITransformerActivity *LIBMATTI_MLA_ITransformerAuditTrail_GetActivityFor(
    LIBMATTI_MLA_ITransformerAuditTrail *trail, const char *className, size_t *count);
// Java: String getAuditString(String clazz); caller frees
char *LIBMATTI_MLA_ITransformerAuditTrail_GetAuditString(LIBMATTI_MLA_ITransformerAuditTrail *trail,
                                                         const char *clazz);

#endif //MATTICRAFT_MODLAUNCHER_ITRANSFORMERAUDITTRAIL_H
