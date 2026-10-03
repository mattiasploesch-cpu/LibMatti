// Port of net.minecraft.util.datafix.fixes.ChunkStatusFix and ChunkStatusFix2
// (P7.3) - the 1.16.5/1.17 status renames, both through
// DSL.fieldFinder("Level") + remainderFinder().set("Status", ...).

#include "libmatti/net/minecraft/util/datafix/fixes/ChunkStatusFix.h"
#include "libmatti/net/minecraft/util/datafix/fixes/FixChunks.h"

#include <string.h>

bool LIBMATTI_MC_ChunkStatusFix_Apply(LIBMATTI_MC_Nbt_Tag *rootTag)
{
    if (rootTag == NULL || rootTag->id != LIBMATTI_MC_Nbt_TAG_COMPOUND)
        return false;
    LIBMATTI_MC_Nbt_CompoundTag *level = LIBMATTI_MC_FixChunks_LevelOf((LIBMATTI_MC_Nbt_CompoundTag *) rootTag);
    if (level == NULL)
        return false;

    // Java: String s = dynamic.get("Status").asString("empty");
    //        if (Objects.equals(s, "postprocessed")) set("Status", "fullchunk")
    const char *status = LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(level, "Status", "empty");
    if (strcmp(status, "postprocessed") != 0)
        return false;
    LIBMATTI_MC_Nbt_CompoundTag_PutString(level, "Status", "fullchunk");
    return true;
}

bool LIBMATTI_MC_ChunkStatusFix2_Apply(LIBMATTI_MC_Nbt_Tag *rootTag)
{
    if (rootTag == NULL || rootTag->id != LIBMATTI_MC_Nbt_TAG_COMPOUND)
        return false;
    LIBMATTI_MC_Nbt_CompoundTag *level = LIBMATTI_MC_FixChunks_LevelOf((LIBMATTI_MC_Nbt_CompoundTag *) rootTag);
    if (level == NULL)
        return false;

    // Java: RENAMES_AND_DOWNGRADES.getOrDefault(s, "empty") - every status the
    // table does not name (which only means a status newer than this rule)
    // falls back to "empty". The port leaves an unknown status alone instead:
    // its chain runs the whole list, so a current "full" would be destroyed.
    static const char *const RENAMES[][2] = {
        {"structure_references", "empty"},
        {"biomes", "empty"},
        {"base", "surface"},
        {"carved", "carvers"},
        {"liquid_carved", "liquid_carvers"},
        {"decorated", "features"},
        {"lighted", "light"},
        {"mobs_spawned", "spawn"},
        {"finalized", "heightmaps"},
        {"fullchunk", "full"},
    };
    const char *status = LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(level, "Status", "empty");
    const char *target = NULL;
    for (size_t i = 0; i < sizeof(RENAMES) / sizeof(RENAMES[0]); i++)
        if (strcmp(RENAMES[i][0], status) == 0)
        {
            target = RENAMES[i][1];
            break;
        }
    if (target == NULL || strcmp(status, target) == 0)
        return false; // Java: Objects.equals(s, s1) -> unchanged
    LIBMATTI_MC_Nbt_CompoundTag_PutString(level, "Status", target);
    return true;
}