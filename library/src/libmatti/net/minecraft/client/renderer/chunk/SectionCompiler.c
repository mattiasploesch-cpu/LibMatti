// Port of net.minecraft.client.renderer.chunk.SectionCompiler (implementation).
//
// Java walks the section's blocks, renders every non-air one into the
// per-layer buffers (faces culled against the region neighbours, which include
// the one-block border from the adjacent sections) and feeds the occluding
// blocks into the VisGraph for the face-visibility set.

#include "libmatti/net/minecraft/client/renderer/chunk/SectionCompiler.h"

#include "libmatti/com/mojang/blaze3d/vertex/BufferBuilder.h"
#include "libmatti/com/mojang/blaze3d/vertex/DefaultVertexFormat.h"
#include "libmatti/com/mojang/blaze3d/vertex/VertexFormat.h"
#include "libmatti/com/mojang/blaze3d/buffers/GlBuffer.h"
#include "libmatti/net/minecraft/client/renderer/chunk/ModelBlockRenderer.h"
#include "libmatti/net/minecraft/client/renderer/chunk/SectionBuffers.h"
#include "libmatti/net/minecraft/client/renderer/chunk/VisGraph.h"
#include "libmatti/net/minecraft/client/resources/model/QuadCollection.h"
#include "libmatti/net/minecraft/world/level/block/Block.h"

#include <stdio.h>
#include "libmatti/net/minecraft/world/level/block/state/BlockBehaviour.h"
#include "libmatti/net/minecraft/world/level/chunk/LevelChunkSection.h"
#include "libmatti/net/minecraft/core/Direction.h"

#include <stdlib.h>

// Java: compile -> getOrBeginLayer(layer).getBuffer() - the BufferBuilder over
// the pack's shared ByteBufferBuilder per layer. The port mirrors the two-step
// access the layer enum drives.
typedef struct CompileState
{
    const LIBMATTI_MC_SectionCompiler *compiler;
    const LIBMATTI_MC_RenderSectionRegion *region;
    LIBMATTI_MC_SectionBufferBuilderPack *pack;
    LIBMATTI_MC_VisGraph *visGraph;
    // Java: getOrBeginLayer - one builder per layer over the pack's shared
    // ByteBufferBuilder, begun once and built once per layer.
    LIBMATTI_B3D_BufferBuilder *builders[LIBMATTI_MC_ChunkSectionLayer_COUNT];
    LIBMATTI_B3D_MeshData *meshes[LIBMATTI_MC_ChunkSectionLayer_COUNT];
} CompileState;

// Java: BlockStateBase.canOcclude - the Properties flag rides on the block.
static int state_can_occlude(const LIBMATTI_MC_BlockState *state)
{
    const LIBMATTI_MC_Block *block = LIBMATTI_MC_BlockState_GetBlock(state);
    if (block == NULL || block->properties == NULL)
        return 0;
    return block->properties->canOcclude;
}

static int state_is_air(const LIBMATTI_MC_BlockState *state)
{
    const LIBMATTI_MC_Block *block = LIBMATTI_MC_BlockState_GetBlock(state);
    if (block == NULL || block->properties == NULL)
        return 0;
    return block->properties->isAir;
}

// Java: shouldFaceRender - a face is hidden when the neighbour in the face
// direction is a full occluding cube (the region answers with air outside its
// window, so the world-border faces always render).
static int face_is_visible(const CompileState *state, int blockX, int blockY, int blockZ,
                           LIBMATTI_MC_Direction direction)
{
    LIBMATTI_MC_BlockPos neighbourPos = {
        {blockX + LIBMATTI_MC_Direction_GetStepX(direction),
         blockY + LIBMATTI_MC_Direction_GetStepY(direction),
         blockZ + LIBMATTI_MC_Direction_GetStepZ(direction)}};
    const LIBMATTI_MC_BlockState *neighbour =
        LIBMATTI_MC_RenderSectionRegion_GetBlockState(state->region, &neighbourPos);
    if (neighbour == NULL)
        return 1;
    return !state_can_occlude(neighbour);
}

