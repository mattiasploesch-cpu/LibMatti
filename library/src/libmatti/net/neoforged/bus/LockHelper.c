// Port of net.neoforged.bus.LockHelper.

#include "libmatti/net/neoforged/bus/LockHelper.h"

#include <stdlib.h>

struct LIBMATTI_BUS_LockHelper
{
    // Java: private final IntFunction<Map<K, V>> mapConstructor - IdentityHashMap::new vs size -> new HashMap<>(size)
    int identity;
    // Java: private final Map<K, V> backingMap
    const void **keys;
    void **values;
    size_t count;
    // Java: @Nullable private volatile Map<K, V> readOnlyView = null;
    const void **readKeys;
    void **readValues;
    size_t readCount;
    int hasReadOnlyView;
    // Java: private Object lock = new Object();
    pthread_mutex_t lock;
};

static LIBMATTI_BUS_LockHelper *new_helper(int identity)
{
    LIBMATTI_BUS_LockHelper *helper = calloc(1, sizeof(LIBMATTI_BUS_LockHelper));
    helper->identity = identity;
    pthread_mutex_init(&helper->lock, NULL);
    return helper;
}

// Java: public static <K, V> LockHelper<K, V> withHashMap()
LIBMATTI_BUS_LockHelper *LIBMATTI_BUS_LockHelper_WithHashMap(void)
{
    return new_helper(0);
}

// Java: public static <K, V> LockHelper<K, V> withIdentityHashMap()
LIBMATTI_BUS_LockHelper *LIBMATTI_BUS_LockHelper_WithIdentityHashMap(void)
{
    return new_helper(1);
}

void LIBMATTI_BUS_LockHelper_Free(LIBMATTI_BUS_LockHelper *helper)
{
    pthread_mutex_destroy(&helper->lock);
    free(helper->keys);
    free(helper->values);
    free(helper->readKeys);
    free(helper->readValues);
    free(helper);
}

static size_t find(const void *const *keys, size_t count, const void *key)
{
    for (size_t i = 0; i < count; i++)
        if (keys[i] == key) return i;
    return count;
}

// Java: Map<K, V> getReadMap()
size_t LIBMATTI_BUS_LockHelper_GetReadMap(LIBMATTI_BUS_LockHelper *helper,
                                          const void ***keys, void ***values)
{
    if (!helper->hasReadOnlyView)
    {
        // Java: need to update the read map
        pthread_mutex_lock(&helper->lock);
        free(helper->readKeys);
        free(helper->readValues);
        helper->readCount = helper->count;
        helper->readKeys = malloc(sizeof(*helper->readKeys) * (helper->count + 1));
        helper->readValues = malloc(sizeof(*helper->readValues) * (helper->count + 1));
        for (size_t i = 0; i < helper->count; i++)
        {
            helper->readKeys[i] = helper->keys[i];
            helper->readValues[i] = helper->values[i];
        }
        helper->hasReadOnlyView = 1;
        pthread_mutex_unlock(&helper->lock);
    }

    if (keys != NULL) *keys = helper->readKeys;
    if (values != NULL) *values = helper->readValues;
    return helper->readCount;
}

// Java: V get(K key)
void *LIBMATTI_BUS_LockHelper_Get(LIBMATTI_BUS_LockHelper *helper, const void *key)
{
    const void **keys;
    void **values;
    size_t count = LIBMATTI_BUS_LockHelper_GetReadMap(helper, &keys, &values);

    size_t index = find(keys, count, key);
    return index == count ? NULL : values[index];
}

// Java: boolean containsKey(K key)
int LIBMATTI_BUS_LockHelper_ContainsKey(LIBMATTI_BUS_LockHelper *helper, const void *key)
{
    const void **keys;
    size_t count = LIBMATTI_BUS_LockHelper_GetReadMap(helper, &keys, NULL);
    return find(keys, count, key) != count;
}

// Java: V computeIfAbsent(K key, Function<K, V> factory)
void *LIBMATTI_BUS_LockHelper_ComputeIfAbsent(LIBMATTI_BUS_LockHelper *helper, const void *key,
                                              void *(*factory)(void *userdata, const void *key), void *userdata)
{
    // Java: try lock-free get first
    void *ret = LIBMATTI_BUS_LockHelper_Get(helper, key);
    if (ret != NULL) return ret;

    // Java: pre-compute the new value without holding the lock (factory may recurse into this helper)
    void *intermediate = factory(userdata, key);

    pthread_mutex_lock(&helper->lock);
    size_t index = find(helper->keys, helper->count, key);
    if (index == helper->count)
    {
        // Java: ret = finalizer.apply(intermediate) - the identity finalizer keeps the value
        helper->keys = realloc(helper->keys, sizeof(*helper->keys) * (helper->count + 1));
        helper->values = realloc(helper->values, sizeof(*helper->values) * (helper->count + 1));
        helper->keys[helper->count] = key;
        helper->values[helper->count] = intermediate;
        helper->count++;
        helper->hasReadOnlyView = 0;
        ret = intermediate;
    }
    else
    {
        ret = helper->values[index];
    }
    pthread_mutex_unlock(&helper->lock);

    return ret;
}

// Java: public void clearAll()
void LIBMATTI_BUS_LockHelper_ClearAll(LIBMATTI_BUS_LockHelper *helper)
{
    pthread_mutex_lock(&helper->lock);
    helper->count = 0;
    helper->hasReadOnlyView = 0;
    pthread_mutex_unlock(&helper->lock);
}
