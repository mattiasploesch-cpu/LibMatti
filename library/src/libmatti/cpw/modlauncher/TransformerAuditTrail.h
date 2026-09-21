// Port of cpw.mods.modlauncher.TransformerAuditTrail.
// Java's TransformerAuditTrail implements ITransformerAuditTrail; in C the
// concrete type completes the interface struct.

#ifndef MATTICRAFT_MODLAUNCHER_TRANSFORMERAUDITTRAIL_H
#define MATTICRAFT_MODLAUNCHER_TRANSFORMERAUDITTRAIL_H

#include "libmatti/cpw/modlauncher/api/ITransformationService.h"
#include "libmatti/cpw/modlauncher/api/ITransformerActivity.h"
#include "libmatti/cpw/modlauncher/api/ITransformerAuditTrail.h"
#include "libmatti/cpw/modlauncher/serviceapi/ILaunchPluginService.h"

#include <stddef.h>

// Java: Map<String, List<ITransformerActivity>> audit = new ConcurrentHashMap<>()
typedef struct
{
    char *className;
    LIBMATTI_MLA_ITransformerActivity *activities;
    size_t count;
} LIBMATTI_ML_TransformerAuditTrailEntry;

struct LIBMATTI_MLA_ITransformerAuditTrail
{
    LIBMATTI_ML_TransformerAuditTrailEntry *entries;
    size_t count;
};

typedef LIBMATTI_MLA_ITransformerAuditTrail LIBMATTI_ML_TransformerAuditTrail;

LIBMATTI_ML_TransformerAuditTrail *LIBMATTI_ML_TransformerAuditTrail_New(void);
void LIBMATTI_ML_TransformerAuditTrail_Free(LIBMATTI_ML_TransformerAuditTrail *trail);

// Java: public void addReason(String clazz, String reason)
void LIBMATTI_ML_TransformerAuditTrail_AddReason(LIBMATTI_ML_TransformerAuditTrail *trail, const char *clazz,
                                                 const char *reason);
// Java: public void addPluginCustomAuditTrail(String clazz, ILaunchPluginService plugin, String... data)
void LIBMATTI_ML_TransformerAuditTrail_AddPluginCustomAuditTrail(LIBMATTI_ML_TransformerAuditTrail *trail,
                                                                 const char *clazz,
                                                                 LIBMATTI_MLS_ILaunchPluginService *plugin,
                                                                 char **data, size_t dataCount);
// Java: public void addPluginAuditTrail(String clazz, ILaunchPluginService plugin, ILaunchPluginService.Phase phase)
void LIBMATTI_ML_TransformerAuditTrail_AddPluginAuditTrail(LIBMATTI_ML_TransformerAuditTrail *trail, const char *clazz,
                                                           LIBMATTI_MLS_ILaunchPluginService *plugin,
                                                           LIBMATTI_MLS_Phase phase);
// Java: public void addTransformerAuditTrail(String clazz, ITransformationService, ITransformer<?>)
void LIBMATTI_ML_TransformerAuditTrail_AddTransformerAuditTrail(LIBMATTI_ML_TransformerAuditTrail *trail,
                                                               const char *clazz,
                                                               LIBMATTI_MLA_ITransformationService *transformService,
                                                               LIBMATTI_MLA_ITransformer *transformer);

#endif //MATTICRAFT_MODLAUNCHER_TRANSFORMERAUDITTRAIL_H
