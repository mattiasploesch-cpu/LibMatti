// Port of com.mojang.blaze3d.opengl.GlProgram - the linked shader program with
// its Uniform map (Sampler/Ubo/Utb), built by GlProgram.link.

#ifndef MATTICRAFT_BLAZE3D_OPENGL_GLPROGRAM_H
#define MATTICRAFT_BLAZE3D_OPENGL_GLPROGRAM_H

#include "libmatti/com/mojang/blaze3d/opengl/GlShaderModule.h"
#include "libmatti/com/mojang/blaze3d/opengl/Uniform.h"
#include "libmatti/com/mojang/blaze3d/vertex/VertexFormat.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public static Set<String> BUILT_IN_UNIFORMS = {"Projection", "Lighting", "Fog", "Globals"}
extern const char *const LIBMATTI_B3D_GlProgram_BUILT_IN_UNIFORMS[5];

// Java: public class GlProgram implements AutoCloseable
typedef struct LIBMATTI_B3D_GlProgramUniformEntry
{
    char *name;
    LIBMATTI_B3D_Uniform *uniform;
    struct LIBMATTI_B3D_GlProgramUniformEntry *next;
} LIBMATTI_B3D_GlProgramUniformEntry;

typedef struct LIBMATTI_B3D_GlProgram
{
    int programId;
    char *debugLabel;
    LIBMATTI_B3D_GlProgramUniformEntry *uniforms;
    int nextUboBinding;   // Java: int i (ubo binding counter)
    int nextSamplerIndex; // Java: int j (sampler counter)
} LIBMATTI_B3D_GlProgram;

// Java: public static GlProgram link(GlShaderModule, GlShaderModule, VertexFormat, String)
// NULL on a link failure (Java throws ShaderManager.CompilationException).
LIBMATTI_B3D_GlProgram *LIBMATTI_B3D_GlProgram_Link(const LIBMATTI_B3D_GlShaderModule *vertexShader,
                                                    const LIBMATTI_B3D_GlShaderModule *fragmentShader,
                                                    const LIBMATTI_B3D_VertexFormat *format,
                                                    const char *debugLabel);
// Java: the invalid singleton carries program id -1.
LIBMATTI_B3D_GlProgram *LIBMATTI_B3D_GlProgram_Invalid(void);
int LIBMATTI_B3D_GlProgram_IsInvalid(const LIBMATTI_B3D_GlProgram *program);
void LIBMATTI_B3D_GlProgram_Free(LIBMATTI_B3D_GlProgram *program);

// Java: public void setupUniforms(List<UniformDescription>, List<String>) -
// the port takes parallel arrays of uniform descriptions (name + kind + format)
// and a NULL-terminated sampler name list.
void LIBMATTI_B3D_GlProgram_SetupUniforms(
    LIBMATTI_B3D_GlProgram *program, const char *const *uniformNames,
    const LIBMATTI_B3D_UniformKind *uniformKinds, const int *uniformFormats, size_t uniformCount,
    const char *const *samplerNames);

// Java: public @Nullable Uniform getUniform(String)
LIBMATTI_B3D_Uniform *LIBMATTI_B3D_GlProgram_GetUniform(const LIBMATTI_B3D_GlProgram *program, const char *name);
// Java: public int programId()
int LIBMATTI_B3D_GlProgram_GetProgramId(const LIBMATTI_B3D_GlProgram *program);

#ifdef __cplusplus
}
#endif

#endif
