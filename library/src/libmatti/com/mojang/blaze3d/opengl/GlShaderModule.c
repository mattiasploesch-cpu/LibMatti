#include "libmatti/com/mojang/blaze3d/opengl/GlShaderModule.h"

#include "libmatti/com/mojang/blaze3d/opengl/GlStateManager.h"
#include "libmatti/com/mojang/blaze3d/systems/RenderSystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: public static final GlShaderModule INVALID_SHADER =
//       new GlShaderModule(-1, Identifier.withDefaultNamespace("invalid"), ShaderType.VERTEX);
static LIBMATTI_B3D_GlShaderModule invalidShader = {NULL, -1, LIBMATTI_B3D_ShaderType_VERTEX};

LIBMATTI_B3D_GlShaderModule *LIBMATTI_B3D_GlShaderModule_Invalid(void)
{
    if (invalidShader.id == NULL)
        invalidShader.id = strdup("minecraft:invalid");
    return &invalidShader;
}

int LIBMATTI_B3D_GlShaderModule_IsInvalid(const LIBMATTI_B3D_GlShaderModule *module)
{
    return module == &invalidShader || module->shaderId == -1;
}

LIBMATTI_B3D_GlShaderModule *LIBMATTI_B3D_GlShaderModule_New(int shaderId, const char *id,
                                                             LIBMATTI_B3D_ShaderType type)
{
    LIBMATTI_B3D_GlShaderModule *module = calloc(1, sizeof(LIBMATTI_B3D_GlShaderModule));
    module->id = strdup(id);
    module->shaderId = shaderId;
    module->type = type;
    return module;
}

void LIBMATTI_B3D_GlShaderModule_Free(LIBMATTI_B3D_GlShaderModule *module)
{
    if (module == NULL)
        return;
    // Java: public void close() - delete the shader, mark NOT_ALLOCATED.
    if (module->shaderId != -1)
    {
        LIBMATTI_B3D_RenderSystem_AssertOnRenderThread();
        LIBMATTI_B3D_GlStateManager_DeleteShader(module->shaderId);
        module->shaderId = -1;
    }
    free(module->id);
    free(module);
}

const char *LIBMATTI_B3D_GlShaderModule_GetId(const LIBMATTI_B3D_GlShaderModule *module)
{
    return module->id;
}

int LIBMATTI_B3D_GlShaderModule_GetShaderId(const LIBMATTI_B3D_GlShaderModule *module)
{
    return module->shaderId;
}

char *LIBMATTI_B3D_GlShaderModule_GetDebugLabel(const LIBMATTI_B3D_GlShaderModule *module)
{
    // Java: idConverter().idToFile(id) -> "shaders/" + id + extension.
    const char *extension = LIBMATTI_B3D_ShaderType_GetExtension(module->type);
    char *label = malloc(strlen(module->id) + strlen(extension) + 9);
    sprintf(label, "shaders/%s%s", module->id, extension);
    return label;
}
