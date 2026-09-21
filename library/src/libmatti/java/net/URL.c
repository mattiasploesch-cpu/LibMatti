#include "libmatti/java/net/URL.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_JN_URL *LIBMATTI_JN_URL_New(const char *value)
{
    LIBMATTI_JN_URL *url = calloc(1, sizeof(LIBMATTI_JN_URL));
    url->value = value != NULL ? strdup(value) : NULL;
    return url;
}

void LIBMATTI_JN_URL_Free(LIBMATTI_JN_URL *url)
{
    if (url == NULL) return;
    free(url->value);
    free(url);
}

const char *LIBMATTI_JN_URL_ToString(const LIBMATTI_JN_URL *url)
{
    return url->value;
}
