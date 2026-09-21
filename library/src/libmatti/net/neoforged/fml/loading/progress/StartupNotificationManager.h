// Port of net.neoforged.fml.loading.progress.StartupNotificationManager.

#ifndef MATTICRAFT_FML_PROGRESS_STARTUPNOTIFICATIONMANAGER_H
#define MATTICRAFT_FML_PROGRESS_STARTUPNOTIFICATIONMANAGER_H

#include "libmatti/net/neoforged/fml/loading/progress/Message.h"
#include "libmatti/net/neoforged/fml/loading/progress/ProgressMeter.h"

#include <stddef.h>

// Java: public static List<ProgressMeter> getCurrentProgress()
// returns the internal array, do not free
LIBMATTI_FML_ProgressMeter **LIBMATTI_FML_StartupNotificationManager_GetCurrentProgress(size_t *count);

// Java: public static ProgressMeter prependProgressBar(String barName, int count)
LIBMATTI_FML_ProgressMeter *LIBMATTI_FML_StartupNotificationManager_PrependProgressBar(const char *barName, int count);
// Java: public static ProgressMeter addProgressBar(String barName, int count)
LIBMATTI_FML_ProgressMeter *LIBMATTI_FML_StartupNotificationManager_AddProgressBar(const char *barName, int count);
// Java: public static void popBar(ProgressMeter progressMeter)
void LIBMATTI_FML_StartupNotificationManager_PopBar(LIBMATTI_FML_ProgressMeter *progressMeter);

// Java: public record AgeMessage(int age, Message message)
typedef struct
{
    int age;
    LIBMATTI_FML_Message *message;
} LIBMATTI_FML_StartupNotificationManager_AgeMessage;

// Java: public static List<AgeMessage> getMessages() - caller frees the array and each element's 'message'
LIBMATTI_FML_StartupNotificationManager_AgeMessage *LIBMATTI_FML_StartupNotificationManager_GetMessages(size_t *count);

// Java: public static void addModMessage(String message)
void LIBMATTI_FML_StartupNotificationManager_AddModMessage(const char *message);
// Java: public static void modLoaderMessage(String message)
void LIBMATTI_FML_StartupNotificationManager_ModLoaderMessage(const char *message);

// Java: java.util.function.Consumer<String>
typedef void (*LIBMATTI_FML_StartupNotificationManager_Consumer)(const char *message);

// Java: public static Optional<Consumer<String>> modLoaderConsumer() - NULL = Optional.empty()
LIBMATTI_FML_StartupNotificationManager_Consumer LIBMATTI_FML_StartupNotificationManager_ModLoaderConsumer(void);
// Java: public static Optional<Consumer<String>> locatorConsumer()
LIBMATTI_FML_StartupNotificationManager_Consumer LIBMATTI_FML_StartupNotificationManager_LocatorConsumer(void);
// Java: public static Optional<Consumer<String>> mcLoaderConsumer()
LIBMATTI_FML_StartupNotificationManager_Consumer LIBMATTI_FML_StartupNotificationManager_McLoaderConsumer(void);

#endif //MATTICRAFT_FML_PROGRESS_STARTUPNOTIFICATIONMANAGER_H
