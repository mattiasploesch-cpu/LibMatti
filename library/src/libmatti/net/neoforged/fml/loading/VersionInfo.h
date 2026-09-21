// Port of net.neoforged.fml.loading.VersionInfo.

#ifndef MATTICRAFT_FML_VERSIONINFO_H
#define MATTICRAFT_FML_VERSIONINFO_H

// Java: public record VersionInfo(String neoForgeVersion, String mcVersion, String neoFormVersion)
typedef struct
{
    char *neoForgeVersion;
    char *mcVersion;
    char *neoFormVersion;
} LIBMATTI_FML_VersionInfo;

LIBMATTI_FML_VersionInfo *LIBMATTI_FML_VersionInfo_New(const char *neoForgeVersion, const char *mcVersion,
                                                       const char *neoFormVersion);
void LIBMATTI_FML_VersionInfo_Free(LIBMATTI_FML_VersionInfo *versionInfo);

// Java: public String mcAndNeoFormVersion()
char *LIBMATTI_FML_VersionInfo_McAndNeoFormVersion(const LIBMATTI_FML_VersionInfo *versionInfo);

#endif //MATTICRAFT_FML_VERSIONINFO_H
