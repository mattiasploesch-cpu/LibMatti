// Port of net.minecraft.client.renderer.ShaderManager - loads the GLSL sources
// through the ResourceManager, preprocesses them and compiles the shader
// modules the GlProgram links. Post chains stay game-port content (the port
// keeps the CompilationException channel).

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_SHADERMANAGER_H
#define MATTICRAFT_MC_CLIENT_RENDERER_SHADERMANAGER_H

#include "libmatti/com/mojang/blaze3d/opengl/GlProgram.h"
#include "libmatti/com/mojang/blaze3d/opengl/GlShaderModule.h"
#include "libmatti/com/mojang/blaze3d/shaders/ShaderType.h"
#include "libmatti/net/minecraft/server/packs/resources/MultiPackResourceManager.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public static final int MAX_LOG_LENGTH = 32768;
#define LIBMATTI_MC_ShaderManager_MAX_LOG_LENGTH 32768
// Java: public static final String SHADER_PATH = "shaders";
#define LIBMATTI_MC_ShaderManager_SHADER_PATH "shaders"

// Java: class ShaderManager extends SimplePreparableReloadListener<Configs>
typedef struct LIBMATTI_MC_ShaderManager LIBMATTI_MC_ShaderManager;

// Java: public ShaderManager(TextureManager, Consumer<Exception>)
LIBMATTI_MC_ShaderManager *LIBMATTI_MC_ShaderManager_New(void *textureManager);
void LIBMATTI_MC_ShaderManager_Free(LIBMATTI_MC_ShaderManager *manager);

// Java: protected Configs prepare(ResourceManager) - reads every .vsh/.fsh
// under "shaders" and runs the GLSL preprocessor with the include resolver.
void LIBMATTI_MC_ShaderManager_Prepare(LIBMATTI_MC_ShaderManager *manager,
                                       const LIBMATTI_MC_MultiPackResourceManager *resourceManager);

// Java: CompilationCache.getShaderSource(Identifier, ShaderType) - NULL when
// the source was not loaded.
const char *LIBMATTI_MC_ShaderManager_GetShader(LIBMATTI_MC_ShaderManager *manager, const char *id,
                                                LIBMATTI_B3D_ShaderType type);

// Java: GlDevice.compileShader path - compiles one cached module from the
// preprocessed source (defines injected after the first line). Returns the
// INVALID_SHADER module on failure.
const LIBMATTI_B3D_GlShaderModule *LIBMATTI_MC_ShaderManager_GetOrCompileShader(
    LIBMATTI_MC_ShaderManager *manager, const char *id, LIBMATTI_B3D_ShaderType type,
    const char *const *defines);

// Java: GlDevice.compileProgram path - compile/link + setupUniforms with the
// given uniform/sampler lists. Returns the INVALID_PROGRAM on failure.
const LIBMATTI_B3D_GlProgram *LIBMATTI_MC_ShaderManager_CompileProgram(
    LIBMATTI_MC_ShaderManager *manager, const char *pipelineLocation, const char *vertexShaderId,
    const char *fragmentShaderId, const LIBMATTI_B3D_VertexFormat *vertexFormat,
    const char *const *uniformNames, const LIBMATTI_B3D_UniformKind *uniformKinds,
    const int *uniformFormats, size_t uniformCount, const char *const *samplerNames,
    const char *const *defines);

#ifdef __cplusplus
}
#endif

#endif
