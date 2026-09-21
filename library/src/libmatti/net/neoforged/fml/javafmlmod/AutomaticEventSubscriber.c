// Port of net.neoforged.fml.javafmlmod.AutomaticEventSubscriber.

#include "libmatti/net/neoforged/fml/javafmlmod/AutomaticEventSubscriber.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/bus/EventBus.h"
#include "libmatti/net/neoforged/bus/api/IEventBus.h"
#include "libmatti/net/neoforged/fml/IBindingsProvider.h"
#include "libmatti/net/neoforged/fml/Logging.h"
#include "libmatti/net/neoforged/fml/event/IModBusEvent.h"
#include "libmatti/net/neoforged/fml/loading/FMLLoader.h"
#include "libmatti/org/objectweb/asm/Type.h"

#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: clazz.getDeclaredMethods() with @SubscribeEvent
typedef struct
{
    char *className;
    char *methodName;
    int isStatic;
    LIBMATTI_JL_Class *eventType;
    LIBMATTI_BUS_SubscribeEvent subInfo;
    void (*callback)(void *self, LIBMATTI_BUS_Event *event);
} RegisteredMethod;

static RegisteredMethod *methods = NULL;
static size_t methodCount = 0;

void LIBMATTI_FML_AutomaticEventSubscriber_RegisterMethod(
    const char *className, const char *methodName, int isStatic, LIBMATTI_JL_Class *eventType,
    LIBMATTI_BUS_SubscribeEvent subInfo, void (*callback)(void *self, LIBMATTI_BUS_Event *event))
{
    methods = realloc(methods, sizeof(*methods) * (methodCount + 1));
    RegisteredMethod *method = &methods[methodCount++];

    method->className = strdup(className);
    method->methodName = strdup(methodName);
    method->isStatic = isStatic;
    method->eventType = eventType;
    method->subInfo = subInfo;
    method->callback = callback;
}

// Java: data.annotationData().get(key)
static const char *annotation_value(const LIBMATTI_NEOFORGESPI_AnnotationData *data, const char *key)
{
    for (size_t i = 0; i < data->annotationDataCount; i++)
        if (strcmp(data->annotationDataKeys[i], key) == 0) return data->annotationDataValues[i];

    return NULL;
}

// Java: public static EnumSet<Dist> getSides(Object data)
size_t LIBMATTI_FML_AutomaticEventSubscriber_GetSides(const char *data, LIBMATTI_DIST_Dist *sides, size_t capacity)
{
    // Java: if (data == null) return EnumSet.allOf(Dist.class);
    if (data == NULL)
    {
        size_t count = 0;
        for (int i = LIBMATTI_DIST_CLIENT; i <= LIBMATTI_DIST_DEDICATED_SERVER && count < capacity; i++)
            sides[count++] = (LIBMATTI_DIST_Dist)i;
        return count;
    }

    // Java maps each ModAnnotation.EnumHolder's value() with Dist.valueOf
    size_t count = 0;
    const char *cursor = data;
    while (*cursor != '\0')
    {
        while (*cursor == ',' || *cursor == ';' || *cursor == ' ') cursor++;
        if (*cursor == '\0') break;

        const char *end = cursor;
        while (*end != '\0' && *end != ',' && *end != ';') end++;

        size_t length = (size_t)(end - cursor);
        while (length > 0 && cursor[length - 1] == ' ') length--;

        if (length == strlen("CLIENT") && strncmp(cursor, "CLIENT", length) == 0)
        {
            if (count < capacity) sides[count] = LIBMATTI_DIST_CLIENT;
            count++;
        }
        else if (length == strlen("DEDICATED_SERVER") && strncmp(cursor, "DEDICATED_SERVER", length) == 0)
        {
            if (count < capacity) sides[count] = LIBMATTI_DIST_DEDICATED_SERVER;
            count++;
        }

        cursor = end;
    }

    return count;
}

// Java: modids.getOrDefault(clazz, mod.getModId()) - modids is the @Mod annotation's clazz -> value map
static const char *mod_id_for(const LIBMATTI_NEOFORGESPI_ModFileScanData *scanData, const char *clazz,
                              LIBMATTI_FML_ModContainer *mod)
{
    for (size_t i = 0; i < scanData->annotationCount; i++)
    {
        const LIBMATTI_NEOFORGESPI_AnnotationData *annotation = &scanData->annotations[i];
        char *annotationName = LIBMATTI_ASM_Type_GetClassName(annotation->annotationType);
        int isMod = annotationName != NULL && strcmp(annotationName, LIBMATTI_FML_AutomaticEventSubscriber_MOD) == 0;
        free(annotationName);
        if (!isMod) continue;

        char *annotationClass = LIBMATTI_ASM_Type_GetClassName(annotation->clazz);
        int sameClass = annotationClass != NULL && strcmp(annotationClass, clazz) == 0;
        free(annotationClass);
        if (!sameClass) continue;

        const char *value = annotation_value(annotation, "value");
        if (value != NULL) return value;
    }

    return LIBMATTI_FML_ModContainer_GetModId(mod);
}

static int sides_contain(const LIBMATTI_DIST_Dist *sides, size_t count, LIBMATTI_DIST_Dist side)
{
    for (size_t i = 0; i < count; i++)
        if (sides[i] == side) return 1;

    return 0;
}

