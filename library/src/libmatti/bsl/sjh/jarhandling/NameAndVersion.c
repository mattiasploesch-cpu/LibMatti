//
// Created by administrator on 09.09.26.
//

#include "NameAndVersion.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_JH_NameAndVersion *LIBMATTI_JH_NameAndVersion_Create(const char *name, const char *version)
{
    LIBMATTI_JH_NameAndVersion *result = calloc(1, sizeof(LIBMATTI_JH_NameAndVersion));

    result->name = strdup(name);
    if (version != NULL) result->version = strdup(version);

    return result;
}

void LIBMATTI_JH_NameAndVersion_Free(LIBMATTI_JH_NameAndVersion *nameAndVersion)
{
    if (nameAndVersion == NULL) return;

    free(nameAndVersion->name);
    free(nameAndVersion->version);
    free(nameAndVersion);
}