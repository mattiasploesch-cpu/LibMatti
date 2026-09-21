#include "libmatti/com/mojang/blaze3d/opengl/Uniform.h"

#include "libmatti/com/mojang/blaze3d/opengl/GlStateManager.h"

#include <stdlib.h>

LIBMATTI_B3D_Uniform *LIBMATTI_B3D_Uniform_Sampler(int location, int samplerIndex)
{
    LIBMATTI_B3D_Uniform *uniform = calloc(1, sizeof(LIBMATTI_B3D_Uniform));
    uniform->kind = LIBMATTI_B3D_UniformKind_SAMPLER;
    uniform->sampler.location = location;
    uniform->sampler.samplerIndex = samplerIndex;
    return uniform;
}

LIBMATTI_B3D_Uniform *LIBMATTI_B3D_Uniform_Ubo(int blockBinding)
{
    LIBMATTI_B3D_Uniform *uniform = calloc(1, sizeof(LIBMATTI_B3D_Uniform));
    uniform->kind = LIBMATTI_B3D_UniformKind_UBO;
    uniform->ubo.blockBinding = blockBinding;
    return uniform;
}

LIBMATTI_B3D_Uniform *LIBMATTI_B3D_Uniform_Utb(int location, int samplerIndex, int format)
{
    LIBMATTI_B3D_Uniform *uniform = calloc(1, sizeof(LIBMATTI_B3D_Uniform));
    uniform->kind = LIBMATTI_B3D_UniformKind_UTB;
    uniform->utb.location = location;
    uniform->utb.samplerIndex = samplerIndex;
    uniform->utb.format = format;
    uniform->utb.texture = 0;
    return uniform;
}

void LIBMATTI_B3D_Uniform_Free(LIBMATTI_B3D_Uniform *uniform)
{
    if (uniform == NULL)
        return;
    // Java: the Utb close() deletes the bound texture when set.
    if (uniform->kind == LIBMATTI_B3D_UniformKind_UTB && uniform->utb.texture != 0)
        LIBMATTI_B3D_GlStateManager_DeleteTexture(uniform->utb.texture);
    free(uniform);
}
