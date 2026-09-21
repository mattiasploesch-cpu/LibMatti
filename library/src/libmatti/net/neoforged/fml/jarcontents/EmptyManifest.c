#include "libmatti/net/neoforged/fml/jarcontents/EmptyManifest.h"

static LIBMATTI_JU_Manifest *instance = NULL;

// Java: static final Manifest INSTANCE = new Manifest()
LIBMATTI_JU_Manifest *LIBMATTI_FML_EmptyManifest_Instance(void)
{
    if (instance == NULL)
        instance = LIBMATTI_JU_Manifest_CreateEmpty();
    return instance;
}
