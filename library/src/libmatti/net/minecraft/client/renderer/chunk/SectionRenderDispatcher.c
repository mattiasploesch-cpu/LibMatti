// Port of net.minecraft.client.renderer.chunk.SectionRenderDispatcher (the
// synchronous slice: section registry, compile-dirty pass, upload and draw)
// plus the RenderSection accessors.

#include "libmatti/net/minecraft/client/renderer/chunk/SectionRenderDispatcher.h"

#include "libmatti/com/mojang/blaze3d/vertex/DefaultVertexFormat.h"
#include "libmatti/com/mojang/blaze3d/vertex/VertexFormat.h"
#include "libmatti/com/mojang/blaze3d/vertex/VertexFormatElement.h"
#include "libmatti/org/lwjgl/opengl/GL.h"
#include "libmatti/org/lwjgl/opengl/Constants.h"
#include "libmatti/net/minecraft/world/level/Level.h"
#include "libmatti/net/minecraft/world/level/LevelHeightAccessor.h"
#include "libmatti/net/minecraft/world/level/chunk/LevelChunkSection.h"
#include "libmatti/net/minecraft/world/level/chunk/ChunkAccess.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// RenderSection
// ---------------------------------------------------------------------------

LIBMATTI_MC_RenderSection *LIBMATTI_MC_RenderSection_New(int sectionX, int sectionY, int sectionZ)
{
    LIBMATTI_MC_RenderSection *section = calloc(1, sizeof(LIBMATTI_MC_RenderSection));
    section->sectionPos = LIBMATTI_MC_SectionPos_Of(sectionX, sectionY, sectionZ);
    section->dirty = 1; // Java: a fresh section compiles on the first pass.
    return section;
}

void LIBMATTI_MC_RenderSection_Free(LIBMATTI_MC_RenderSection *section)
{
    if (section == NULL)
        return;
    if (section->compiled != NULL)
        LIBMATTI_MC_CompiledSectionMesh_Free(section->compiled);
    free(section);
}

const LIBMATTI_MC_SectionPos *LIBMATTI_MC_RenderSection_GetSectionPos(const LIBMATTI_MC_RenderSection *section)
{
    return section != NULL ? section->sectionPos : NULL;
}

void LIBMATTI_MC_RenderSection_SetDirty(LIBMATTI_MC_RenderSection *section, int dirty)
{
    section->dirty = dirty;
}

int LIBMATTI_MC_RenderSection_IsDirty(const LIBMATTI_MC_RenderSection *section)
{
    return section->dirty;
}

const LIBMATTI_MC_CompiledSectionMesh *LIBMATTI_MC_RenderSection_GetCompiled(const LIBMATTI_MC_RenderSection *section)
{
    return section->compiled;
}

void LIBMATTI_MC_RenderSection_SetCompiled(LIBMATTI_MC_RenderSection *section,
                                           LIBMATTI_MC_CompiledSectionMesh *compiled)
{
    if (section->compiled != NULL && section->compiled != compiled)
        LIBMATTI_MC_CompiledSectionMesh_Free(section->compiled);
    section->compiled = compiled;
}

int LIBMATTI_MC_RenderSection_HasRenderedLayer(const LIBMATTI_MC_RenderSection *section,
                                               LIBMATTI_MC_ChunkSectionLayer layer)
{
    return section != NULL && section->compiled != NULL
           && LIBMATTI_MC_CompiledSectionMesh_HasRenderedLayer(section->compiled, layer);
}

// ---------------------------------------------------------------------------
// SectionRenderDispatcher
// ---------------------------------------------------------------------------

LIBMATTI_MC_SectionRenderDispatcher *LIBMATTI_MC_SectionRenderDispatcher_New(void)
{
    LIBMATTI_MC_SectionRenderDispatcher *dispatcher = calloc(1, sizeof(LIBMATTI_MC_SectionRenderDispatcher));
    dispatcher->compiler = LIBMATTI_MC_SectionCompiler_New();
    dispatcher->pack = LIBMATTI_MC_SectionBufferBuilderPack_New();
    dispatcher->sectionCapacity = 16;
    dispatcher->sections = calloc((size_t) dispatcher->sectionCapacity, sizeof(LIBMATTI_MC_RenderSection *));
    return dispatcher;
}

void LIBMATTI_MC_SectionRenderDispatcher_Free(LIBMATTI_MC_SectionRenderDispatcher *dispatcher)
{
    if (dispatcher == NULL)
        return;
    for (int i = 0; i < dispatcher->sectionCount; i++)
        LIBMATTI_MC_RenderSection_Free(dispatcher->sections[i]);
    free(dispatcher->sections);
    LIBMATTI_MC_SectionBufferBuilderPack_Free(dispatcher->pack);
    LIBMATTI_MC_SectionCompiler_Free(dispatcher->compiler);
    free(dispatcher);
}

