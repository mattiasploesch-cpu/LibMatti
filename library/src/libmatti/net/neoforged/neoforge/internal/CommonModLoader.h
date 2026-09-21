// Port of net.neoforged.neoforge.internal.CommonModLoader.
// Java's Executors are the port's ModWorkManager executors; both run on the caller's thread,
// so begin/load/finish keep Java's submission order.

#ifndef MATTICRAFT_NEOFORGE_INTERNAL_COMMONMODLOADER_H
#define MATTICRAFT_NEOFORGE_INTERNAL_COMMONMODLOADER_H

#include <stddef.h>

// Java: public static boolean areRegistriesLoaded()
int LIBMATTI_NEOFORGE_CommonModLoader_AreRegistriesLoaded(void);

// Java: protected static void begin(Runnable periodicTask, boolean datagen)
void LIBMATTI_NEOFORGE_CommonModLoader_Begin(void (*periodicTask)(void *userdata), void *userdata, int datagen);

// Java: protected static void load(Executor syncExecutor, Executor parallelExecutor)
void LIBMATTI_NEOFORGE_CommonModLoader_Load(void);

// Java: protected static void finish(Executor syncExecutor, Executor parallelExecutor)
void LIBMATTI_NEOFORGE_CommonModLoader_Finish(void);

#endif //MATTICRAFT_NEOFORGE_INTERNAL_COMMONMODLOADER_H
