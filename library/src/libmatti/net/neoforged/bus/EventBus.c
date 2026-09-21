// Port of net.neoforged.bus.EventBus.

#include "libmatti/net/neoforged/bus/EventBus.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/lang/System.h"
#include "libmatti/java/lang/reflect/Modifier.h"
#include "libmatti/net/neoforged/bus/ConsumerEventHandler.h"
#include "libmatti/net/neoforged/bus/EventListenerFactory.h"
#include "libmatti/net/neoforged/bus/EventBusErrorMessage.h"
#include "libmatti/net/neoforged/bus/api/ICancellableEvent.h"
#include "libmatti/net/neoforged/bus/ListenerList.h"
#include "libmatti/net/neoforged/bus/LogMarkers.h"
#include "libmatti/net/neoforged/bus/SubscribeEventListener.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: Boolean.parseBoolean(System.getProperty("eventbus.checkTypesOnDispatch", "false"))
static int check_types_on_dispatch_property(void)
{
    const char *value = LIBMATTI_JL_System_GetProperty("eventbus.checkTypesOnDispatch");
    return value != NULL && strcmp(value, "false") != 0;
}

// Java: private final ConcurrentHashMap<Object, List<EventListener>> listeners
typedef struct
{
    void *target;
    LIBMATTI_BUS_EventListener **listeners;
    size_t count;
} LIBMATTI_BUS_EventBus_ListenerMapEntry;

struct LIBMATTI_BUS_EventBus_ListenerMap
{
    LIBMATTI_BUS_EventBus_ListenerMapEntry *entries;
    size_t count;
    pthread_mutex_t lock;
};

static LIBMATTI_BUS_EventBus_ListenerMap *listener_map_new(void)
{
    LIBMATTI_BUS_EventBus_ListenerMap *map = calloc(1, sizeof(*map));
    pthread_mutex_init(&map->lock, NULL);
    return map;
}

static void listener_map_free(LIBMATTI_BUS_EventBus_ListenerMap *map)
{
    for (size_t i = 0; i < map->count; i++)
        free(map->entries[i].listeners);
    pthread_mutex_destroy(&map->lock);
    free(map->entries);
    free(map);
}

static size_t listener_map_find(LIBMATTI_BUS_EventBus_ListenerMap *map, void *target)
{
    for (size_t i = 0; i < map->count; i++)
        if (map->entries[i].target == target) return i;
    return map->count;
}

// Java: listeners.containsKey(target)
static int listener_map_contains(LIBMATTI_BUS_EventBus_ListenerMap *map, void *target)
{
    pthread_mutex_lock(&map->lock);
    size_t index = listener_map_find(map, target);
    pthread_mutex_unlock(&map->lock);
    return index != map->count;
}

// Java: listeners.computeIfAbsent(target, k -> Collections.synchronizedList(new ArrayList<>())).add(listener)
static void listener_map_add(LIBMATTI_BUS_EventBus_ListenerMap *map, void *target,
                             LIBMATTI_BUS_EventListener *listener)
{
    pthread_mutex_lock(&map->lock);

    size_t index = listener_map_find(map, target);
    if (index == map->count)
    {
        map->entries = realloc(map->entries, sizeof(*map->entries) * (map->count + 1));
        map->entries[map->count].target = target;
        map->entries[map->count].listeners = NULL;
        map->entries[map->count].count = 0;
        index = map->count++;
    }

    LIBMATTI_BUS_EventBus_ListenerMapEntry *entry = &map->entries[index];
    entry->listeners = realloc(entry->listeners, sizeof(*entry->listeners) * (entry->count + 1));
    entry->listeners[entry->count++] = listener;

    pthread_mutex_unlock(&map->lock);
}

