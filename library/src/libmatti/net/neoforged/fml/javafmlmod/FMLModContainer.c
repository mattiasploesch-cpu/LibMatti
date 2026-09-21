// Port of net.neoforged.fml.javafmlmod.FMLModContainer.

#include "libmatti/net/neoforged/fml/javafmlmod/FMLModContainer.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/bus/EventBusErrorMessage.h"
#include "libmatti/net/neoforged/bus/api/BusBuilder.h"
#include "libmatti/net/neoforged/fml/Logging.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/fml/ModLoadingException.h"
#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"
#include "libmatti/net/neoforged/fml/event/IModBusEvent.h"
#include "libmatti/net/neoforged/fml/javafmlmod/AutomaticEventSubscriber.h"
#include "libmatti/net/neoforged/fml/loading/FMLLoader.h"
#include "libmatti/net/neoforged/fml/loading/ModLibrary.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModFileInfo.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final Logger LOGGER = LogManager.getLogger();
static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: private void onEventFailed(IEventBus iEventBus, Event event, EventListener[] iEventListeners, int i, Throwable throwable)
static void fml_mod_container_on_event_failed(void *self, LIBMATTI_BUS_IEventBus *bus, LIBMATTI_BUS_Event *event,
                                             LIBMATTI_BUS_EventListener **listeners, size_t listenerCount, int index,
                                             LIBMATTI_JL_Throwable *throwable)
{
    (void)self;
    (void)bus;

    LIBMATTI_BUS_EventBusErrorMessage *message =
        LIBMATTI_BUS_EventBusErrorMessage_New(event, index, listeners, listenerCount, throwable);
    char *text = LIBMATTI_BUS_EventBusErrorMessage_FormatTo(message);
    LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "{}", text);
    free(text);
    LIBMATTI_BUS_EventBusErrorMessage_Free(message);
}

// Java: throw new ModLoadingException(issue)
static void throw_mod_loading_exception(LIBMATTI_FML_ModLoadingIssue *issue)
{
    // Java throws out of the caller and the load fails at the next throwIfErrors; the port records the
    // issue instead, which is what ModLoader.throwIfErrors then finds.
    LIBMATTI_FML_ModLoader_AddLoadingIssue(issue);

    LIBMATTI_FML_ModLoadingException *exception = LIBMATTI_FML_ModLoadingException_New(issue);
    char *message = LIBMATTI_FML_ModLoadingException_GetMessage(exception);
    // Java throws out of constructMod; the port records the issue and keeps the message for the log.
    LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "{}", message);
    free(message);
    LIBMATTI_FML_ModLoadingException_Free(exception);
}

// Java: Issue.withAffectedMod(info)
static LIBMATTI_FML_ModLoadingIssue *with_affected_mod(LIBMATTI_FML_ModLoadingIssue *issue,
                                                       LIBMATTI_NEOFORGESPI_IModInfo *info)
{
    LIBMATTI_NEOFORGESPI_IModFileInfo *owningFile = LIBMATTI_NEOFORGESPI_IModInfo_GetOwningFile(info);
    LIBMATTI_NEOFORGESPI_IModFile *file =
        owningFile != NULL ? LIBMATTI_NEOFORGESPI_IModFileInfo_GetFile(owningFile) : NULL;
    return LIBMATTI_FML_ModLoadingIssue_WithAffectedMod(issue, info, file,
                                                        file != NULL ? LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(file)
                                                                     : NULL);
}

// Java: parameterTypes[i].getSimpleName() in the "unsupported argument" message
static const char *arg_name(LIBMATTI_FML_FMLModContainer_Arg arg)
{
    switch (arg)
    {
    case LIBMATTI_FML_FMLModContainer_ARG_EVENT_BUS: return "IEventBus";
    case LIBMATTI_FML_FMLModContainer_ARG_MOD_CONTAINER: return "ModContainer";
    case LIBMATTI_FML_FMLModContainer_ARG_FML_MOD_CONTAINER: return "FMLModContainer";
    case LIBMATTI_FML_FMLModContainer_ARG_DIST: return "Dist";
    }
    return "?";
}

// Java: the entrypoints list in "Creating FMLModContainer instance for {} with entrypoints {}"
static char *join_entrypoints(const char **entrypoints, size_t count)
{
    size_t length = 3; // "[ ]"
    for (size_t i = 0; i < count; i++)
        length += strlen(entrypoints[i]) + 2;

    char *result = malloc(length);
    result[0] = '\0';
    strcat(result, "[");
    for (size_t i = 0; i < count; i++)
    {
        if (i > 0) strcat(result, ", ");
        strcat(result, entrypoints[i]);
    }
    strcat(result, "]");
    return result;
}

static void fml_mod_container_construct_mod(void *self);
static LIBMATTI_BUS_IEventBus *fml_mod_container_get_event_bus(void *self);

