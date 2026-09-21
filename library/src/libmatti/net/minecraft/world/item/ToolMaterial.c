// Port of net.minecraft.world.item.ToolMaterial.

#include "libmatti/net/minecraft/world/item/ToolMaterial.h"

#include "libmatti/net/minecraft/world/item/Item.h"

// Java: WOOD = new ToolMaterial(BlockTags.INCORRECT_FOR_WOODEN_TOOL, 59, 2.0F, 0.0F, 15, ItemTags.WOODEN_TOOL_MATERIALS)
// (mutable - ensure_tags fills the interned tag fields lazily)
static LIBMATTI_MC_ToolMaterial WOOD = {NULL, 59, 2.0f, 0.0f, 15, NULL};
// Java: STONE = (BlockTags.INCORRECT_FOR_STONE_TOOL, 131, 4.0F, 1.0F, 5, ItemTags.STONE_TOOL_MATERIALS)
static LIBMATTI_MC_ToolMaterial STONE = {NULL, 131, 4.0f, 1.0f, 5, NULL};
// Java: COPPER = (BlockTags.INCORRECT_FOR_COPPER_TOOL, 190, 5.0F, 1.0F, 13, ItemTags.COPPER_TOOL_MATERIALS)
static LIBMATTI_MC_ToolMaterial COPPER = {NULL, 190, 5.0f, 1.0f, 13, NULL};
// Java: IRON = (BlockTags.INCORRECT_FOR_IRON_TOOL, 250, 6.0F, 2.0F, 14, ItemTags.IRON_TOOL_MATERIALS)
static LIBMATTI_MC_ToolMaterial IRON = {NULL, 250, 6.0f, 2.0f, 14, NULL};
// Java: DIAMOND = (BlockTags.INCORRECT_FOR_DIAMOND_TOOL, 1561, 8.0F, 3.0F, 10, ItemTags.DIAMOND_TOOL_MATERIALS)
static LIBMATTI_MC_ToolMaterial DIAMOND = {NULL, 1561, 8.0f, 3.0f, 10, NULL};
// Java: GOLD = (BlockTags.INCORRECT_FOR_GOLD_TOOL, 32, 12.0F, 0.0F, 22, ItemTags.GOLD_TOOL_MATERIALS)
static LIBMATTI_MC_ToolMaterial GOLD = {NULL, 32, 12.0f, 0.0f, 22, NULL};
// Java: NETHERITE = (BlockTags.INCORRECT_FOR_NETHERITE_TOOL, 2031, 9.0F, 4.0F, 15, ItemTags.NETHERITE_TOOL_MATERIALS)
static LIBMATTI_MC_ToolMaterial NETHERITE = {NULL, 2031, 9.0f, 4.0f, 15, NULL};

// The tag fields are filled lazily once TagKey interning is used elsewhere too.
static int tags_ready;

static void ensure_tags(void)
{
    if (tags_ready)
        return;
    WOOD.incorrectBlocksForDrops =
        LIBMATTI_MC_TagKey_CreateBlock("minecraft:incorrect_for_wooden_tool");
    WOOD.repairItems =
        LIBMATTI_MC_TagKey_CreateItem("minecraft:wooden_tool_materials");
    STONE.incorrectBlocksForDrops =
        LIBMATTI_MC_TagKey_CreateBlock("minecraft:incorrect_for_stone_tool");
    STONE.repairItems =
        LIBMATTI_MC_TagKey_CreateItem("minecraft:stone_tool_materials");
    COPPER.incorrectBlocksForDrops =
        LIBMATTI_MC_TagKey_CreateBlock("minecraft:incorrect_for_copper_tool");
    COPPER.repairItems =
        LIBMATTI_MC_TagKey_CreateItem("minecraft:copper_tool_materials");
    IRON.incorrectBlocksForDrops =
        LIBMATTI_MC_TagKey_CreateBlock("minecraft:incorrect_for_iron_tool");
    IRON.repairItems =
        LIBMATTI_MC_TagKey_CreateItem("minecraft:iron_tool_materials");
    DIAMOND.incorrectBlocksForDrops =
        LIBMATTI_MC_TagKey_CreateBlock("minecraft:incorrect_for_diamond_tool");
    DIAMOND.repairItems =
        LIBMATTI_MC_TagKey_CreateItem("minecraft:diamond_tool_materials");
    GOLD.incorrectBlocksForDrops =
        LIBMATTI_MC_TagKey_CreateBlock("minecraft:incorrect_for_gold_tool");
    GOLD.repairItems =
        LIBMATTI_MC_TagKey_CreateItem("minecraft:gold_tool_materials");
    NETHERITE.incorrectBlocksForDrops =
        LIBMATTI_MC_TagKey_CreateBlock("minecraft:incorrect_for_netherite_tool");
    NETHERITE.repairItems =
        LIBMATTI_MC_TagKey_CreateItem("minecraft:netherite_tool_materials");
    tags_ready = 1;
}

const LIBMATTI_MC_ToolMaterial *LIBMATTI_MC_ToolMaterial_WOOD(void)
{
    ensure_tags();
    return &WOOD;
}

const LIBMATTI_MC_ToolMaterial *LIBMATTI_MC_ToolMaterial_STONE(void)
{
    ensure_tags();
    return &STONE;
}

const LIBMATTI_MC_ToolMaterial *LIBMATTI_MC_ToolMaterial_COPPER(void)
{
    ensure_tags();
    return &COPPER;
}

const LIBMATTI_MC_ToolMaterial *LIBMATTI_MC_ToolMaterial_IRON(void)
{
    ensure_tags();
    return &IRON;
}

const LIBMATTI_MC_ToolMaterial *LIBMATTI_MC_ToolMaterial_DIAMOND(void)
{
    ensure_tags();
    return &DIAMOND;
}

const LIBMATTI_MC_ToolMaterial *LIBMATTI_MC_ToolMaterial_GOLD(void)
{
    ensure_tags();
    return &GOLD;
}

const LIBMATTI_MC_ToolMaterial *LIBMATTI_MC_ToolMaterial_NETHERITE(void)
{
    ensure_tags();
    return &NETHERITE;
}

void LIBMATTI_MC_ToolMaterial_ApplyCommonProperties(const LIBMATTI_MC_ToolMaterial *material,
                                                    LIBMATTI_MC_ItemProperties *properties)
{
    // Java: p.durability(this.durability).repairable(this.repairItems).enchantable(this.enchantmentValue)
    LIBMATTI_MC_ItemProperties_Durability(properties, material->durability);
    LIBMATTI_MC_ItemProperties_Enchantable(properties, material->enchantmentValue);
}