// Java: List<EventListener> list = listeners.remove(object) - NULL when the target was not registered.
// The returned array is owned by the caller.
static LIBMATTI_BUS_EventListener **listener_map_remove(LIBMATTI_BUS_EventBus_ListenerMap *map, void *target,
                                                        size_t *count)
{
    pthread_mutex_lock(&map->lock);

    size_t index = listener_map_find(map, target);
    if (index == map->count)
    {
        pthread_mutex_unlock(&map->lock);
        return NULL;
    }

    LIBMATTI_BUS_EventBus_ListenerMapEntry entry = map->entries[index];
    memmove(&map->entries[index], &map->entries[index + 1],
            sizeof(*map->entries) * (map->count - index - 1));
    map->count--;

    pthread_mutex_unlock(&map->lock);

    *count = entry.count;
    return entry.listeners;
}

// Java: passNotGenericFilter(boolean receiveCanceled) - @Nullable Predicate<T>
static int not_canceled_test(void *self, void *value)
{
    (void)self;
    return !LIBMATTI_BUS_ICancellableEvent_IsCanceled(value);
}

static LIBMATTI_JU_Predicate *pass_not_generic_filter(int receiveCanceled)
{
    static LIBMATTI_JU_Predicate notCanceled = {NULL, not_canceled_test};
    return receiveCanceled ? NULL : &notCanceled;
}

// Java: private static void validateAbstractChain(Class<?> eventType)
static void validate_abstract_chain(LIBMATTI_JL_Class *eventType)
{
    while (eventType != LIBMATTI_BUS_Event_Class())
    {
        LIBMATTI_JL_Class *superclass = LIBMATTI_JL_Class_GetSuperclass(eventType);
        if (!LIBMATTI_JL_Modifier_IsAbstract(LIBMATTI_JL_Class_GetModifiers(superclass)))
        {
            // Java: throw new IllegalArgumentException("Abstract event " + eventType + " has a non-abstract superclass " + eventType.getSuperclass() + ". The superclass must be made abstract.")
            LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_BUS_LogMarkers_EVENTBUS,
                                     "Abstract event {} has a non-abstract superclass {}. The superclass must be made abstract.",
                                     LIBMATTI_JL_Class_GetName(eventType),
                                     LIBMATTI_JL_Class_GetName(superclass));
            return;
        }

        eventType = superclass;
    }
}

static LIBMATTI_BUS_ListenerList *get_listener_list(LIBMATTI_BUS_EventBus *bus, LIBMATTI_JL_Class *eventType);

// Java: listenerLists.computeIfAbsent(eventType, e -> new ListenerList(e, [getListenerList(e.getSuperclass()),] allowPerPhasePost))
static void *listener_list_factory(void *userdata, const void *key)
{
    LIBMATTI_BUS_EventBus *bus = userdata;
    LIBMATTI_JL_Class *eventType = (LIBMATTI_JL_Class *)key;
    LIBMATTI_JL_Class *superclass = LIBMATTI_JL_Class_GetSuperclass(eventType);

    if (LIBMATTI_JL_Modifier_IsAbstract(LIBMATTI_JL_Class_GetModifiers(superclass)))
    {
        validate_abstract_chain(superclass);
        return LIBMATTI_BUS_ListenerList_New(eventType, bus->allowPerPhasePost);
    }

    return LIBMATTI_BUS_ListenerList_NewWithParent(eventType, get_listener_list(bus, superclass),
                                                  bus->allowPerPhasePost);
}

// Java: private ListenerList getListenerList(Class<?> eventType)
static LIBMATTI_BUS_ListenerList *get_listener_list(LIBMATTI_BUS_EventBus *bus, LIBMATTI_JL_Class *eventType)
{
    LIBMATTI_BUS_ListenerList *list = LIBMATTI_BUS_LockHelper_Get(bus->listenerLists, eventType);
    if (list != NULL) return list;

    return LIBMATTI_BUS_LockHelper_ComputeIfAbsent(bus->listenerLists, eventType, listener_list_factory, bus);
}

