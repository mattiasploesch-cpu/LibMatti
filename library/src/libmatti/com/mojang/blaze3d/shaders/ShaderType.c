#include "libmatti/com/mojang/blaze3d/shaders/ShaderType.h"

#include <string.h>

const char *LIBMATTI_B3D_ShaderType_GetName(LIBMATTI_B3D_ShaderType type)
{
    return type == LIBMATTI_B3D_ShaderType_FRAGMENT ? "fragment" : "vertex";
}

const char *LIBMATTI_B3D_ShaderType_GetExtension(LIBMATTI_B3D_ShaderType type)
{
    return type == LIBMATTI_B3D_ShaderType_FRAGMENT ? ".fsh" : ".vsh";
}

int LIBMATTI_B3D_ShaderType_ByPath(const char *path)
{
    // Java: byLocation walks TYPES and matches the path suffix.
    if (path == NULL)
        return -1;
    size_t length = strlen(path);
    if (length >= 4 && strcmp(path + length - 4, ".fsh") == 0)
        return LIBMATTI_B3D_ShaderType_FRAGMENT;
    if (length >= 4 && strcmp(path + length - 4, ".vsh") == 0)
        return LIBMATTI_B3D_ShaderType_VERTEX;
    return -1;
}

unsigned int LIBMATTI_B3D_ShaderType_ToGl(LIBMATTI_B3D_ShaderType type)
{
    // Java: GlConst.toGl - GL_VERTEX_SHADER 35633, GL_FRAGMENT_SHADER 35632.
    return type == LIBMATTI_B3D_ShaderType_FRAGMENT ? 35632u : 35633u;
}
