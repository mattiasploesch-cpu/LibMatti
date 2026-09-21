// Port of org.spongepowered.asm.launch.platform.MixinPlatformAgentAbstract
// and org.spongepowered.asm.launch.platform.MixinPlatformAgentDefault.

#include "libmatti/org/spongepowered/asm/launch/platform/MixinPlatformAgentDefault.h"

#include "libmatti/org/spongepowered/asm/launch/platform/MixinPlatformManager.h"
#include "libmatti/org/spongepowered/asm/service/IMixinService.h"
#include "libmatti/org/spongepowered/asm/util/Constants.h"

#include <stdlib.h>
#include <string.h>

// Java: protected static final ILogger logger = MixinService.getService().getLogger("mixin");
static LIBMATTI_SP_ILogger *LOGGER(void)
{
    LIBMATTI_SP_IMixinService *service = LIBMATTI_SP_MixinService_GetService();
    return service != NULL ? service->getLogger(service->self, "mixin") : NULL;
}

// ---------------------------------------------------------------------------
// MixinPlatformAgentAbstract (the base behaviour)
// ---------------------------------------------------------------------------

// Java: public AcceptResult accept(MixinPlatformManager manager, IContainerHandle handle)
//       { this.manager = manager; this.handle = handle; return AcceptResult.ACCEPTED; }
static LIBMATTI_SP_AcceptResult abstract_accept(void *self, LIBMATTI_SP_MixinPlatformManager *manager,
                                                LIBMATTI_SP_ContainerHandle *handle)
{
    LIBMATTI_SP_MixinPlatformAgentAbstract *agent = self;
    agent->manager = manager;
    agent->handle = handle;
    return LIBMATTI_SP_AcceptResult_ACCEPTED;
}

// Java: public String getPhaseProvider() { return null; }
static const char *abstract_get_phase_provider(void *self)
{
    (void) self;
    return NULL;
}

// Java: public void prepare() {}
static void abstract_prepare(void *self)
{
    (void) self;
}

// Java: public void initPrimaryContainer() {}
static void abstract_init_primary_container(void *self)
{
    (void) self;
}

// Java: public void inject() {}
static void abstract_inject(void *self)
{
    (void) self;
}

// Java: protected MixinPlatformAgentAbstract()
void LIBMATTI_SP_MixinPlatformAgentAbstract_Init(LIBMATTI_SP_MixinPlatformAgentAbstract *agent)
{
    agent->manager = NULL;
    agent->handle = NULL;

    agent->base.self = agent;
    agent->base.accept = abstract_accept;
    agent->base.getPhaseProvider = abstract_get_phase_provider;
    agent->base.prepare = abstract_prepare;
    agent->base.initPrimaryContainer = abstract_init_primary_container;
    agent->base.inject = abstract_inject;
}

// ---------------------------------------------------------------------------
// MixinPlatformAgentDefault
// ---------------------------------------------------------------------------

// Java: public void prepare()
static void default_prepare(void *self)
{
    LIBMATTI_SP_MixinPlatformAgentAbstract *agent = self;
    LIBMATTI_SP_MixinPlatformManager *manager = agent->manager;
    LIBMATTI_SP_ContainerHandle *handle = agent->handle;

    // Java: String compatibilityLevel = this.handle.getAttribute(ManifestAttributes.COMPATIBILITY);
    const char *compatibilityLevel = handle->getAttribute(handle->self, LIBMATTI_SP_Constants_ATTR_COMPATIBILITY);
    if (compatibilityLevel != NULL)
    {
        // Java: this.manager.setCompatibilityLevel(compatibilityLevel);
        LIBMATTI_SP_MixinPlatformManager_SetCompatibilityLevel(manager, compatibilityLevel);
    }

    // Java: String mixinConfigs = this.handle.getAttribute(ManifestAttributes.MIXINCONFIGS);
    const char *mixinConfigs = handle->getAttribute(handle->self, LIBMATTI_SP_Constants_ATTR_MIXINCONFIGS);
    if (mixinConfigs != NULL)
    {
        // Java: for (String config : mixinConfigs.split(",")) this.manager.addConfig(config.trim(), this.handle);
        char *split = strdup(mixinConfigs);
        char *saveptr = NULL;
        for (char *token = strtok_r(split, ",", &saveptr); token != NULL; token = strtok_r(NULL, ",", &saveptr))
        {
            // Java: config.trim() - the tokenizer already dropped the separators; trim spaces
            while (*token == ' ' || *token == '\t') token++;
            char *end = token + strlen(token);
            while (end > token && (end[-1] == ' ' || end[-1] == '\t')) *--end = '\0';
            LIBMATTI_SP_MixinPlatformManager_AddConfig(manager, token, handle);
        }
        free(split);
    }

    // Java: String tokenProviders = this.handle.getAttribute(ManifestAttributes.TOKENPROVIDERS);
    const char *tokenProviders = handle->getAttribute(handle->self, LIBMATTI_SP_Constants_ATTR_TOKENPROVIDERS);
    if (tokenProviders != NULL)
    {
        // Java: for (String provider : tokenProviders.split(",")) this.manager.addTokenProvider(provider.trim());
        char *split = strdup(tokenProviders);
        char *saveptr = NULL;
        for (char *token = strtok_r(split, ",", &saveptr); token != NULL; token = strtok_r(NULL, ",", &saveptr))
        {
            while (*token == ' ' || *token == '\t') token++;
            char *end = token + strlen(token);
            while (end > token && (end[-1] == ' ' || end[-1] == '\t')) *--end = '\0';
            LIBMATTI_SP_MixinPlatformManager_AddTokenProvider(manager, token);
        }
        free(split);
    }

    // Java: String connectorClass = this.handle.getAttribute(ManifestAttributes.MIXINCONNECTOR);
    const char *connectorClass = handle->getAttribute(handle->self, LIBMATTI_SP_Constants_ATTR_MIXINCONNECTOR);
    if (connectorClass != NULL)
    {
        // Java: this.manager.addConnector(connectorClass.trim());
        LIBMATTI_SP_MixinPlatformManager_AddConnector(manager, connectorClass);
    }
}

// Java: public class MixinPlatformAgentDefault extends MixinPlatformAgentAbstract
LIBMATTI_SP_MixinPlatformAgentDefault *LIBMATTI_SP_MixinPlatformAgentDefault_New(void)
{
    LIBMATTI_SP_MixinPlatformAgentDefault *agent = calloc(1, sizeof(*agent));
    LIBMATTI_SP_MixinPlatformAgentAbstract_Init(&agent->super);
    agent->super.base.prepare = default_prepare;
    return agent;
}

void LIBMATTI_SP_MixinPlatformAgentDefault_Free(LIBMATTI_SP_MixinPlatformAgentDefault *agent)
{
    free(agent);
}
