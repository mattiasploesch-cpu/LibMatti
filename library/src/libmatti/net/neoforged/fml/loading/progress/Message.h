// Port of net.neoforged.fml.loading.progress.Message.

#ifndef MATTICRAFT_FML_PROGRESS_MESSAGE_H
#define MATTICRAFT_FML_PROGRESS_MESSAGE_H

#include <stddef.h>

// Java: public enum MessageType { MC, ML, LOC, MOD }
typedef enum
{
    LIBMATTI_FML_Message_MessageType_MC,
    LIBMATTI_FML_Message_MessageType_ML,
    LIBMATTI_FML_Message_MessageType_LOC,
    LIBMATTI_FML_Message_MessageType_MOD
} LIBMATTI_FML_Message_MessageType;

#define LIBMATTI_FML_MESSAGE_MESSAGE_TYPE_COUNT 4

// Java: public class Message
typedef struct LIBMATTI_FML_Message
{
    char *text;
    LIBMATTI_FML_Message_MessageType type;
    long timestamp;
} LIBMATTI_FML_Message;

// Java: public Message(String text, MessageType type)
LIBMATTI_FML_Message *LIBMATTI_FML_Message_New(const char *text, LIBMATTI_FML_Message_MessageType type);
void LIBMATTI_FML_Message_Free(LIBMATTI_FML_Message *message);

// Java: public String getText()
const char *LIBMATTI_FML_Message_GetText(const LIBMATTI_FML_Message *message);
// Java: MessageType getType()
LIBMATTI_FML_Message_MessageType LIBMATTI_FML_Message_GetType(const LIBMATTI_FML_Message *message);
// Java: long timestamp() - System.nanoTime()
long LIBMATTI_FML_Message_Timestamp(const LIBMATTI_FML_Message *message);

// Java: public float[] getTypeColour()
// 'colour' receives r, g, b
void LIBMATTI_FML_Message_GetTypeColour(const LIBMATTI_FML_Message *message, float *colour);
// Java: MessageType.colour()
void LIBMATTI_FML_Message_MessageType_Colour(LIBMATTI_FML_Message_MessageType type, float *colour);
// Java: the enum's name()
const char *LIBMATTI_FML_Message_MessageType_Name(LIBMATTI_FML_Message_MessageType type);

// Java: System.nanoTime()
long LIBMATTI_FML_Message_NanoTime(void);

#endif //MATTICRAFT_FML_PROGRESS_MESSAGE_H
