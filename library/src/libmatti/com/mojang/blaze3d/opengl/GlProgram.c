#include "libmatti/com/mojang/blaze3d/opengl/GlProgram.h"

#include "libmatti/com/mojang/blaze3d/opengl/GlStateManager.h"
#include "libmatti/com/mojang/blaze3d/systems/RenderSystem.h"
#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/org/lwjgl/opengl/GL.h"

#include <stdlib.h>
#include <string.h>

// Java: public static Set<String> BUILT_IN_UNIFORMS
const char *const LIBMATTI_B3D_GlProgram_BUILT_IN_UNIFORMS[5] = {"Projection", "Lighting", "Fog", "Globals", NULL};

// Java: GlProgram.INVALID_PROGRAM = new GlProgram(-1, "invalid")
static LIBMATTI_B3D_GlProgram invalidProgram = {-1, NULL, NULL, 0, 0};

static LIBMATTI_ML_Logger *logger(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

LIBMATTI_B3D_GlProgram *LIBMATTI_B3D_GlProgram_Invalid(void)
{
    return &invalidProgram;
}

int LIBMATTI_B3D_GlProgram_IsInvalid(const LIBMATTI_B3D_GlProgram *program)
{
    return program == &invalidProgram || program->programId <= 0;
}

// Java: public static GlProgram link(GlShaderModule vertex, GlShaderModule fragment,
//        VertexFormat, String debugLabel) throws CompilationException
LIBMATTI_B3D_GlProgram *LIBMATTI_B3D_GlProgram_Link(const LIBMATTI_B3D_GlShaderModule *vertexShader,
                                                    const LIBMATTI_B3D_GlShaderModule *fragmentShader,
                                                    const LIBMATTI_B3D_VertexFormat *format,
                                                    const char *debugLabel)
{
    // Java: int i = GlStateManager.glCreateProgram();
    int programId = (int) LIBMATTI_B3D_GlStateManager_CreateProgram();
    if (programId <= 0)
        return NULL;

    // Java: for every attribute name, _glBindAttribLocation(i, j, s).
    int attributeIndex = 0;
    for (int i = 0; i < LIBMATTI_B3D_VertexFormat_GetElementCount(format); i++)
    {
        const LIBMATTI_B3D_VertexFormatElement *element = LIBMATTI_B3D_VertexFormat_GetElement(format, i);
        const char *name = LIBMATTI_B3D_VertexFormat_GetElementName(format, element);
        if (name != NULL)
            LIBMATTI_B3D_GlStateManager_BindAttribLocation((unsigned int) programId, (unsigned int) attributeIndex, name);
        attributeIndex++;
    }

    LIBMATTI_B3D_GlStateManager_AttachShader(programId, LIBMATTI_B3D_GlShaderModule_GetShaderId(vertexShader));
    LIBMATTI_B3D_GlStateManager_AttachShader(programId, LIBMATTI_B3D_GlShaderModule_GetShaderId(fragmentShader));
    LIBMATTI_B3D_GlStateManager_LinkProgram(programId);

    // Java: int k = glGetProgrami(i, 35714) - GL_LINK_STATUS.
    int linkStatus = LIBMATTI_B3D_GlStateManager_GetProgrami(programId, 35714);
    char infoLog[32768];
    int logLength = 0;
    infoLog[0] = '\0';
    LIBMATTI_B3D_GlStateManager_GetProgramInfoLog(programId, 32768, &logLength, infoLog);
    if (linkStatus != 0 && strstr(infoLog, "Failed for unknown reason") == NULL)
    {
        if (infoLog[0] != '\0')
            LIBMATTI_ML_Logger_Info(logger(), NULL, "Info log when linking program. Log output:", infoLog);
        LIBMATTI_B3D_GlProgram *program = calloc(1, sizeof(LIBMATTI_B3D_GlProgram));
        program->programId = programId;
        program->debugLabel = strdup(debugLabel);
        return program;
    }

    LIBMATTI_B3D_GlStateManager_DeleteProgram((unsigned int) programId);
    return NULL;
}

// Java: setupUniforms - UBO/UTB descriptions, then samplers, then built-ins.
void LIBMATTI_B3D_GlProgram_SetupUniforms(
    LIBMATTI_B3D_GlProgram *program, const char *const *uniformNames,
    const LIBMATTI_B3D_UniformKind *uniformKinds, const int *uniformFormats, size_t uniformCount,
    const char *const *samplerNames)
{
    for (size_t i = 0; i < uniformCount; i++)
    {
        const char *name = uniformNames[i];
        LIBMATTI_B3D_Uniform *uniform = NULL;
        if (uniformKinds[i] == LIBMATTI_B3D_UniformKind_UBO)
        {
            // Java: GL31.glGetUniformBlockIndex + glUniformBlockBinding.
            int blockIndex = LIBMATTI_GL_glGetUniformBlockIndex((unsigned int) program->programId, name);
            if (blockIndex != -1)
            {
                int binding = program->nextUboBinding++;
                LIBMATTI_GL_glUniformBlockBinding((unsigned int) program->programId, (unsigned int) blockIndex,
                                                  (unsigned int) binding);
                uniform = LIBMATTI_B3D_Uniform_Ubo(binding);
            }
        }
        else // UNIFORM_BUFFER's sibling TEXEL_BUFFER.
        {
            int location = LIBMATTI_B3D_GlStateManager_GetUniformLocation(program->programId, name);
            if (location == -1)
            {
                LIBMATTI_ML_Logger_Warn(logger(), NULL,
                                        "Shader program does not use utb defined in the pipeline. This might be a bug.");
            }
            else
            {
                uniform = LIBMATTI_B3D_Uniform_Utb(location, program->nextSamplerIndex++, uniformFormats[i]);
            }
        }
        if (uniform != NULL)
        {
            LIBMATTI_B3D_GlProgramUniformEntry *entry = malloc(sizeof(LIBMATTI_B3D_GlProgramUniformEntry));
            entry->name = strdup(name);
            entry->uniform = uniform;
            entry->next = program->uniforms;
            program->uniforms = entry;
        }
    }

    for (const char *const *sampler = samplerNames; sampler != NULL && *sampler != NULL; sampler++)
    {
        int location = LIBMATTI_B3D_GlStateManager_GetUniformLocation(program->programId, *sampler);
        if (location == -1)
        {
            LIBMATTI_ML_Logger_Warn(logger(), NULL,
                                    "Shader program does not use sampler defined in the pipeline. This might be a bug.");
        }
        else
        {
            LIBMATTI_B3D_GlProgramUniformEntry *entry = malloc(sizeof(LIBMATTI_B3D_GlProgramUniformEntry));
            entry->name = strdup(*sampler);
            entry->uniform = LIBMATTI_B3D_Uniform_Sampler(location, program->nextSamplerIndex++);
            entry->next = program->uniforms;
            program->uniforms = entry;
        }
    }

    // Java: walk the active uniform blocks and bind the built-in ones.
    int blockCount = LIBMATTI_B3D_GlStateManager_GetProgrami(program->programId, 35382);
    for (int i = 0; i < blockCount; i++)
    {
        char blockName[256];
        unsigned int index = (unsigned int) i;
        (void) index;
        // Java: GL31.glGetActiveUniformBlockName; the port keeps the name in the
        // uniformsByName check below through the entry list scan.
        LIBMATTI_GL_glGetActiveUniformBlockName((unsigned int) program->programId, index, blockName);
        if (LIBMATTI_B3D_GlProgram_GetUniform(program, blockName) != NULL)
            continue;

        int isSampler = 0;
        for (const char *const *sampler = samplerNames; sampler != NULL && *sampler != NULL; sampler++)
        {
            if (strcmp(*sampler, blockName) == 0)
            {
                isSampler = 1;
                break;
            }
        }
        int isBuiltIn = 0;
        for (const char *const *builtin = LIBMATTI_B3D_GlProgram_BUILT_IN_UNIFORMS;
             *builtin != NULL; builtin++)
        {
            if (strcmp(*builtin, blockName) == 0)
            {
                isBuiltIn = 1;
                break;
            }
        }
        if (!isSampler && isBuiltIn)
        {
            int binding = program->nextUboBinding++;
            LIBMATTI_GL_glUniformBlockBinding((unsigned int) program->programId, index, (unsigned int) binding);
            LIBMATTI_B3D_GlProgramUniformEntry *entry = malloc(sizeof(LIBMATTI_B3D_GlProgramUniformEntry));
            entry->name = strdup(blockName);
            entry->uniform = LIBMATTI_B3D_Uniform_Ubo(binding);
            entry->next = program->uniforms;
            program->uniforms = entry;
        }
        else if (!isSampler)
        {
            LIBMATTI_ML_Logger_Warn(logger(), NULL, "Found unknown and unsupported uniform in program");
        }
    }
}

LIBMATTI_B3D_Uniform *LIBMATTI_B3D_GlProgram_GetUniform(const LIBMATTI_B3D_GlProgram *program, const char *name)
{
    // Java: RenderSystem.assertOnRenderThread() inside getUniform.
    LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
    for (LIBMATTI_B3D_GlProgramUniformEntry *entry = program->uniforms; entry != NULL; entry = entry->next)
    {
        if (strcmp(entry->name, name) == 0)
            return entry->uniform;
    }
    return NULL;
}

int LIBMATTI_B3D_GlProgram_GetProgramId(const LIBMATTI_B3D_GlProgram *program)
{
    return program->programId;
}

void LIBMATTI_B3D_GlProgram_Free(LIBMATTI_B3D_GlProgram *program)
{
    if (program == NULL || program == &invalidProgram)
        return;
    // Java: public void close() - uniforms close, then delete the program.
    LIBMATTI_B3D_GlProgramUniformEntry *entry = program->uniforms;
    while (entry != NULL)
    {
        LIBMATTI_B3D_GlProgramUniformEntry *next = entry->next;
        LIBMATTI_B3D_Uniform_Free(entry->uniform);
        free(entry->name);
        free(entry);
        entry = next;
    }
    LIBMATTI_B3D_GlStateManager_DeleteProgram((unsigned int) program->programId);
    free(program->debugLabel);
    free(program);
}
