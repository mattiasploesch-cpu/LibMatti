// Port of net.minecraft.world.item.Item (the data part: Properties, components,
// descriptionId, registry id). Java's behaviour methods (use, interact, ...) stay
// with the game port.

#ifndef MATTICRAFT_MC_WORLD_ITEM_ITEM_H
#define MATTICRAFT_MC_WORLD_ITEM_ITEM_H

#include "libmatti/net/minecraft/core/component/DataComponentMap.h"
#include "libmatti/net/minecraft/resources/ResourceKey.h"

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// forward tags for the material parameters below (defined in ToolMaterial.h /
// equipment/ArmorMaterials.h - both include this header)
struct LIBMATTI_MC_ToolMaterial;
struct LIBMATTI_MC_ArmorMaterial;

// Java: public class Item
typedef struct LIBMATTI_MC_Item
{
    // Java: the registered key (Properties.setId in the register path)
    LIBMATTI_MC_ResourceKey *key;
    // Java: BlockItem.block / Item.BY_BLOCK - the block a BlockItem places (NULL otherwise)
    void *block;
    // Java: private final DataComponentMap components
    const LIBMATTI_MC_DataComponentMap *components;
    // Java: private final String descriptionId (resolved at register time)
    char *descriptionId;
    // Java: private final Properties properties - the builder record the item was built from
    struct LIBMATTI_MC_ItemProperties *properties;
} LIBMATTI_MC_Item;

// Java: public static class Properties
typedef struct LIBMATTI_MC_ItemProperties
{
    // Java: private final DataComponentMap.Builder components (starts from COMMON_ITEM_COMPONENTS)
    LIBMATTI_MC_DataComponentMap *components;
    // Java: @Nullable Item craftingRemainingItem
    LIBMATTI_MC_Item *craftingRemainingItem;
    // Java: @Nullable ResourceKey<Item> id
    LIBMATTI_MC_ResourceKey *id;
    // Java: private String descriptionId - NULL until overrideDescription/useBlockDescriptionPrefix
    char *descriptionId;
    // Java: private boolean useBlockDescriptionPrefix
    bool useBlockDescriptionPrefix;
    // Java autoboxes int component values; the port boxes them into a per-properties
    // arena so every properties object owns its values (the map stores void* only)
    int *boxedInts;
    size_t boxedIntCount;
    size_t boxedIntCapacity;
} LIBMATTI_MC_ItemProperties;

// Java: public Properties()
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_New(void);

// Java: public Properties stacksTo(int) - component(MAX_STACK_SIZE, value)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_StacksTo(LIBMATTI_MC_ItemProperties *properties, int size);
// Java: public Properties durability(int) - MAX_DAMAGE + MAX_STACK_SIZE 1 + DAMAGE 0
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_Durability(LIBMATTI_MC_ItemProperties *properties, int durability);
// Java: public Properties enchantable(int) - component(ENCHANTABLE, new Enchantable(value))
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_Enchantable(LIBMATTI_MC_ItemProperties *properties, int value);
// Java: public Properties rarity(Rarity) - component(RARITY, rarity)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_Rarity(LIBMATTI_MC_ItemProperties *properties, int rarity);
// Java: public Properties fireResistant() - component(DAMAGE_RESISTANT, new DamageResistant(DamageTypeTags.IS_FIRE))
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_FireResistant(LIBMATTI_MC_ItemProperties *properties);
// Java: public Properties tool(ToolMaterial, TagKey<Block>, float, float, float) - the common
// tool path the sword/pickaxe/shovel/axe/hoe builders funnel into (sets durability +
// enchantable; the TOOL/WEAPON/attributes payloads are game-port content)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_Tool(LIBMATTI_MC_ItemProperties *properties,
                                                            const struct LIBMATTI_MC_ToolMaterial *material);
// Java: public Properties sword/pickaxe/shovel/axe/hoe(ToolMaterial, float, float)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_Sword(LIBMATTI_MC_ItemProperties *properties,
                                                             const struct LIBMATTI_MC_ToolMaterial *material, float attackDamage, float attackSpeed);
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_Pickaxe(LIBMATTI_MC_ItemProperties *properties,
                                                               const struct LIBMATTI_MC_ToolMaterial *material, float attackDamage, float attackSpeed);
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_Shovel(LIBMATTI_MC_ItemProperties *properties,
                                                              const struct LIBMATTI_MC_ToolMaterial *material, float attackDamage, float attackSpeed);
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_Axe(LIBMATTI_MC_ItemProperties *properties,
                                                           const struct LIBMATTI_MC_ToolMaterial *material, float attackDamage, float attackSpeed);
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_Hoe(LIBMATTI_MC_ItemProperties *properties,
                                                           const struct LIBMATTI_MC_ToolMaterial *material, float attackDamage, float attackSpeed);
// Java: public Properties humanoidArmor(ArmorMaterial, ArmorType)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_HumanoidArmor(LIBMATTI_MC_ItemProperties *properties,
                                                                     const struct LIBMATTI_MC_ArmorMaterial *material, int armorType);
// Java: public Properties wolfArmor(ArmorMaterial)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_WolfArmor(LIBMATTI_MC_ItemProperties *properties,
                                                                 const struct LIBMATTI_MC_ArmorMaterial *material);
// Java: public Properties craftRemainder(Item)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_CraftRemainder(LIBMATTI_MC_ItemProperties *properties,
                                                                      LIBMATTI_MC_Item *remainder);
// Java: public Properties component(DataComponentType<T>, T) - the generic setter the
// specialized builders funnel into (food/rarity/tool/... carry typed payloads the
// game port owns)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_Component(LIBMATTI_MC_ItemProperties *properties,
                                                                 const LIBMATTI_MC_DataComponentType *type, void *value);
// Java: public Properties overrideDescription(String)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_OverrideDescription(LIBMATTI_MC_ItemProperties *properties,
                                                                           const char *description);
// Java: public Properties useBlockDescriptionPrefix() / useItemDescriptionPrefix()
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_UseBlockDescriptionPrefix(LIBMATTI_MC_ItemProperties *properties);
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_UseItemDescriptionPrefix(LIBMATTI_MC_ItemProperties *properties);
// Java: public Properties setId(ResourceKey<Item>)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_SetId(LIBMATTI_MC_ItemProperties *properties,
                                                             LIBMATTI_MC_ResourceKey *id);

// Java: Item::new - the port takes the finished properties
LIBMATTI_MC_Item *LIBMATTI_MC_Item_New(LIBMATTI_MC_ItemProperties *properties);
// Java: BlockItem::new - carries the placed block
LIBMATTI_MC_Item *LIBMATTI_MC_BlockItem_New(void *block, LIBMATTI_MC_ItemProperties *properties);

// Java: public final DataComponentMap components()
const LIBMATTI_MC_DataComponentMap *LIBMATTI_MC_Item_GetComponents(const LIBMATTI_MC_Item *item);
// Java: public final String getDescriptionId()
const char *LIBMATTI_MC_Item_GetDescriptionId(const LIBMATTI_MC_Item *item);
// Java: Item.BY_BLOCK / asItem()
LIBMATTI_MC_Item *LIBMATTI_MC_Item_AsItem(LIBMATTI_MC_Item *item);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_ITEM_ITEM_H
