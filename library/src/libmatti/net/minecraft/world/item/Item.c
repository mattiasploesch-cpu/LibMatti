// Port of net.minecraft.world.item.Item (the data part).

#include "libmatti/net/minecraft/world/item/Item.h"

#include "libmatti/net/minecraft/core/component/DataComponents.h"
#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/net/minecraft/world/item/Rarity.h"
#include "libmatti/net/minecraft/world/item/ToolMaterial.h"
#include "libmatti/net/minecraft/world/item/equipment/ArmorMaterials.h"
#include "libmatti/net/minecraft/world/item/equipment/ArmorType.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: public Properties()
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_New(void)
{
    LIBMATTI_MC_ItemProperties *properties = calloc(1, sizeof(LIBMATTI_MC_ItemProperties));
    // Java: DataComponentMap.builder().addAll(DataComponents.COMMON_ITEM_COMPONENTS)
    properties->components = LIBMATTI_MC_DataComponentMap_Copy(LIBMATTI_MC_DataComponents_CommonItemComponents());
    return properties;
}

// Java autoboxes every int component value; the port boxes into the properties'
// own arena so the value outlives the call and stays per-properties
static int *box_int(LIBMATTI_MC_ItemProperties *properties, int value)
{
    if (properties->boxedIntCount == properties->boxedIntCapacity)
    {
        properties->boxedIntCapacity = properties->boxedIntCapacity > 0 ? properties->boxedIntCapacity * 2 : 8;
        properties->boxedInts = realloc(properties->boxedInts, sizeof(int) * properties->boxedIntCapacity);
    }
    properties->boxedInts[properties->boxedIntCount] = value;
    return &properties->boxedInts[properties->boxedIntCount++];
}

// Java: public Properties stacksTo(int) = component(MAX_STACK_SIZE, size)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_StacksTo(LIBMATTI_MC_ItemProperties *properties, int size)
{
    return LIBMATTI_MC_ItemProperties_Component(properties, LIBMATTI_MC_DataComponents_MAX_STACK_SIZE(), box_int(properties, size));
}

// Java: public Properties durability(int)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_Durability(LIBMATTI_MC_ItemProperties *properties, int durability)
{
    LIBMATTI_MC_ItemProperties_Component(properties, LIBMATTI_MC_DataComponents_MAX_DAMAGE(), box_int(properties, durability));
    LIBMATTI_MC_ItemProperties_Component(properties, LIBMATTI_MC_DataComponents_MAX_STACK_SIZE(), box_int(properties, 1));
    LIBMATTI_MC_ItemProperties_Component(properties, LIBMATTI_MC_DataComponents_DAMAGE(), box_int(properties, 0));
    return properties;
}

// Java: public Properties craftRemainder(Item)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_CraftRemainder(LIBMATTI_MC_ItemProperties *properties,
                                                                      LIBMATTI_MC_Item *remainder)
{
    properties->craftingRemainingItem = remainder;
    return properties;
}

// Java: public Properties component(DataComponentType<T>, T)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_Component(LIBMATTI_MC_ItemProperties *properties,
                                                                 const LIBMATTI_MC_DataComponentType *type, void *value)
{
    LIBMATTI_MC_DataComponentMap_Set(properties->components, type, value);
    return properties;
}

// Java: public Properties overrideDescription(String)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_OverrideDescription(LIBMATTI_MC_ItemProperties *properties,
                                                                           const char *description)
{
    free(properties->descriptionId);
    properties->descriptionId = strdup(description);
    return properties;
}

// Java: public Properties useBlockDescriptionPrefix()
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_UseBlockDescriptionPrefix(LIBMATTI_MC_ItemProperties *properties)
{
    properties->useBlockDescriptionPrefix = true;
    return properties;
}

// Java: public Properties useItemDescriptionPrefix()
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_UseItemDescriptionPrefix(LIBMATTI_MC_ItemProperties *properties)
{
    properties->useBlockDescriptionPrefix = false;
    return properties;
}

// Java: public Properties setId(ResourceKey<Item>)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_SetId(LIBMATTI_MC_ItemProperties *properties,
                                                             LIBMATTI_MC_ResourceKey *id)
{
    properties->id = id;
    return properties;
}

