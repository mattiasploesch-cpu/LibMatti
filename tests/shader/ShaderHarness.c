// Shader harness: the GlslPreprocessor import/version passes over real GLSL
// fixtures, the ShaderManager source loading and the compile fallbacks (no GL
// context here - the harness checks the no-driver path like the GL harness).

#include "libmatti/com/mojang/blaze3d/opengl/GlProgram.h"
#include "libmatti/com/mojang/blaze3d/opengl/GlShaderModule.h"
#include "libmatti/com/mojang/blaze3d/preprocessor/GlslPreprocessor.h"
#include "libmatti/com/mojang/blaze3d/vertex/DefaultVertexFormat.h"
#include "libmatti/com/mojang/blaze3d/shaders/ShaderType.h"
#include "libmatti/net/minecraft/client/renderer/ShaderManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks = 0;
static int failures = 0;

#define CHECK(cond)                                            \
    do                                                         \
    {                                                          \
        checks++;                                              \
        if (!(cond))                                           \
        {                                                      \
            failures++;                                        \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        }                                                      \
    } while (0)

// A file-backed import resolver for the preprocessor checks. The harness
// runs with the tests/ directory as the working directory (see add_test).
static const char *SHADER_ROOT = "shader/";

static char *read_file(const char *relativePath)
{
    char path[512];
    snprintf(path, sizeof(path), "%s%s", SHADER_ROOT, relativePath);
    FILE *file = fopen(path, "rb");
    if (file == NULL)
        return NULL;
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *text = malloc((size_t) size + 1);
    if (fread(text, 1, (size_t) size, file) != (size_t) size)
    {
        fclose(file);
        free(text);
        return NULL;
    }
    text[size] = '\0';
    fclose(file);
    return text;
}

static char *file_import(void *userdata, const char *prefix, const char *location)
{
    (void) userdata;
    char path[512];
    snprintf(path, sizeof(path), "shaders/include/%s%s", prefix, location);
    return read_file(path);
}

int main(void)
{
    // ---- ShaderType ---------------------------------------------------------
    CHECK(LIBMATTI_B3D_ShaderType_ByPath("shaders/core/position.vsh") == LIBMATTI_B3D_ShaderType_VERTEX);
    CHECK(LIBMATTI_B3D_ShaderType_ByPath("shaders/core/position.fsh") == LIBMATTI_B3D_ShaderType_FRAGMENT);
    CHECK(LIBMATTI_B3D_ShaderType_ByPath("shaders/include/fog.glsl") == -1);
    CHECK(LIBMATTI_B3D_ShaderType_ToGl(LIBMATTI_B3D_ShaderType_VERTEX) == 35633);
    CHECK(strcmp(LIBMATTI_B3D_ShaderType_GetName(LIBMATTI_B3D_ShaderType_FRAGMENT), "fragment") == 0);

    // ---- GlslPreprocessor: imports resolve, directive is replaced -----------
    char *source = read_file("shaders/position.vsh");
    CHECK(source != NULL);
    if (source == NULL)
    {
        printf("shader harness: fixture not found, %d checks, %d failures\n", checks, failures);
        return 1;
    }

    size_t processedSize = 0;
    char *processed = LIBMATTI_B3D_GlslPreprocessor_Process(source, file_import, NULL, &processedSize);
    CHECK(processed != NULL);
    // The fog.glsl body is inlined, the import directive is gone.
    CHECK(strstr(processed, "fog_distance") != NULL);
    CHECK(strstr(processed, "#moj_import") == NULL);
    // The version stays.
    CHECK(strstr(processed, "#version 150") != NULL);
    free(processed);
    free(source);

    // ---- injectDefines ------------------------------------------------------
    const char *plain = "#version 150\nin vec3 Position;\n";
    char *noDefines = LIBMATTI_B3D_GlslPreprocessor_InjectDefines(plain, NULL);
    CHECK(strcmp(noDefines, plain) == 0);
    free(noDefines);
    const char *defines[] = {"#define ALPHA_CUT 0.5", NULL};
    char *withDefines = LIBMATTI_B3D_GlslPreprocessor_InjectDefines(plain, defines);
    CHECK(strstr(withDefines, "#define ALPHA_CUT 0.5\n#line 1 0\nin vec3 Position;") != NULL);
    free(withDefines);

    // ---- ShaderManager: invalid source/module paths --------------------------
    LIBMATTI_MC_ShaderManager *manager = LIBMATTI_MC_ShaderManager_New(NULL);
    CHECK(manager != NULL);
    CHECK(LIBMATTI_MC_ShaderManager_GetShader(manager, "core/position", LIBMATTI_B3D_ShaderType_VERTEX) == NULL);
    const LIBMATTI_B3D_GlShaderModule *missing =
        LIBMATTI_MC_ShaderManager_GetOrCompileShader(manager, "core/missing", LIBMATTI_B3D_ShaderType_VERTEX, NULL);
    CHECK(LIBMATTI_B3D_GlShaderModule_IsInvalid(missing));
    const LIBMATTI_B3D_GlProgram *missingProgram = LIBMATTI_MC_ShaderManager_CompileProgram(
        manager, "minecraft:position_color", "core/missing", "core/position_color",
        LIBMATTI_B3D_DefaultVertexFormat_POSITION_COLOR(), NULL, NULL, NULL, 0, NULL, NULL);
    CHECK(LIBMATTI_B3D_GlProgram_IsInvalid(missingProgram));
    LIBMATTI_MC_ShaderManager_Free(manager);

    // ---- GlProgram invalid singleton ----------------------------------------
    CHECK(LIBMATTI_B3D_GlProgram_IsInvalid(LIBMATTI_B3D_GlProgram_Invalid()));
    CHECK(LIBMATTI_B3D_GlProgram_GetProgramId(LIBMATTI_B3D_GlProgram_Invalid()) == -1);
    CHECK(LIBMATTI_B3D_GlProgram_BUILT_IN_UNIFORMS[0] != NULL &&
          strcmp(LIBMATTI_B3D_GlProgram_BUILT_IN_UNIFORMS[0], "Projection") == 0);

    // ---- Uniform records -----------------------------------------------------
    LIBMATTI_B3D_Uniform *sampler = LIBMATTI_B3D_Uniform_Sampler(3, 0);
    CHECK(sampler->kind == LIBMATTI_B3D_UniformKind_SAMPLER && sampler->sampler.location == 3);
    LIBMATTI_B3D_Uniform *ubo = LIBMATTI_B3D_Uniform_Ubo(2);
    CHECK(ubo->kind == LIBMATTI_B3D_UniformKind_UBO && ubo->ubo.blockBinding == 2);
    LIBMATTI_B3D_Uniform *utb = LIBMATTI_B3D_Uniform_Utb(4, 1, 0);
    CHECK(utb->kind == LIBMATTI_B3D_UniformKind_UTB && utb->utb.samplerIndex == 1);
    LIBMATTI_B3D_Uniform_Free(sampler);
    LIBMATTI_B3D_Uniform_Free(ubo);
    LIBMATTI_B3D_Uniform_Free(utb);

    printf("shader harness: %d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
