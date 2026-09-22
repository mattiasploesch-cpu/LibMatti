// Port of net.minecraft.client.renderer.chunk.SectionRenderDispatcher and
// net.minecraft.client.renderer.chunk.RenderSection (the compile/upload/render
// state machine per section).

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_SECTIONRENDERDISPATCHER_H
#define MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_SECTIONRENDERDISPATCHER_H

#include "libmatti/net/minecraft/client/renderer/chunk/ChunkSectionLayer.h"
#include "libmatti/net/minecraft/client/renderer/chunk/CompiledSectionMesh.h"
#include "libmatti/net/minecraft/client/renderer/chunk/RenderSectionRegion.h"
#include "libmatti/net/minecraft/client/renderer/chunk/SectionBufferBuilderPack.h"
#include "libmatti/net/minecraft/client/renderer/chunk/SectionCompiler.h"
#include "libmatti/net/minecraft/core/SectionPos.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class RenderSection implements BufferedRenderableSection -
// the per-section compile result and its world anchor.
typedef struct LIBMATTI_MC_RenderSection
{
    // Java: private final SectionPos sectionPos
    LIBMATTI_MC_SectionPos *sectionPos;
    // Java: private volatile CompiledSectionMesh compiledSectionMesh
    LIBMATTI_MC_CompiledSectionMesh *compiled;
    // Java: private boolean dirty - set on every block change in the section.
    int dirty;
    // Java: private long lastSubmittedTimeMs / lastPrevBuiltTimeMs (the port
    // keeps the one rebuild counter the skeleton needs).
    long long lastBuiltAtMs;
} LIBMATTI_MC_RenderSection;

// Java: public RenderSection(SectionRenderDispatcher, Level, int, int, int)
LIBMATTI_MC_RenderSection *LIBMATTI_MC_RenderSection_New(int sectionX, int sectionY, int sectionZ);
// Java: public void close() - releases the compiled mesh.
void LIBMATTI_MC_RenderSection_Free(LIBMATTI_MC_RenderSection *section);

// Java: public SectionPos getSectionPos()
const LIBMATTI_MC_SectionPos *LIBMATTI_MC_RenderSection_GetSectionPos(const LIBMATTI_MC_RenderSection *section);
// Java: public void setDirty(boolean)
void LIBMATTI_MC_RenderSection_SetDirty(LIBMATTI_MC_RenderSection *section, int dirty);
// Java: public boolean isDirty()
int LIBMATTI_MC_RenderSection_IsDirty(const LIBMATTI_MC_RenderSection *section);
// Java: public CompiledSectionMesh getCompiled()
const LIBMATTI_MC_CompiledSectionMesh *LIBMATTI_MC_RenderSection_GetCompiled(const LIBMATTI_MC_RenderSection *section);
// Java: public void setCompiled(CompiledSectionMesh) - releases the previous mesh.
void LIBMATTI_MC_RenderSection_SetCompiled(LIBMATTI_MC_RenderSection *section,
                                           LIBMATTI_MC_CompiledSectionMesh *compiled);
// Java: public boolean hasRenderedLayer(ChunkSectionLayer)
int LIBMATTI_MC_RenderSection_HasRenderedLayer(const LIBMATTI_MC_RenderSection *section,
                                               LIBMATTI_MC_ChunkSectionLayer layer);

// Java: public class SectionRenderDispatcher - the compile queue and the
// per-section upload/draw. The port runs the compile synchronously (Java's
// ReentrantSectorLock + worker pool are the parallel-dispatch part).
typedef struct LIBMATTI_MC_SectionRenderDispatcher
{
    // Java: private final SectionCompiler compiler
    LIBMATTI_MC_SectionCompiler *compiler;
    // Java: private final SectionBufferBuilderPack.FixedSizeArray pool - the
    // port keeps one shared pack the synchronous compile borrows.
    LIBMATTI_MC_SectionBufferBuilderPack *pack;
    // Java: private final List<RenderSection> sections - the port stores the
    // sections the client registered for the rebuild pass.
    LIBMATTI_MC_RenderSection **sections;
    int sectionCount;
    int sectionCapacity;
} LIBMATTI_MC_SectionRenderDispatcher;

LIBMATTI_MC_SectionRenderDispatcher *LIBMATTI_MC_SectionRenderDispatcher_New(void);
void LIBMATTI_MC_SectionRenderDispatcher_Free(LIBMATTI_MC_SectionRenderDispatcher *dispatcher);

// Java: the sprite-rect resolver the compiler consults per block face texture.
void LIBMATTI_MC_SectionRenderDispatcher_SetSpriteResolver(
    LIBMATTI_MC_SectionRenderDispatcher *dispatcher,
    void (*spriteRectForBlock)(void *userdata, const struct LIBMATTI_MC_Block *block, float uv[4]),
    void *userdata);

// Java: the block-model resolver (ModelManager) - the compiler renders the
// baked model quads when the block carries one.
void LIBMATTI_MC_SectionRenderDispatcher_SetModelResolver(
    LIBMATTI_MC_SectionRenderDispatcher *dispatcher,
    const struct LIBMATTI_MC_QuadCollection *(*modelForBlock)(void *userdata, const struct LIBMATTI_MC_Block *block),
    void *userdata);

// Java: the section registry (createRenderSection on the level renderer).
LIBMATTI_MC_RenderSection *LIBMATTI_MC_SectionRenderDispatcher_CreateSection(
    LIBMATTI_MC_SectionRenderDispatcher *dispatcher, int sectionX, int sectionY, int sectionZ);

// Java: compileOrBake - rebuilds every dirty section: builds the 18^3 region
// from the level, compiles into the shared pack and uploads the result.
void LIBMATTI_MC_SectionRenderDispatcher_CompileDirty(LIBMATTI_MC_SectionRenderDispatcher *dispatcher,
                                                      struct LIBMATTI_MC_Level *level,
                                                      long long nowMs);

// Java: renderSection - draws one compiled layer through the section shader.
// mvpMatrix is the column-major projection*view, modelOrigin the section's
// world offset (section corner minus the camera-relative render origin).
void LIBMATTI_MC_SectionRenderDispatcher_RenderLayer(
    const LIBMATTI_MC_SectionRenderDispatcher *dispatcher, LIBMATTI_MC_ChunkSectionLayer layer,
    unsigned int program, const float *mvpMatrix, const float *modelOrigin);

// The terrain program (Java: RenderPipelines' terrain shader): compiles once
// per process, the out locations ride with the returned program.
unsigned int LIBMATTI_MC_SectionShader_Compile(int *mvpLocation, int *originLocation, int *useTextureLocation);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_SECTIONRENDERDISPATCHER_H
