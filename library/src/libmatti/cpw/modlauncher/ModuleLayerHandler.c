//
// Port of cpw.mods.modlauncher.ModuleLayerHandler.
//

#include "ModuleLayerHandler.h"

#include "LogManager.h"
#include "LogMarkers.h"
#include "libmatti/bsl/sjh/cl/JarModuleFinder.h"
#include "libmatti/java/lang/module/ResolvedModule.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private record PathOrJar(NamedPath path, SecureJar jar)
typedef struct
{
    LIBMATTI_MLA_NamedPath *path;
    LIBMATTI_JH_SecureJar *jar;
} PathOrJar;

typedef struct
{
    PathOrJar *items;
    size_t count;
} PathOrJarList;

// Java: private final EnumMap<Layer, List<PathOrJar>> layers
//       private final EnumMap<Layer, LayerInfo> completedLayers
struct LIBMATTI_ML_ModuleLayerHandler
{
    PathOrJarList layers[4];
    LIBMATTI_ML_ModuleLayerHandler_LayerInfo completedLayers[4];
    LIBMATTI_JL_ModuleLayerController *layerControllers[4];
    int completedFlags[4];
    // Java: the BOOT class loader may come from getClass().getClassLoader()
    int bootClassLoaderOwned;
};

// Java: getClass().getClassLoader()
static LIBMATTI_CL_ModuleClassLoader *bootClassLoader = NULL;

// Java: PathOrJar.build() - jar != null ? jar : SecureJar.from(path.paths())
static LIBMATTI_JH_SecureJar *path_or_jar_build(const PathOrJar *pathOrJar)
{
    if (pathOrJar->jar != NULL) return pathOrJar->jar;

    return LIBMATTI_JH_SecureJar_FromPaths((const char **)pathOrJar->path->paths, pathOrJar->path->pathCount);
}

// Java: f -> classLoader (the defineModules class loader function)
static void *class_loader_for_module(const char *moduleName, void *userdata)
{
    (void)moduleName;
    return userdata;
}

// Java: (cf, p) -> new ModuleClassLoader("LAYER " + layer.name(), cf, p)
static LIBMATTI_CL_ModuleClassLoader *default_class_loader_supplier(LIBMATTI_JL_Configuration *configuration,
                                                                    LIBMATTI_JL_ModuleLayer **parents,
                                                                    size_t parentCount, void *userdata)
{
    (void)parents;
    (void)parentCount;

    LIBMATTI_MLA_Layer layer = (LIBMATTI_MLA_Layer)(size_t)userdata;

    char name[64];
    snprintf(name, sizeof(name), "LAYER %s", LIBMATTI_MLA_Layer_Name(layer));

    return LIBMATTI_CL_ModuleClassLoader_New(name, configuration);
}

void LIBMATTI_ML_ModuleLayerHandler_SetBootClassLoader(LIBMATTI_CL_ModuleClassLoader *classLoader)
{
    bootClassLoader = classLoader;
}

// Java: ModuleLayerHandler()
LIBMATTI_ML_ModuleLayerHandler *LIBMATTI_ML_ModuleLayerHandler_New(void)
{
    LIBMATTI_ML_ModuleLayerHandler *handler = calloc(1, sizeof(LIBMATTI_ML_ModuleLayerHandler));
    if (handler == NULL) return NULL;

    // Java: ClassLoader classLoader = getClass().getClassLoader();
    //       ModuleClassLoader cl = classLoader instanceof ModuleClassLoader moduleCl ? moduleCl
    //           : new ModuleClassLoader("BOOT", ModuleLayer.boot().configuration(), List.of());
    LIBMATTI_CL_ModuleClassLoader *classLoader = bootClassLoader;
    if (classLoader == NULL)
    {
        classLoader = LIBMATTI_CL_ModuleClassLoader_New("BOOT",
                                                        LIBMATTI_JL_ModuleLayer_Configuration(
                                                            LIBMATTI_JL_ModuleLayer_Boot()));
        handler->bootClassLoaderOwned = 1;
    }

    // Java: completedLayers.put(Layer.BOOT, new LayerInfo(getClass().getModule().getLayer(), cl))
    handler->completedLayers[LIBMATTI_MLA_LAYER_BOOT].layer = LIBMATTI_JL_ModuleLayer_Boot();
    handler->completedLayers[LIBMATTI_MLA_LAYER_BOOT].cl = classLoader;
    handler->completedFlags[LIBMATTI_MLA_LAYER_BOOT] = 1;

    return handler;
}