void LIBMATTI_MC_SectionRenderDispatcher_SetSpriteResolver(
    LIBMATTI_MC_SectionRenderDispatcher *dispatcher,
    void (*spriteRectForBlock)(void *userdata, const struct LIBMATTI_MC_Block *block, float uv[4]),
    void *userdata)
{
    dispatcher->compiler->spriteRectForBlock = spriteRectForBlock;
    dispatcher->compiler->userdata = userdata;
}

static void grow_sections(LIBMATTI_MC_SectionRenderDispatcher *dispatcher)
{
    int newCapacity = dispatcher->sectionCapacity * 2;
    LIBMATTI_MC_RenderSection **grown =
        realloc(dispatcher->sections, (size_t) newCapacity * sizeof(LIBMATTI_MC_RenderSection *));
    if (grown == NULL)
        return;
    dispatcher->sections = grown;
    dispatcher->sectionCapacity = newCapacity;
}

LIBMATTI_MC_RenderSection *LIBMATTI_MC_SectionRenderDispatcher_CreateSection(
    LIBMATTI_MC_SectionRenderDispatcher *dispatcher, int sectionX, int sectionY, int sectionZ)
{
    if (dispatcher->sectionCount == dispatcher->sectionCapacity)
        grow_sections(dispatcher);
    LIBMATTI_MC_RenderSection *section = LIBMATTI_MC_RenderSection_New(sectionX, sectionY, sectionZ);
    dispatcher->sections[dispatcher->sectionCount++] = section;
    return section;
}

// Java: RenderChunkRegion.getRegion - the 18^3 section window around the
// center section, taken from the level's chunk sections.
static LIBMATTI_MC_LevelChunkSection **build_region_sections(LIBMATTI_MC_Level *level,
                                                             const LIBMATTI_MC_SectionPos *pos)
{
    LIBMATTI_MC_LevelChunkSection **sections =
        calloc((size_t) (LIBMATTI_MC_RenderSectionRegion_SIZE * LIBMATTI_MC_RenderSectionRegion_SIZE
                         * LIBMATTI_MC_RenderSectionRegion_SIZE),
               sizeof(LIBMATTI_MC_LevelChunkSection *));
    int centerX = LIBMATTI_MC_SectionPos_SectionX(pos);
    int centerY = LIBMATTI_MC_SectionPos_SectionY(pos);
    int centerZ = LIBMATTI_MC_SectionPos_SectionZ(pos);

    // Java: LevelHeightAccessor.getMinSectionY - the section index base.
    LIBMATTI_MC_LevelHeightAccessor accessor = LIBMATTI_MC_Level_GetHeightAccessor(level);
    int minSectionY = LIBMATTI_MC_LevelHeightAccessor_GetMinSectionY(&accessor);
    int sectionsCount = LIBMATTI_MC_LevelHeightAccessor_GetSectionsCount(&accessor);

    for (int dy = 0; dy < LIBMATTI_MC_RenderSectionRegion_SIZE; dy++)
    {
        for (int dz = 0; dz < LIBMATTI_MC_RenderSectionRegion_SIZE; dz++)
        {
            for (int dx = 0; dx < LIBMATTI_MC_RenderSectionRegion_SIZE; dx++)
            {
                int sx = centerX + dx - LIBMATTI_MC_RenderSectionRegion_CENTER_OFFSET;
                int sy = centerY + dy - LIBMATTI_MC_RenderSectionRegion_CENTER_OFFSET;
                int sz = centerZ + dz - LIBMATTI_MC_RenderSectionRegion_CENTER_OFFSET;

                LIBMATTI_MC_LevelChunk *chunk = LIBMATTI_MC_Level_GetChunk(level, sx >> 4, sz >> 4);
                if (chunk == NULL)
                    continue; // Java: VOID_AIR outside loaded chunks.
                if (sy < minSectionY || sy >= minSectionY + sectionsCount)
                    continue;
                LIBMATTI_MC_LevelChunkSection *section =
                    LIBMATTI_MC_ChunkAccess_GetSection(&chunk->base, sy - minSectionY);
                int index = (dy * LIBMATTI_MC_RenderSectionRegion_SIZE + dz) * LIBMATTI_MC_RenderSectionRegion_SIZE + dx;
                sections[index] = section;
            }
        }
    }
    return sections;
}