// Java: private final IEventBus eventBus = BusBuilder.builder()...
static LIBMATTI_BUS_IEventBus *create_event_bus(LIBMATTI_FML_FMLModContainer *container)
{
    LIBMATTI_BUS_BusBuilder *builder = LIBMATTI_BUS_BusBuilder_Builder();
    LIBMATTI_BUS_IEventExceptionHandler handler = {.self = container,
                                                  .handleException = fml_mod_container_on_event_failed};

    builder = builder->setExceptionHandler(builder, handler);
    builder = builder->markerType(builder, LIBMATTI_FML_Event_IModBusEvent_Class());
    builder = builder->allowPerPhasePost(builder);
    return builder->build(builder);
}

// Java: public FMLModContainer(IModInfo info, List<String> entrypoints, ModFileScanData modFileScanResults, ModuleLayer gameLayer)
LIBMATTI_FML_FMLModContainer *LIBMATTI_FML_FMLModContainer_New(
    LIBMATTI_NEOFORGESPI_IModInfo *info, const char **entrypoints, size_t entrypointCount,
    LIBMATTI_NEOFORGESPI_ModFileScanData *modFileScanResults, const char *gameLayer)
{
    LIBMATTI_FML_FMLModContainer *container = calloc(1, sizeof(LIBMATTI_FML_FMLModContainer));
    LIBMATTI_FML_ModContainer_Init(&container->base, info);

    container->base.constructMod = fml_mod_container_construct_mod;
    container->base.getEventBus = fml_mod_container_get_event_bus;
    // Java: the subclass adds no fields ModContainer does not already release
    container->base.free = NULL;

    container->scanResults = modFileScanResults;
    container->eventBus = create_event_bus(container);
    container->layer = strdup(gameLayer != NULL ? gameLayer : "");

    // Java: Class.forName(layer, entrypoint) - the port keeps the entrypoint names; the mod registers
    // its constructor with RegisterModClass
    container->entrypoints = calloc(entrypointCount > 0 ? entrypointCount : 1, sizeof(char *));
    container->entrypointCount = entrypointCount;
    for (size_t i = 0; i < entrypointCount; i++)
        container->entrypoints[i] = strdup(entrypoints[i]);

    char *joined = join_entrypoints(entrypoints, entrypointCount);
    LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_Logging_LOADING,
                             "Creating FMLModContainer instance for {} with entrypoints {}",
                             LIBMATTI_NEOFORGESPI_IModInfo_GetModId(info), joined);
    free(joined);

    // Java: Class.forName(layer, entrypoint) defines the mod's classes here; the port's mod code is the
    // shared object of the mod file, so it is loaded here and its entry point is run by constructMod.
    LIBMATTI_NEOFORGESPI_IModFileInfo *owningFile = LIBMATTI_NEOFORGESPI_IModInfo_GetOwningFile(info);
    LIBMATTI_NEOFORGESPI_IModFile *modFile =
        owningFile != NULL ? LIBMATTI_NEOFORGESPI_IModFileInfo_GetFile(owningFile) : NULL;
    const char *modPath = modFile != NULL ? LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(modFile) : NULL;

    container->library = LIBMATTI_FML_ModLibrary_Load(modPath, LIBMATTI_NEOFORGESPI_IModInfo_GetModId(info));
    if (container->library == NULL)
        throw_mod_loading_exception(with_affected_mod(
            LIBMATTI_FML_ModLoadingIssue_Error("fml.modloadingissue.failedtoloadmodclass", NULL, 0),
            container->base.modInfo));

    return container;
}

void LIBMATTI_FML_FMLModContainer_Free(LIBMATTI_FML_FMLModContainer *container)
{
    if (container == NULL) return;

    for (size_t i = 0; i < container->entrypointCount; i++)
        free(container->entrypoints[i]);
    free(container->entrypoints);

    for (size_t i = 0; i < container->modClassCount; i++)
    {
        free(container->modClasses[i].className);
        free(container->modClasses[i].args);
    }
    free(container->modClasses);
    free(container->layer);
    LIBMATTI_FML_ModLibrary_Free(container->library);
    LIBMATTI_FML_ModContainer_Free(&container->base);
}

// Java: Class.forName(layer, entrypoint) + constructor.newInstance
void LIBMATTI_FML_FMLModContainer_RegisterModClass(
    LIBMATTI_FML_FMLModContainer *container, const char *className,
    const LIBMATTI_FML_FMLModContainer_Arg *args, size_t argCount,
    void (*construct)(void *self, LIBMATTI_FML_FMLModContainer *container,
                      const LIBMATTI_FML_FMLModContainer_ConstructorArgs *args),
    void *self)
{
    container->modClasses =
        realloc(container->modClasses, sizeof(*container->modClasses) * (container->modClassCount + 1));
    LIBMATTI_FML_FMLModContainer_ModClass *modClass = &container->modClasses[container->modClassCount++];

    modClass->className = strdup(className);
    modClass->args = calloc(argCount > 0 ? argCount : 1, sizeof(*modClass->args));
    for (size_t i = 0; i < argCount; i++)
        modClass->args[i] = args[i];
    modClass->argCount = argCount;
    modClass->construct = construct;
    modClass->self = self;
}

