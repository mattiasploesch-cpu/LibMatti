// Port of com.mojang.blaze3d.shaders.ShaderType.

#ifndef MATTICRAFT_BLAZE3D_SHADERS_SHADERTYPE_H
#define MATTICRAFT_BLAZE3D_SHADERS_SHADERTYPE_H

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public enum ShaderType { VERTEX("vertex", ".vsh"), FRAGMENT("fragment", ".fsh") }
typedef enum LIBMATTI_B3D_ShaderType
{
    LIBMATTI_B3D_ShaderType_VERTEX = 0,
    LIBMATTI_B3D_ShaderType_FRAGMENT
} LIBMATTI_B3D_ShaderType;

// Java: public String getName()
const char *LIBMATTI_B3D_ShaderType_GetName(LIBMATTI_B3D_ShaderType type);
// Java: public String getExtension() (the port exposes it for path building).
const char *LIBMATTI_B3D_ShaderType_GetExtension(LIBMATTI_B3D_ShaderType type);
// Java: public static ShaderType byLocation(Identifier) - by path extension;
// -1 when the path is no shader source.
int LIBMATTI_B3D_ShaderType_ByPath(const char *path);
// Java: GlConst.toGl(type) - the GL enum for glCreateShader.
unsigned int LIBMATTI_B3D_ShaderType_ToGl(LIBMATTI_B3D_ShaderType type);

#ifdef __cplusplus
}
#endif

#endif
