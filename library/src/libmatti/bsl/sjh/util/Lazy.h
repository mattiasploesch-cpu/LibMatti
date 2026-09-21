#ifndef LIBMATTI_BSL_SJH_UTIL_LAZY_H
#define LIBMATTI_BSL_SJH_UTIL_LAZY_H

#include <stdbool.h>
#include <stddef.h>

typedef void *(*LIBMATTI_SJH_Lazy_Provider)(void *userData);
typedef void (*LIBMATTI_SJH_Lazy_Consumer)(void *value, void *userData);

// Port of cpw.mods.util.Lazy<T>.
// T maps to void *; the provider fills the value on first get().
typedef struct LIBMATTI_SJH_Lazy
{
    void *value;
    bool initialized;
    LIBMATTI_SJH_Lazy_Provider provider;
    void *providerUserData;
} LIBMATTI_SJH_Lazy;

LIBMATTI_SJH_Lazy *LIBMATTI_SJH_Lazy_Of(void);
LIBMATTI_SJH_Lazy *LIBMATTI_SJH_Lazy_OfValue(void *value);
LIBMATTI_SJH_Lazy *LIBMATTI_SJH_Lazy_OfProvider(LIBMATTI_SJH_Lazy_Provider provider, void *userData);
void *LIBMATTI_SJH_Lazy_Get(LIBMATTI_SJH_Lazy *lazy);
void LIBMATTI_SJH_Lazy_IfPresent(const LIBMATTI_SJH_Lazy *lazy, LIBMATTI_SJH_Lazy_Consumer consumer, void *userData);
void *LIBMATTI_SJH_Lazy_OrElse(const LIBMATTI_SJH_Lazy *lazy, void *elseValue);
void LIBMATTI_SJH_Lazy_Free(LIBMATTI_SJH_Lazy *lazy);

#endif