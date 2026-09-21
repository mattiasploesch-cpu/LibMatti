// Port of net.minecraft.world.item.ToolMaterial.

#ifndef MATTICRAFT_MC_WORLD_ITEM_TOOLMATERIAL_H
#define MATTICRAFT_MC_WORLD_ITEM_TOOLMATERIAL_H

#include "libmatti/net/minecraft/tags/TagKey.h"
#include "libmatti/net/minecraft/world/item/Item.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public record ToolMaterial(TagKey<Block> incorrectBlocksForDrops, int durability,
// float speed, float attackDamageBonus, int enchantmentValue, TagKey<Item> repairItems)
typedef struct LIBMATTI_MC_ToolMaterial
{
    LIBMATTI_MC_TagKey *incorrectBlocksForDrops;
    int durability;
    float speed;
    float attackDamageBonus;
    int enchantmentValue;
    LIBMATTI_MC_TagKey *repairItems;
} LIBMATTI_MC_ToolMaterial;

// Java: the seven static constants (BlockTags.INCORRECT_FOR_* / ItemTags.*_TOOL_MATERIALS)
const LIBMATTI_MC_ToolMaterial *LIBMATTI_MC_ToolMaterial_WOOD(void);
const LIBMATTI_MC_ToolMaterial *LIBMATTI_MC_ToolMaterial_STONE(void);
const LIBMATTI_MC_ToolMaterial *LIBMATTI_MC_ToolMaterial_COPPER(void);
const LIBMATTI_MC_ToolMaterial *LIBMATTI_MC_ToolMaterial_IRON(void);
const LIBMATTI_MC_ToolMaterial *LIBMATTI_MC_ToolMaterial_DIAMOND(void);
const LIBMATTI_MC_ToolMaterial *LIBMATTI_MC_ToolMaterial_GOLD(void);
const LIBMATTI_MC_ToolMaterial *LIBMATTI_MC_ToolMaterial_NETHERITE(void);

// Java: private Item.Properties applyCommonProperties(Item.Properties) - durability +
// repairable + enchantable. The Repairable payload model is game-port content, the
// component is not set here yet.
void LIBMATTI_MC_ToolMaterial_ApplyCommonProperties(const LIBMATTI_MC_ToolMaterial *material,
                                                    LIBMATTI_MC_ItemProperties *properties);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_ITEM_TOOLMATERIAL_H
