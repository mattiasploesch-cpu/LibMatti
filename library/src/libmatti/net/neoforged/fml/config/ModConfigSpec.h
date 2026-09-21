// Port-only stand-in for NeoForge's ModConfigSpec (net.neoforged.neoforge.common.ModConfigSpec).
// The real spec is the game port's part; until it exists a mod (or a test) builds this
// simple spec, which mirrors the SimpleConfigSpec of FML's ConfigTrackerTest: one integer
// value "configEntry" that is corrected to its default and commented.

#ifndef MATTICRAFT_FML_CONFIG_MODCONFIGSPEC_H
#define MATTICRAFT_FML_CONFIG_MODCONFIGSPEC_H

#include "libmatti/net/neoforged/fml/config/IConfigSpec.h"

typedef struct LIBMATTI_FML_ModConfigSpec LIBMATTI_FML_ModConfigSpec;

struct LIBMATTI_FML_ModConfigSpec
{
    // Java: implements IConfigSpec - the interface ModContainer.registerConfig receives
    LIBMATTI_FML_IConfigSpec spec;

    // the loaded value (Java: spec.acceptConfig stores the config; SimpleConfigSpec keeps an int)
    int loadedValue;
};

// Java: new SimpleConfigSpec() - "configEntry" with the default 4 and the comment "Test comment:"
LIBMATTI_FML_ModConfigSpec *LIBMATTI_FML_ModConfigSpec_New(void);
void LIBMATTI_FML_ModConfigSpec_Free(LIBMATTI_FML_ModConfigSpec *spec);

#endif //MATTICRAFT_FML_CONFIG_MODCONFIGSPEC_H