// Java: private void addToListeners(Object target, Class<?> eventType, EventListener listener, EventPriority priority)
static void add_to_listeners(LIBMATTI_BUS_EventBus *bus, void *target, LIBMATTI_JL_Class *eventType,
                             LIBMATTI_BUS_EventListener *listener, LIBMATTI_BUS_EventPriority priority)
{
    // Java: if (Modifier.isAbstract(eventType.getModifiers())) throw new IllegalArgumentException("Cannot register listeners for abstract " + eventType + ". ...")
    if (LIBMATTI_JL_Modifier_IsAbstract(LIBMATTI_JL_Class_GetModifiers(eventType)))
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_BUS_LogMarkers_EVENTBUS,
                                 "Cannot register listeners for abstract {}. Register a listener to one of its subclasses instead!",
                                 LIBMATTI_JL_Class_GetName(eventType));
        return;
    }

    LIBMATTI_BUS_ListenerList_Register(get_listener_list(bus, eventType), priority, listener);
    listener_map_add(bus->listeners, target, listener);
}

// Java: private <T extends Event> void addListener(EventPriority priority, @Nullable Predicate<? super T> filter,
//                                                  Class<T> eventClass, Consumer<T> consumer)
static void add_listener_with_filter(LIBMATTI_BUS_EventBus *bus, LIBMATTI_BUS_EventPriority priority,
                                     LIBMATTI_JU_Predicate *filter, LIBMATTI_JL_Class *eventClass,
                                     const LIBMATTI_JU_Consumer *consumer)
{
    // Java: try { classChecker.check(eventClass); } catch (IllegalArgumentException e) { throw ... } - the checker logs
    if (bus->classChecker.check != NULL)
        bus->classChecker.check(bus->classChecker.self, eventClass);

    LIBMATTI_BUS_EventListener *listener;
    if (filter == NULL)
    {
        listener = &LIBMATTI_BUS_ConsumerEventHandler_New(consumer)->base;
    }
    else
    {
        listener = &LIBMATTI_BUS_ConsumerEventHandler_WithPredicate_New(consumer, filter)->base.base;
    }

    add_to_listeners(bus, (void *)consumer, eventClass, listener, priority);
}

// Java: private void register(Class<?> eventType, Object target, Method method)
static void register_subscribe_listener(LIBMATTI_BUS_EventBus *bus, void *target, LIBMATTI_JL_Class *eventType,
                                        LIBMATTI_BUS_SubscribeEventListener *listener)
{
    add_to_listeners(bus, target, eventType, &listener->base,
                     LIBMATTI_BUS_SubscribeEventListener_GetPriority(listener));
}

// Java: private void doPostChecks(Event event)
static void do_post_checks(LIBMATTI_BUS_EventBus *bus, LIBMATTI_BUS_Event *event)
{
    if (!bus->checkTypesOnDispatch) return;

    // Java: try { classChecker.check(event.getClass()); } catch (IllegalArgumentException e) { throw new
    //       IllegalArgumentException("Cannot post event of type " + event.getClass().getSimpleName() + " to this bus", e); }
    if (bus->classChecker.check != NULL)
        bus->classChecker.check(bus->classChecker.self, event->clazz);
}

// Java: private <T extends Event> T post(T event, EventListener[] listeners)
static LIBMATTI_BUS_Event *post_to_listeners(LIBMATTI_BUS_EventBus *bus, LIBMATTI_BUS_Event *event,
                                             LIBMATTI_BUS_EventListener **listeners, size_t listenerCount)
{
    (void)bus;

    // Java: int index = 0; try { for (; index < listeners.length; index++) listeners[index].invoke(event); }
    //       catch (Throwable throwable) { exceptionHandler.handleException(this, event, listeners, index, throwable); throw throwable; }
    // A C callback cannot unwind into the bus, so a failing listener reports through the exception
    // handler itself (LIBMATTI_BUS_EventBus_HandleException).
    for (size_t index = 0; index < listenerCount; index++)
        LIBMATTI_BUS_EventListener_Invoke(listeners[index], event);

    return event;
}

// Java: public void register(final Object target)
static void event_bus_register(void *self, void *target)
{
    LIBMATTI_BUS_EventBus *bus = self;
    if (listener_map_contains(bus->listeners, target)) return;

    // Java: scans target.getClass().getDeclaredMethods() (or the passed Method) for @SubscribeEvent
    // methods through reflection and creates a listener per method. C has neither reflection nor
    // annotations, so the port registers through LIBMATTI_BUS_EventBus_RegisterListener.
    LIBMATTI_ML_Logger_Warn(LOGGER(), &LIBMATTI_BUS_LogMarkers_EVENTBUS,
                            "register(Object) needs @SubscribeEvent method scanning, which the C port has no reflection for; use LIBMATTI_BUS_EventBus_RegisterListener");
}

