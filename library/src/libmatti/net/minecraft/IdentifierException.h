// Port of net.minecraft.IdentifierException.
// Java: public class IdentifierException extends RuntimeException

#ifndef MATTICRAFT_NET_MINECRAFT_IDENTIFIEREXCEPTION_H
#define MATTICRAFT_NET_MINECRAFT_IDENTIFIEREXCEPTION_H

#include "libmatti/java/lang/Throwable.h"

// Java: public IdentifierException(String message)
LIBMATTI_JL_Throwable *LIBMATTI_MC_IdentifierException_New(const char *message);

#endif //MATTICRAFT_NET_MINECRAFT_IDENTIFIEREXCEPTION_H
