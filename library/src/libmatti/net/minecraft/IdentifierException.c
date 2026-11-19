


#include "libmatti/net/minecraft/IdentifierException.h"

#include "libmatti/java/lang/Class.h"

LIBMATTI_JL_Throwable *LIBMATTI_MC_IdentifierException_New(const char *message)
{



    return LIBMATTI_JL_Throwable_NewNamed("net.minecraft.IdentifierException", message);
}
