#include "libmatti/net/neoforged/neoforgespi/transformation/NameValidation.h"

#include "libmatti/java/lang/constant/ClassDesc.h"
#include "libmatti/java/lang/constant/MethodTypeDesc.h"

#include <string.h>

int LIBMATTI_NEOFORGESPI_NameValidation_ValidateClassName(const char *name)
{
    // Java: ClassDesc.of(name)
    return LIBMATTI_JLC_ClassDesc_Of(name);
}

int LIBMATTI_NEOFORGESPI_NameValidation_ValidateUnqualified(const char *name)
{
    // Java: ".;[/<>".chars().forEach(c -> { if (name.indexOf(c) != -1) throw new IllegalArgumentException(...) })
    return strpbrk(name, ".;[/<>") == NULL;
}

int LIBMATTI_NEOFORGESPI_NameValidation_ValidateMethod(const char *name, const char *descriptor)
{
    // Java: if (name.equals("<init>") || (name.equals("<clinit>") && descriptor.equals("()V"))) return;
    if (strcmp(name, "<init>") == 0) return 1;
    if (strcmp(name, "<clinit>") == 0 && strcmp(descriptor, "()V") == 0) return 1;

    if (!LIBMATTI_NEOFORGESPI_NameValidation_ValidateUnqualified(name)) return 0;
    // Java: MethodTypeDesc.ofDescriptor(descriptor)
    return LIBMATTI_JLC_MethodTypeDesc_OfDescriptor(descriptor);
}
