// Port of net.neoforged.bus.ListenerList.

#include "libmatti/net/neoforged/bus/ListenerList.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/bus/api/ICancellableEvent.h"

#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

static void force_rebuild(LIBMATTI_BUS_ListenerList *list);

// Java: private ArrayList<EventListener> getListeners(EventPriority priority)
// Returns a fresh list: the listeners of this list for the priority, then the parent's.
static LIBMATTI_BUS_EventListener **get_listeners(LIBMATTI_BUS_ListenerList *list,
                                                  LIBMATTI_BUS_EventPriority priority, size_t *count)
{
    pthread_mutex_lock(&list->writeLock);
    size_t size = list->priorityCounts[priority];
    LIBMATTI_BUS_EventListener **ret = malloc(sizeof(*ret) * (size + 1));
    memcpy(ret, list->priorities[priority], sizeof(*ret) * size);
    pthread_mutex_unlock(&list->writeLock);

    if (list->parent != NULL)
    {
        size_t parentCount;
        LIBMATTI_BUS_EventListener **parentListeners = get_listeners(list->parent, priority, &parentCount);
        ret = realloc(ret, sizeof(*ret) * (size + parentCount + 1));
        memcpy(ret + size, parentListeners, sizeof(*ret) * parentCount);
        free(parentListeners);
        size += parentCount;
    }

    *count = size;
    return ret;
}

// Java: private void unwrapListeners(List<EventListener> ret)
static void unwrap_listeners(LIBMATTI_BUS_ListenerList *list, LIBMATTI_BUS_EventListener **listeners, size_t count)
{
    if (!list->canUnwrapListeners) return;

    for (size_t i = 0; i < count; i++)
        if (listeners[i]->getWithoutCheck != NULL)
            listeners[i] = listeners[i]->getWithoutCheck(listeners[i]->self);
}

static void free_per_phase(LIBMATTI_BUS_ListenerList *list)
{
    if (list->perPhaseListeners == NULL) return;

    for (int phase = 0; phase < LIBMATTI_BUS_EventPriority_COUNT; phase++)
        free(list->perPhaseListeners[phase]);
    free(list->perPhaseListeners);
    list->perPhaseListeners = NULL;
}

// Java: private void buildCache()
static void build_cache(LIBMATTI_BUS_ListenerList *list)
{
    if (list->parent != NULL && list->parent->rebuild)
        build_cache(list->parent);

    LIBMATTI_BUS_EventListener **all = NULL;
    size_t total = 0;
    LIBMATTI_BUS_EventListener ***perPhase = NULL;
    if (list->buildPerPhaseList)
        perPhase = calloc(LIBMATTI_BUS_EventPriority_COUNT, sizeof(*perPhase));

    for (int phase = 0; phase < LIBMATTI_BUS_EventPriority_COUNT; phase++)
    {
        size_t count;
        LIBMATTI_BUS_EventListener **phaseListeners =
            get_listeners(list, (LIBMATTI_BUS_EventPriority)phase, &count);
        unwrap_listeners(list, phaseListeners, count);

        all = realloc(all, sizeof(*all) * (total + count + 1));
        memcpy(all + total, phaseListeners, sizeof(*all) * count);
        total += count;

        if (perPhase != NULL)
        {
            perPhase[phase] = phaseListeners;
            list->perPhaseCounts[phase] = count;
        }
        else
        {
            free(phaseListeners);
        }
    }

    // Java: this.listeners.set(ret.toArray(new EventListener[0]));
    free(list->listeners);
    list->listeners = all;
    list->listenerCount = total;

    // Java: this.perPhaseListeners.set(perPhaseListeners);
    free_per_phase(list);
    list->perPhaseListeners = perPhase;

    list->rebuild = 0;
}

// Java: protected void forceRebuild()
static void force_rebuild(LIBMATTI_BUS_ListenerList *list)
{
    list->rebuild = 1;
    for (size_t i = 0; i < list->childCount; i++)
        force_rebuild(list->children[i]);
}

// Java: private void addChild(ListenerList child)
static void add_child(LIBMATTI_BUS_ListenerList *list, LIBMATTI_BUS_ListenerList *child)
{
    list->children = realloc(list->children, sizeof(*list->children) * (list->childCount + 1));
    list->children[list->childCount++] = child;
}

