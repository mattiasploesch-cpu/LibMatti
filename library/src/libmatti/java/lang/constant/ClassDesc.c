#include "libmatti/java/lang/constant/ClassDesc.h"

#include <string.h>

// Java: ClassDescImpl.validateBinaryClassName(String) called from ClassDesc.of(String)
// The JDK validates against the JVMS 4.2.1 binary name rules; only that validation is reproduced here,
// since the descriptor itself is never materialised by the ported code.
static int validate_binary_class_name(const char *name)
{
    if (name == NULL) return 0;

    // JVMS 4.2.1: an array type is not a binary class name, and ';' is descriptor syntax
    if (strchr(name, '[') != NULL || strchr(name, ';') != NULL) return 0;

    // Every segment between the '/' separators has to be non-empty
    const char *segment = name;
    for (const char *p = name;; p++)
    {
        if (*p == '/' || *p == '\0')
        {
            if (p == segment) return 0; // empty segment (leading, trailing or doubled separator)
            if (*p == '\0') break;
            segment = p + 1;
        }
    }
    return 1;
}

int LIBMATTI_JLC_ClassDesc_Of(const char *name)
{
    return validate_binary_class_name(name);
}
