// Port of net.neoforged.fml.InterModComms.
// Java's Stream<IMCMessage> becomes a consumer callback; Java's spliterator removes every message it
// yields, so the port drains the queue the same way.

#ifndef MATTICRAFT_FML_INTERMODCOMMS_H
#define MATTICRAFT_FML_INTERMODCOMMS_H

#include "libmatti/java/util/function/Supplier.h"

#include <stddef.h>

// Java: public record IMCMessage(String senderModId, String modId, String method, Supplier<?> messageSupplier)
typedef struct
{
    char *senderModId;
    char *modId;
    char *method;
    LIBMATTI_JU_Supplier messageSupplier;
} LIBMATTI_FML_InterModComms_IMCMessage;

// Java: public static boolean sendTo(String modId, String method, Supplier<?> thing)
// Java defaults the sender to ModLoadingContext.get().getActiveContainer().getModId()
int LIBMATTI_FML_InterModComms_SendTo(const char *modId, const char *method, LIBMATTI_JU_Supplier thing);
// Java: public static boolean sendTo(String senderModId, String modId, String method, Supplier<?> thing)
int LIBMATTI_FML_InterModComms_SendToFrom(const char *senderModId, const char *modId, const char *method,
                                          LIBMATTI_JU_Supplier thing);

// Java: public static Stream<IMCMessage> getMessages(String modId, Predicate<String> methodMatcher)
void LIBMATTI_FML_InterModComms_GetMessages(
    const char *modId, int (*methodMatcher)(const char *method, void *userdata), void *matcherUserdata,
    void (*consumer)(const LIBMATTI_FML_InterModComms_IMCMessage *message, void *userdata), void *consumerUserdata);
// Java: public static Stream<IMCMessage> getMessages(String modId)
void LIBMATTI_FML_InterModComms_GetMessagesAll(
    const char *modId,
    void (*consumer)(const LIBMATTI_FML_InterModComms_IMCMessage *message, void *userdata), void *consumerUserdata);

#endif //MATTICRAFT_FML_INTERMODCOMMS_H