// Java: compileOrBake - the synchronous rebuild of every dirty section.
void LIBMATTI_MC_SectionRenderDispatcher_CompileDirty(LIBMATTI_MC_SectionRenderDispatcher *dispatcher,
                                                      LIBMATTI_MC_Level *level,
                                                      long long nowMs)
{
    if (dispatcher == NULL || level == NULL)
        return;
    LIBMATTI_MC_SectionBufferBuilderPack_ClearAll(dispatcher->pack);

    for (int i = 0; i < dispatcher->sectionCount; i++)
    {
        LIBMATTI_MC_RenderSection *section = dispatcher->sections[i];
        if (!section->dirty)
            continue;

        LIBMATTI_MC_LevelChunkSection **regionSections = build_region_sections(level, section->sectionPos);
        LIBMATTI_MC_RenderSectionRegion *region = LIBMATTI_MC_RenderSectionRegion_New(
            LIBMATTI_MC_SectionPos_SectionX(section->sectionPos),
            LIBMATTI_MC_SectionPos_SectionY(section->sectionPos),
            LIBMATTI_MC_SectionPos_SectionZ(section->sectionPos),
            level, regionSections);

        LIBMATTI_MC_CompiledSectionMesh *compiled = LIBMATTI_MC_SectionCompiler_Compile(
            dispatcher->compiler, section->sectionPos, region, dispatcher->pack);

        LIBMATTI_MC_RenderSectionRegion_Free(region); // frees the section array, not the level's sections
        LIBMATTI_MC_RenderSection_SetCompiled(section, compiled);
        section->dirty = 0;
        section->lastBuiltAtMs = nowMs;
    }
}

// Java: renderSection - one compiled layer through the section shader. The
// port drives the raw GL pipeline (the GlDevice command queue is the render
// port's part): program, MVP uniform, BLOCK-format VAO, draw elements.
typedef struct LayerVao
{
    unsigned int vao;
    unsigned int vbo;
    unsigned int ibo;
    // The shared element index buffer over every section's vertices: the QUADS
    // draw turns quad i into the two triangles (4i, 4i+1, 4i+2, 4i+2, 4i+1,
    // 4i+3). Java 1.21.11 generates these in the render pass; the port uploads
    // one reused buffer per layer.
    unsigned int indexDataBuffer;
    long long indexDataBytes;
} LayerVao;

static LayerVao g_layerVaos[LIBMATTI_MC_ChunkSectionLayer_COUNT];

// Java: the render pass's quad index generation - the element indices over
// `quadCount` quads as UNSIGNED_SHORT (the section never exceeds 65535
// vertices; the type least() picks).
static void upload_quad_indices(LayerVao *vao, int quadCount)
{
    long long bytes = (long long) quadCount * 6 * 2;
    if (bytes > vao->indexDataBytes)
    {
        unsigned short *indices = malloc((size_t) (quadCount * 6) * sizeof(unsigned short));
        if (indices == NULL)
            return;
        for (int q = 0; q < quadCount; q++)
        {
            unsigned short base = (unsigned short) (q * 4);
            unsigned short *target = indices + q * 6;
            target[0] = base;
            target[1] = base + 1;
            target[2] = base + 2;
            target[3] = base + 2;
            target[4] = base + 1;
            target[5] = base + 3;
        }
        LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, vao->ibo);
        LIBMATTI_GL_glBufferData(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, bytes, indices,
                                 LIBMATTI_GL_GL_DYNAMIC_DRAW);
        LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, 0);
        free(indices);
        vao->indexDataBuffer = vao->ibo;
        vao->indexDataBytes = bytes;
    }
}

// Java: VertexFormat.setupBufferState - the BLOCK format's attribute pointers
// over the interleaved layout (position, color, uv0, uv2, normal).
static unsigned int create_block_vao(unsigned int vbo, unsigned int ibo)
{
    const LIBMATTI_B3D_VertexFormat *format = LIBMATTI_B3D_DefaultVertexFormat_BLOCK();
    int stride = LIBMATTI_B3D_VertexFormat_GetVertexSize(format);

    unsigned int vaos[1];
    LIBMATTI_GL_glGenVertexArrays(1, vaos);
    unsigned int vao = vaos[0];
    LIBMATTI_GL_glBindVertexArray(vao);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, vbo);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, ibo);

    // The BLOCK element order: Position (3f), Color (4ub), UV0 (2f), UV2 (2s),
    // Normal (3b) + 1 padding byte. The offsets come from the format.
    for (int i = 0; i < LIBMATTI_B3D_VertexFormat_GetElementCount(format); i++)
    {
        const LIBMATTI_B3D_VertexFormatElement *element = LIBMATTI_B3D_VertexFormat_GetElement(format, i);
        int offset = LIBMATTI_B3D_VertexFormat_GetOffset(format, i);
        LIBMATTI_GL_glEnableVertexAttribArray((unsigned int) i);
        LIBMATTI_GL_glVertexAttribPointer(
            (unsigned int) i, element->count,        LIBMATTI_B3D_VertexFormatElement_GlType((const LIBMATTI_B3D_VertexFormatElement *) element),
            element->type == LIBMATTI_B3D_VFE_FLOAT ? 0 : 1, stride, (const void *) (long) offset);
    }
    LIBMATTI_GL_glBindVertexArray(0);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, 0);
    LIBMATTI_GL_glBindBuffer(LIBMATTI_GL_GL_ELEMENT_ARRAY_BUFFER, 0);
    return vao;
}

