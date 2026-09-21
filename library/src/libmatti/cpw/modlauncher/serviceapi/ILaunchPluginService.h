// Port of cpw.mods.modlauncher.serviceapi.ILaunchPluginService.
// Java's default methods become optional function pointers: NULL is "the default".

#ifndef MATTICRAFT_MODLAUNCHER_ILAUNCHPLUGINSERVICE_H
#define MATTICRAFT_MODLAUNCHER_ILAUNCHPLUGINSERVICE_H

#include "libmatti/bsl/sjh/jarhandling/SecureJar.h"
#include "libmatti/cpw/modlauncher/api/NamedPath.h"
#include "libmatti/org/objectweb/asm/Type.h"
#include "libmatti/org/objectweb/asm/tree/ClassNode.h"

#include <stddef.h>

// Java: enum Phase { BEFORE, AFTER }
typedef enum
{
    LIBMATTI_MLS_PHASE_BEFORE = 0,
    LIBMATTI_MLS_PHASE_AFTER
} LIBMATTI_MLS_Phase;

// Java: EnumSet<Phase> - expressed as a bit set of the two phases
#define LIBMATTI_MLS_PHASES_NONE 0
#define LIBMATTI_MLS_PHASES_BEFORE 1
#define LIBMATTI_MLS_PHASES_AFTER 2

// Java: class ComputeFlags
#define LIBMATTI_MLS_COMPUTE_NO_REWRITE 0
#define LIBMATTI_MLS_COMPUTE_SIMPLE_REWRITE 0x100
#define LIBMATTI_MLS_COMPUTE_MAXS 1
#define LIBMATTI_MLS_COMPUTE_FRAMES 2

// Java: interface ITransformerLoader { byte[] buildTransformedClassNodeFor(String className); }
typedef struct LIBMATTI_MLS_ITransformerLoader
{
    unsigned char *(*buildTransformedClassNodeFor)(struct LIBMATTI_MLS_ITransformerLoader *self, const char *className,
                                                   size_t *length);
} LIBMATTI_MLS_ITransformerLoader;

// Java: interface ILaunchPluginService
typedef struct LIBMATTI_MLS_ILaunchPluginService
{
    // Java: String name()
    const char *(*name)(struct LIBMATTI_MLS_ILaunchPluginService *self);
    // Java: EnumSet<Phase> handlesClass(Type classType, boolean isEmpty)
    int (*handlesClass)(struct LIBMATTI_MLS_ILaunchPluginService *self, const LIBMATTI_ASM_Type *classType,
                        int isEmpty);
    // Java: default EnumSet<Phase> handlesClass(Type, boolean, String reason)
    int (*handlesClassWithReason)(struct LIBMATTI_MLS_ILaunchPluginService *self, const LIBMATTI_ASM_Type *classType,
                                  int isEmpty, const char *reason);
    // Java: default boolean processClass(Phase, ClassNode, Type)
    int (*processClass)(struct LIBMATTI_MLS_ILaunchPluginService *self, LIBMATTI_MLS_Phase phase,
                        LIBMATTI_ASMT_ClassNode *classNode, const LIBMATTI_ASM_Type *classType);
    // Java: default boolean processClass(Phase, ClassNode, Type, String reason)
    int (*processClassWithReason)(struct LIBMATTI_MLS_ILaunchPluginService *self, LIBMATTI_MLS_Phase phase,
                                  LIBMATTI_ASMT_ClassNode *classNode, const LIBMATTI_ASM_Type *classType,
                                  const char *reason);
    // Java: default int processClassWithFlags(Phase, ClassNode, Type, String reason)
    int (*processClassWithFlags)(struct LIBMATTI_MLS_ILaunchPluginService *self, LIBMATTI_MLS_Phase phase,
                                 LIBMATTI_ASMT_ClassNode *classNode, const LIBMATTI_ASM_Type *classType,
                                 const char *reason);
    // Java: default void offerResource(Path resource, String name)
    void (*offerResource)(struct LIBMATTI_MLS_ILaunchPluginService *self, const char *resource, const char *name);
    // Java: default void addResources(List<SecureJar> resources)
    void (*addResources)(struct LIBMATTI_MLS_ILaunchPluginService *self, LIBMATTI_JH_SecureJar **resources,
                         size_t resourceCount);
    // Java: default void initializeLaunch(ITransformerLoader, NamedPath[] specialPaths)
    void (*initializeLaunch)(struct LIBMATTI_MLS_ILaunchPluginService *self, LIBMATTI_MLS_ITransformerLoader *loader,
                             LIBMATTI_MLA_NamedPath *specialPaths, size_t specialPathCount);
    // Java: default <T> T getExtension() { return null; }
    void *(*getExtension)(struct LIBMATTI_MLS_ILaunchPluginService *self);
    // Java: default void customAuditConsumer(String className, Consumer<String[]> auditDataAcceptor)
    void (*customAuditConsumer)(struct LIBMATTI_MLS_ILaunchPluginService *self, const char *className,
                                void (*auditDataAcceptor)(char **data, size_t count, void *userdata), void *userdata);
} LIBMATTI_MLS_ILaunchPluginService;

// Java: the interface methods, dispatched to the implementing vtable (including defaults)
const char *LIBMATTI_MLS_ILaunchPluginService_Name(LIBMATTI_MLS_ILaunchPluginService *service);
// Java: EnumSet<Phase> handlesClass(Type, boolean, String reason)
int LIBMATTI_MLS_ILaunchPluginService_HandlesClass(LIBMATTI_MLS_ILaunchPluginService *service,
                                                   const LIBMATTI_ASM_Type *classType, int isEmpty, const char *reason);
// Java: int processClassWithFlags(Phase, ClassNode, Type, String reason)
int LIBMATTI_MLS_ILaunchPluginService_ProcessClassWithFlags(LIBMATTI_MLS_ILaunchPluginService *service,
                                                            LIBMATTI_MLS_Phase phase,
                                                            LIBMATTI_ASMT_ClassNode *classNode,
                                                            const LIBMATTI_ASM_Type *classType, const char *reason);
// Java: default void addResources(List<SecureJar> resources)
void LIBMATTI_MLS_ILaunchPluginService_AddResources(LIBMATTI_MLS_ILaunchPluginService *service,
                                                    LIBMATTI_JH_SecureJar **resources, size_t resourceCount);
// Java: default void initializeLaunch(ITransformerLoader, NamedPath[] specialPaths)
void LIBMATTI_MLS_ILaunchPluginService_InitializeLaunch(LIBMATTI_MLS_ILaunchPluginService *service,
                                                        LIBMATTI_MLS_ITransformerLoader *loader,
                                                        LIBMATTI_MLA_NamedPath *specialPaths,
                                                        size_t specialPathCount);
// Java: default void customAuditConsumer(String className, Consumer<String[]> auditDataAcceptor)
void LIBMATTI_MLS_ILaunchPluginService_CustomAuditConsumer(
    LIBMATTI_MLS_ILaunchPluginService *service, const char *className,
    void (*auditDataAcceptor)(char **data, size_t count, void *userdata), void *userdata);

#endif //MATTICRAFT_MODLAUNCHER_ILAUNCHPLUGINSERVICE_H