void LIBMATTI_ML_ModuleLayerHandler_Free(LIBMATTI_ML_ModuleLayerHandler *handler)
{
    if (handler == NULL) return;

    for (size_t layer = 0; layer < 4; layer++)
    {
        for (size_t i = 0; i < handler->layers[layer].count; i++)
        {
            // Java: PathOrJar owns the NamedPath/Jar it was created from
            LIBMATTI_MLA_NamedPath *path = handler->layers[layer].items[i].path;
            if (path != NULL)
            {
                free(path->name);
                for (size_t p = 0; p < path->pathCount; p++) free(path->paths[p]);
                free(path->paths);
                free(path);
            }

            if (handler->layers[layer].items[i].jar != NULL)
                LIBMATTI_JH_SecureJar_Close(handler->layers[layer].items[i].jar);
        }
        free(handler->layers[layer].items);
    }

    // the BOOT layer is the JVM's boot layer and is not released
    for (size_t layer = LIBMATTI_MLA_LAYER_BOOT; layer < 4; layer++)
    {
        if (!handler->completedFlags[layer]) continue;

        if (layer != LIBMATTI_MLA_LAYER_BOOT)
        {
            LIBMATTI_JL_Configuration *configuration =
                LIBMATTI_JL_ModuleLayer_Configuration(handler->completedLayers[layer].layer);
            LIBMATTI_JL_ModuleLayerController_Free(handler->layerControllers[layer]);
            LIBMATTI_JL_ModuleLayer_Free(handler->completedLayers[layer].layer);
            LIBMATTI_JL_Configuration_Free(configuration);
        }

        if (layer != LIBMATTI_MLA_LAYER_BOOT || handler->bootClassLoaderOwned)
            LIBMATTI_CL_ModuleClassLoader_Free(handler->completedLayers[layer].cl);
    }

    free(handler);
}

// Java: void addToLayer(Layer layer, SecureJar jar)
void LIBMATTI_ML_ModuleLayerHandler_AddToLayer(LIBMATTI_ML_ModuleLayerHandler *handler, LIBMATTI_MLA_Layer layer,
                                               LIBMATTI_JH_SecureJar *jar)
{
    // Java: if (completedLayers.containsKey(layer)) throw new IllegalStateException("Layer already populated")
    if (handler->completedFlags[layer])
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                                 "Layer {} already populated", LIBMATTI_MLA_Layer_Name(layer));
        return;
    }

    // Java: layers.computeIfAbsent(layer, l -> new ArrayList<>()).add(PathOrJar.from(jar))
    PathOrJarList *list = &handler->layers[layer];
    list->items = realloc(list->items, sizeof(PathOrJar) * (list->count + 1));
    list->items[list->count].path = NULL;
    list->items[list->count].jar = jar;
    list->count++;
}

// Java: void addToLayer(Layer layer, NamedPath namedPath)
void LIBMATTI_ML_ModuleLayerHandler_AddNamedPathToLayer(LIBMATTI_ML_ModuleLayerHandler *handler,
                                                        LIBMATTI_MLA_Layer layer, LIBMATTI_MLA_NamedPath *namedPath)
{
    if (handler->completedFlags[layer])
    {
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                                 "Layer {} already populated", LIBMATTI_MLA_Layer_Name(layer));
        return;
    }

    PathOrJarList *list = &handler->layers[layer];
    list->items = realloc(list->items, sizeof(PathOrJar) * (list->count + 1));
    list->items[list->count].path = namedPath;
    list->items[list->count].jar = NULL;
    list->count++;
}

