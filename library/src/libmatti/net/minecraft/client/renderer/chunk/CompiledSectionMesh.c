// Port of net.minecraft.client.renderer.chunk.CompiledSectionMesh (implementation).

#include "libmatti/net/minecraft/client/renderer/chunk/CompiledSectionMesh.h"

#include <stdlib.h>

LIBMATTI_MC_CompiledSectionMesh *LIBMATTI_MC_CompiledSectionMesh_New(
    LIBMATTI_MC_SectionBuffers *const *buffers, const LIBMATTI_MC_VisibilitySet *visibilitySet,
    LIBMATTI_B3D_MeshData_SortState *transparencyState)
{
    LIBMATTI_MC_CompiledSectionMesh *mesh = calloc(1, sizeof(LIBMATTI_MC_CompiledSectionMesh));
    if (buffers != NULL)
    {
        for (int layer = 0; layer < LIBMATTI_MC_ChunkSectionLayer_COUNT; layer++)
            mesh->buffers[layer] = buffers[layer];
    }
    mesh->visibilitySet = visibilitySet != NULL
                              ? LIBMATTI_MC_VisibilitySet_New()
                              : NULL;
    if (visibilitySet != NULL)
        *mesh->visibilitySet = *visibilitySet;
    mesh->transparencyState = transparencyState;
    return mesh;
}

void LIBMATTI_MC_CompiledSectionMesh_Free(LIBMATTI_MC_CompiledSectionMesh *mesh)
{
    if (mesh == NULL)
        return;
    for (int layer = 0; layer < LIBMATTI_MC_ChunkSectionLayer_COUNT; layer++)
    {
        if (mesh->buffers[layer] != NULL)
            LIBMATTI_MC_SectionBuffers_Free(mesh->buffers[layer]);
    }
    if (mesh->visibilitySet != NULL)
        free(mesh->visibilitySet);
    if (mesh->transparencyState != NULL)
        LIBMATTI_B3D_MeshData_SortState_Free(mesh->transparencyState);
    free(mesh);
}

int LIBMATTI_MC_CompiledSectionMesh_HasRenderedLayer(const LIBMATTI_MC_CompiledSectionMesh *mesh,
                                                     LIBMATTI_MC_ChunkSectionLayer layer)
{
    if (mesh == NULL || layer < 0 || layer >= LIBMATTI_MC_ChunkSectionLayer_COUNT)
        return 0;
    return mesh->buffers[layer] != NULL;
}

LIBMATTI_MC_SectionBuffers *LIBMATTI_MC_CompiledSectionMesh_GetBuffers(
    const LIBMATTI_MC_CompiledSectionMesh *mesh, LIBMATTI_MC_ChunkSectionLayer layer)
{
    if (mesh == NULL || layer < 0 || layer >= LIBMATTI_MC_ChunkSectionLayer_COUNT)
        return NULL;
    return mesh->buffers[layer];
}

const LIBMATTI_MC_VisibilitySet *LIBMATTI_MC_CompiledSectionMesh_Visibility(const LIBMATTI_MC_CompiledSectionMesh *mesh)
{
    if (mesh == NULL)
        return NULL;
    return mesh->visibilitySet;
}
