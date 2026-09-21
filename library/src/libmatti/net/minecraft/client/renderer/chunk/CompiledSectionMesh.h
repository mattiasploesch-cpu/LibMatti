// Port of net.minecraft.client.renderer.chunk.CompiledSectionMesh - the
// compiled section state: per-layer GPU buffers, visibility and transparency.

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_COMPILEDSECTIONMESH_H
#define MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_COMPILEDSECTIONMESH_H

#include "libmatti/com/mojang/blaze3d/vertex/ByteBufferBuilder.h"
#include "libmatti/com/mojang/blaze3d/vertex/MeshData.h"
#include "libmatti/net/minecraft/client/renderer/chunk/ChunkSectionLayer.h"
#include "libmatti/net/minecraft/client/renderer/chunk/SectionBuffers.h"
#include "libmatti/net/minecraft/client/renderer/chunk/VisibilitySet.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public final class CompiledSectionMesh implements AutoCloseable -
// the empty section shares one EMPTY instance (the port exposes the NULL
// layers form; hasRenderedLayer reports the emptiness).
typedef struct LIBMATTI_MC_CompiledSectionMesh
{
    // Java: private final Map<ChunkSectionLayer, SectionBuffers> buffers
    LIBMATTI_MC_SectionBuffers *buffers[LIBMATTI_MC_ChunkSectionLayer_COUNT];
    // Java: private final VisibilitySet visibilitySet
    LIBMATTI_MC_VisibilitySet *visibilitySet;
    // Java: private @Nullable MeshData.SortState transparencyState
    LIBMATTI_B3D_MeshData_SortState *transparencyState;
} LIBMATTI_MC_CompiledSectionMesh;

// Java: CompiledSectionMesh(Map<ChunkSectionLayer, SectionBuffers>, VisibilitySet,
// @Nullable SortState) - takes ownership of every argument.
LIBMATTI_MC_CompiledSectionMesh *LIBMATTI_MC_CompiledSectionMesh_New(
    LIBMATTI_MC_SectionBuffers *const *buffers, const LIBMATTI_MC_VisibilitySet *visibilitySet,
    LIBMATTI_B3D_MeshData_SortState *transparencyState);
// Java: public void close() - frees every layer's buffers.
void LIBMATTI_MC_CompiledSectionMesh_Free(LIBMATTI_MC_CompiledSectionMesh *mesh);

// Java: public boolean hasRenderedLayer(ChunkSectionLayer)
int LIBMATTI_MC_CompiledSectionMesh_HasRenderedLayer(const LIBMATTI_MC_CompiledSectionMesh *mesh,
                                                     LIBMATTI_MC_ChunkSectionLayer layer);
// Java: public SectionBuffers getBuffers(ChunkSectionLayer) - NULL when the layer is empty.
LIBMATTI_MC_SectionBuffers *LIBMATTI_MC_CompiledSectionMesh_GetBuffers(
    const LIBMATTI_MC_CompiledSectionMesh *mesh, LIBMATTI_MC_ChunkSectionLayer layer);
// Java: public VisibilitySet visibility()
const LIBMATTI_MC_VisibilitySet *LIBMATTI_MC_CompiledSectionMesh_Visibility(const LIBMATTI_MC_CompiledSectionMesh *mesh);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_COMPILEDSECTIONMESH_H
