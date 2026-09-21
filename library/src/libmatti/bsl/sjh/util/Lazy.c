#include "Lazy.h"

#include <stdlib.h>

LIBMATTI_SJH_Lazy *LIBMATTI_SJH_Lazy_Of(void)
{
    return LIBMATTI_SJH_Lazy_OfProvider(NULL, NULL);
}

LIBMATTI_SJH_Lazy *LIBMATTI_SJH_Lazy_OfValue(void *value)
{
    LIBMATTI_SJH_Lazy *lazy = malloc(sizeof(LIBMATTI_SJH_Lazy));
    lazy->value = value;
    lazy->initialized = true;
    lazy->provider = NULL;
    lazy->providerUserData = NULL;
    return lazy;
}

LIBMATTI_SJH_Lazy *LIBMATTI_SJH_Lazy_OfProvider(LIBMATTI_SJH_Lazy_Provider provider, void *userData)
{
    LIBMATTI_SJH_Lazy *lazy = malloc(sizeof(LIBMATTI_SJH_Lazy));
    lazy->value = NULL;
    lazy->initialized = false;
    lazy->provider = provider;
    lazy->providerUserData = userData;
    return lazy;
}

void *LIBMATTI_SJH_Lazy_Get(LIBMATTI_SJH_Lazy *lazy)
{
    // Java: synchronized (lock) { if (!initialized && provider != null) { initialized = true; value = provider.get(); } return value; }
    if (!lazy->initialized && lazy->provider != NULL)
    {
        lazy->initialized = true;
        lazy->value = lazy->provider(lazy->providerUserData);
    }
    return lazy->value;
}

void LIBMATTI_SJH_Lazy_IfPresent(const LIBMATTI_SJH_Lazy *lazy, LIBMATTI_SJH_Lazy_Consumer consumer, void *userData)
{
    if (!lazy->initialized)
        return;
    consumer(lazy->value, userData);
}

void *LIBMATTI_SJH_Lazy_OrElse(const LIBMATTI_SJH_Lazy *lazy, void *elseValue)
{
    if (!lazy->initialized)
        return elseValue;
    return lazy->value;
}

void LIBMATTI_SJH_Lazy_Free(LIBMATTI_SJH_Lazy *lazy)
{
    free(lazy);
}