static LIBMATTI_FML_FMLModContainer_ModClass *find_mod_class(LIBMATTI_FML_FMLModContainer *container,
                                                             const char *className)
{
    for (size_t i = 0; i < container->modClassCount; i++)
        if (strcmp(container->modClasses[i].className, className) == 0) return &container->modClasses[i];

    return NULL;
}

// Java: @Override protected void constructMod()
static void fml_mod_container_construct_mod(void *self)
{
    LIBMATTI_FML_FMLModContainer *container = self;

    // Java: Map<Class<?>, Object> allowedConstructorArgs
    LIBMATTI_FML_FMLModContainer_ConstructorArgs args = {
        .eventBus = container->eventBus,
        .modContainer = &container->base,
        .fmlModContainer = container,
        .dist = LIBMATTI_FML_FMLLoader_GetDist(LIBMATTI_FML_FMLLoader_GetCurrent()),
    };

    for (size_t i = 0; i < container->entrypointCount; i++)
    {
        const char *entrypoint = container->entrypoints[i];
        LIBMATTI_FML_FMLModContainer_ModClass *modClass = find_mod_class(container, entrypoint);

        // Java: Class.forName(layer, entrypoint) throws ClassNotFoundException
        if (modClass == NULL)
        {
            throw_mod_loading_exception(with_affected_mod(
                LIBMATTI_FML_ModLoadingIssue_Error("fml.modloadingissue.failedtoloadmodclass", &entrypoint, 1),
                container->base.modInfo));
            continue;
        }

        // Java: every parameter type must be one of the allowed arguments and occur once
        int invalid = 0;
        for (size_t j = 0; j < modClass->argCount; j++)
        {
            if ((int)modClass->args[j] < 0 || modClass->args[j] > LIBMATTI_FML_FMLModContainer_ARG_DIST)
            {
                LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_Logging_LOADING,
                                         "Mod constructor has unsupported argument {}. Allowed optional argument classes: IEventBus, ModContainer, FMLModContainer, Dist",
                                         "?");
                invalid = 1;
                break;
            }

            for (size_t k = 0; k < j; k++)
            {
                if (modClass->args[k] == modClass->args[j])
                {
                    LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_Logging_LOADING,
                                             "Duplicate mod constructor argument type: {}",
                                             arg_name(modClass->args[j]));
                    invalid = 1;
                    break;
                }
            }
            if (invalid) break;
        }

        if (invalid)
        {
            throw_mod_loading_exception(with_affected_mod(
                LIBMATTI_FML_ModLoadingIssue_Error("fml.modloadingissue.failedtoloadmod", NULL, 0),
                container->base.modInfo));
            continue;
        }

        // Java: constructor.newInstance(constructorArgs)
        if (modClass->construct != NULL) modClass->construct(modClass->self, container, &args);

        LIBMATTI_ML_Logger_Trace(LOGGER(), &LIBMATTI_FML_Logging_LOADING, "Loaded mod instance {} of type {}",
                                 container->base.modId, modClass->className);
    }

    // Java: constructor.newInstance(constructorArgs) - the port's mod instance is created by the mod
    // file's entry point.
    if (LIBMATTI_FML_ModLibrary_HasEntrypoint(container->library))
    {
        LIBMATTI_FML_ModLibrary_Construct(container->library, &args);
        LIBMATTI_ML_Logger_Trace(LOGGER(), &LIBMATTI_FML_Logging_LOADING, "Loaded mod instance {} of type {}",
                                 container->base.modId,
                                 LIBMATTI_FML_ModLibrary_EntrypointName(container->library));
    }

    // Java: AutomaticEventSubscriber.inject(this, this.scanResults, layer)
    LIBMATTI_ML_Logger_Trace(LOGGER(), &LIBMATTI_FML_Logging_LOADING,
                             "Injecting Automatic event subscribers for {}", container->base.modId);
    LIBMATTI_FML_AutomaticEventSubscriber_Inject(&container->base, container->scanResults, container->layer);
    LIBMATTI_ML_Logger_Trace(LOGGER(), &LIBMATTI_FML_Logging_LOADING,
                             "Completed Automatic event subscribers for {}", container->base.modId);
}

// Java: @Override public IEventBus getEventBus()
LIBMATTI_BUS_IEventBus *LIBMATTI_FML_FMLModContainer_GetEventBus(LIBMATTI_FML_FMLModContainer *container)
{
    return container->eventBus;
}

static LIBMATTI_BUS_IEventBus *fml_mod_container_get_event_bus(void *self)
{
    return LIBMATTI_FML_FMLModContainer_GetEventBus(self);
}
