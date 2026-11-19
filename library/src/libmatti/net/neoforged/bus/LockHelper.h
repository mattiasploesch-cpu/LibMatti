





#ifndef MATTICRAFT_BUS_LOCKHELPER_H
#define MATTICRAFT_BUS_LOCKHELPER_H

#include <pthread.h>
#include <stddef.h>

typedef struct LIBMATTI_BUS_LockHelper LIBMATTI_BUS_LockHelper;

// Java: public static <K, V> LockHelper<K, V> withHashMap()
LIBMATTI_BUS_LockHelper *LIBMATTI_BUS_LockHelper_WithHashMap(void);
// Java: public static <K, V> LockHelper<K, V> withIdentityHashMap()
LIBMATTI_BUS_LockHelper *LIBMATTI_BUS_LockHelper_WithIdentityHashMap(void);
void LIBMATTI_BUS_LockHelper_Free(LIBMATTI_BUS_LockHelper *helper);

// Java: V get(K key)
void *LIBMATTI_BUS_LockHelper_Get(LIBMATTI_BUS_LockHelper *helper, const void *key);
// Java: boolean containsKey(K key)
int LIBMATTI_BUS_LockHelper_ContainsKey(LIBMATTI_BUS_LockHelper *helper, const void *key);

// Java: V computeIfAbsent(K key, Function<K, V> factory)
// The Java lambda captures, so the C port passes the captured state as userdata.
// The two-function overload (factory + finalizer) is only used by EventListenerFactory's cache.
void *LIBMATTI_BUS_LockHelper_ComputeIfAbsent(LIBMATTI_BUS_LockHelper *helper, const void *key,
                                              void *(*factory)(void *userdata, const void *key), void *userdata);

// Java: Map<K, V> getReadMap() - the read view, published as parallel key/value arrays
size_t LIBMATTI_BUS_LockHelper_GetReadMap(LIBMATTI_BUS_LockHelper *helper,
                                          const void ***keys, void ***values);

// Java: public void clearAll()
void LIBMATTI_BUS_LockHelper_ClearAll(LIBMATTI_BUS_LockHelper *helper);

#endif
