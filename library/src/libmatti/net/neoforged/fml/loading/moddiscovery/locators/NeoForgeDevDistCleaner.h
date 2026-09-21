// Port of net.neoforged.fml.loading.moddiscovery.locators.NeoForgeDevDistCleaner.

#ifndef MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_NEOFORGEDEVDISTCLEANER_H
#define MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_NEOFORGEDEVDISTCLEANER_H

#include "libmatti/net/neoforged/api/distmarker/Dist.h"
#include "libmatti/net/neoforged/fml/jarcontents/JarContents.h"
#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessor.h"

#include <stddef.h>

// Java: private static final Attributes.Name NAME_DISTS = new Attributes.Name("Minecraft-Dists")
#define LIBMATTI_FML_DIST_CLEANER_NAME_DISTS "Minecraft-Dists"
// Java: private static final Attributes.Name NAME_DIST = new Attributes.Name("Minecraft-Dist")
#define LIBMATTI_FML_DIST_CLEANER_NAME_DIST "Minecraft-Dist"

// Java: public static boolean supportsDistCleaning(JarContents minecraftModFile)
int LIBMATTI_FML_NeoForgeDevDistCleaner_SupportsDistCleaning(LIBMATTI_FML_JarContents *minecraftModFile);

// Java: public static Stream<String> getMaskedFiles(JarContents minecraftJar, Dist currentDist)
// The caller frees the paths and the array. NULL when the jar has no masking attributes (Java: throws ModLoadingException).
char **LIBMATTI_FML_NeoForgeDevDistCleaner_GetMaskedFiles(LIBMATTI_FML_JarContents *minecraftJar, LIBMATTI_DIST_Dist currentDist,
                                                          size_t *count);

// Java: public class NeoForgeDevDistCleaner implements ClassProcessor
typedef struct LIBMATTI_FML_NeoForgeDevDistCleaner
{
    // Java: implements ClassProcessor
    LIBMATTI_NEOFORGESPI_ClassProcessor processor;

    // Java: private final Dist dist
    LIBMATTI_DIST_Dist dist;
    // Java: private final Set<String> maskedClasses
    char **maskedClasses;
    size_t maskedClassCount;
} LIBMATTI_FML_NeoForgeDevDistCleaner;

// Java: public NeoForgeDevDistCleaner(JarContents minecraftModFile, Dist requestedDist)
LIBMATTI_FML_NeoForgeDevDistCleaner *LIBMATTI_FML_NeoForgeDevDistCleaner_New(
    LIBMATTI_FML_JarContents *minecraftModFile, LIBMATTI_DIST_Dist requestedDist);
void LIBMATTI_FML_NeoForgeDevDistCleaner_Free(LIBMATTI_FML_NeoForgeDevDistCleaner *cleaner);

#endif //MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_NEOFORGEDEVDISTCLEANER_H