// Java: the vertex packer for a BakedQuad - the model quad's positions are
// in block-model space (0..1 per element after FaceBakery's /16), the block
// offset moves it into world space; the UVs rescale from the sprite rect.
static void emit_baked_quad(CompileState *state, LIBMATTI_B3D_BufferBuilder *builder,
                            const LIBMATTI_MC_BakedQuad *quad, int blockX, int blockY, int blockZ,
                            const float spriteUv[4])
{
    (void) state;
    for (int corner = 0; corner < 4; corner++)
    {
        float x = quad->pos[corner][0] + (float) blockX;
        float y = quad->pos[corner][1] + (float) blockY;
        float z = quad->pos[corner][2] + (float) blockZ;
        // Java: the model UVs sit in the sprite's 0..1 space; the atlas rect
        // maps them in (the vanilla quads bake sprite-relative UVs).
        float u = spriteUv[0] + quad->uv[corner][0] * (spriteUv[2] - spriteUv[0]);
        float v = spriteUv[1] + quad->uv[corner][1] * (spriteUv[3] - spriteUv[1]);
        LIBMATTI_B3D_BufferBuilder_AddVertexFull(
            builder,
            x, y, z,
            -1, // Java: the untinted white color
            u, v,
            0, quad->shade ? 0 : 0, // Java: the lightmap pass rides on the game port
            (float) LIBMATTI_MC_Direction_GetStepX(quad->direction),
            (float) LIBMATTI_MC_Direction_GetStepY(quad->direction),
            (float) LIBMATTI_MC_Direction_GetStepZ(quad->direction));
    }
}

// Java: the baked-model path of ModelBlockRenderer.tesselateBlock - the model
// quads render with the block offset; the model's UVs are already in atlas
// space, the sprite rect scales them onto the block's sprite (the demo uses
// one sprite per block, so the rect is the same for every face).
static void render_model_quads(CompileState *state, const LIBMATTI_MC_QuadCollection *model,
                               const LIBMATTI_MC_BlockState *blockState, int blockX, int blockY, int blockZ,
                               const float spriteUv[4])
{
    const LIBMATTI_MC_Block *block = LIBMATTI_MC_BlockState_GetBlock(blockState);

    LIBMATTI_MC_ChunkSectionLayer layer = LIBMATTI_MC_ChunkSectionLayer_SOLID;
    if (!state_can_occlude(blockState))
        layer = LIBMATTI_MC_ChunkSectionLayer_CUTOUT;

    LIBMATTI_B3D_BufferBuilder *builder = state->builders[layer];
    if (builder == NULL)
    {
        builder = LIBMATTI_B3D_BufferBuilder_New(
            LIBMATTI_MC_SectionBufferBuilderPack_Buffer(state->pack, layer),
            LIBMATTI_B3D_Mode_QUADS, LIBMATTI_B3D_DefaultVertexFormat_BLOCK());
        state->builders[layer] = builder;
    }
    if (builder == NULL)
        return;

    // Java: getUnculledFaces() first, then every cull bucket whose face passes
    // the region visibility gate.
    size_t count = 0;
    const LIBMATTI_MC_BakedQuad *quads = LIBMATTI_MC_QuadCollection_GetUnculled(model, &count);
    for (size_t i = 0; i < count; i++)
        emit_baked_quad(state, builder, &quads[i], blockX, blockY, blockZ, spriteUv);

    for (int d = 0; d < 6; d++)
    {
        LIBMATTI_MC_Direction direction = (LIBMATTI_MC_Direction) d;
        if (!face_is_visible(state, blockX, blockY, blockZ, direction))
            continue;
        quads = LIBMATTI_MC_QuadCollection_GetCulled(model, direction, &count);
        for (size_t i = 0; i < count; i++)
            emit_baked_quad(state, builder, &quads[i], blockX, blockY, blockZ, spriteUv);
    }
    (void) block;
}

