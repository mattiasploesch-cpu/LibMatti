// Port of com.mojang.brigadier.Message and com.mojang.brigadier.LiteralMessage.
// Java's interface becomes the struct; the only implementation Brigadier ships
// is LiteralMessage, so the port folds both into one type.

#ifndef MATTICRAFT_BR_MESSAGE_H
#define MATTICRAFT_BR_MESSAGE_H

// Java: public interface Message { String getString(); }
typedef struct LIBMATTI_BR_Message
{
    // Java: LiteralMessage.string (owned)
    char *string;
} LIBMATTI_BR_Message;

// Java: public LiteralMessage(final String string)
LIBMATTI_BR_Message *LIBMATTI_BR_LiteralMessage_New(const char *string);
void LIBMATTI_BR_Message_Free(LIBMATTI_BR_Message *message);

#endif