// Java: public Properties enchantable(int) = component(ENCHANTABLE, new Enchantable(value))
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_Enchantable(LIBMATTI_MC_ItemProperties *properties, int value)
{
    return LIBMATTI_MC_ItemProperties_Component(properties, LIBMATTI_MC_DataComponents_ENCHANTABLE(),
                                                box_int(properties, value));
}

// Java: public Properties rarity(Rarity) = component(RARITY, rarity)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_Rarity(LIBMATTI_MC_ItemProperties *properties, int rarity)
{
    return LIBMATTI_MC_ItemProperties_Component(properties, LIBMATTI_MC_DataComponents_RARITY(),
                                                box_int(properties, rarity));
}

// Java: public Properties fireResistant() = component(DAMAGE_RESISTANT, new DamageResistant(DamageTypeTags.IS_FIRE))
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_FireResistant(LIBMATTI_MC_ItemProperties *properties)
{
    // The DamageResistant record wraps exactly one tag; the port carries the tag location
    return LIBMATTI_MC_ItemProperties_Component(properties, LIBMATTI_MC_DataComponents_DAMAGE_RESISTANT(),
                                                (void *) "minecraft:is_fire");
}

// Java: ToolMaterial.applyToolProperties applies durability + enchantable and the
// TOOL/WEAPON/attribute payloads; the payload models are game-port content
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_Tool(LIBMATTI_MC_ItemProperties *properties,
                                                            const struct LIBMATTI_MC_ToolMaterial *material)
{
    LIBMATTI_MC_ToolMaterial_ApplyCommonProperties((const LIBMATTI_MC_ToolMaterial *) material, properties);
    return properties;
}

// Java: public Properties sword(...) = material.applySwordProperties(this, damage, speed)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_Sword(LIBMATTI_MC_ItemProperties *properties,
                                                             const struct LIBMATTI_MC_ToolMaterial *material, float attackDamage, float attackSpeed)
{
    (void) attackDamage;
    (void) attackSpeed;
    return LIBMATTI_MC_ItemProperties_Tool(properties, material);
}

// Java: public Properties pickaxe(...) = tool(material, MINEABLE_WITH_PICKAXE, speed, damage, 0.0F)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_Pickaxe(LIBMATTI_MC_ItemProperties *properties,
                                                               const struct LIBMATTI_MC_ToolMaterial *material, float attackDamage, float attackSpeed)
{
    (void) attackDamage;
    (void) attackSpeed;
    return LIBMATTI_MC_ItemProperties_Tool(properties, material);
}

// Java: public Properties shovel(...) = tool(material, MINEABLE_WITH_SHOVEL, speed, damage, 0.0F)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_Shovel(LIBMATTI_MC_ItemProperties *properties,
                                                              const struct LIBMATTI_MC_ToolMaterial *material, float attackDamage, float attackSpeed)
{
    (void) attackDamage;
    (void) attackSpeed;
    return LIBMATTI_MC_ItemProperties_Tool(properties, material);
}

// Java: public Properties axe(...) = tool(material, MINEABLE_WITH_AXE, speed, damage, 5.0F)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_Axe(LIBMATTI_MC_ItemProperties *properties,
                                                           const struct LIBMATTI_MC_ToolMaterial *material, float attackDamage, float attackSpeed)
{
    (void) attackDamage;
    (void) attackSpeed;
    return LIBMATTI_MC_ItemProperties_Tool(properties, material);
}

// Java: public Properties hoe(...) = tool(material, MINEABLE_WITH_HOE, speed, damage, 0.0F)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_Hoe(LIBMATTI_MC_ItemProperties *properties,
                                                           const struct LIBMATTI_MC_ToolMaterial *material, float attackDamage, float attackSpeed)
{
    (void) attackDamage;
    (void) attackSpeed;
    return LIBMATTI_MC_ItemProperties_Tool(properties, material);
}

// Java: public Properties humanoidArmor(ArmorMaterial, ArmorType) = durability(type.getDurability(
// material.durability)).attributes(...).enchantable(material.enchantmentValue()).component(EQUIPPABLE, ...)
// (the attribute/EQUIPPABLE payloads are game-port content)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_HumanoidArmor(LIBMATTI_MC_ItemProperties *properties,
                                                                     const struct LIBMATTI_MC_ArmorMaterial *material, int armorType)
{
    const LIBMATTI_MC_ArmorMaterial *mat = (const LIBMATTI_MC_ArmorMaterial *) material;
    LIBMATTI_MC_ItemProperties_Durability(properties,
                                          LIBMATTI_MC_ArmorType_GetDurability((LIBMATTI_MC_ArmorType) armorType, mat->durability));
    LIBMATTI_MC_ItemProperties_Enchantable(properties, mat->enchantmentValue);
    return properties;
}

