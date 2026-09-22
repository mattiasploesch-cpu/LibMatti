// Port of net.minecraft.client.renderer.chunk.SectionCompiler - the compile
// pass that walks the section's 18^3 block region, skips the occluded faces,
// packs the visible geometry into the per-layer buffers and resolves the face
// visibility through the VisGraph.

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_SECTIONCOMPILER_H
#define MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_SECTIONCOMPILER_H

#include "libmatti/net/minecraft/client/renderer/chunk/ChunkSectionLayer.h"
#include "libmatti/net/minecraft/client/renderer/chunk/CompiledSectionMesh.h"
#include "libmatti/net/minecraft/client/renderer/chunk/RenderSectionRegion.h"
#include "libmatti/net/minecraft/client/renderer/chunk/SectionBufferBuilderPack.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public SectionCompiler(Context) - the context carries the level and the
// block-model resolver; the port keeps both on the region and the sprite lookup
// callback the game layer wires.
typedef struct LIBMATTI_MC_SectionCompiler
{
    // The sprite rect provider for a block's face texture: fills uv with the
    // {u0, v0, u1, v1} atlas rect of the block's default sprite. The game layer
    // resolves through the block atlas; NULL means the missing-texture sprite.
    void (*spriteRectForBlock)(void *userdata, const struct LIBMATTI_MC_Block *block, float uv[4]);
    void *userdata;
} LIBMATTI_MC_SectionCompiler;

// Java: the compile result (Java returns CompiledSectionMesh directly).
LIBMATTI_MC_SectionCompiler *LIBMATTI_MC_SectionCompiler_New(void);
void LIBMATTI_MC_SectionCompiler_Free(LIBMATTI_MC_SectionCompiler *compiler);

// Java: public CompiledSectionMesh compile(SectionPos, RenderSectionRegion,
// SectionBufferBuilderPack) - compiles the center section of the region from
// the pack's per-layer buffers. Returns NULL when nothing rendered.
LIBMATTI_MC_CompiledSectionMesh *LIBMATTI_MC_SectionCompiler_Compile(
    const LIBMATTI_MC_SectionCompiler *compiler, const LIBMATTI_MC_SectionPos *pos,
    const LIBMATTI_MC_RenderSectionRegion *region, LIBMATTI_MC_SectionBufferBuilderPack *pack);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_SECTIONCOMPILER_H
