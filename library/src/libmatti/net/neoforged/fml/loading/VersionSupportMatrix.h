// Port of net.neoforged.fml.loading.VersionSupportMatrix.

#ifndef MATTICRAFT_FML_VERSIONSUPPORTMATRIX_H
#define MATTICRAFT_FML_VERSIONSUPPORTMATRIX_H

#include "libmatti/net/neoforged/fml/loading/VersionInfo.h"
#include "libmatti/net/neoforged/neoforgespi/language/MavenVersionAdapter.h"

#include <stddef.h>

// Java: class VersionSupportMatrix
typedef struct LIBMATTI_FML_VersionSupportMatrix LIBMATTI_FML_VersionSupportMatrix;

// Java: public VersionSupportMatrix(VersionInfo versionInfo)
LIBMATTI_FML_VersionSupportMatrix *LIBMATTI_FML_VersionSupportMatrix_New(const LIBMATTI_FML_VersionInfo *versionInfo);
void LIBMATTI_FML_VersionSupportMatrix_Free(LIBMATTI_FML_VersionSupportMatrix *matrix);

// Java: private void add(String key, String value)
void LIBMATTI_FML_VersionSupportMatrix_Add(LIBMATTI_FML_VersionSupportMatrix *matrix, const char *key,
                                           const char *value);

// Java: public boolean testVersionSupportMatrix(VersionRange declaredRange, String lookupId, String type,
//         BiPredicate<String, VersionRange> standardLookup)
typedef int (*LIBMATTI_FML_VersionSupportMatrix_StandardLookup)(const char *lookupId,
                                                               const LIBMATTI_NEOFORGESPI_VersionRange *declaredRange,
                                                               void *userdata);

int LIBMATTI_FML_VersionSupportMatrix_TestVersionSupportMatrix(
    LIBMATTI_FML_VersionSupportMatrix *matrix, const LIBMATTI_NEOFORGESPI_VersionRange *declaredRange,
    const char *lookupId, const char *type, LIBMATTI_FML_VersionSupportMatrix_StandardLookup standardLookup,
    void *userdata);

#endif //MATTICRAFT_FML_VERSIONSUPPORTMATRIX_H
