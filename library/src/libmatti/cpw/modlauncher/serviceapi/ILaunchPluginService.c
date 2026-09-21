// Port of cpw.mods.modlauncher.serviceapi.ILaunchPluginService.

#include "libmatti/cpw/modlauncher/serviceapi/ILaunchPluginService.h"

const char *LIBMATTI_MLS_ILaunchPluginService_Name(LIBMATTI_MLS_ILaunchPluginService *service)
{
    return service->name(service);
}

int LIBMATTI_MLS_ILaunchPluginService_HandlesClass(LIBMATTI_MLS_ILaunchPluginService *service,
                                                   const LIBMATTI_ASM_Type *classType, int isEmpty, const char *reason)
{
    // Java: default EnumSet<Phase> handlesClass(Type, boolean, String) { return handlesClass(classType, isEmpty); }
    if (service->handlesClassWithReason != NULL)
        return service->handlesClassWithReason(service, classType, isEmpty, reason);
    if (service->handlesClass != NULL) return service->handlesClass(service, classType, isEmpty);
    return LIBMATTI_MLS_PHASES_NONE;
}

int LIBMATTI_MLS_ILaunchPluginService_ProcessClassWithFlags(LIBMATTI_MLS_ILaunchPluginService *service,
                                                            LIBMATTI_MLS_Phase phase,
                                                            LIBMATTI_ASMT_ClassNode *classNode,
                                                            const LIBMATTI_ASM_Type *classType, const char *reason)
{
    if (service->processClassWithFlags != NULL)
        return service->processClassWithFlags(service, phase, classNode, classType, reason);

    int changed;
    // Java: default boolean processClass(Phase, ClassNode, Type, String) { return processClass(phase, node, type); }
    if (service->processClassWithReason != NULL)
        changed = service->processClassWithReason(service, phase, classNode, classType, reason);
    else
        changed = service->processClass(service, phase, classNode, classType);

    // Java: return processClass(...) ? ComputeFlags.COMPUTE_FRAMES : ComputeFlags.NO_REWRITE
    return changed ? LIBMATTI_MLS_COMPUTE_FRAMES : LIBMATTI_MLS_COMPUTE_NO_REWRITE;
}

void LIBMATTI_MLS_ILaunchPluginService_AddResources(LIBMATTI_MLS_ILaunchPluginService *service,
                                                    LIBMATTI_JH_SecureJar **resources, size_t resourceCount)
{
    // Java: default void addResources(List<SecureJar> resources) {}
    if (service->addResources != NULL) service->addResources(service, resources, resourceCount);
}

void LIBMATTI_MLS_ILaunchPluginService_InitializeLaunch(LIBMATTI_MLS_ILaunchPluginService *service,
                                                        LIBMATTI_MLS_ITransformerLoader *loader,
                                                        LIBMATTI_MLA_NamedPath *specialPaths, size_t specialPathCount)
{
    // Java: default void initializeLaunch(ITransformerLoader, NamedPath[]) {}
    if (service->initializeLaunch != NULL) service->initializeLaunch(service, loader, specialPaths, specialPathCount);
}

void LIBMATTI_MLS_ILaunchPluginService_CustomAuditConsumer(
    LIBMATTI_MLS_ILaunchPluginService *service, const char *className,
    void (*auditDataAcceptor)(char **data, size_t count, void *userdata), void *userdata)
{
    // Java: default void customAuditConsumer(String, Consumer<String[]>) {}
    if (service->customAuditConsumer != NULL)
        service->customAuditConsumer(service, className, auditDataAcceptor, userdata);
}
