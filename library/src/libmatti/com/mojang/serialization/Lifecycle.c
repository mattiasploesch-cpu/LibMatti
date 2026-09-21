// Port of com.mojang.serialization.Lifecycle.

#include "libmatti/com/mojang/serialization/Lifecycle.h"

// Java: private static final Lifecycle STABLE
LIBMATTI_MJS_Lifecycle LIBMATTI_MJS_Lifecycle_Stable(void)
{
    LIBMATTI_MJS_Lifecycle lifecycle = {LIBMATTI_MJS_Lifecycle_STABLE, 0};
    return lifecycle;
}

// Java: private static final Lifecycle EXPERIMENTAL
LIBMATTI_MJS_Lifecycle LIBMATTI_MJS_Lifecycle_Experimental(void)
{
    LIBMATTI_MJS_Lifecycle lifecycle = {LIBMATTI_MJS_Lifecycle_EXPERIMENTAL, 0};
    return lifecycle;
}

// Java: public static Lifecycle deprecated(int since)
LIBMATTI_MJS_Lifecycle LIBMATTI_MJS_Lifecycle_Deprecated(int since)
{
    LIBMATTI_MJS_Lifecycle lifecycle = {LIBMATTI_MJS_Lifecycle_DEPRECATED, since};
    return lifecycle;
}

// Java: public Lifecycle add(Lifecycle other)
LIBMATTI_MJS_Lifecycle LIBMATTI_MJS_Lifecycle_Add(LIBMATTI_MJS_Lifecycle lifecycle, LIBMATTI_MJS_Lifecycle other)
{
    if (lifecycle.kind == LIBMATTI_MJS_Lifecycle_EXPERIMENTAL ||
        other.kind == LIBMATTI_MJS_Lifecycle_EXPERIMENTAL)
        return LIBMATTI_MJS_Lifecycle_Experimental();
    if (lifecycle.kind == LIBMATTI_MJS_Lifecycle_DEPRECATED)
    {
        if (other.kind == LIBMATTI_MJS_Lifecycle_DEPRECATED && other.since < lifecycle.since)
            return other;
        return lifecycle;
    }
    if (other.kind == LIBMATTI_MJS_Lifecycle_DEPRECATED)
        return other;
    return LIBMATTI_MJS_Lifecycle_Stable();
}
