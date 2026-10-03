// Port of net.minecraft.util.datafix.fixes.ChunkToProtochunkFix (P7.3).
//
// Java: fixChunkData(Dynamic Level) - the two legacy booleans pick the new
// status, the biome byte array widens to 256 ints, the tile ticks pack into
// the 16 per-section short lists, and the result is written back with
// hasLegacyStructureData set.

#include "libmatti/net/minecraft/util/datafix/fixes/ChunkToProtochunkFix.h"
#include "libmatti/net/minecraft/nbt/CompoundTag.h"
#include "libmatti/net/minecraft/nbt/ListTag.h"
#include "libmatti/net/minecraft/util/datafix/fixes/FixChunks.h"

#include <stdlib.h>
#include <string.h>

// Java: packOffsetCoordinates(x, y, z) - (x & 15) | (y & 15) << 4 | (z & 15) << 8
static int16_t pack_offset(int x, int y, int z)
{
    return (int16_t) ((x & 15) | (y & 15) << 4 | (z & 15) << 8);
}

bool LIBMATTI_MC_ChunkToProtochunkFix_Apply(LIBMATTI_MC_Nbt_Tag *rootTag)
{
    bool changed = false;
    if (rootTag == NULL || rootTag->id != LIBMATTI_MC_Nbt_TAG_COMPOUND)
        return changed;
    LIBMATTI_MC_Nbt_CompoundTag *level = LIBMATTI_MC_FixChunks_LevelOf((LIBMATTI_MC_Nbt_CompoundTag *) rootTag);
    if (level == NULL)
        return false;

    // Java: repackBiomes - the 256-entry byte buffer becomes an int list
    changed = true;
    const int8_t *biomeBytes = NULL;
    size_t biomeLength = 0;
    if (LIBMATTI_MC_Nbt_CompoundTag_GetByteArray(level, "Biomes", &biomeBytes, &biomeLength))
    {
        int32_t biomes[256];
        for (int i = 0; i < 256; i++)
            biomes[i] = i < (int) biomeLength ? (int32_t) ((uint8_t) biomeBytes[i]) : 0;
        LIBMATTI_MC_Nbt_CompoundTag_PutIntArray(level, "Biomes", biomes, 256);
    }

    // Java: repackTicks - TileTicks become ToBeTicked, 16 short lists
    LIBMATTI_MC_Nbt_ListTag *ticks = NULL;
    if (LIBMATTI_MC_Nbt_CompoundTag_GetList(level, "TileTicks", &ticks) && ticks != NULL)
    {
        LIBMATTI_MC_Nbt_ListTag *perSection[16];
        for (int i = 0; i < 16; i++)
            perSection[i] = LIBMATTI_MC_Nbt_ListTag_New();
        int count = LIBMATTI_MC_Nbt_ListTag_Size(ticks);
        for (int i = 0; i < count; i++)
        {
            LIBMATTI_MC_Nbt_CompoundTag *tick = NULL;
            if (!LIBMATTI_MC_Nbt_ListTag_GetCompound(ticks, i, &tick) || tick == NULL)
                continue;
            int y = LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(tick, "y", 0);
            int section = y >> 4;
            if (section < 0 || section >= 16)
                continue;
            int16_t packed = pack_offset(LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(tick, "x", 0), y,
                                         LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(tick, "z", 0));
            LIBMATTI_MC_Nbt_ListTag_Add(perSection[section], LIBMATTI_MC_Nbt_ShortTag_Of(packed));
        }
        LIBMATTI_MC_Nbt_ListTag *outer = LIBMATTI_MC_Nbt_ListTag_New();
        for (int i = 0; i < 16; i++)
            LIBMATTI_MC_Nbt_ListTag_Add(outer, (LIBMATTI_MC_Nbt_Tag *) perSection[i]);
        LIBMATTI_MC_Nbt_CompoundTag_Remove(level, "TileTicks");
        LIBMATTI_MC_Nbt_CompoundTag_Put(level, "ToBeTicked", (LIBMATTI_MC_Nbt_Tag *) outer);
    }

    // Java: fixChunkData - TerrainPopulated + LightPopulated pick the status.
    // A chunk that no longer carries the legacy flags has already been through
    // this rule, so its status must stay as it is (Java never sees this shape
    // because the rule stops running for newer versions).
    if (!LIBMATTI_MC_Nbt_CompoundTag_Contains(level, "TerrainPopulated") &&
        !LIBMATTI_MC_Nbt_CompoundTag_Contains(level, "LightPopulated"))
        return changed;

    int terrainPopulated = LIBMATTI_MC_Nbt_CompoundTag_GetBooleanOr(level, "TerrainPopulated", 0);
    int lightPopulated = LIBMATTI_MC_Nbt_CompoundTag_GetBooleanOr(level, "LightPopulated", 1);
    const char *status = terrainPopulated ? (lightPopulated ? "mobs_spawned" : "decorated") : "carved";
    LIBMATTI_MC_Nbt_CompoundTag_PutString(level, "Status", status);
    LIBMATTI_MC_Nbt_CompoundTag_PutBoolean(level, "hasLegacyStructureData", 1);
    return true;
}