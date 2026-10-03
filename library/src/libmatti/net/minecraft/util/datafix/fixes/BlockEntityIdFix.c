// Port of net.minecraft.util.datafix.fixes.BlockEntityIdFix (P7.3).
//
// Java maps the tagged choice key (the "id" of every block entity) through
// ID_MAP and rewrites the item stack name hook; the port walks the chunk's
// block-entity list (Java called it TileEntities back then) and the item
// stacks those block entities carry.

#include "libmatti/net/minecraft/util/datafix/fixes/BlockEntityIdFix.h"
#include "libmatti/net/minecraft/util/datafix/fixes/FixChunks.h"

#include <string.h>

// Java: the ID_MAP literal, in source order
static const char *const ID_MAP[][2] = {
    {"Airportal", "minecraft:end_portal"},
    {"Banner", "minecraft:banner"},
    {"Beacon", "minecraft:beacon"},
    {"Cauldron", "minecraft:brewing_stand"},
    {"Chest", "minecraft:chest"},
    {"Comparator", "minecraft:comparator"},
    {"Control", "minecraft:command_block"},
    {"DLDetector", "minecraft:daylight_detector"},
    {"Dropper", "minecraft:dropper"},
    {"EnchantTable", "minecraft:enchanting_table"},
    {"EndGateway", "minecraft:end_gateway"},
    {"EnderChest", "minecraft:ender_chest"},
    {"FlowerPot", "minecraft:flower_pot"},
    {"Furnace", "minecraft:furnace"},
    {"Hopper", "minecraft:hopper"},
    {"MobSpawner", "minecraft:mob_spawner"},
    {"Music", "minecraft:noteblock"},
    {"Piston", "minecraft:piston"},
    {"RecordPlayer", "minecraft:jukebox"},
    {"Sign", "minecraft:sign"},
    {"Skull", "minecraft:skull"},
    {"Structure", "minecraft:structure_block"},
    {"Trap", "minecraft:dispenser"},
};

const char *LIBMATTI_MC_BlockEntityIdFix_MapId(const char *id)
{
    if (id == NULL)
        return NULL;
    for (size_t i = 0; i < sizeof(ID_MAP) / sizeof(ID_MAP[0]); i++)
        if (strcmp(ID_MAP[i][0], id) == 0)
            return ID_MAP[i][1];
    return id; // Java: ID_MAP.getOrDefault(id, id)
}

// Java: the item stack name hook converter - every stack's "id" inside a block
// entity's item list
static bool map_item_stacks(LIBMATTI_MC_Nbt_CompoundTag *compound)
{
    static const char *const ITEM_KEYS[] = {"Items", "Inventory", "items"};
    bool changed = false;
    for (size_t k = 0; k < sizeof(ITEM_KEYS) / sizeof(ITEM_KEYS[0]); k++)
    {
        LIBMATTI_MC_Nbt_ListTag *items = NULL;
        if (!LIBMATTI_MC_Nbt_CompoundTag_GetList(compound, ITEM_KEYS[k], &items) || items == NULL)
            continue;
        int count = LIBMATTI_MC_Nbt_ListTag_Size(items);
        for (int i = 0; i < count; i++)
        {
            LIBMATTI_MC_Nbt_CompoundTag *stack = NULL;
            if (!LIBMATTI_MC_Nbt_ListTag_GetCompound(items, i, &stack) || stack == NULL)
                continue;
            const char *id = LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(stack, "id", "");
            const char *mapped = LIBMATTI_MC_BlockEntityIdFix_MapId(id);
            if (mapped != NULL && strcmp(mapped, id) != 0)
            {
                LIBMATTI_MC_Nbt_CompoundTag_PutString(stack, "id", mapped);
                changed = true;
            }
        }
    }
    return changed;
}

bool LIBMATTI_MC_BlockEntityIdFix_Apply(LIBMATTI_MC_Nbt_Tag *rootTag)
{
    if (rootTag == NULL || rootTag->id != LIBMATTI_MC_Nbt_TAG_COMPOUND)
        return false;
    LIBMATTI_MC_Nbt_CompoundTag *level = LIBMATTI_MC_FixChunks_LevelOf((LIBMATTI_MC_Nbt_CompoundTag *) rootTag);
    if (level == NULL)
        return false;

    // Java: TileEntities (1.12) / block_entities (1.13+)
    static const char *const LIST_KEYS[] = {"TileEntities", "block_entities"};
    bool changed = false;
    for (size_t k = 0; k < sizeof(LIST_KEYS) / sizeof(LIST_KEYS[0]); k++)
    {
        LIBMATTI_MC_Nbt_ListTag *list = NULL;
        if (!LIBMATTI_MC_Nbt_CompoundTag_GetList(level, LIST_KEYS[k], &list) || list == NULL)
            continue;
        int count = LIBMATTI_MC_Nbt_ListTag_Size(list);
        for (int i = 0; i < count; i++)
        {
            LIBMATTI_MC_Nbt_CompoundTag *entity = NULL;
            if (!LIBMATTI_MC_Nbt_ListTag_GetCompound(list, i, &entity) || entity == NULL)
                continue;
            const char *id = LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(entity, "id", "");
            const char *mapped = LIBMATTI_MC_BlockEntityIdFix_MapId(id);
            if (mapped != NULL && strcmp(mapped, id) != 0)
            {
                LIBMATTI_MC_Nbt_CompoundTag_PutString(entity, "id", mapped);
                changed = true;
            }
            if (map_item_stacks(entity))
                changed = true;
        }
    }
    return changed;
}