#include "libmatti/net/neoforged/neoforgespi/locating/ForgeFeature.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final Map<String, IFeatureTest<?>> features = new HashMap<>()
typedef struct
{
    char *name;
    LIBMATTI_NEOFORGESPI_ForgeFeature_IFeatureTest *featureTest;
} RegisteredFeature;

static RegisteredFeature *features = NULL;
static size_t featureCount = 0;

// --- MissingFeatureTest (Java: private static final MissingFeatureTest MISSING) ---

static LIBMATTI_NEOFORGESPI_IModInfo_DependencySide missingApplicableSides(void *self)
{
    (void) self;
    return LIBMATTI_NEOFORGESPI_IModInfo_DependencySide_BOTH;
}

static void *missingConvertFromString(void *self, const char *value)
{
    (void) self;
    (void) value;
    return NULL;
}

static const char *missingFeatureValue(void *self)
{
    (void) self;
    return "NONE";
}

static int missingTest(void *self, void *value)
{
    (void) self;
    (void) value;
    return 0;
}

// Java: public default boolean testSideWithString(Dist side, String value)
int LIBMATTI_NEOFORGESPI_ForgeFeature_TestSideWithString(
    LIBMATTI_NEOFORGESPI_ForgeFeature_IFeatureTest *featureTest, LIBMATTI_DIST_Dist side, const char *value)
{
    // Java: return !applicableSides().isContained(side) || test(convertFromString(value));
    if (!LIBMATTI_NEOFORGESPI_IModInfo_DependencySide_IsContained(featureTest->applicableSides(featureTest->self), side))
        return 1;
    return featureTest->test(featureTest->self, featureTest->convertFromString(featureTest->self, value));
}

static LIBMATTI_NEOFORGESPI_ForgeFeature_IFeatureTest *getFeatureTest(const char *featureName)
{
    for (size_t i = 0; i < featureCount; i++)
        if (strcmp(features[i].name, featureName) == 0)
            return features[i].featureTest;

    // Java: features.getOrDefault(bound.featureName(), MISSING)
    static LIBMATTI_NEOFORGESPI_ForgeFeature_IFeatureTest missing = {
        .self = NULL,
        .applicableSides = missingApplicableSides,
        .convertFromString = missingConvertFromString,
        .featureValue = missingFeatureValue,
        .test = missingTest,
    };
    return &missing;
}

// Java: public static <T> void registerFeature(String featureName, IFeatureTest<T> featureTest)
void LIBMATTI_NEOFORGESPI_ForgeFeature_RegisterFeature(const char *featureName,
                                                      LIBMATTI_NEOFORGESPI_ForgeFeature_IFeatureTest *featureTest)
{
    for (size_t i = 0; i < featureCount; i++)
    {
        if (strcmp(features[i].name, featureName) == 0)
        {
            features[i].featureTest = featureTest;
            return;
        }
    }

    features = realloc(features, sizeof(*features) * (featureCount + 1));
    features[featureCount].name = strdup(featureName);
    features[featureCount].featureTest = featureTest;
    featureCount++;
}

// Java: public static boolean testFeature(Dist side, Bound bound)
int LIBMATTI_NEOFORGESPI_ForgeFeature_TestFeature(LIBMATTI_DIST_Dist side,
                                                  const LIBMATTI_NEOFORGESPI_ForgeFeature_Bound *bound)
{
    return LIBMATTI_NEOFORGESPI_ForgeFeature_TestSideWithString(getFeatureTest(bound->featureName), side,
                                                                bound->featureBound);
}

// Java: public static Object featureValue(Bound bound)
void *LIBMATTI_NEOFORGESPI_ForgeFeature_FeatureValue(const LIBMATTI_NEOFORGESPI_ForgeFeature_Bound *bound)
{
    return (void *)getFeatureTest(bound->featureName)->featureValue(getFeatureTest(bound->featureName)->self);
}

// --- VersionFeatureTest -----------------------------------------------------

typedef struct
{
    LIBMATTI_NEOFORGESPI_ForgeFeature_IFeatureTest base;
    LIBMATTI_NEOFORGESPI_IModInfo_DependencySide applicableSides;
    LIBMATTI_NEOFORGESPI_ArtifactVersion *version;
} VersionFeatureTest;

static LIBMATTI_NEOFORGESPI_IModInfo_DependencySide versionApplicableSides(void *self)
{
    return ((VersionFeatureTest *)self)->applicableSides;
}

// Java: public String featureValue() { return version.toString(); }
static const char *versionFeatureValue(void *self)
{
    return ((VersionFeatureTest *)self)->version->version;
}