// Java: public LayerInfo buildLayer(Layer layer, BiFunction<Configuration, List<ModuleLayer>, ModuleClassLoader> classLoaderSupplier)
LIBMATTI_ML_ModuleLayerHandler_LayerInfo LIBMATTI_ML_ModuleLayerHandler_BuildLayerWith(
    LIBMATTI_ML_ModuleLayerHandler *handler, LIBMATTI_MLA_Layer layer,
    LIBMATTI_CL_ModuleClassLoader *(*classLoaderSupplier)(LIBMATTI_JL_Configuration *configuration,
                                                          LIBMATTI_JL_ModuleLayer **parents, size_t parentCount,
                                                          void *userdata),
    void *userdata)
{
    LIBMATTI_ML_ModuleLayerHandler_LayerInfo empty = {NULL, NULL};

    // Java: layers.getOrDefault(layer, List.of()).stream().map(PathOrJar::build).toArray(SecureJar[]::new)
    PathOrJarList *list = &handler->layers[layer];
    LIBMATTI_JH_SecureJar **jars = calloc(list->count > 0 ? list->count : 1, sizeof(*jars));
    for (size_t i = 0; i < list->count; i++) jars[i] = path_or_jar_build(&list->items[i]);

    // Java: targets = Arrays.stream(finder).map(SecureJar::name).toList()
    char **targets = calloc(list->count > 0 ? list->count : 1, sizeof(*targets));
    for (size_t i = 0; i < list->count; i++) targets[i] = strdup(LIBMATTI_JH_SecureJar_Name(jars[i]));

    // Java: Configuration.resolveAndBind(JarModuleFinder.of(finder), <parent configurations>,
    //                                     ModuleFinder.of(), targets)
    LIBMATTI_CL_JarModuleFinder *jarFinder = LIBMATTI_CL_JarModuleFinder_Of(jars, list->count);
    size_t referenceCount = 0;
    LIBMATTI_CL_JarModuleReference **references = LIBMATTI_CL_JarModuleFinder_FindAll(jarFinder, &referenceCount);

    LIBMATTI_JL_Configuration *newConf = LIBMATTI_JL_Configuration_ResolveAndBind(
        (LIBMATTI_JL_ModuleReference **)references, referenceCount, (const char **)targets, list->count);

    if (newConf == NULL)
    {
        // Java: catch (ResolutionException e) { LOGGER.error("Error while resolving modules.", e); throw e; }
        LIBMATTI_ML_Logger_Error(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                                 "Error while resolving modules.");
        return empty;
    }

    // Java: allParents = layer.getParent().map(completedLayers::get).map(LayerInfo::layer)
    //                       .mapMulti((moduleLayer, comp) -> { comp.accept(moduleLayer); moduleLayer.parents().forEach(comp); })
    size_t parentLayerCount = 0;
    const LIBMATTI_MLA_Layer *parents = LIBMATTI_MLA_Layer_GetParent(layer, &parentLayerCount);

    LIBMATTI_JL_ModuleLayer **allParents = NULL;
    size_t allParentCount = 0;

    for (size_t i = 0; i < parentLayerCount; i++)
    {
        LIBMATTI_MLA_Layer parent = parents[i];
        if (!handler->completedFlags[parent]) continue;

        LIBMATTI_JL_ModuleLayer *parentLayer = handler->completedLayers[parent].layer;
        if (parentLayer == NULL) continue;

        allParents = realloc(allParents, sizeof(*allParents) * (allParentCount + 1));
        allParents[allParentCount++] = parentLayer;

        size_t grandParentCount = 0;
        LIBMATTI_JL_ModuleLayer **grandParents = LIBMATTI_JL_ModuleLayer_Parents(parentLayer, &grandParentCount);
        for (size_t g = 0; g < grandParentCount; g++)
        {
            allParents = realloc(allParents, sizeof(*allParents) * (allParentCount + 1));
            allParents[allParentCount++] = grandParents[g];
        }
    }

    // Java: classLoader = classLoaderSupplier.apply(newConf, allParents)
    LIBMATTI_CL_ModuleClassLoader *classLoader =
        classLoaderSupplier(newConf, allParents, allParentCount, userdata);

    // Java: ModuleLayer.defineModules(newConf, parents, f -> classLoader)
    LIBMATTI_JL_ModuleLayerController *controller = LIBMATTI_JL_ModuleLayer_DefineModules(
        newConf, allParents, allParentCount, class_loader_for_module, classLoader);

    // Java: completedLayers.put(layer, new LayerInfo(modController.layer(), classLoader))
    handler->completedLayers[layer].layer = LIBMATTI_JL_ModuleLayerController_Layer(controller);
    handler->completedLayers[layer].cl = classLoader;
    handler->layerControllers[layer] = controller;
    handler->completedFlags[layer] = 1;

    // Java: classLoader.setFallbackClassLoader(completedLayers.get(Layer.BOOT).cl())
    LIBMATTI_CL_ModuleClassLoader_SetFallbackClassLoader(
        classLoader, (LIBMATTI_CL_ClassLoader *)handler->completedLayers[LIBMATTI_MLA_LAYER_BOOT].cl);

    free(allParents);
    free(references);
    LIBMATTI_CL_JarModuleFinder_Free(jarFinder);
    for (size_t i = 0; i < list->count; i++) free(targets[i]);
    free(targets);
    free(jars);

    return handler->completedLayers[layer];
}

