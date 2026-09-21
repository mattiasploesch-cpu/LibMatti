#include "libmatti/net/neoforged/fml/loading/moddiscovery/IncompatibleModReason.h"

#include <stdlib.h>
#include <string.h>

// Java: the enum constants, in declaration order
static const char *const IDENTIFIERS[] = {
    "mcmod.info",
    "META-INF/mods.toml",
    "fabric.mod.json",
    "quilt.mod.json",
    "litemod.json",
    "optifine/Installer.class",
    "plugin.yml",
};

static const char *const NAMES[] = {
    "OLDFORGE", "MINECRAFT_FORGE", "FABRIC", "QUILT", "LITELOADER", "OPTIFINE", "BUKKIT",
};

// Java: public String getReason()
char *LIBMATTI_FML_IncompatibleModReason_GetReason(LIBMATTI_FML_IncompatibleModReason reason)
{
    // Java: "fml.modloadingissue.brokenfile." + StringUtils.toLowerCase(name())
    const char *name = NAMES[reason];
    size_t length = strlen("fml.modloadingissue.brokenfile.") + strlen(name) + 1;
    char *result = malloc(length);
    strcpy(result, "fml.modloadingissue.brokenfile.");
    size_t written = strlen(result);
    for (const char *p = name; *p != '\0'; p++)
        result[written++] = (char) (*p >= 'A' && *p <= 'Z' ? *p + 32 : *p);
    result[written] = '\0';
    return result;
}

// Java: public static Optional<IncompatibleModReason> detect(JarContents jar)
int LIBMATTI_FML_IncompatibleModReason_Detect(const LIBMATTI_FML_JarContents *jar,
                                              LIBMATTI_FML_IncompatibleModReason *out)
{
    for (int i = 0; i < 7; i++)
    {
        if (LIBMATTI_FML_JarContents_ContainsFile(jar, IDENTIFIERS[i]))
        {
            *out = (LIBMATTI_FML_IncompatibleModReason) i;
            return 1;
        }
    }
    return 0;
}