// Java: public boolean test(VersionRange versionRange) { return versionRange.containsVersion(version); }
static int versionTest(void *self, void *value)
{
    VersionFeatureTest *featureTest = self;
    return LIBMATTI_NEOFORGESPI_MavenVersionAdapter_ContainsVersion((const LIBMATTI_NEOFORGESPI_VersionRange *)value,
                                                                   featureTest->version);
}

// Java: public VersionRange convertFromString(String value) { VersionRange.createFromVersionSpec(value); }
static void *versionConvertFromString(void *self, const char *value)
{
    (void) self;
    // Java: the InvalidVersionSpecificationException becomes a RuntimeException through
    //       MavenVersionAdapter; the port terminates with the stack trace like an uncaught throw.
    return LIBMATTI_NEOFORGESPI_MavenVersionAdapter_CreateFromVersionSpec(value);
}

// Java: public record VersionFeatureTest(IModInfo.DependencySide applicableSides, ArtifactVersion version)
LIBMATTI_NEOFORGESPI_ForgeFeature_IFeatureTest *LIBMATTI_NEOFORGESPI_ForgeFeature_VersionFeatureTest_New(
    LIBMATTI_NEOFORGESPI_IModInfo_DependencySide applicableSides, const char *version)
{
    VersionFeatureTest *featureTest = calloc(1, sizeof(VersionFeatureTest));
    featureTest->applicableSides = applicableSides;
    featureTest->version = LIBMATTI_NEOFORGESPI_MavenVersionAdapter_CreateFromVersion(version);
    featureTest->base.self = featureTest;
    featureTest->base.applicableSides = versionApplicableSides;
    featureTest->base.convertFromString = versionConvertFromString;
    featureTest->base.featureValue = versionFeatureValue;
    featureTest->base.test = versionTest;
    return &featureTest->base;
}

// Java: public static VersionFeatureTest forVersionString(IModInfo.DependencySide side, String version)
LIBMATTI_NEOFORGESPI_ForgeFeature_IFeatureTest *LIBMATTI_NEOFORGESPI_ForgeFeature_VersionFeatureTest_ForVersionString(
    LIBMATTI_NEOFORGESPI_IModInfo_DependencySide side, const char *version)
{
    return LIBMATTI_NEOFORGESPI_ForgeFeature_VersionFeatureTest_New(side, version);
}

// --- BooleanFeatureTest -----------------------------------------------------

typedef struct
{
    LIBMATTI_NEOFORGESPI_ForgeFeature_IFeatureTest base;
    LIBMATTI_NEOFORGESPI_IModInfo_DependencySide applicableSides;
    int value;
} BooleanFeatureTest;

static LIBMATTI_NEOFORGESPI_IModInfo_DependencySide booleanApplicableSides(void *self)
{
    return ((BooleanFeatureTest *)self)->applicableSides;
}

// Java: public String featureValue() { return Boolean.toString(value); }
static const char *booleanFeatureValue(void *self)
{
    return ((BooleanFeatureTest *)self)->value ? "true" : "false";
}

// Java: public boolean test(Boolean aBoolean) { return aBoolean.equals(value); }
static int booleanTest(void *self, void *value)
{
    (void) self;
    return value != NULL && *(int *)value == ((BooleanFeatureTest *)self)->value;
}

// Java: public Boolean convertFromString(String value) { return Boolean.parseBoolean(value); }
static void *booleanConvertFromString(void *self, const char *value)
{
    (void) self;
    int *parsed = malloc(sizeof(int));
    *parsed = strcmp(value, "true") == 0 ? 1 : 0;
    return parsed;
}

// Java: public record BooleanFeatureTest(IModInfo.DependencySide applicableSides, boolean value)
LIBMATTI_NEOFORGESPI_ForgeFeature_IFeatureTest *LIBMATTI_NEOFORGESPI_ForgeFeature_BooleanFeatureTest_New(
    LIBMATTI_NEOFORGESPI_IModInfo_DependencySide applicableSides, int value)
{
    BooleanFeatureTest *featureTest = calloc(1, sizeof(BooleanFeatureTest));
    featureTest->applicableSides = applicableSides;
    featureTest->value = value;
    featureTest->base.self = featureTest;
    featureTest->base.applicableSides = booleanApplicableSides;
    featureTest->base.convertFromString = booleanConvertFromString;
    featureTest->base.featureValue = booleanFeatureValue;
    featureTest->base.test = booleanTest;
    return &featureTest->base;
}