// Java: ModelBlockRenderer.tesselateBlock - one quad per visible face of the
// default cube. The port writes the BLOCK-format vertices (position, color,
// uv, light, normal) straight into the layer buffer; the dispatcher uploads.
static void render_block_faces(CompileState *state, const LIBMATTI_MC_BlockState *blockState,
                               int blockX, int blockY, int blockZ)
{
    const LIBMATTI_MC_Block *block = LIBMATTI_MC_BlockState_GetBlock(blockState);

    // Java: the layer the block renders into - the port maps occluding blocks
    // to SOLID and everything else to CUTOUT until the JSON models carry the
    // render-type rules (the P4.2 part).
    LIBMATTI_MC_ChunkSectionLayer layer = LIBMATTI_MC_ChunkSectionLayer_SOLID;
    if (!state_can_occlude(blockState))
        layer = LIBMATTI_MC_ChunkSectionLayer_CUTOUT;

    // Java: getOrBeginLayer - lazily begin the builder over the pack buffer.
    LIBMATTI_B3D_BufferBuilder *builder = state->builders[layer];
    if (builder == NULL)
    {
        builder = LIBMATTI_B3D_BufferBuilder_New(
            LIBMATTI_MC_SectionBufferBuilderPack_Buffer(state->pack, layer),
            LIBMATTI_B3D_Mode_QUADS, LIBMATTI_B3D_DefaultVertexFormat_BLOCK());
        state->builders[layer] = builder;
    }
    if (builder == NULL)
        return;

    float spriteUv[4] = {0.0f, 0.0f, 1.0f, 1.0f};
    if (state->compiler->spriteRectForBlock != NULL)
        state->compiler->spriteRectForBlock(state->compiler->userdata, block, spriteUv);

    // Java: the baked block-model quads (ModelManager) replace the hardcoded
    // cube when the block carries a model. The unculled quads + the per-
    // direction culled buckets map onto the same face-visibility gate.
    const LIBMATTI_MC_QuadCollection *model = NULL;
    if (state->compiler->modelForBlock != NULL)
        model = state->compiler->modelForBlock(state->compiler->userdata, block);
    if (model != NULL)
    {
        render_model_quads(state, model, blockState, blockX, blockY, blockZ, spriteUv);
        return;
    }

    for (int facing = 0; facing < 6; facing++)
    {
        LIBMATTI_MC_Direction direction = (LIBMATTI_MC_Direction) facing;
        if (!face_is_visible(state, blockX, blockY, blockZ, direction))
            continue;

        LIBMATTI_MC_ModelQuad quad;
        LIBMATTI_MC_ModelBlockRenderer_BakeCubeFace(direction, (float) blockX, (float) blockY, (float) blockZ,
                                                    spriteUv, &quad);

        for (int corner = 0; corner < 4; corner++)
        {
            LIBMATTI_B3D_BufferBuilder_AddVertexFull(
                builder,
                quad.pos[corner][0], quad.pos[corner][1], quad.pos[corner][2],
                -1, // Java: the untinted white color (-1 = 0xFFFFFFFF)
                quad.uv[corner][0], quad.uv[corner][1],
                0, quad.light,
                (float) LIBMATTI_MC_Direction_GetStepX(direction),
                (float) LIBMATTI_MC_Direction_GetStepY(direction),
                (float) LIBMATTI_MC_Direction_GetStepZ(direction));
        }
    }
}

// Java: compile's walk - the center section's 4096 blocks; the border only
// feeds the face culls through the region lookups.
static void extract_geometry(CompileState *state)
{
    const LIBMATTI_MC_RenderSectionRegion *region = state->region;
    // Java: region.sections[SECTION_SIZE/2] - the center section of the 18^3
    // window (region-relative 1, 1, 1).
    const int centerIndex =
        LIBMATTI_MC_RenderSectionRegion_CENTER_OFFSET * LIBMATTI_MC_RenderSectionRegion_SIZE * LIBMATTI_MC_RenderSectionRegion_SIZE
        + LIBMATTI_MC_RenderSectionRegion_CENTER_OFFSET * LIBMATTI_MC_RenderSectionRegion_SIZE
        + LIBMATTI_MC_RenderSectionRegion_CENTER_OFFSET;
    const LIBMATTI_MC_LevelChunkSection *section = region->sections[centerIndex];
    if (section == NULL)
        return;

    // Java: SectionPos.origin() - the section's first block corner.
    int originX = region->centerX << 4;
    int originY = region->centerY << 4;
    int originZ = region->centerZ << 4;

    for (int by = 0; by < LIBMATTI_MC_LevelChunkSection_SECTION_HEIGHT; by++)
    {
        for (int bz = 0; bz < LIBMATTI_MC_LevelChunkSection_SECTION_WIDTH; bz++)
        {
            for (int bx = 0; bx < LIBMATTI_MC_LevelChunkSection_SECTION_WIDTH; bx++)
            {
                const LIBMATTI_MC_BlockState *blockState =
                    LIBMATTI_MC_LevelChunkSection_GetBlockState(section, bx, by, bz);
                if (blockState == NULL || state_is_air(blockState))
                    continue;

                if (state_can_occlude(blockState))
                {
                    // Java: VisGraph positions are section-relative.
                    LIBMATTI_MC_BlockPos visPos = {{bx, by, bz}};
                    LIBMATTI_MC_VisGraph_SetOpaque(state->visGraph, &visPos);
                }

                render_block_faces(state, blockState, originX + bx, originY + by, originZ + bz);
            }
        }
    }
}

LIBMATTI_MC_SectionCompiler *LIBMATTI_MC_SectionCompiler_New(void)
{
    return calloc(1, sizeof(LIBMATTI_MC_SectionCompiler));
}

void LIBMATTI_MC_SectionCompiler_Free(LIBMATTI_MC_SectionCompiler *compiler)
{
    free(compiler);
}

