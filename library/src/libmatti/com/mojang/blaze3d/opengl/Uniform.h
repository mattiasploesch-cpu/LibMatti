// Port of com.mojang.blaze3d.opengl.Uniform - the sealed interface with the
// Sampler/Ubo/Utb records GlProgram.setupUniforms fills.

#ifndef MATTICRAFT_BLAZE3D_OPENGL_UNIFORM_H
#define MATTICRAFT_BLAZE3D_OPENGL_UNIFORM_H

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public sealed interface Uniform extends AutoCloseable
typedef enum LIBMATTI_B3D_UniformKind
{
    LIBMATTI_B3D_UniformKind_SAMPLER = 0,
    LIBMATTI_B3D_UniformKind_UBO,
    LIBMATTI_B3D_UniformKind_UTB
} LIBMATTI_B3D_UniformKind;

typedef struct LIBMATTI_B3D_Uniform
{
    LIBMATTI_B3D_UniformKind kind;
    union
    {
        // Java: record Sampler(int location, int samplerIndex)
        struct
        {
            int location;
            int samplerIndex;
        } sampler;
        // Java: record Ubo(int blockBinding)
        struct
        {
            int blockBinding;
        } ubo;
        // Java: record Utb(int location, int samplerIndex, TextureFormat format, int texture)
        struct
        {
            int location;
            int samplerIndex;
            int format;
            int texture;
        } utb;
    };
} LIBMATTI_B3D_Uniform;

LIBMATTI_B3D_Uniform *LIBMATTI_B3D_Uniform_Sampler(int location, int samplerIndex);
LIBMATTI_B3D_Uniform *LIBMATTI_B3D_Uniform_Ubo(int blockBinding);
LIBMATTI_B3D_Uniform *LIBMATTI_B3D_Uniform_Utb(int location, int samplerIndex, int format);
void LIBMATTI_B3D_Uniform_Free(LIBMATTI_B3D_Uniform *uniform);

#ifdef __cplusplus
}
#endif

#endif
