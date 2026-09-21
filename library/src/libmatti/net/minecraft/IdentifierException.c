// Port of net.minecraft.IdentifierException.

#include "libmatti/net/minecraft/IdentifierException.h"

#include "libmatti/java/lang/Class.h"

LIBMATTI_JL_Throwable *LIBMATTI_MC_IdentifierException_New(const char *message)
{
    // Java has no class object for the exception in the C port; the Throwable port carries
    // the runtime type as its name.
    return LIBMATTI_JL_Throwable_NewNamed("net.minecraft.IdentifierException", message);
}
