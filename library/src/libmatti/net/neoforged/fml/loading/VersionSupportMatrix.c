#include "libmatti/net/neoforged/fml/loading/VersionSupportMatrix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final HashMap<String, List<ArtifactVersion>> overrideVersions = new HashMap<>();
typedef struct
{
    char *key;
    char *version;
} OverrideVersion;

static OverrideVersion *overrideVersions = NULL;
static size_t overrideVersionCount = 0;

struct LIBMATTI_FML_VersionSupportMatrix
{
    int unused;
};

// Java: private void add(String key, String value)
void LIBMATTI_FML_VersionSupportMatrix_Add(LIBMATTI_FML_VersionSupportMatrix *matrix, const char *key,
                                           const char *value)
{
    // Java: overrideVersions.computeIfAbsent(key, k -> new ArrayList<>()).add(new DefaultArtifactVersion(value));
    (void) matrix;

    overrideVersions = realloc(overrideVersions, sizeof(*overrideVersions) * (overrideVersionCount + 1));
    overrideVersions[overrideVersionCount].key = strdup(key);
    overrideVersions[overrideVersionCount].version = strdup(value);
    overrideVersionCount++;
}

// Java: public VersionSupportMatrix(VersionInfo versionInfo)
LIBMATTI_FML_VersionSupportMatrix *LIBMATTI_FML_VersionSupportMatrix_New(const LIBMATTI_FML_VersionInfo *versionInfo)
{
    LIBMATTI_FML_VersionSupportMatrix *matrix = calloc(1, sizeof(LIBMATTI_FML_VersionSupportMatrix));

    // Java: var mcVersion = new DefaultArtifactVersion(versionInfo.mcVersion());
    //       if (MavenVersionAdapter.createFromVersionSpec("[1.21.8]").containsVersion(mcVersion)) { add("mod.minecraft", "1.21.7"); add("mod.neoforge", "21.7.26-beta"); }
    LIBMATTI_NEOFORGESPI_VersionRange *range =
        LIBMATTI_NEOFORGESPI_MavenVersionAdapter_CreateFromVersionSpec("[1.21.8]");
    LIBMATTI_NEOFORGESPI_ArtifactVersion *mcVersion =
        LIBMATTI_NEOFORGESPI_MavenVersionAdapter_CreateFromVersion(versionInfo->mcVersion != NULL ? versionInfo->mcVersion : "");

    if (LIBMATTI_NEOFORGESPI_MavenVersionAdapter_ContainsVersion(range, mcVersion))
    {
        LIBMATTI_FML_VersionSupportMatrix_Add(matrix, "mod.minecraft", "1.21.7");
        LIBMATTI_FML_VersionSupportMatrix_Add(matrix, "mod.neoforge", "21.7.26-beta");
    }

    LIBMATTI_NEOFORGESPI_MavenVersionAdapter_FreeRange(range);
    LIBMATTI_NEOFORGESPI_MavenVersionAdapter_FreeVersion(mcVersion);
    return matrix;
}

void LIBMATTI_FML_VersionSupportMatrix_Free(LIBMATTI_FML_VersionSupportMatrix *matrix)
{
    free(matrix);
}

// Java: public boolean testVersionSupportMatrix(VersionRange declaredRange, String lookupId, String type, BiPredicate<String, VersionRange> standardLookup)
int LIBMATTI_FML_VersionSupportMatrix_TestVersionSupportMatrix(
    LIBMATTI_FML_VersionSupportMatrix *matrix, const LIBMATTI_NEOFORGESPI_VersionRange *declaredRange,
    const char *lookupId, const char *type, LIBMATTI_FML_VersionSupportMatrix_StandardLookup standardLookup,
    void *userdata)
{
    // Java: if (standardLookup.test(lookupId, declaredRange)) return true;
    if (standardLookup != NULL && standardLookup(lookupId, declaredRange, userdata)) return 1;

    // Java: var custom = overrideVersions.get(type + "." + lookupId);
    (void) matrix;
    size_t length = strlen(type) + strlen(lookupId) + 2;
    char *compositeKey = malloc(length);
    snprintf(compositeKey, length, "%s.%s", type, lookupId);

    // Java: return custom != null && custom.stream().anyMatch(declaredRange::containsVersion);
    for (size_t i = 0; i < overrideVersionCount; i++)
    {
        if (strcmp(overrideVersions[i].key, compositeKey) != 0) continue;

        LIBMATTI_NEOFORGESPI_ArtifactVersion *customVersion =
            LIBMATTI_NEOFORGESPI_MavenVersionAdapter_CreateFromVersion(overrideVersions[i].version);
        int supported = LIBMATTI_NEOFORGESPI_MavenVersionAdapter_ContainsVersion(declaredRange, customVersion);
        LIBMATTI_NEOFORGESPI_MavenVersionAdapter_FreeVersion(customVersion);

        if (supported)
        {
            free(compositeKey);
            return 1;
        }
    }

    free(compositeKey);
    return 0;
}
