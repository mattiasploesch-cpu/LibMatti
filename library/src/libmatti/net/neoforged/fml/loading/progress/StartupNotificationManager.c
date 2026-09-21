#include "libmatti/net/neoforged/fml/loading/progress/StartupNotificationManager.h"

#include <stdlib.h>
#include <string.h>

// Java: private static volatile EnumMap<Message.MessageType, List<Message>> messages = new EnumMap<>(Message.MessageType.class);
static LIBMATTI_FML_Message **messages[LIBMATTI_FML_MESSAGE_MESSAGE_TYPE_COUNT];
static size_t messageCounts[LIBMATTI_FML_MESSAGE_MESSAGE_TYPE_COUNT];

// Java: private static final Deque<ProgressMeter> progressMeters = new ArrayDeque<>();
static LIBMATTI_FML_ProgressMeter **progressMeters = NULL;
static size_t progressMeterCount = 0;

// Java: private synchronized static void addMessage(Message.MessageType type, String message, int maxSize)
static void add_message(LIBMATTI_FML_Message_MessageType type, const char *message, int maxSize)
{
    LIBMATTI_FML_Message *entry = LIBMATTI_FML_Message_New(message, type);

    // Java: var newList = new ArrayList<Message>(); ... newList.add(new Message(message, type));
    if (maxSize < 0 || (size_t) maxSize >= messageCounts[type])
    {
        messages[type] = realloc(messages[type], sizeof(*messages[type]) * (messageCounts[type] + 1));
        messages[type][messageCounts[type]++] = entry;
        return;
    }

    // Java: newList.addAll(existingList.subList(0, Math.min(existingList.size(), maxSize)));
    size_t kept = (size_t) maxSize;
    LIBMATTI_FML_Message **newList = malloc(sizeof(*newList) * (kept + 1));
    for (size_t i = 0; i < kept; i++)
        newList[i] = messages[type][i];
    for (size_t i = kept; i < messageCounts[type]; i++)
        LIBMATTI_FML_Message_Free(messages[type][i]);
    newList[kept] = entry;

    free(messages[type]);
    messages[type] = newList;
    messageCounts[type] = kept + 1;
}

// Java: com.google.common.base.CharMatcher.ascii().retainFrom(message)
static char *retain_ascii(const char *message)
{
    size_t length = strlen(message);
    char *result = malloc(length + 1);
    size_t written = 0;

    for (size_t i = 0; i < length; i++)
    {
        if ((unsigned char) message[i] > 0x7F) continue;
        result[written++] = message[i];
    }

    result[written] = '\0';
    return result;
}

// Java: com.google.common.base.Ascii.truncate(seq, maxLength, truncationIndicator)
static char *ascii_truncate(const char *value, size_t maxLength, const char *truncationIndicator)
{
    size_t length = strlen(value);
    if (length <= maxLength) return strdup(value);

    size_t truncationLength = maxLength - strlen(truncationIndicator);
    size_t resultLength = truncationLength + strlen(truncationIndicator) + 1;
    char *result = malloc(resultLength);

    memcpy(result, value, truncationLength);
    strcpy(result + truncationLength, truncationIndicator);
    return result;
}

// Java: public static List<ProgressMeter> getCurrentProgress()
LIBMATTI_FML_ProgressMeter **LIBMATTI_FML_StartupNotificationManager_GetCurrentProgress(size_t *count)
{
    // Java: synchronized (progressMeters) { return progressMeters.stream().toList(); }
    *count = progressMeterCount;
    return progressMeters;
}

// Java: public static ProgressMeter prependProgressBar(String barName, int count)
LIBMATTI_FML_ProgressMeter *LIBMATTI_FML_StartupNotificationManager_PrependProgressBar(const char *barName, int count)
{
    LIBMATTI_FML_ProgressMeter *progressMeter =
        LIBMATTI_FML_ProgressMeter_New(barName, count, 0, LIBMATTI_FML_Message_New(barName, LIBMATTI_FML_Message_MessageType_ML));

    // Java: progressMeters.addFirst(pm);
    progressMeters = realloc(progressMeters, sizeof(*progressMeters) * (progressMeterCount + 1));
    memmove(&progressMeters[1], &progressMeters[0], sizeof(*progressMeters) * progressMeterCount);
    progressMeters[0] = progressMeter;
    progressMeterCount++;

    return progressMeter;
}

// Java: public static ProgressMeter addProgressBar(String barName, int count)
LIBMATTI_FML_ProgressMeter *LIBMATTI_FML_StartupNotificationManager_AddProgressBar(const char *barName, int count)
{
    LIBMATTI_FML_ProgressMeter *progressMeter =
        LIBMATTI_FML_ProgressMeter_New(barName, count, 0, LIBMATTI_FML_Message_New(barName, LIBMATTI_FML_Message_MessageType_ML));

    // Java: progressMeters.addLast(pm);
    progressMeters = realloc(progressMeters, sizeof(*progressMeters) * (progressMeterCount + 1));
    progressMeters[progressMeterCount++] = progressMeter;

    return progressMeter;
}

