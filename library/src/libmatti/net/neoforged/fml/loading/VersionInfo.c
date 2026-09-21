#include "libmatti/net/neoforged/fml/loading/VersionInfo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LIBMATTI_FML_VersionInfo *LIBMATTI_FML_VersionInfo_New(const char *neoForgeVersion, const char *mcVersion,
                                                       const char *neoFormVersion)
{
    LIBMATTI_FML_VersionInfo *versionInfo = calloc(1, sizeof(LIBMATTI_FML_VersionInfo));
    versionInfo->neoForgeVersion = neoForgeVersion != NULL ? strdup(neoForgeVersion) : NULL;
    versionInfo->mcVersion = mcVersion != NULL ? strdup(mcVersion) : NULL;
    versionInfo->neoFormVersion = neoFormVersion != NULL ? strdup(neoFormVersion) : NULL;
    return versionInfo;
}

void LIBMATTI_FML_VersionInfo_Free(LIBMATTI_FML_VersionInfo *versionInfo)
{
    if (versionInfo == NULL) return;

    free(versionInfo->neoForgeVersion);
    free(versionInfo->mcVersion);
    free(versionInfo->neoFormVersion);
    free(versionInfo);
}

// Java: public String mcAndNeoFormVersion()
char *LIBMATTI_FML_VersionInfo_McAndNeoFormVersion(const LIBMATTI_FML_VersionInfo *versionInfo)
{
    // Java: return mcVersion + "-" + neoFormVersion;
    size_t length = strlen(versionInfo->mcVersion) + strlen(versionInfo->neoFormVersion) + 2;
    char *result = malloc(length);
    snprintf(result, length, "%s-%s", versionInfo->mcVersion, versionInfo->neoFormVersion);
    return result;
}