// Java: public LayerInfo buildLayer(Layer layer)
LIBMATTI_ML_ModuleLayerHandler_LayerInfo LIBMATTI_ML_ModuleLayerHandler_BuildLayer(
    LIBMATTI_ML_ModuleLayerHandler *handler, LIBMATTI_MLA_Layer layer)
{
    // Java: (cf, p) -> new ModuleClassLoader("LAYER " + layer.name(), cf, p)
    return LIBMATTI_ML_ModuleLayerHandler_BuildLayerWith(handler, layer, default_class_loader_supplier,
                                                         (void *)(size_t)layer);
}

// Java: public Optional<ModuleLayer> getLayer(Layer layer) - NULL means empty
LIBMATTI_JL_ModuleLayer *LIBMATTI_ML_ModuleLayerHandler_GetLayer(LIBMATTI_ML_ModuleLayerHandler *handler,
                                                                 LIBMATTI_MLA_Layer layer)
{
    if (!handler->completedFlags[layer]) return NULL;
    return handler->completedLayers[layer].layer;
}

// Java: public void updateLayer(Layer layer, Consumer<LayerInfo> action)
void LIBMATTI_ML_ModuleLayerHandler_UpdateLayer(LIBMATTI_ML_ModuleLayerHandler *handler, LIBMATTI_MLA_Layer layer,
                                                void (*action)(LIBMATTI_ML_ModuleLayerHandler_LayerInfo *layerInfo,
                                                               void *userdata),
                                                void *userdata)
{
    // Java: action.accept(completedLayers.get(layer))
    if (action != NULL) action(&handler->completedLayers[layer], userdata);
}

// Java: public Optional<ModuleLayer> getLayer(Layer layer) - the IModuleLayerManager contract
LIBMATTI_JL_ModuleLayer *LIBMATTI_MLA_IModuleLayerManager_GetLayer(LIBMATTI_MLA_IModuleLayerManager *manager,
                                                                   LIBMATTI_MLA_Layer layer)
{
    return LIBMATTI_ML_ModuleLayerHandler_GetLayer((LIBMATTI_ML_ModuleLayerHandler *)manager, layer);
}