// Java: ListenerList(Class<?> eventClass, @Nullable ListenerList parent, boolean buildPerPhaseList)
static LIBMATTI_BUS_ListenerList *new_list(LIBMATTI_JL_Class *eventClass, LIBMATTI_BUS_ListenerList *parent,
                                           int buildPerPhaseList)
{
    LIBMATTI_BUS_ListenerList *list = calloc(1, sizeof(LIBMATTI_BUS_ListenerList));
    list->rebuild = 1;
    // Java: canUnwrapListeners = !ICancellableEvent.class.isAssignableFrom(eventClass);
    list->canUnwrapListeners =
        !LIBMATTI_JL_Class_IsAssignableFrom(LIBMATTI_BUS_ICancellableEvent_Class(), eventClass);
    list->buildPerPhaseList = buildPerPhaseList;
    list->parent = parent;
    pthread_mutex_init(&list->writeLock, NULL);

    if (parent != NULL) add_child(parent, list);
    return list;
}

// Java: ListenerList(Class<?> eventClass, boolean buildPerPhaseList)
LIBMATTI_BUS_ListenerList *LIBMATTI_BUS_ListenerList_New(LIBMATTI_JL_Class *eventClass, int buildPerPhaseList)
{
    return new_list(eventClass, NULL, buildPerPhaseList);
}

// Java: ListenerList(Class<?> eventClass, @Nullable ListenerList parent, boolean buildPerPhaseList)
LIBMATTI_BUS_ListenerList *LIBMATTI_BUS_ListenerList_NewWithParent(LIBMATTI_JL_Class *eventClass,
                                                                   LIBMATTI_BUS_ListenerList *parent,
                                                                   int buildPerPhaseList)
{
    return new_list(eventClass, parent, buildPerPhaseList);
}

void LIBMATTI_BUS_ListenerList_Free(LIBMATTI_BUS_ListenerList *list)
{
    for (int phase = 0; phase < LIBMATTI_BUS_EventPriority_COUNT; phase++)
        free(list->priorities[phase]);
    free(list->listeners);
    free_per_phase(list);
    free(list->children);
    pthread_mutex_destroy(&list->writeLock);
    free(list);
}

// Java: public EventListener[] getListeners()
LIBMATTI_BUS_EventListener **LIBMATTI_BUS_ListenerList_GetListeners(LIBMATTI_BUS_ListenerList *list, size_t *count)
{
    if (list->rebuild) build_cache(list);

    if (count != NULL) *count = list->listenerCount;
    return list->listeners;
}

// Java: public EventListener[] getPhaseListeners(EventPriority phase)
LIBMATTI_BUS_EventListener **LIBMATTI_BUS_ListenerList_GetPhaseListeners(LIBMATTI_BUS_ListenerList *list,
                                                                         LIBMATTI_BUS_EventPriority phase,
                                                                         size_t *count)
{
    if (!list->buildPerPhaseList)
    {
        // Java: throw new IllegalStateException("buildPerPhaseList is false!")
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "buildPerPhaseList is false!");
        if (count != NULL) *count = 0;
        return NULL;
    }

    if (list->rebuild) build_cache(list);

    if (count != NULL) *count = list->perPhaseCounts[phase];
    return list->perPhaseListeners[phase];
}

// Java: public void register(EventPriority priority, EventListener listener)
void LIBMATTI_BUS_ListenerList_Register(LIBMATTI_BUS_ListenerList *list, LIBMATTI_BUS_EventPriority priority,
                                        LIBMATTI_BUS_EventListener *listener)
{
    int index = LIBMATTI_BUS_EventPriority_Ordinal(priority);

    pthread_mutex_lock(&list->writeLock);
    size_t size = list->priorityCounts[index];
    list->priorities[index] = realloc(list->priorities[index], sizeof(**list->priorities) * (size + 1));
    list->priorities[index][size] = listener;
    list->priorityCounts[index] = size + 1;
    pthread_mutex_unlock(&list->writeLock);

    force_rebuild(list);
}

// Java: public void unregister(EventListener listener)
void LIBMATTI_BUS_ListenerList_Unregister(LIBMATTI_BUS_ListenerList *list, LIBMATTI_BUS_EventListener *listener)
{
    pthread_mutex_lock(&list->writeLock);

    for (int phase = 0; phase < LIBMATTI_BUS_EventPriority_COUNT; phase++)
    {
        size_t size = list->priorityCounts[phase];
        for (size_t i = 0; i < size; i++)
        {
            if (list->priorities[phase][i] != listener) continue;

            memmove(&list->priorities[phase][i], &list->priorities[phase][i + 1],
                    sizeof(**list->priorities) * (size - i - 1));
            list->priorityCounts[phase] = size - 1;
            force_rebuild(list);
            break;
        }
    }

    pthread_mutex_unlock(&list->writeLock);
}