LIBMATTI_MC_CompiledSectionMesh *LIBMATTI_MC_SectionCompiler_Compile(
    const LIBMATTI_MC_SectionCompiler *compiler, const LIBMATTI_MC_SectionPos *pos,
    const LIBMATTI_MC_RenderSectionRegion *region, LIBMATTI_MC_SectionBufferBuilderPack *pack)
{
    (void) pos;
    if (compiler == NULL || region == NULL || pack == NULL)
        return NULL;

    CompileState state;
    state.compiler = compiler;
    state.region = region;
    state.pack = pack;
    state.visGraph = LIBMATTI_MC_VisGraph_New();
    for (int layer = 0; layer < LIBMATTI_MC_ChunkSectionLayer_COUNT; layer++)
    {
        state.builders[layer] = NULL;
        state.meshes[layer] = NULL;
    }

    extract_geometry(&state);

    // Java: the end of the walk builds every begun layer's mesh.
    for (int layer = 0; layer < LIBMATTI_MC_ChunkSectionLayer_COUNT; layer++)
    {
        if (state.builders[layer] == NULL)
            continue;
        state.meshes[layer] = LIBMATTI_B3D_BufferBuilder_Build(state.builders[layer]);
        LIBMATTI_B3D_BufferBuilder_Free(state.builders[layer]);
        state.builders[layer] = NULL;
    }

    // Java: visgraph.resolve() - an empty graph answers the all-visible set.
    LIBMATTI_MC_VisibilitySet *visibility = LIBMATTI_MC_VisGraph_Resolve(state.visGraph);
    LIBMATTI_MC_VisGraph_Free(state.visGraph);

    // Java: the finished per-layer vertex data becomes the mesh's
    // SectionBuffers; nothing rendered means the NULL mesh.
    int any = 0;
    for (int layer = 0; layer < LIBMATTI_MC_ChunkSectionLayer_COUNT; layer++)
        any |= state.meshes[layer] != NULL;
    if (!any)
    {
        // Java: the resolved visibility dies with the empty mesh.
        free(visibility);
        return NULL;
    }

    LIBMATTI_MC_SectionBuffers *buffers[LIBMATTI_MC_ChunkSectionLayer_COUNT] = {0};
    for (int layer = 0; layer < LIBMATTI_MC_ChunkSectionLayer_COUNT; layer++)
    {
        LIBMATTI_B3D_MeshData *mesh = state.meshes[layer];
        if (mesh == NULL)
            continue;

        size_t vertexSize = 0;
        const unsigned char *vertexData = LIBMATTI_B3D_MeshData_VertexBuffer(mesh, &vertexSize);
        size_t indexSize = 0;
        const unsigned char *indexData = LIBMATTI_B3D_MeshData_IndexBuffer(mesh, &indexSize);
        int vertexCount = (int) (vertexSize / (size_t) LIBMATTI_B3D_VertexFormat_GetVertexSize(
                                                       LIBMATTI_B3D_DefaultVertexFormat_BLOCK()));
        if (getenv("MATTI_CHUNK_DUMP") != NULL)
        {
            const float *fv = (const float *) vertexData;
            fprintf(stderr,
                    "[CHUNKDEBUG] mesh layer=%d vertexSize=%zu indexSize=%zu v0=(%.2f,%.2f,%.2f) v1=(%.2f,%.2f,%.2f) stride=%d\n",
                    layer, vertexSize, indexSize,
                    fv[0], fv[1], fv[2], fv[8], fv[9], fv[10],
                    LIBMATTI_B3D_VertexFormat_GetVertexSize(LIBMATTI_B3D_DefaultVertexFormat_BLOCK()));
        }
        // Java: the buffers carry the draw count - mode.indexCount(vertices)
        // turns the quad vertices into the 6-per-quad element count.
        int indexCount = vertexCount / 4 * 6;

        // Java: GlDevice.createBuffers with the store's byte size - the GL
        // store is sized for the whole upload up front, the later BufferData
        // only refills it.
        LIBMATTI_B3D_GpuBuffer *vertexBuffer = LIBMATTI_B3D_GlBuffer_New(
            LIBMATTI_B3D_USAGE_VERTEX | LIBMATTI_B3D_USAGE_COPY_DST, (long) vertexSize);
        LIBMATTI_B3D_GpuBuffer *indexBuffer = NULL;
        if (indexData != NULL && indexSize > 0)
            indexBuffer = LIBMATTI_B3D_GlBuffer_New(
                LIBMATTI_B3D_USAGE_INDEX | LIBMATTI_B3D_USAGE_COPY_DST, (long) indexSize);

        LIBMATTI_MC_SectionBuffers *layerBuffers = LIBMATTI_MC_SectionBuffers_New(
            vertexBuffer, indexBuffer, indexCount, LIBMATTI_B3D_VertexFormat_IndexType_Least(vertexCount));
        LIBMATTI_MC_SectionBuffers_Upload(layerBuffers, vertexData, (long) vertexSize,
                                          indexData, indexBuffer != NULL ? (long) indexSize : 0);
        buffers[layer] = layerBuffers;

        LIBMATTI_B3D_MeshData_Free(mesh);
    }

    return LIBMATTI_MC_CompiledSectionMesh_New(buffers, visibility, NULL);
}
