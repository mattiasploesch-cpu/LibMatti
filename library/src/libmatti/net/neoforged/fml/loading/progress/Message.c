#include "libmatti/net/neoforged/fml/loading/progress/Message.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

// Java: the enum's colours
static const float COLOURS[LIBMATTI_FML_MESSAGE_MESSAGE_TYPE_COUNT][3] = {
    {1.0f, 1.0f, 1.0f}, // MC
    {0.0f, 0.0f, 0.5f}, // ML
    {0.0f, 0.5f, 0.0f}, // LOC
    {0.5f, 0.0f, 0.0f}, // MOD
};

static const char *const NAMES[LIBMATTI_FML_MESSAGE_MESSAGE_TYPE_COUNT] = {"MC", "ML", "LOC", "MOD"};

// Java: System.nanoTime()
long LIBMATTI_FML_Message_NanoTime(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (long) now.tv_sec * 1000000000L + now.tv_nsec;
}

// Java: public Message(String text, MessageType type)
LIBMATTI_FML_Message *LIBMATTI_FML_Message_New(const char *text, LIBMATTI_FML_Message_MessageType type)
{
    LIBMATTI_FML_Message *message = calloc(1, sizeof(LIBMATTI_FML_Message));
    message->text = text != NULL ? strdup(text) : NULL;
    message->type = type;
    message->timestamp = LIBMATTI_FML_Message_NanoTime();
    return message;
}

void LIBMATTI_FML_Message_Free(LIBMATTI_FML_Message *message)
{
    if (message == NULL) return;

    free(message->text);
    free(message);
}

// Java: public String getText()
const char *LIBMATTI_FML_Message_GetText(const LIBMATTI_FML_Message *message)
{
    return message->text;
}

// Java: MessageType getType()
LIBMATTI_FML_Message_MessageType LIBMATTI_FML_Message_GetType(const LIBMATTI_FML_Message *message)
{
    return message->type;
}

// Java: long timestamp()
long LIBMATTI_FML_Message_Timestamp(const LIBMATTI_FML_Message *message)
{
    return message->timestamp;
}

// Java: public float[] getTypeColour()
void LIBMATTI_FML_Message_GetTypeColour(const LIBMATTI_FML_Message *message, float *colour)
{
    LIBMATTI_FML_Message_MessageType_Colour(message->type, colour);
}

// Java: MessageType.colour()
void LIBMATTI_FML_Message_MessageType_Colour(LIBMATTI_FML_Message_MessageType type, float *colour)
{
    memcpy(colour, COLOURS[type], sizeof(COLOURS[type]));
}

// Java: the enum's name()
const char *LIBMATTI_FML_Message_MessageType_Name(LIBMATTI_FML_Message_MessageType type)
{
    return NAMES[type];
}
