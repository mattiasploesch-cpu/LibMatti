// Port of com.mojang.brigadier.Message / LiteralMessage.

#include "libmatti/com/mojang/brigadier/Message.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_BR_Message *LIBMATTI_BR_LiteralMessage_New(const char *string)
{
    LIBMATTI_BR_Message *message = calloc(1, sizeof(LIBMATTI_BR_Message));
    message->string = strdup(string);
    return message;
}

void LIBMATTI_BR_Message_Free(LIBMATTI_BR_Message *message)
{
    free(message->string);
    free(message);
}
