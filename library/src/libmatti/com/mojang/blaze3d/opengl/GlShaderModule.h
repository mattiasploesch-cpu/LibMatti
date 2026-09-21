// Port of com.mojang.blaze3d.opengl.GlShaderModule.

#ifndef MATTICRAFT_BLAZE3D_OPENGL_GLSHADERMODULE_H
#define MATTICRAFT_BLAZE3D_OPENGL_GLSHADERMODULE_H

#include "libmatti/com/mojang/blaze3d/shaders/ShaderType.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public class GlShaderModule implements AutoCloseable
// The port keeps the id as a char* (Java: Identifier) and the GL shader id.
typedef struct LIBMATTI_B3D_GlShaderModule
{
    char *id;      // Java: private final Identifier id
    int shaderId;  // Java: private int shaderId (NOT_ALLOCATED = -1)
    LIBMATTI_B3D_ShaderType type;
} LIBMATTI_B3D_GlShaderModule;

// Java: public GlShaderModule(int shaderId, Identifier id, ShaderType type)
LIBMATTI_B3D_GlShaderModule *LIBMATTI_B3D_GlShaderModule_New(int shaderId, const char *id,
                                                             LIBMATTI_B3D_ShaderType type);
void LIBMATTI_B3D_GlShaderModule_Free(LIBMATTI_B3D_GlShaderModule *module);

// Java: public static final GlShaderModule INVALID_SHADER (id -1) - the shared
// singleton; shaderId stays untouched.
LIBMATTI_B3D_GlShaderModule *LIBMATTI_B3D_GlShaderModule_Invalid(void);
int LIBMATTI_B3D_GlShaderModule_IsInvalid(const LIBMATTI_B3D_GlShaderModule *module);

// Java: public Identifier getId()
const char *LIBMATTI_B3D_GlShaderModule_GetId(const LIBMATTI_B3D_GlShaderModule *module);
// Java: public int getShaderId()
int LIBMATTI_B3D_GlShaderModule_GetShaderId(const LIBMATTI_B3D_GlShaderModule *module);
// Java: public String getDebugLabel() - "shaders/<id><extension>".
char *LIBMATTI_B3D_GlShaderModule_GetDebugLabel(const LIBMATTI_B3D_GlShaderModule *module);

#ifdef __cplusplus
}
#endif

#endif
