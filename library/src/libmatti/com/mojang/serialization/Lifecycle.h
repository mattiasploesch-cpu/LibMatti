// Port of com.mojang.serialization.Lifecycle (the subset the registries use).
// Java's singleton instances become the enum-like constants; Deprecated carries its since.

#ifndef MATTICRAFT_MOJANG_SERIALIZATION_LIFECYCLE_H
#define MATTICRAFT_MOJANG_SERIALIZATION_LIFECYCLE_H

// Java: public class Lifecycle - STABLE, EXPERIMENTAL and Deprecated(since)
typedef enum LIBMATTI_MJS_LifecycleKind
{
    LIBMATTI_MJS_Lifecycle_STABLE = 0,
    LIBMATTI_MJS_Lifecycle_EXPERIMENTAL = 1,
    LIBMATTI_MJS_Lifecycle_DEPRECATED = 2
} LIBMATTI_MJS_LifecycleKind;

typedef struct LIBMATTI_MJS_Lifecycle
{
    LIBMATTI_MJS_LifecycleKind kind;
    // Java: Deprecated.since
    int since;
} LIBMATTI_MJS_Lifecycle;

// Java: public static Lifecycle stable()
LIBMATTI_MJS_Lifecycle LIBMATTI_MJS_Lifecycle_Stable(void);
// Java: public static Lifecycle experimental()
LIBMATTI_MJS_Lifecycle LIBMATTI_MJS_Lifecycle_Experimental(void);
// Java: public static Lifecycle deprecated(int since)
LIBMATTI_MJS_Lifecycle LIBMATTI_MJS_Lifecycle_Deprecated(int since);
// Java: public Lifecycle add(Lifecycle other)
LIBMATTI_MJS_Lifecycle LIBMATTI_MJS_Lifecycle_Add(LIBMATTI_MJS_Lifecycle lifecycle, LIBMATTI_MJS_Lifecycle other);

#endif //MATTICRAFT_MOJANG_SERIALIZATION_LIFECYCLE_H
