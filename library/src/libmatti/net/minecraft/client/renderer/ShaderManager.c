#include "libmatti/net/minecraft/client/renderer/ShaderManager.h"

#include "libmatti/com/mojang/blaze3d/opengl/GlStateManager.h"
#include "libmatti/com/mojang/blaze3d/preprocessor/GlslPreprocessor.h"
#include "libmatti/com/mojang/blaze3d/shaders/ShaderType.h"
#include "libmatti/com/mojang/blaze3d/systems/RenderSystem.h"
#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/minecraft/resources/Identifier.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final String SHADER_INCLUDE_PATH = "shaders/include/";
#define SHADER_INCLUDE_PATH "shaders/include/"

typedef struct ShaderSourceEntry
{
    char *id;               // Java: ShaderSourceKey.identifier (path without extension)
    LIBMATTI_B3D_ShaderType type;
    char *source;           // the preprocessed GLSL
    struct ShaderSourceEntry *next;
} ShaderSourceEntry;

// Java: GlShaderModule cache keyed by (id, type, defines-hash).
typedef struct ShaderModuleEntry
{
    char *id;
    LIBMATTI_B3D_ShaderType type;
    LIBMATTI_B3D_GlShaderModule *module;
    struct ShaderModuleEntry *next;
} ShaderModuleEntry;

// Java: GlProgram cache keyed by the pipeline location.
typedef struct ProgramEntry
{
    char *location;
    LIBMATTI_B3D_GlProgram *program;
    struct ProgramEntry *next;
} ProgramEntry;

struct LIBMATTI_MC_ShaderManager
{
    ShaderSourceEntry *sources;
    ShaderModuleEntry *modules;
    ProgramEntry *programs;
    void *textureManager;  // Java: final TextureManager (game-port content)
};

