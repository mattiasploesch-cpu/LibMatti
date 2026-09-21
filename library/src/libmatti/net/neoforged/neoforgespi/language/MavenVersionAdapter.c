// Port of net.neoforged.neoforgespi.language.MavenVersionAdapter.

#include "libmatti/net/neoforged/neoforgespi/language/MavenVersionAdapter.h"

#include "libmatti/cpw/modlauncher/LogManager.h"

// Java: private static final Logger LOGGER = LogManager.getLogger();
static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: throw new RuntimeException(...) - the port terminates like an uncaught exception
#include "libmatti/java/lang/Throwable.h"
#include <stdlib.h>

// Java: public static VersionRange createFromVersionSpec(String spec)
LIBMATTI_NEOFORGESPI_VersionRange *LIBMATTI_NEOFORGESPI_MavenVersionAdapter_CreateFromVersionSpec(const char *spec)
{
    // Java: try { return VersionRange.createFromVersionSpec(spec); }
    //       catch (InvalidVersionSpecificationException e) { LOGGER.fatal("Failed to parse version spec {}", spec, e);
    //                                                       throw new RuntimeException("Failed to parse spec", e); }
    LIBMATTI_NEOFORGESPI_VersionRange *range = LIBMATTI_MAVEN_VersionRange_CreateFromVersionSpec(spec);
    if (range == NULL)
    {
        // Java: LOGGER.fatal(...); throw new RuntimeException("Failed to parse spec", e);
        LIBMATTI_ML_Logger_Fatal(LOGGER(), NULL, "Failed to parse version spec {}", spec);
        LIBMATTI_JL_Throwable *exception =
            LIBMATTI_JL_Throwable_NewNamed("java.lang.RuntimeException", "Failed to parse spec");
        LIBMATTI_JL_Throwable_PrintStackTrace(exception);
        exit(1);
    }
    return range;
}

// Java: new DefaultArtifactVersion(version)
LIBMATTI_NEOFORGESPI_ArtifactVersion *LIBMATTI_NEOFORGESPI_MavenVersionAdapter_CreateFromVersion(const char *version)
{
    return LIBMATTI_MAVEN_ArtifactVersion_New(version);
}

// Java: VersionRange.createFromVersion(String)
LIBMATTI_NEOFORGESPI_VersionRange *LIBMATTI_NEOFORGESPI_MavenVersionAdapter_RangeFromVersion(const char *version)
{
    return LIBMATTI_MAVEN_VersionRange_CreateFromVersion(version);
}

// Java: VersionRange.containsVersion(ArtifactVersion)
int LIBMATTI_NEOFORGESPI_MavenVersionAdapter_ContainsVersion(const LIBMATTI_NEOFORGESPI_VersionRange *range,
                                                            const LIBMATTI_NEOFORGESPI_ArtifactVersion *version)
{
    return LIBMATTI_MAVEN_VersionRange_ContainsVersion(range, version);
}

// Java: DefaultArtifactVersion.compareTo(ArtifactVersion)
int LIBMATTI_NEOFORGESPI_MavenVersionAdapter_Compare(const LIBMATTI_NEOFORGESPI_ArtifactVersion *a,
                                                     const LIBMATTI_NEOFORGESPI_ArtifactVersion *b)
{
    return LIBMATTI_MAVEN_ArtifactVersion_Compare(a, b);
}

// Java: ArtifactVersion.toString()
const char *LIBMATTI_NEOFORGESPI_MavenVersionAdapter_VersionToString(
    const LIBMATTI_NEOFORGESPI_ArtifactVersion *version)
{
    return LIBMATTI_MAVEN_ArtifactVersion_ToString(version);
}

void LIBMATTI_NEOFORGESPI_MavenVersionAdapter_FreeRange(LIBMATTI_NEOFORGESPI_VersionRange *range)
{
    LIBMATTI_MAVEN_VersionRange_Free(range);
}

void LIBMATTI_NEOFORGESPI_MavenVersionAdapter_FreeVersion(LIBMATTI_NEOFORGESPI_ArtifactVersion *version)
{
    LIBMATTI_MAVEN_ArtifactVersion_Free(version);
}
