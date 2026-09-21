// Port of net.neoforged.fml.InterModComms.

#include "libmatti/net/neoforged/fml/InterModComms.h"

#include "libmatti/net/neoforged/fml/ModList.h"
#include "libmatti/net/neoforged/fml/ModLoadingContext.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

// Java: private static ConcurrentMap<String, ConcurrentLinkedQueue<IMCMessage>> containerQueues
typedef struct
{
    char *modId;
    LIBMATTI_FML_InterModComms_IMCMessage *messages;
    size_t count;
} IMCQueue;

static IMCQueue *containerQueues = NULL;
static size_t containerQueueCount = 0;
static pthread_mutex_t queuesLock = PTHREAD_MUTEX_INITIALIZER;

static IMCQueue *queue_for(const char *modId, int create)
{
    for (size_t i = 0; i < containerQueueCount; i++)
        if (strcmp(containerQueues[i].modId, modId) == 0) return &containerQueues[i];

    if (!create) return NULL;

    // Java: containerQueues.computeIfAbsent(modId, k -> new ConcurrentLinkedQueue<>())
    containerQueues = realloc(containerQueues, sizeof(*containerQueues) * (containerQueueCount + 1));
    IMCQueue *queue = &containerQueues[containerQueueCount++];
    queue->modId = strdup(modId);
    queue->messages = NULL;
    queue->count = 0;
    return queue;
}

// Java: public static boolean sendTo(String senderModId, String modId, String method, Supplier<?> thing)
int LIBMATTI_FML_InterModComms_SendToFrom(const char *senderModId, const char *modId, const char *method,
                                          LIBMATTI_JU_Supplier thing)
{
    LIBMATTI_FML_ModList *modList = LIBMATTI_FML_ModList_Get();
    if (modList == NULL || !LIBMATTI_FML_ModList_IsLoaded(modList, modId)) return 0;

    pthread_mutex_lock(&queuesLock);
    IMCQueue *queue = queue_for(modId, 1);
    queue->messages = realloc(queue->messages, sizeof(*queue->messages) * (queue->count + 1));
    LIBMATTI_FML_InterModComms_IMCMessage *message = &queue->messages[queue->count++];
    message->senderModId = strdup(senderModId);
    message->modId = strdup(modId);
    message->method = strdup(method);
    message->messageSupplier = thing;
    pthread_mutex_unlock(&queuesLock);

    return 1;
}

// Java: containerQueues...add(new IMCMessage(ModLoadingContext.get().getActiveContainer().getModId(), modId, method, thing))
int LIBMATTI_FML_InterModComms_SendTo(const char *modId, const char *method, LIBMATTI_JU_Supplier thing)
{
    LIBMATTI_FML_ModLoadingContext *context = LIBMATTI_FML_ModLoadingContext_Get();
    LIBMATTI_FML_ModContainer *activeContainer = LIBMATTI_FML_ModLoadingContext_GetActiveContainer(context);
    if (activeContainer == NULL) return 0;

    return LIBMATTI_FML_InterModComms_SendToFrom(LIBMATTI_FML_ModContainer_GetModId(activeContainer), modId, method,
                                                 thing);
}

// Java: public static Stream<IMCMessage> getMessages(String modId, Predicate<String> methodMatcher)
void LIBMATTI_FML_InterModComms_GetMessages(
    const char *modId, int (*methodMatcher)(const char *method, void *userdata), void *matcherUserdata,
    void (*consumer)(const LIBMATTI_FML_InterModComms_IMCMessage *message, void *userdata), void *consumerUserdata)
{
    pthread_mutex_lock(&queuesLock);
    IMCQueue *queue = queue_for(modId, 0);
    if (queue == NULL)
    {
        pthread_mutex_unlock(&queuesLock);
        return;
    }

    // Java: tryAdvance yields the next matching message and removes it from the queue
    for (size_t i = 0; i < queue->count;)
    {
        LIBMATTI_FML_InterModComms_IMCMessage *message = &queue->messages[i];
        if (!methodMatcher(message->method, matcherUserdata))
        {
            i++;
            continue;
        }

        consumer(message, consumerUserdata);
        memmove(&queue->messages[i], &queue->messages[i + 1],
                sizeof(*queue->messages) * (queue->count - i - 1));
        queue->count--;
    }
    pthread_mutex_unlock(&queuesLock);
}

static int accept_any_method(const char *method, void *userdata)
{
    (void)method;
    (void)userdata;
    return 1;
}

// Java: public static Stream<IMCMessage> getMessages(String modId) { return getMessages(modId, s -> Boolean.TRUE); }
void LIBMATTI_FML_InterModComms_GetMessagesAll(
    const char *modId,
    void (*consumer)(const LIBMATTI_FML_InterModComms_IMCMessage *message, void *userdata), void *consumerUserdata)
{
    LIBMATTI_FML_InterModComms_GetMessages(modId, accept_any_method, NULL, consumer, consumerUserdata);
}
