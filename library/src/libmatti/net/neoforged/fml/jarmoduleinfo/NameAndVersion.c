#include "libmatti/net/neoforged/fml/jarmoduleinfo/NameAndVersion.h"

#include <stdlib.h>

void LIBMATTI_FML_NameAndVersion_Free(LIBMATTI_FML_NameAndVersion *nameAndVersion)
{
    if (nameAndVersion == NULL)
        return;
    free(nameAndVersion->name);
    free(nameAndVersion->version);
    free(nameAndVersion);
}
