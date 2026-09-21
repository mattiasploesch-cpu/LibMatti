// Port of org.spongepowered.asm.launch.platform.MixinConnectorManager.
// "Manager for Mixin containers bootstrapping via IMixinConnector"

#include "libmatti/org/spongepowered/asm/launch/platform/MixinConnectorManager.h"

#include "libmatti/java/lang/Class.h"
#include "libmatti/org/spongepowered/asm/service/IMixinService.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final ILogger logger = MixinService.getService().getLogger("mixin");
static LIBMATTI_SP_ILogger *LOGGER(void)
{
    LIBMATTI_SP_IMixinService *service = LIBMATTI_SP_MixinService_GetService();
    return service != NULL ? service->getLogger(service->self, "mixin") : NULL;
}

// Java: IMixinConnector connector = connectorClass.getDeclaredConstructor().newInstance();
// A C connector is a registered class with the static "connect" method in its method table
// (the port's equivalent of implementing the interface); the adapter dispatches onto it.
static void connector_connect(void *self)
{
    LIBMATTI_JL_Class_StaticMethod *method = self;
    method->function(0, NULL);
}

// Java: @SuppressWarnings("unchecked") void loadConnectors()
static void load_connectors(LIBMATTI_SP_MixinConnectorManager *manager)
{
    LIBMATTI_SP_IMixinService *service = LIBMATTI_SP_MixinService_GetService();
    void *classProvider = service != NULL && service->getClassProvider != NULL
                              ? service->getClassProvider(service->self)
                              : NULL;

    for (size_t i = 0; i < manager->connectorClassCount; i++)
    {
        const char *connectorClassName = manager->connectorClasses[i];

        // Java: Class<?> clazz = classProvider.findClass(connectorClassName);
        LIBMATTI_JL_Class *clazz = NULL;
        if (classProvider != NULL)
        {
            LIBMATTI_SP_IClassProvider *provider = classProvider;
            clazz = provider->findClass(provider->self, connectorClassName, 1);
        }

        // Java: catch (ClassNotFoundException ex) { logger.catching(ex); continue; }
        if (clazz == NULL)
        {
            char *msg = malloc(strlen("Mixin Connector [] was not found") + strlen(connectorClassName) + 1);
            sprintf(msg, "Mixin Connector [%s] was not found", connectorClassName);
            LIBMATTI_SP_ILogger_Warn(LOGGER(), msg);
            free(msg);
            continue;
        }

        // Java: if (!IMixinConnector.class.isAssignableFrom(clazz)) { logger.error(...); continue; }
        LIBMATTI_JL_Class_StaticMethod *method = LIBMATTI_JL_Class_FindStaticMethod(clazz, "connect");
        if (method == NULL)
        {
            char *msg = malloc(strlen("Mixin Connector [] does not implement IMixinConnector") + strlen(connectorClassName) + 1);
            sprintf(msg, "Mixin Connector [%s] does not implement IMixinConnector", connectorClassName);
            LIBMATTI_SP_ILogger_Error(LOGGER(), msg);
            free(msg);
            continue;
        }

        // Java: IMixinConnector connector = connectorClass.getDeclaredConstructor().newInstance();
        LIBMATTI_SP_IMixinConnector *connector = malloc(sizeof(*connector));
        connector->self = method;
        connector->connect = connector_connect;
        manager->connectors = realloc(manager->connectors,
                                      sizeof(LIBMATTI_SP_IMixinConnector *) * (manager->connectorCount + 1));
        manager->connectors[manager->connectorCount++] = connector;

        // Java: logger.info("Successfully loaded Mixin Connector [{}]", connectorClassName);
        char *msg = malloc(strlen("Successfully loaded Mixin Connector []") + strlen(connectorClassName) + 1);
        sprintf(msg, "Successfully loaded Mixin Connector [%s]", connectorClassName);
        LIBMATTI_SP_ILogger_Info(LOGGER(), msg);
        free(msg);
    }
}

// Java: void initConnectors()
static void init_connectors(LIBMATTI_SP_MixinConnectorManager *manager)
{
    for (size_t i = 0; i < manager->connectorCount; i++)
    {
        LIBMATTI_SP_IMixinConnector *connector = manager->connectors[i];
        // Java: try { connector.connect(); } catch (Exception ex) { logger.warn(...); }
        connector->connect(connector->self);
    }
}

// Java: MixinConnectorManager()
LIBMATTI_SP_MixinConnectorManager *LIBMATTI_SP_MixinConnectorManager_New(void)
{
    return calloc(1, sizeof(LIBMATTI_SP_MixinConnectorManager));
}

void LIBMATTI_SP_MixinConnectorManager_Free(LIBMATTI_SP_MixinConnectorManager *manager)
{
    if (manager == NULL) return;
    for (size_t i = 0; i < manager->connectorClassCount; i++) free(manager->connectorClasses[i]);
    free(manager->connectorClasses);
    for (size_t i = 0; i < manager->connectorCount; i++) free(manager->connectors[i]);
    free(manager->connectors);
    free(manager);
}

// Java: void addConnector(String connectorClass) { this.connectorClasses.add(connectorClass); }
void LIBMATTI_SP_MixinConnectorManager_AddConnector(LIBMATTI_SP_MixinConnectorManager *manager,
                                                    const char *connectorClass)
{
    // Java: a LinkedHashSet
    for (size_t i = 0; i < manager->connectorClassCount; i++)
        if (strcmp(manager->connectorClasses[i], connectorClass) == 0) return;

    manager->connectorClasses = realloc(manager->connectorClasses, sizeof(char *) * (manager->connectorClassCount + 1));
    manager->connectorClasses[manager->connectorClassCount++] = strdup(connectorClass);
}

// Java: void inject() { this.loadConnectors(); this.initConnectors(); }
void LIBMATTI_SP_MixinConnectorManager_Inject(LIBMATTI_SP_MixinConnectorManager *manager)
{
    load_connectors(manager);
    init_connectors(manager);

    // Java: this.connectorClasses.clear();
    for (size_t i = 0; i < manager->connectorClassCount; i++) free(manager->connectorClasses[i]);
    free(manager->connectorClasses);
    manager->connectorClasses = NULL;
    manager->connectorClassCount = 0;
}
