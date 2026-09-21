// Port of net.neoforged.neoforgespi.locating.ForgeFeature.
// TODO: java.util.function.Predicate - IFeatureTest extends Predicate<F>; the port uses a test hook.
// TODO: java.lang.Boolean - BooleanFeatureTest keeps the value as an int.
// TODO: org.apache.maven.artifact.versioning (MavenVersionAdapter) / InvalidVersionSpecificationException

#ifndef MATTICRAFT_NEOFORGESPI_LOCATING_FORGEFEATURE_H
#define MATTICRAFT_NEOFORGESPI_LOCATING_FORGEFEATURE_H

#include <stddef.h>

#include "libmatti/net/neoforged/api/distmarker/Dist.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModInfo.h"
#include "libmatti/net/neoforged/neoforgespi/language/MavenVersionAdapter.h"

// Java: sealed interface IFeatureTest<F> extends Predicate<F>
typedef struct LIBMATTI_NEOFORGESPI_ForgeFeature_IFeatureTest LIBMATTI_NEOFORGESPI_ForgeFeature_IFeatureTest;

struct LIBMATTI_NEOFORGESPI_ForgeFeature_IFeatureTest
{
    void *self;

    // Java: IModInfo.DependencySide applicableSides()
    LIBMATTI_NEOFORGESPI_IModInfo_DependencySide (*applicableSides)(void *self);
    // Java: F convertFromString(String value) - the C port keeps the converted value as a handle
    void *(*convertFromString)(void *self, const char *value);
    // Java: String featureValue()
    const char *(*featureValue)(void *self);
    // Java: boolean test(F f)
    int (*test)(void *self, void *value);
};

// Java: public default boolean testSideWithString(Dist side, String value)
int LIBMATTI_NEOFORGESPI_ForgeFeature_TestSideWithString(
    LIBMATTI_NEOFORGESPI_ForgeFeature_IFeatureTest *featureTest, LIBMATTI_DIST_Dist side, const char *value);

// Java: public record Bound(String featureName, String featureBound, IModInfo modInfo)
typedef struct
{
    char *featureName;
    char *featureBound;
    LIBMATTI_NEOFORGESPI_IModInfo *modInfo;
} LIBMATTI_NEOFORGESPI_ForgeFeature_Bound;

// Java: public static <T> void registerFeature(String featureName, IFeatureTest<T> featureTest)
void LIBMATTI_NEOFORGESPI_ForgeFeature_RegisterFeature(const char *featureName,
                                                      LIBMATTI_NEOFORGESPI_ForgeFeature_IFeatureTest *featureTest);
// Java: public static boolean testFeature(Dist side, Bound bound)
int LIBMATTI_NEOFORGESPI_ForgeFeature_TestFeature(LIBMATTI_DIST_Dist side, const LIBMATTI_NEOFORGESPI_ForgeFeature_Bound *bound);
// Java: public static Object featureValue(Bound bound)
void *LIBMATTI_NEOFORGESPI_ForgeFeature_FeatureValue(const LIBMATTI_NEOFORGESPI_ForgeFeature_Bound *bound);

// Java: public record VersionFeatureTest(IModInfo.DependencySide applicableSides, ArtifactVersion version)
LIBMATTI_NEOFORGESPI_ForgeFeature_IFeatureTest *LIBMATTI_NEOFORGESPI_ForgeFeature_VersionFeatureTest_New(
    LIBMATTI_NEOFORGESPI_IModInfo_DependencySide applicableSides, const char *version);
// Java: public static VersionFeatureTest forVersionString(IModInfo.DependencySide side, String version)
LIBMATTI_NEOFORGESPI_ForgeFeature_IFeatureTest *LIBMATTI_NEOFORGESPI_ForgeFeature_VersionFeatureTest_ForVersionString(
    LIBMATTI_NEOFORGESPI_IModInfo_DependencySide side, const char *version);

// Java: public record BooleanFeatureTest(IModInfo.DependencySide applicableSides, boolean value)
LIBMATTI_NEOFORGESPI_ForgeFeature_IFeatureTest *LIBMATTI_NEOFORGESPI_ForgeFeature_BooleanFeatureTest_New(
    LIBMATTI_NEOFORGESPI_IModInfo_DependencySide applicableSides, int value);

#endif //MATTICRAFT_NEOFORGESPI_LOCATING_FORGEFEATURE_H
