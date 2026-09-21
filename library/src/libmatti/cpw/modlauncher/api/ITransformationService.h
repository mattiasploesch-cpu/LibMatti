// Port of cpw.mods.modlauncher.api.ITransformationService.
// Java's default methods (arguments, argumentValues, beginScanning, completeScan)
// become optional function pointers: a NULL entry is "the default".

#ifndef MATTICRAFT_MODLAUNCHER_ITRANSFORMATIONSERVICE_H
#define MATTICRAFT_MODLAUNCHER_ITRANSFORMATIONSERVICE_H

#include "libmatti/bsl/sjh/jarhandling/SecureJar.h"
#include "libmatti/cpw/modlauncher/api/IEnvironment.h"
#include "libmatti/cpw/modlauncher/api/IModuleLayerManager.h"
#include "libmatti/cpw/modlauncher/api/ITransformer.h"
#include "libmatti/joptsimple/OptionSpec.h"

#include <stddef.h>

// Java: record Resource(IModuleLayerManager.Layer target, List<SecureJar> resources)
typedef struct
{
    LIBMATTI_MLA_Layer target;
    LIBMATTI_JH_SecureJar **resources;
    size_t resourceCount;
} LIBMATTI_MLA_ITransformationService_Resource;

// Java: interface OptionResult { <V> V value(OptionSpec<V>); List<V> values(OptionSpec<V>); }
typedef struct LIBMATTI_MLA_ITransformationService_OptionResult
{
    const char *(*value)(struct LIBMATTI_MLA_ITransformationService_OptionResult *self,
                         const LIBMATTI_JOPT_OptionSpec *option);
    char **(*values)(struct LIBMATTI_MLA_ITransformationService_OptionResult *self,
                     const LIBMATTI_JOPT_OptionSpec *option, size_t *count);
} LIBMATTI_MLA_ITransformationService_OptionResult;

// Java: interface ITransformationService
typedef struct LIBMATTI_MLA_ITransformationService
{
    // Java: String name()
    const char *(*name)(struct LIBMATTI_MLA_ITransformationService *self);
    // Java: default void arguments(BiFunction<String, String, OptionSpecBuilder> argumentBuilder)
    void (*arguments)(struct LIBMATTI_MLA_ITransformationService *self,
                      LIBMATTI_JOPT_OptionSpec *(*builder)(const char *name, const char *description, void *userdata),
                      void *userdata);
    // Java: default void argumentValues(OptionResult option)
    void (*argumentValues)(struct LIBMATTI_MLA_ITransformationService *self,
                           LIBMATTI_MLA_ITransformationService_OptionResult *option);
    // Java: void initialize(IEnvironment environment)
    void (*initialize)(struct LIBMATTI_MLA_ITransformationService *self, LIBMATTI_MLA_IEnvironment *environment);
    // Java: void onLoad(IEnvironment env, Set<String> otherServices) - 1 = loaded, 0 = IncompatibleEnvironmentException
    int (*onLoad)(struct LIBMATTI_MLA_ITransformationService *self, LIBMATTI_MLA_IEnvironment *env,
                  char **otherServices, size_t otherServiceCount);
    // Java: List<? extends ITransformer<?>> transformers()
    LIBMATTI_MLA_ITransformer **(*transformers)(struct LIBMATTI_MLA_ITransformationService *self, size_t *count);
    // Java: default List<Resource> beginScanning(IEnvironment environment)
    LIBMATTI_MLA_ITransformationService_Resource *(*beginScanning)(struct LIBMATTI_MLA_ITransformationService *self,
                                                                   LIBMATTI_MLA_IEnvironment *environment,
                                                                   size_t *count);
    // Java: default List<Resource> completeScan(IModuleLayerManager layerManager)
    LIBMATTI_MLA_ITransformationService_Resource *(*completeScan)(struct LIBMATTI_MLA_ITransformationService *self,
                                                                  LIBMATTI_MLA_IModuleLayerManager *layerManager,
                                                                  size_t *count);
} LIBMATTI_MLA_ITransformationService;

// Java: the interface methods, dispatched to the implementing vtable (including defaults)
const char *LIBMATTI_MLA_ITransformationService_Name(LIBMATTI_MLA_ITransformationService *service);
void LIBMATTI_MLA_ITransformationService_Arguments(
    LIBMATTI_MLA_ITransformationService *service,
    LIBMATTI_JOPT_OptionSpec *(*builder)(const char *name, const char *description, void *userdata), void *userdata);
void LIBMATTI_MLA_ITransformationService_ArgumentValues(LIBMATTI_MLA_ITransformationService *service,
                                                        LIBMATTI_MLA_ITransformationService_OptionResult *option);
void LIBMATTI_MLA_ITransformationService_Initialize(LIBMATTI_MLA_ITransformationService *service,
                                                    LIBMATTI_MLA_IEnvironment *environment);
int LIBMATTI_MLA_ITransformationService_OnLoad(LIBMATTI_MLA_ITransformationService *service,
                                               LIBMATTI_MLA_IEnvironment *env, char **otherServices,
                                               size_t otherServiceCount);
LIBMATTI_MLA_ITransformer **LIBMATTI_MLA_ITransformationService_Transformers(
    LIBMATTI_MLA_ITransformationService *service, size_t *count);
// Java: default List<Resource> beginScanning(IEnvironment) - NULL means List.of()
LIBMATTI_MLA_ITransformationService_Resource *LIBMATTI_MLA_ITransformationService_BeginScanning(
    LIBMATTI_MLA_ITransformationService *service, LIBMATTI_MLA_IEnvironment *environment, size_t *count);
// Java: default List<Resource> completeScan(IModuleLayerManager) - NULL means List.of()
LIBMATTI_MLA_ITransformationService_Resource *LIBMATTI_MLA_ITransformationService_CompleteScan(
    LIBMATTI_MLA_ITransformationService *service, LIBMATTI_MLA_IModuleLayerManager *layerManager, size_t *count);

#endif //MATTICRAFT_MODLAUNCHER_ITRANSFORMATIONSERVICE_H
