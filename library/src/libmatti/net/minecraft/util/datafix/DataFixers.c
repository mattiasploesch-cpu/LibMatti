// Port of net.minecraft.util.datafix.DataFixers (P7.3) - the game's fix
// registry. Vanilla registers 262 rules; the port registers the chunk rules
// the storage path can act on, in the same version order DataFixers.addFixers
// does (the list order is the order the update chain runs in).

#include "libmatti/net/minecraft/util/datafix/DataFixers.h"
#include "libmatti/net/minecraft/SharedConstants.h"
#include "libmatti/net/minecraft/util/datafix/DataFixTypes.h"
#include "libmatti/net/minecraft/util/datafix/fixes/BlockEntityIdFix.h"
#include "libmatti/net/minecraft/util/datafix/fixes/BitStorageAlignFix.h"
#include "libmatti/net/minecraft/util/datafix/fixes/ChunkBiomeFix.h"
#include "libmatti/net/minecraft/util/datafix/fixes/ChunkDeleteIgnoredLightDataFix.h"
#include "libmatti/net/minecraft/util/datafix/fixes/ChunkDeleteLightFix.h"
#include "libmatti/net/minecraft/util/datafix/fixes/ChunkHeightAndBiomeFix.h"
#include "libmatti/net/minecraft/util/datafix/fixes/ChunkLightRemoveFix.h"
#include "libmatti/net/minecraft/util/datafix/fixes/ChunkPalettedStorageFix.h"
#include "libmatti/net/minecraft/util/datafix/fixes/ChunkStatusFix.h"
#include "libmatti/net/minecraft/util/datafix/fixes/ChunkToProtochunkFix.h"

#define CHUNK LIBMATTI_MC_DataFixTypes_CHUNK

// Java: DataFixers.addFixers - the port's rule list, in the same order
static const LIBMATTI_MC_DataFix FIXES[] = {
    // the 1.13 block entity names ("Chest" -> "minecraft:chest"). Java applies
    // it through the chunk type, which embeds the BLOCK_ENTITY choice - the port
    // registers it on the chunk directly and walks the block entity list
    {704, 0, CHUNK, "BlockEntityIdFix", LIBMATTI_MC_BlockEntityIdFix_Apply},
    // 1.16.2: the 12 bit section storage becomes a palette
    {1451, 1, CHUNK, "ChunkPalettedStorageFix", LIBMATTI_MC_ChunkPalettedStorageFix_Apply},
    // 1.16.3: terrain/light flags become the ChunkStatus
    {1466, 0, CHUNK, "ChunkToProtoChunkFix", LIBMATTI_MC_ChunkToProtochunkFix_Apply},
    // 1.16.5 / 1.17: the status renames
    {1905, 0, CHUNK, "ChunkStatusFix", LIBMATTI_MC_ChunkStatusFix_Apply},
    {1911, 0, CHUNK, "ChunkStatusFix2", LIBMATTI_MC_ChunkStatusFix2_Apply},
    {1961, 0, CHUNK, "ChunkLightRemoveFix", LIBMATTI_MC_ChunkLightRemoveFix_Apply},
    // 1.17: the 4x4 corner biome ids expand to the 16x16 grid
    {2202, 0, CHUNK, "ChunkBiomeFix", LIBMATTI_MC_ChunkBiomeFix_Apply},
    // 1.18: the bit storage drops its per-word padding
    {2527, 0, CHUNK, "BitStorageAlignFix", LIBMATTI_MC_BitStorageAlignFix_Apply},
    // 1.18: 256 -> 384 block world height, heightmaps shift by 64, biomes split
    {2832, 0, CHUNK, "ChunkHeightAndBiomeFix", LIBMATTI_MC_ChunkHeightAndBiomeFix_Apply},
    // 1.19: the light arrays only survive on lit chunks
    {3077, 0, CHUNK, "ChunkDeleteIgnoredLightDataFix", LIBMATTI_MC_ChunkDeleteIgnoredLightDataFix_Apply},
    // 1.20 / 1.21.4: the light data is gone for good
    {3451, 0, CHUNK, "ChunkDeleteLightFix for 3451", LIBMATTI_MC_ChunkDeleteLightFix_Apply},
    {4537, 0, CHUNK, "ChunkDeleteLightFix for 4537", LIBMATTI_MC_ChunkDeleteLightFix_Apply},
};

static LIBMATTI_MC_DataFixer *g_fixer = NULL;

const LIBMATTI_MC_DataFixer *LIBMATTI_MC_DataFixers_GetDataFixer(void)
{
    // Java: the class initializer builds DATA_FIXER on first access
    if (g_fixer == NULL)
        g_fixer = LIBMATTI_MC_DataFixer_New(LIBMATTI_MC_SharedConstants_GetDataVersion(), FIXES,
                                             sizeof(FIXES) / sizeof(FIXES[0]));
    return g_fixer;
}

void LIBMATTI_MC_DataFixers_Free(void)
{
    LIBMATTI_MC_DataFixer_Free(g_fixer);
    g_fixer = NULL;
}