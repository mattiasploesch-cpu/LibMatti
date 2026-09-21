// Port of net.neoforged.bus.ListenerList.
// Java's ArrayList<EventListener> lists become arrays and the AtomicReference fields become plain
// members, because the writeLock (Java: Semaphore(1, true)) already serialises the writers.
// The returned arrays are owned by the list and are replaced by the next rebuild.

#ifndef MATTICRAFT_BUS_LISTENERLIST_H
#define MATTICRAFT_BUS_LISTENERLIST_H

#include "libmatti/net/neoforged/bus/api/EventListener.h"
#include "libmatti/net/neoforged/bus/api/EventPriority.h"

#include <pthread.h>
#include <stddef.h>

typedef struct LIBMATTI_BUS_ListenerList LIBMATTI_BUS_ListenerList;

struct LIBMATTI_BUS_ListenerList
{
    // Java: private boolean rebuild = true;
    int rebuild;
    // Java: private final AtomicReference<EventListener[]> listeners
    LIBMATTI_BUS_EventListener **listeners;
    size_t listenerCount;
    // Java: private final AtomicReference<EventListener[][]> perPhaseListeners
    LIBMATTI_BUS_EventListener ***perPhaseListeners;
    size_t perPhaseCounts[LIBMATTI_BUS_EventPriority_COUNT];
    // Java: private final ArrayList<ArrayList<EventListener>> priorities
    LIBMATTI_BUS_EventListener **priorities[LIBMATTI_BUS_EventPriority_COUNT];
    size_t priorityCounts[LIBMATTI_BUS_EventPriority_COUNT];
    // Java: @Nullable private final ListenerList parent;
    LIBMATTI_BUS_ListenerList *parent;
    // Java: private List<ListenerList> children;
    LIBMATTI_BUS_ListenerList **children;
    size_t childCount;
    // Java: private final boolean canUnwrapListeners;
    int canUnwrapListeners;
    // Java: private final boolean buildPerPhaseList;
    int buildPerPhaseList;
    // Java: private final Semaphore writeLock = new Semaphore(1, true);
    pthread_mutex_t writeLock;
};

// Java: ListenerList(Class<?> eventClass, boolean buildPerPhaseList)
LIBMATTI_BUS_ListenerList *LIBMATTI_BUS_ListenerList_New(LIBMATTI_JL_Class *eventClass, int buildPerPhaseList);
// Java: ListenerList(Class<?> eventClass, @Nullable ListenerList parent, boolean buildPerPhaseList)
LIBMATTI_BUS_ListenerList *LIBMATTI_BUS_ListenerList_NewWithParent(LIBMATTI_JL_Class *eventClass,
                                                                   LIBMATTI_BUS_ListenerList *parent,
                                                                   int buildPerPhaseList);
void LIBMATTI_BUS_ListenerList_Free(LIBMATTI_BUS_ListenerList *list);

// Java: public EventListener[] getListeners()
LIBMATTI_BUS_EventListener **LIBMATTI_BUS_ListenerList_GetListeners(LIBMATTI_BUS_ListenerList *list,
                                                                    size_t *count);
// Java: public EventListener[] getPhaseListeners(EventPriority phase)
LIBMATTI_BUS_EventListener **LIBMATTI_BUS_ListenerList_GetPhaseListeners(LIBMATTI_BUS_ListenerList *list,
                                                                         LIBMATTI_BUS_EventPriority phase,
                                                                         size_t *count);
// Java: public void register(EventPriority priority, EventListener listener)
void LIBMATTI_BUS_ListenerList_Register(LIBMATTI_BUS_ListenerList *list, LIBMATTI_BUS_EventPriority priority,
                                        LIBMATTI_BUS_EventListener *listener);
// Java: public void unregister(EventListener listener)
void LIBMATTI_BUS_ListenerList_Unregister(LIBMATTI_BUS_ListenerList *list, LIBMATTI_BUS_EventListener *listener);

#endif //MATTICRAFT_BUS_LISTENERLIST_H
