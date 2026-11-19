// The terrain draw program the SectionRenderDispatcher's render layer uses
// (Java: RenderPipelines.SOLID_TERRAIN's shader - position + color + lightmap
// through the fog setup; the port keeps the uniform set the skeleton needs).

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_SECTIONSHADER_H
#define MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_SECTIONSHADER_H

#ifdef __cplusplus
extern "C" {
#endif




unsigned int LIBMATTI_MC_SectionShader_Compile(int *mvpLocation, int *originLocation, int *useTextureLocation);

#ifdef __cplusplus
}
#endif

#endif