// Java: <T extends Event> void addListener(EventPriority priority, boolean receiveCanceled, Class<T> eventType, Consumer<T> consumer)
static void event_bus_add_listener(void *self, LIBMATTI_BUS_EventPriority priority, int receiveCanceled,
                                   LIBMATTI_JL_Class *eventType, const LIBMATTI_JU_Consumer *consumer)
{
    add_listener_with_filter(self, priority, pass_not_generic_filter(receiveCanceled), eventType, consumer);
}

// Java: public void unregister(Object object)
static void event_bus_unregister(void *self, void *object)
{
    LIBMATTI_BUS_EventBus *bus = self;

    size_t count;
    LIBMATTI_BUS_EventListener **removed = listener_map_remove(bus->listeners, object, &count);
    if (removed == NULL) return;

    const void **keys;
    void **values;
    size_t listCount = LIBMATTI_BUS_LockHelper_GetReadMap(bus->listenerLists, &keys, &values);

    for (size_t i = 0; i < listCount; i++)
        for (size_t j = 0; j < count; j++)
            LIBMATTI_BUS_ListenerList_Unregister(values[i], removed[j]);

    free(removed);
}

// Java: public <T extends Event> T post(T event)
static LIBMATTI_BUS_Event *event_bus_post(void *self, LIBMATTI_BUS_Event *event)
{
    LIBMATTI_BUS_EventBus *bus = self;
    if (bus->shutdown) return event;

    do_post_checks(bus, event);

    size_t count;
    LIBMATTI_BUS_EventListener **listeners =
        LIBMATTI_BUS_ListenerList_GetListeners(get_listener_list(bus, event->clazz), &count);
    return post_to_listeners(bus, event, listeners, count);
}

// Java: public <T extends Event> T post(EventPriority phase, T event)
static LIBMATTI_BUS_Event *event_bus_post_phase(void *self, LIBMATTI_BUS_EventPriority phase,
                                               LIBMATTI_BUS_Event *event)
{
    LIBMATTI_BUS_EventBus *bus = self;

    if (!bus->allowPerPhasePost)
    {
        // Java: throw new IllegalStateException("This bus does not allow calling phase-specific post.")
        LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_BUS_LogMarkers_EVENTBUS,
                                 "This bus does not allow calling phase-specific post.");
        return event;
    }

    if (bus->shutdown) return event;

    do_post_checks(bus, event);

    size_t count;
    LIBMATTI_BUS_EventListener **listeners =
        LIBMATTI_BUS_ListenerList_GetPhaseListeners(get_listener_list(bus, event->clazz), phase, &count);
    return post_to_listeners(bus, event, listeners, count);
}

// Java: public void handleException(IEventBus bus, Event event, EventListener[] listeners, int index, Throwable throwable)
static void event_bus_handle_exception(void *self, LIBMATTI_BUS_IEventBus *bus, LIBMATTI_BUS_Event *event,
                                       LIBMATTI_BUS_EventListener **listeners, size_t listenerCount, int index,
                                       LIBMATTI_JL_Throwable *throwable)
{
    (void)self;
    (void)bus;

    // Java: LOGGER.error(EVENTBUS, () -> new EventBusErrorMessage(event, index, listeners, throwable));
    LIBMATTI_BUS_EventBusErrorMessage *message =
        LIBMATTI_BUS_EventBusErrorMessage_New(event, index, listeners, listenerCount, throwable);
    char *formatted = LIBMATTI_BUS_EventBusErrorMessage_FormatTo(message);
    LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_BUS_LogMarkers_EVENTBUS, "{}", formatted);
    free(formatted);
    LIBMATTI_BUS_EventBusErrorMessage_Free(message);
}

// Java: public void start()
static void event_bus_start(void *self)
{
    LIBMATTI_BUS_EventBus *bus = self;
    bus->shutdown = 0;
}

