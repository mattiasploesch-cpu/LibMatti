





#include "libmatti/net/minecraft/Bootstrap.h"
#include "libmatti/net/minecraft/client/renderer/chunk/SectionRenderDispatcher.h"
#include "libmatti/net/minecraft/client/renderer/chunk/SectionCompiler.h"
#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/world/level/Level.h"

#include <stdio.h>
#include <string.h>

static int checks = 0;
static int failures = 0;

#define CHECK(cond)                                                                                  \
    do                                                                                               \
    {                                                                                                \
        checks++;                                                                                    \
        if (!(cond))                                                                                 \
        {                                                                                            \
            failures++;                                                                              \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);                                   \
        }                                                                                            \
    } while (0)

int main(void)
{
    LIBMATTI_MC_Bootstrap_BootStrap();

// Java: the client level - a flat stone platform at y=64 the section
    // compiler meshes into the SOLID layer.
    LIBMATTI_MC_Level *level = LIBMATTI_MC_Level_New(-64, 384, LIBMATTI_MC_Level_OVERWORLD, true);
    CHECK(level != NULL);
    LIBMATTI_MC_Block *stone = LIBMATTI_MC_VanillaBlocks_GetByName("STONE");
    CHECK(stone != NULL);
    for (int x = 0; x < 16; x++)
    {
        for (int z = 0; z < 16; z++)
        {
            LIBMATTI_MC_BlockPos ground = {{x, 64, z}};
            LIBMATTI_MC_Level_SetBlock(level, &ground, LIBMATTI_MC_Block_DefaultBlockState(stone),
                                       LIBMATTI_MC_Level_UPDATE_CLIENTS);
        }
    }

    LIBMATTI_MC_SectionRenderDispatcher *dispatcher = LIBMATTI_MC_SectionRenderDispatcher_New();
    CHECK(dispatcher != NULL);



    LIBMATTI_MC_RenderSection *section = LIBMATTI_MC_SectionRenderDispatcher_CreateSection(dispatcher, 0, 4, 0);
    CHECK(section != NULL);
    CHECK(LIBMATTI_MC_RenderSection_IsDirty(section));

// Java: compileOrBake - the dirty pass compiles the section.
    LIBMATTI_MC_SectionRenderDispatcher_CompileDirty(dispatcher, level, 1000);
    CHECK(!LIBMATTI_MC_RenderSection_IsDirty(section));
    CHECK(LIBMATTI_MC_SectionRenderDispatcher_CompileDirty == NULL ? 0 : 1);




    const LIBMATTI_MC_CompiledSectionMesh *compiled = LIBMATTI_MC_RenderSection_GetCompiled(section);
    CHECK(compiled != NULL);
    CHECK(LIBMATTI_MC_CompiledSectionMesh_HasRenderedLayer(compiled, LIBMATTI_MC_ChunkSectionLayer_SOLID));
    CHECK(!LIBMATTI_MC_CompiledSectionMesh_HasRenderedLayer(compiled, LIBMATTI_MC_ChunkSectionLayer_TRANSLUCENT));

    const LIBMATTI_MC_SectionBuffers *solid =
        LIBMATTI_MC_CompiledSectionMesh_GetBuffers(compiled, LIBMATTI_MC_ChunkSectionLayer_SOLID);
    CHECK(solid != NULL);




    int indexCount = LIBMATTI_MC_SectionBuffers_GetIndexCount(solid);
    CHECK(indexCount > 0);
    CHECK(indexCount % 6 == 0);




    LIBMATTI_MC_SectionRenderDispatcher_CompileDirty(dispatcher, level, 1100);
    CHECK(LIBMATTI_MC_RenderSection_GetCompiled(section) == compiled);




    LIBMATTI_MC_SectionRenderDispatcher_RenderLayer(dispatcher, LIBMATTI_MC_ChunkSectionLayer_SOLID, 0, NULL, NULL, NULL);

    LIBMATTI_MC_SectionRenderDispatcher_Free(dispatcher);
    LIBMATTI_MC_Level_Free(level);

    printf("%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