// Java: public Properties wolfArmor(ArmorMaterial) = durability(BODY.getDurability(
// material.durability)).attributes(...).repairable(...).component(EQUIPPABLE, ...)
LIBMATTI_MC_ItemProperties *LIBMATTI_MC_ItemProperties_WolfArmor(LIBMATTI_MC_ItemProperties *properties,
                                                                 const struct LIBMATTI_MC_ArmorMaterial *material)
{
    const LIBMATTI_MC_ArmorMaterial *mat = (const LIBMATTI_MC_ArmorMaterial *) material;
    LIBMATTI_MC_ItemProperties_Durability(properties,
                                          LIBMATTI_MC_ArmorType_GetDurability(LIBMATTI_MC_ArmorType_BODY, mat->durability));
    LIBMATTI_MC_ItemProperties_Enchantable(properties, mat->enchantmentValue);
    return properties;
}

// Java: Item::new - the constructor resolves descriptionId through the dependant names
LIBMATTI_MC_Item *LIBMATTI_MC_Item_New(LIBMATTI_MC_ItemProperties *properties)
{
    LIBMATTI_MC_Item *item = calloc(1, sizeof(LIBMATTI_MC_Item));
    item->properties = properties;
    item->components = properties != NULL ? properties->components : LIBMATTI_MC_DataComponentMap_Empty();
    if (item->components == NULL)
        item->components = LIBMATTI_MC_DataComponentMap_Empty();
    item->key = properties != NULL ? properties->id : NULL;

    // Java: descriptionId defaults through ITEM_DESCRIPTION_ID / BLOCK_DESCRIPTION_ID
    // (Util.makeDescriptionId("item"/"block", key.identifier()))
    if (properties != NULL && properties->descriptionId != NULL)
    {
        item->descriptionId = strdup(properties->descriptionId);
    }
    else if (properties != NULL && properties->useBlockDescriptionPrefix && item->key != NULL)
    {
        const LIBMATTI_MC_Identifier *identifier = LIBMATTI_MC_ResourceKey_Identifier(item->key);
        char *path = LIBMATTI_MC_Identifier_ToString((LIBMATTI_MC_Identifier *) identifier);
        size_t length = strlen("block.") + strlen(path) + 1;
        item->descriptionId = malloc(length);
        snprintf(item->descriptionId, length, "block.%s", path);
        free(path);
    }
    else if (item->key != NULL)
    {
        const LIBMATTI_MC_Identifier *identifier = LIBMATTI_MC_ResourceKey_Identifier(item->key);
        char *path = LIBMATTI_MC_Identifier_ToString((LIBMATTI_MC_Identifier *) identifier);
        size_t length = strlen("item.") + strlen(path) + 1;
        item->descriptionId = malloc(length);
        snprintf(item->descriptionId, length, "item.%s", path);
        free(path);
    }
    return item;
}

// Java: BlockItem::new
LIBMATTI_MC_Item *LIBMATTI_MC_BlockItem_New(void *block, LIBMATTI_MC_ItemProperties *properties)
{
    LIBMATTI_MC_Item *item = LIBMATTI_MC_Item_New(properties);
    item->block = block;
    return item;
}

// Java: public final DataComponentMap components()
const LIBMATTI_MC_DataComponentMap *LIBMATTI_MC_Item_GetComponents(const LIBMATTI_MC_Item *item)
{
    if (item == NULL || item->components == NULL)
        return LIBMATTI_MC_DataComponentMap_Empty();
    return item->components;
}

// Java: public final String getDescriptionId()
const char *LIBMATTI_MC_Item_GetDescriptionId(const LIBMATTI_MC_Item *item)
{
    return item != NULL ? item->descriptionId : NULL;
}

// Java: public final Item asItem()
LIBMATTI_MC_Item *LIBMATTI_MC_Item_AsItem(LIBMATTI_MC_Item *item)
{
    return item;
}