// Java: the exception handler and the bus are both this instance
void LIBMATTI_BUS_EventBus_HandleException(LIBMATTI_BUS_EventBus *bus, LIBMATTI_BUS_IEventBus *eventBus,
                                           LIBMATTI_BUS_Event *event, LIBMATTI_BUS_EventListener **listeners,
                                           size_t listenerCount, int index, LIBMATTI_JL_Throwable *throwable)
{
    event_bus_handle_exception(bus, eventBus, event, listeners, listenerCount, index, throwable);
}

// Java: public EventBus(final BusBuilderImpl busBuilder)
LIBMATTI_BUS_EventBus *LIBMATTI_BUS_EventBus_New(LIBMATTI_BUS_BusBuilderImpl *busBuilder)
{
    LIBMATTI_BUS_EventBus *bus = calloc(1, sizeof(LIBMATTI_BUS_EventBus));

    // Java: implements IEventExceptionHandler, IEventBus
    bus->handlerBase.self = bus;
    bus->handlerBase.handleException = event_bus_handle_exception;
    bus->busBase.self = bus;
    bus->busBase.registerTarget = event_bus_register;
    bus->busBase.addListener = event_bus_add_listener;
    bus->busBase.unregister = event_bus_unregister;
    bus->busBase.post = event_bus_post;
    bus->busBase.postPhase = event_bus_post_phase;
    bus->busBase.start = event_bus_start;

    bus->listenerLists = LIBMATTI_BUS_LockHelper_WithIdentityHashMap();
    bus->listeners = listener_map_new();

    // Java: if (handler == null) exceptionHandler = this; else exceptionHandler = handler;
    bus->exceptionHandler = busBuilder->exceptionHandler.handleException == NULL
                                ? &bus->handlerBase
                                : &busBuilder->exceptionHandler;
    bus->shutdown = busBuilder->startShutdown;
    bus->classChecker = busBuilder->classChecker;
    bus->checkTypesOnDispatch = busBuilder->checkTypesOnDispatch || check_types_on_dispatch_property();
    bus->allowPerPhasePost = busBuilder->allowPerPhasePost;
    return bus;
}

void LIBMATTI_BUS_EventBus_Free(LIBMATTI_BUS_EventBus *bus)
{
    const void **keys;
    void **values;
    size_t count = LIBMATTI_BUS_LockHelper_GetReadMap(bus->listenerLists, &keys, &values);
    for (size_t i = 0; i < count; i++)
        LIBMATTI_BUS_ListenerList_Free(values[i]);

    LIBMATTI_BUS_LockHelper_Free(bus->listenerLists);
    listener_map_free(bus->listeners);
    free(bus);
}

// Java: private void registerListener(Object target, Method method) + the SubscribeEventListener
void LIBMATTI_BUS_EventBus_RegisterListener(LIBMATTI_BUS_IEventBus *eventBus, LIBMATTI_JL_Class *eventType,
                                            void *target, const char *targetName, const char *methodName,
                                            const char *methodDescriptor,
                                            void (*callback)(void *target, LIBMATTI_BUS_Event *event),
                                            LIBMATTI_BUS_SubscribeEvent subInfo)
{
    LIBMATTI_BUS_EventBus *bus = eventBus->self;

    // Java: registerListener validates that the method takes a single Event parameter
    if (!LIBMATTI_JL_Class_IsAssignableFrom(LIBMATTI_BUS_Event_Class(), eventType))
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_BUS_LogMarkers_EVENTBUS,
                                 "Method {} has @SubscribeEvent annotation, but takes an argument that is not an Event subtype: {}",
                                 methodName, LIBMATTI_JL_Class_GetName(eventType));
        return;
    }

    LIBMATTI_BUS_EventListener *handler = LIBMATTI_BUS_EventListenerFactory_Create(target, callback);
    LIBMATTI_BUS_SubscribeEventListener *listener = LIBMATTI_BUS_SubscribeEventListener_New(
        targetName, methodName, methodDescriptor, handler, subInfo);

    register_subscribe_listener(bus, target, eventType, listener);
}