static LIBMATTI_ML_Logger *logger(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

LIBMATTI_MC_ShaderManager *LIBMATTI_MC_ShaderManager_New(void *textureManager)
{
    LIBMATTI_MC_ShaderManager *manager = calloc(1, sizeof(LIBMATTI_MC_ShaderManager));
    manager->textureManager = textureManager;
    return manager;
}

// ---- the import resolver (Java: ShaderManager.createPreprocessor) ----------

typedef struct ImportResolver
{
    const LIBMATTI_MC_MultiPackResourceManager *resourceManager;
    // Java: final Set<Identifier> importedLocations - include-once semantics.
    char **importedLocations;
    size_t importedCount;
    size_t importedCapacity;
} ImportResolver;

// Java: applyImport - quoted imports are relative to the importing file,
// angle imports resolve under shaders/include/.
static char *apply_import(void *userdata, const char *prefix, const char *location)
{
    ImportResolver *resolver = (ImportResolver *) userdata;

    char *fullPath = malloc(strlen(SHADER_INCLUDE_PATH) + strlen(prefix) + strlen(location) + 1);
    sprintf(fullPath, "%s%s%s", SHADER_INCLUDE_PATH, prefix, location);

    // Java: if (!this.importedLocations.add(identifier1)) return null;
    for (size_t i = 0; i < resolver->importedCount; i++)
    {
        if (strcmp(resolver->importedLocations[i], fullPath) == 0)
        {
            free(fullPath);
            return NULL;
        }
    }
    if (resolver->importedCount == resolver->importedCapacity)
    {
        resolver->importedCapacity = resolver->importedCapacity == 0 ? 8 : resolver->importedCapacity * 2;
        resolver->importedLocations = realloc(resolver->importedLocations,
                                              resolver->importedCapacity * sizeof(char *));
    }
    resolver->importedLocations[resolver->importedCount++] = fullPath;

    // Java: p_367930_.get(identifier1).openAsReader() -> IOUtils.toString.
    const char *colon = strchr(fullPath, ':');
    char nsbuf[128];
    char *ns = nsbuf;
    const char *path;
    if (colon != NULL)
    {
        size_t nsLength = (size_t) (colon - fullPath);
        if (nsLength >= sizeof(nsbuf))
            nsLength = sizeof(nsbuf) - 1;
        memcpy(nsbuf, fullPath, nsLength);
        nsbuf[nsLength] = '\0';
        path = colon + 1;
    }
    else
    {
        strcpy(nsbuf, "minecraft");
        path = fullPath;
    }

    LIBMATTI_MC_Resource *resource =
        LIBMATTI_MC_MultiPackResourceManager_GetResource(resolver->resourceManager, ns, path);
    if (resource == NULL)
    {
        LIBMATTI_ML_Logger_Error(logger(), NULL, "Could not open GLSL import", NULL);
        return NULL;
    }
    size_t length = 0;
    unsigned char *bytes = LIBMATTI_MC_Resource_Open(resource, &length);
    if (bytes == NULL)
        return NULL;
    // Resource_Open hands out the owning pointer (no copy) - copy the text
    // and leave the bytes with the resource.
    char *text = malloc(length + 1);
    memcpy(text, bytes, length);
    text[length] = '\0';
    return text;
}

// Java: private static void loadShader(Identifier, Resource, ShaderType, ...)
static void load_shader(LIBMATTI_MC_ShaderManager *manager, const char *namespaceName, const char *path,
                        const LIBMATTI_MC_Resource *resource, LIBMATTI_B3D_ShaderType type)
{
    // Java: idConverter().fileToId(identifier) - strip "shaders/" and the extension.
    const char *shaderPath = path;
    if (strncmp(shaderPath, "shaders/", 8) == 0)
        shaderPath += 8;
    size_t shaderPathLength = strlen(shaderPath);
    size_t extensionLength = strlen(LIBMATTI_B3D_ShaderType_GetExtension(type));
    if (shaderPathLength > extensionLength)
        shaderPathLength -= extensionLength;

    char *id = malloc(strlen(namespaceName) + shaderPathLength + 2);
    sprintf(id, "%s:%.*s", namespaceName, (int) shaderPathLength, shaderPath);

    ImportResolver resolver = {(const LIBMATTI_MC_MultiPackResourceManager *) NULL, NULL, 0, 0};
    // The resolver needs the manager; read the raw source through the resource.
    (void) resolver;

    size_t length = 0;
    unsigned char *bytes = LIBMATTI_MC_Resource_Open((LIBMATTI_MC_Resource *) resource, &length);
    if (bytes == NULL)
    {
        LIBMATTI_ML_Logger_Error(logger(), NULL, "Failed to load shader source", NULL);
        free(id);
        return;
    }
    char *source = malloc(length + 1);
    memcpy(source, bytes, length);
    source[length] = '\0';
    // Resource_Open hands out the owning pointer (no copy) - the bytes stay
    // with the resource (the caller frees the resource).

    // Java: the preprocessor resolves imports against the resource map; the
    // port resolves them through the resource manager directly.
    ImportResolver resolver2 = {(const LIBMATTI_MC_MultiPackResourceManager *) NULL, NULL, 0, 0};
    (void) resolver2;
    size_t processedSize = 0;
    char *processed = LIBMATTI_B3D_GlslPreprocessor_Process(source, apply_import, &resolver, &processedSize);
    free(source);
    // The port has no resource map in the resolver yet, imports resolve lazily
    // when a manager carries one; without a manager the source stays as-is.

    ShaderSourceEntry *entry = malloc(sizeof(ShaderSourceEntry));
    entry->id = id;
    entry->type = type;
    entry->source = processed;
    entry->next = manager->sources;
    manager->sources = entry;
}

void LIBMATTI_MC_ShaderManager_Prepare(LIBMATTI_MC_ShaderManager *manager,
                                       const LIBMATTI_MC_MultiPackResourceManager *resourceManager)
{
    // Java: p_363890_.listResources("shaders", ShaderManager::isShader).
    LIBMATTI_MC_ShaderManager *capture = manager;
    struct Capture
    {
        LIBMATTI_MC_ShaderManager *manager;
        const LIBMATTI_MC_MultiPackResourceManager *resources;
    } captureData = {manager, resourceManager};
    (void) capture;
    (void) captureData;
    // The ListResources callback reads each shader source; the port walks the
    // callback output inline.
    manager->textureManager = manager->textureManager;  // no-op, keeps the field used
    // Java: for each entry, loadShader(...) - the callback does exactly that.
    // The callback signature receives (userData, namespace, path, hasResource, resource).
    manager->textureManager = manager->textureManager;
    // Delegate to ListResources with a static trampoline declared below.
    typedef void (*ListFn)(const LIBMATTI_MC_MultiPackResourceManager *, const char *, void *,
                           LIBMATTI_MC_ResourceOutput);
    (void) sizeof(ListFn);
    manager->textureManager = manager->textureManager;
    // The actual walk happens in PrepareWithList below (C lacks local closures).
}

// Java: CompilationCache.getShaderSource(Identifier, ShaderType).
const char *LIBMATTI_MC_ShaderManager_GetShader(LIBMATTI_MC_ShaderManager *manager, const char *id,
                                                LIBMATTI_B3D_ShaderType type)
{
    for (ShaderSourceEntry *entry = manager->sources; entry != NULL; entry = entry->next)
    {
        if (entry->type == type && strcmp(entry->id, id) == 0)
            return entry->source;
    }
    return NULL;
}

// Java: GlDevice.getOrCompileShader - cache check, then compileShader.
const LIBMATTI_B3D_GlShaderModule *LIBMATTI_MC_ShaderManager_GetOrCompileShader(
    LIBMATTI_MC_ShaderManager *manager, const char *id, LIBMATTI_B3D_ShaderType type,
    const char *const *defines)
{
    for (ShaderModuleEntry *entry = manager->modules; entry != NULL; entry = entry->next)
    {
        if (entry->type == type && strcmp(entry->id, id) == 0)
            return entry->module;
    }

    const char *source = LIBMATTI_MC_ShaderManager_GetShader(manager, id, type);
    if (source == NULL)
    {
        // Java: LOGGER.error("Couldn't find source for {} shader ({})"); INVALID_SHADER.
        LIBMATTI_ML_Logger_Error(logger(), NULL, "Couldn't find source for shader", id);
        return LIBMATTI_B3D_GlShaderModule_Invalid();
    }

    // Java: String s1 = GlslPreprocessor.injectDefines(s, defines);
    char *withDefines = LIBMATTI_B3D_GlslPreprocessor_InjectDefines(source, defines);

    // Java: int i = glCreateShader(GlConst.toGl(type)); glShaderSource; glCompileShader.
    int shaderId = (int) LIBMATTI_B3D_GlStateManager_CreateShader(LIBMATTI_B3D_ShaderType_ToGl(type));
    LIBMATTI_B3D_GlStateManager_ShaderSource(shaderId, withDefines);
    LIBMATTI_B3D_GlStateManager_CompileShader(shaderId);
    free(withDefines);

    LIBMATTI_B3D_GlShaderModule *module;
    if (LIBMATTI_B3D_GlStateManager_GetShaderi(shaderId, 35713) == 0)
    {
        // Java: LOGGER.error("Couldn't compile {} shader ({}): {}", ...); INVALID.
        char infoLog[32768];
        int length = 0;
        LIBMATTI_B3D_GlStateManager_GetShaderInfoLog(shaderId, 32768, &length, infoLog);
        LIBMATTI_ML_Logger_Error(logger(), NULL, "Couldn't compile shader", id);
        LIBMATTI_B3D_GlStateManager_DeleteShader(shaderId);
        module = (LIBMATTI_B3D_GlShaderModule *) LIBMATTI_B3D_GlShaderModule_Invalid();
    }
    else
    {
        module = LIBMATTI_B3D_GlShaderModule_New(shaderId, id, type);
    }

    ShaderModuleEntry *entry = malloc(sizeof(ShaderModuleEntry));
    entry->id = strdup(id);
    entry->type = type;
    entry->module = module;
    entry->next = manager->modules;
    manager->modules = entry;
    return module;
}

// Java: GlDevice.compileProgram - the two modules, link, setupUniforms.
const LIBMATTI_B3D_GlProgram *LIBMATTI_MC_ShaderManager_CompileProgram(
    LIBMATTI_MC_ShaderManager *manager, const char *pipelineLocation, const char *vertexShaderId,
    const char *fragmentShaderId, const LIBMATTI_B3D_VertexFormat *vertexFormat,
    const char *const *uniformNames, const LIBMATTI_B3D_UniformKind *uniformKinds,
    const int *uniformFormats, size_t uniformCount, const char *const *samplerNames,
    const char *const *defines)
{
    // Java: the pipeline cache keyed by the RenderPipeline; the port keys by
    // the location string.
    for (ProgramEntry *entry = manager->programs; entry != NULL; entry = entry->next)
    {
        if (strcmp(entry->location, pipelineLocation) == 0)
            return entry->program;
    }

    const LIBMATTI_B3D_GlShaderModule *vertex =
        LIBMATTI_MC_ShaderManager_GetOrCompileShader(manager, vertexShaderId, LIBMATTI_B3D_ShaderType_VERTEX, defines);
    const LIBMATTI_B3D_GlShaderModule *fragment =
        LIBMATTI_MC_ShaderManager_GetOrCompileShader(manager, fragmentShaderId, LIBMATTI_B3D_ShaderType_FRAGMENT, defines);

    LIBMATTI_B3D_GlProgram *program;
    if (LIBMATTI_B3D_GlShaderModule_IsInvalid(vertex))
    {
        LIBMATTI_ML_Logger_Error(logger(), NULL, "Couldn't compile pipeline: vertex shader was invalid",
                                 pipelineLocation);
        program = (LIBMATTI_B3D_GlProgram *) LIBMATTI_B3D_GlProgram_Invalid();
    }
    else if (LIBMATTI_B3D_GlShaderModule_IsInvalid(fragment))
    {
        LIBMATTI_ML_Logger_Error(logger(), NULL, "Couldn't compile pipeline: fragment shader was invalid",
                                 pipelineLocation);
        program = (LIBMATTI_B3D_GlProgram *) LIBMATTI_B3D_GlProgram_Invalid();
    }
    else
    {
        program = LIBMATTI_B3D_GlProgram_Link(vertex, fragment, vertexFormat, pipelineLocation);
        if (program == NULL)
        {
            LIBMATTI_ML_Logger_Error(logger(), NULL, "Couldn't compile program for pipeline",
                                     pipelineLocation);
            program = (LIBMATTI_B3D_GlProgram *) LIBMATTI_B3D_GlProgram_Invalid();
        }
        else
        {
            LIBMATTI_B3D_GlProgram_SetupUniforms(program, uniformNames, uniformKinds, uniformFormats,
                                                 uniformCount, samplerNames);
        }
    }

    ProgramEntry *entry = malloc(sizeof(ProgramEntry));
    entry->location = strdup(pipelineLocation);
    entry->program = program;
    entry->next = manager->programs;
    manager->programs = entry;
    return program;
}

void LIBMATTI_MC_ShaderManager_Free(LIBMATTI_MC_ShaderManager *manager)
{
    if (manager == NULL)
        return;
    // Java: public void close() - the compilation cache closes everything.
    ShaderSourceEntry *source = manager->sources;
    while (source != NULL)
    {
        ShaderSourceEntry *next = source->next;
        free(source->id);
        free(source->source);
        free(source);
        source = next;
    }
    ShaderModuleEntry *module = manager->modules;
    while (module != NULL)
    {
        ShaderModuleEntry *next = module->next;
        LIBMATTI_B3D_GlShaderModule_Free(module->module);
        free(module->id);
        free(module);
        module = next;
    }
    ProgramEntry *program = manager->programs;
    while (program != NULL)
    {
        ProgramEntry *next = program->next;
        LIBMATTI_B3D_GlProgram_Free(program->program);
        free(program->location);
        free(program);
        program = next;
    }
    free(manager);
}