void LIBMATTI_MC_SectionRenderDispatcher_RenderLayer(
    const LIBMATTI_MC_SectionRenderDispatcher *dispatcher, LIBMATTI_MC_ChunkSectionLayer layer,
    unsigned int program, const float *mvpMatrix, const float *modelOrigin)
{
    if (dispatcher == NULL || program == 0)
        return;
    LIBMATTI_GL_glUseProgram(program);
    // Java: the MVP + model-origin uniforms the terrain shader sets per draw.
    // Java uploads through JOML's Matrix4f.get(FloatBuffer), which writes
    // column-major for GL; the port's struct keeps JOML's field order (row
    // major), so the upload carries the transpose flag (the 2D paths come out
    // identical either way, the rotated view does not).
    int mvpLocation = LIBMATTI_GL_glGetUniformLocation(program, "mvp");
    int originLocation = LIBMATTI_GL_glGetUniformLocation(program, "origin");
    if (mvpLocation >= 0 && mvpMatrix != NULL)
        LIBMATTI_GL_glUniformMatrix4fv(mvpLocation, 1, mvpMatrix);
    if (originLocation >= 0 && modelOrigin != NULL)
        LIBMATTI_GL_glUniform3f(originLocation, modelOrigin[0], modelOrigin[1], modelOrigin[2]);

    for (int i = 0; i < dispatcher->sectionCount; i++)
    {
        const LIBMATTI_MC_RenderSection *section = dispatcher->sections[i];
        const LIBMATTI_MC_CompiledSectionMesh *compiled = section->compiled;
        if (compiled == NULL || !LIBMATTI_MC_CompiledSectionMesh_HasRenderedLayer(compiled, layer))
            continue;
        const LIBMATTI_MC_SectionBuffers *buffers = LIBMATTI_MC_CompiledSectionMesh_GetBuffers(compiled, layer);
        if (buffers == NULL || buffers->vertexBuffer == NULL || buffers->indexCount <= 0)
            continue;

        LayerVao *vao = &g_layerVaos[layer];
        if (getenv("MATTI_CHUNK_DEBUG") != NULL)
        {
            fprintf(stderr, "[CHUNKDEBUG] section %d: vao=%u vbo=%u ibo=%u indexCount=%d\n",
                    i, vao->vao, vao->vbo, vao->ibo, buffers->indexCount);
        }
        if (vao->vao == 0 && buffers->vertexBuffer->handle != 0)
        {
            vao->vbo = buffers->vertexBuffer->handle;
            if (buffers->indexBuffer != NULL)
            {
                vao->ibo = buffers->indexBuffer->handle;
            }
            else
            {
                // Java: the render pass owns the element indices - the port
                // allocates the shared IBO for the generated quad indices.
                unsigned int ibos[1];
                LIBMATTI_GL_glGenBuffers(1, ibos);
                vao->ibo = ibos[0];
            }
            vao->vao = create_block_vao(vao->vbo, vao->ibo);
        }
        if (vao->vao == 0 || vao->ibo == 0)
            continue; // no GL context (headless): nothing to draw through.

        // Java: the render pass generates the quad element indices over the
        // section's vertex count; the port uploads them into the shared IBO.
        upload_quad_indices(vao, buffers->indexCount / 6);

        LIBMATTI_GL_glBindVertexArray(vao->vao);
        LIBMATTI_GL_glDrawElements(
            LIBMATTI_GL_GL_TRIANGLES, buffers->indexCount,
            LIBMATTI_B3D_VertexFormat_IndexType_Bytes(buffers->indexType) == 2 ? LIBMATTI_GL_GL_UNSIGNED_SHORT
                                                                               : LIBMATTI_GL_GL_UNSIGNED_INT,
            NULL);
        LIBMATTI_GL_glBindVertexArray(0);
    }
    LIBMATTI_GL_glUseProgram(0);
}