// Java: modBus.register(method) / FMLLoader.getCurrent().getBindings().getGameBus().register(method)
static void subscribe(LIBMATTI_FML_ModContainer *mod, RegisteredMethod *method, const char *className)
{
    // Java: if (!Modifier.isStatic(method.getModifiers())) throw new IllegalArgumentException(...)
    if (!method->isStatic)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_Logging_LOADING,
                                 "Method {} annotated with @SubscribeEvent is not static", method->methodName);
        return;
    }

    // Java: method.getParameterCount() != 1 || !Event.class.isAssignableFrom(method.getParameterTypes()[0])
    if (method->eventType == NULL ||
        !LIBMATTI_JL_Class_IsAssignableFrom(LIBMATTI_BUS_Event_Class(), method->eventType))
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_Logging_LOADING,
                                 "Method {} annotated with @SubscribeEvent must have only one parameter that is an Event subtype",
                                 method->methodName);
        return;
    }

    // Java: if (IModBusEvent.class.isAssignableFrom(eventType)) { var modBus = mod.getEventBus(); ... }
    if (LIBMATTI_JL_Class_IsAssignableFrom(LIBMATTI_FML_Event_IModBusEvent_Class(), method->eventType))
    {
        LIBMATTI_BUS_IEventBus *modBus = LIBMATTI_FML_ModContainer_GetEventBus(mod);
        if (modBus == NULL)
        {
            LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_Logging_LOADING,
                                     "Method {} attempted to register a mod bus event, but mod {} has no event bus",
                                     method->methodName, LIBMATTI_FML_ModContainer_GetModId(mod));
            return;
        }

        LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_Logging_LOADING,
                                 "Subscribing method {} to the event bus of mod {}", method->methodName,
                                 LIBMATTI_FML_ModContainer_GetModId(mod));
        LIBMATTI_BUS_EventBus_RegisterListener(modBus, method->eventType, (void *)className, className,
                                               method->methodName, NULL, method->callback, method->subInfo);
        return;
    }

    LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_Logging_LOADING, "Subscribing method {} to the game event bus",
                             method->methodName);

    // Java: FMLLoader.getCurrent().getBindings().getGameBus().register(method)
    LIBMATTI_FML_FMLLoader *loader = LIBMATTI_FML_FMLLoader_GetCurrent();
    LIBMATTI_FML_IBindingsProvider *bindings =
        loader != NULL ? LIBMATTI_FML_FMLLoader_GetBindings(loader) : NULL;
    if (bindings == NULL)
    {
        // Java: NPE when no mod has provided the game bus
        LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_Logging_LOADING,
                                 "Cannot subscribe method {} to the game event bus: no IBindingsProvider",
                                 method->methodName);
        return;
    }

    LIBMATTI_BUS_EventBus_RegisterListener(LIBMATTI_FML_IBindingsProvider_GetGameBus(bindings), method->eventType,
                                           (void *)className, className, method->methodName, NULL, method->callback,
                                           method->subInfo);
}

// Java: public static void inject(ModContainer mod, ModFileScanData scanData, Module layer)
void LIBMATTI_FML_AutomaticEventSubscriber_Inject(LIBMATTI_FML_ModContainer *mod,
                                                  LIBMATTI_NEOFORGESPI_ModFileScanData *scanData,
                                                  const char *moduleName)
{
    (void)moduleName;

    // Java: if (scanData == null) return;
    if (scanData == NULL) return;

    LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_Logging_LOADING,
                             "Attempting to inject @EventBusSubscriber classes into the eventbus for {}",
                             LIBMATTI_FML_ModContainer_GetModId(mod));

    LIBMATTI_FML_FMLLoader *loader = LIBMATTI_FML_FMLLoader_GetCurrent();
    LIBMATTI_DIST_Dist dist = loader != NULL ? LIBMATTI_FML_FMLLoader_GetDist(loader) : LIBMATTI_DIST_CLIENT;

    // Java: Map<String, String> modids = the @Mod annotations mapped clazz -> value
    for (size_t i = 0; i < scanData->annotationCount; i++)
    {
        LIBMATTI_NEOFORGESPI_AnnotationData *annotation = &scanData->annotations[i];
        char *annotationName = LIBMATTI_ASM_Type_GetClassName(annotation->annotationType);
        int isSubscriber = annotationName != NULL &&
                           strcmp(annotationName, LIBMATTI_FML_AutomaticEventSubscriber_EVENTBUSSUBSCRIBER) == 0;
        free(annotationName);
        if (!isSubscriber) continue;

        // Java: EnumSet<Dist> sides = getSides(ad.annotationData().get("value"));
        LIBMATTI_DIST_Dist sides[2];
        size_t sideCount = LIBMATTI_FML_AutomaticEventSubscriber_GetSides(annotation_value(annotation, "value"),
                                                                          sides, 2);

        // Java: String modId = ad.annotationData().getOrDefault("modid", modids.getOrDefault(ad.clazz(),
        //       mod.getModId()));
        const char *modId = annotation_value(annotation, "modid");
        if (modId == NULL || modId[0] == '\0')
        {
            char *annotationClass = LIBMATTI_ASM_Type_GetClassName(annotation->clazz);
            modId = mod_id_for(scanData, annotationClass, mod);
            free(annotationClass);
        }
        if (modId == NULL) continue;

        char *annotationClass = LIBMATTI_ASM_Type_GetClassName(annotation->clazz);

        if (strcmp(LIBMATTI_FML_ModContainer_GetModId(mod), modId) != 0)
        {
            free(annotationClass);
            continue;
        }
        if (!sides_contain(sides, sideCount, dist))
        {
            free(annotationClass);
            continue;
        }

        LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_Logging_LOADING,
                                 "Scanning class {} for @SubscribeEvent-annotated methods", annotationClass);

        for (size_t j = 0; j < methodCount; j++)
        {
            if (strcmp(methods[j].className, annotationClass) != 0) continue;
            subscribe(mod, &methods[j], annotationClass);
        }
        free(annotationClass);
    }
}