// Java: public static void popBar(ProgressMeter progressMeter)
void LIBMATTI_FML_StartupNotificationManager_PopBar(LIBMATTI_FML_ProgressMeter *progressMeter)
{
    // Java: progressMeters.removeLastOccurrence(progressMeter);
    for (size_t i = progressMeterCount; i > 0; i--)
    {
        if (progressMeters[i - 1] != progressMeter) continue;

        memmove(&progressMeters[i - 1], &progressMeters[i], sizeof(*progressMeters) * (progressMeterCount - i));
        progressMeterCount--;
        return;
    }
}

// Java: Comparator.comparingLong(Message::timestamp).thenComparing(Message::getText)
static int compare_messages(const void *left, const void *right)
{
    const LIBMATTI_FML_Message *a = *(const LIBMATTI_FML_Message *const *) left;
    const LIBMATTI_FML_Message *b = *(const LIBMATTI_FML_Message *const *) right;

    if (a->timestamp != b->timestamp) return a->timestamp < b->timestamp ? -1 : 1;
    return strcmp(a->text, b->text);
}

// Java: public static List<AgeMessage> getMessages()
LIBMATTI_FML_StartupNotificationManager_AgeMessage *LIBMATTI_FML_StartupNotificationManager_GetMessages(size_t *count)
{
    // Java: long ts = System.nanoTime();
    long timestamp = LIBMATTI_FML_Message_NanoTime();

    size_t total = 0;
    for (int type = 0; type < LIBMATTI_FML_MESSAGE_MESSAGE_TYPE_COUNT; type++)
        total += messageCounts[type];

    LIBMATTI_FML_Message **sorted = malloc(sizeof(*sorted) * (total == 0 ? 1 : total));
    size_t index = 0;
    for (int type = 0; type < LIBMATTI_FML_MESSAGE_MESSAGE_TYPE_COUNT; type++)
    {
        for (size_t i = 0; i < messageCounts[type]; i++)
            sorted[index++] = messages[type][i];
    }

    // Java: .sorted(...).reversed().limit(2)
    if (total > 1) qsort(sorted, total, sizeof(*sorted), compare_messages);
    if (total > 2) total = 2;

    LIBMATTI_FML_StartupNotificationManager_AgeMessage *result = malloc(sizeof(*result) * (total == 0 ? 1 : total));
    for (size_t i = 0; i < total; i++)
    {
        LIBMATTI_FML_Message *message = sorted[total - 1 - i];
        result[i].age = (int) ((timestamp - message->timestamp) / 1000000);
        result[i].message = message;
    }

    free(sorted);
    *count = total;
    return result;
}

// Java: public static void addModMessage(String message)
void LIBMATTI_FML_StartupNotificationManager_AddModMessage(const char *message)
{
    // Java: String safeMessage = Ascii.truncate(CharMatcher.ascii().retainFrom(message), 80, "~"); addMessage(MOD, safeMessage, 20);
    char *ascii = retain_ascii(message);
    char *safeMessage = ascii_truncate(ascii, 80, "~");
    free(ascii);

    add_message(LIBMATTI_FML_Message_MessageType_MOD, safeMessage, 20);
    free(safeMessage);
}

// Java: public static void modLoaderMessage(String message)
void LIBMATTI_FML_StartupNotificationManager_ModLoaderMessage(const char *message)
{
    // Java: addMessage(Message.MessageType.ML, message, -1);
    add_message(LIBMATTI_FML_Message_MessageType_ML, message, -1);
}

static void consume_mod_loader_message(const char *message)
{
    add_message(LIBMATTI_FML_Message_MessageType_ML, message, -1);
}

static void consume_locator_message(const char *message)
{
    add_message(LIBMATTI_FML_Message_MessageType_LOC, message, -1);
}

static void consume_mc_loader_message(const char *message)
{
    add_message(LIBMATTI_FML_Message_MessageType_MC, message, -1);
}

// Java: public static Optional<Consumer<String>> modLoaderConsumer()
LIBMATTI_FML_StartupNotificationManager_Consumer LIBMATTI_FML_StartupNotificationManager_ModLoaderConsumer(void)
{
    return consume_mod_loader_message;
}

// Java: public static Optional<Consumer<String>> locatorConsumer()
LIBMATTI_FML_StartupNotificationManager_Consumer LIBMATTI_FML_StartupNotificationManager_LocatorConsumer(void)
{
    return consume_locator_message;
}

// Java: public static Optional<Consumer<String>> mcLoaderConsumer()
LIBMATTI_FML_StartupNotificationManager_Consumer LIBMATTI_FML_StartupNotificationManager_McLoaderConsumer(void)
{
    return consume_mc_loader_message;
}
