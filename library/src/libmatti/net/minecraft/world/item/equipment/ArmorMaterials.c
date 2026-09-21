// Port of net.minecraft.world.item.equipment.ArmorMaterials.

#include "libmatti/net/minecraft/world/item/equipment/ArmorMaterials.h"

// Java: makeDefense(boots, leggings, chestplate, helmet, body) fills the map;
// the struct stores defense[ArmorType.ordinal()]
#define MAKE_DEFENSE(boots, leggings, chestplate, helmet, body) \
    {body, helmet, leggings, boots, chestplate}

// Java: LEATHER = new ArmorMaterial(5, makeDefense(1, 2, 3, 1, 3), 15, SoundEvents.ARMOR_EQUIP_LEATHER, 0.0F, 0.0F, ItemTags.REPAIRS_LEATHER_ARMOR, EquipmentAssets.LEATHER)
// (mutable - ensure_tags fills the interned tag fields lazily)
static LIBMATTI_MC_ArmorMaterial LEATHER = {5, MAKE_DEFENSE(1, 2, 3, 1, 3), 15, "minecraft:item.armor.equip_leather", 0.0f, 0.0f, NULL, "minecraft:leather"};
// Java: COPPER = (11, makeDefense(1, 3, 4, 2, 4), 8, ARMOR_EQUIP_COPPER, 0.0F, 0.0F, REPAIRS_COPPER_ARMOR, EquipmentAssets.COPPER)
static LIBMATTI_MC_ArmorMaterial COPPER = {11, MAKE_DEFENSE(1, 3, 4, 2, 4), 8, "minecraft:item.armor.equip_copper", 0.0f, 0.0f, NULL, "minecraft:copper"};
// Java: CHAINMAIL = (15, makeDefense(1, 4, 5, 2, 4), 12, ARMOR_EQUIP_CHAIN, 0.0F, 0.0F, REPAIRS_CHAIN_ARMOR, EquipmentAssets.CHAINMAIL)
static LIBMATTI_MC_ArmorMaterial CHAINMAIL = {15, MAKE_DEFENSE(1, 4, 5, 2, 4), 12, "minecraft:item.armor.equip_chain", 0.0f, 0.0f, NULL, "minecraft:chainmail"};
// Java: IRON = (15, makeDefense(2, 5, 6, 2, 5), 9, ARMOR_EQUIP_IRON, 0.0F, 0.0F, REPAIRS_IRON_ARMOR, EquipmentAssets.IRON)
static LIBMATTI_MC_ArmorMaterial IRON = {15, MAKE_DEFENSE(2, 5, 6, 2, 5), 9, "minecraft:item.armor.equip_iron", 0.0f, 0.0f, NULL, "minecraft:iron"};
// Java: GOLD = (7, makeDefense(1, 3, 5, 2, 7), 25, ARMOR_EQUIP_GOLD, 0.0F, 0.0F, REPAIRS_GOLD_ARMOR, EquipmentAssets.GOLD)
static LIBMATTI_MC_ArmorMaterial GOLD = {7, MAKE_DEFENSE(1, 3, 5, 2, 7), 25, "minecraft:item.armor.equip_gold", 0.0f, 0.0f, NULL, "minecraft:gold"};
// Java: DIAMOND = (33, makeDefense(3, 6, 8, 3, 11), 10, ARMOR_EQUIP_DIAMOND, 2.0F, 0.0F, REPAIRS_DIAMOND_ARMOR, EquipmentAssets.DIAMOND)
static LIBMATTI_MC_ArmorMaterial DIAMOND = {33, MAKE_DEFENSE(3, 6, 8, 3, 11), 10, "minecraft:item.armor.equip_diamond", 2.0f, 0.0f, NULL, "minecraft:diamond"};
// Java: TURTLE_SCUTE = (25, makeDefense(2, 5, 6, 2, 5), 9, ARMOR_EQUIP_TURTLE, 0.0F, 0.0F, REPAIRS_TURTLE_HELMET, EquipmentAssets.TURTLE_SCUTE)
static LIBMATTI_MC_ArmorMaterial TURTLE_SCUTE = {25, MAKE_DEFENSE(2, 5, 6, 2, 5), 9, "minecraft:item.armor.equip_turtle", 0.0f, 0.0f, NULL, "minecraft:turtle_scute"};
// Java: NETHERITE = (37, makeDefense(3, 6, 8, 3, 19), 15, ARMOR_EQUIP_NETHERITE, 3.0F, 0.1F, REPAIRS_NETHERITE_ARMOR, EquipmentAssets.NETHERITE)
static LIBMATTI_MC_ArmorMaterial NETHERITE = {37, MAKE_DEFENSE(3, 6, 8, 3, 19), 15, "minecraft:item.armor.equip_netherite", 3.0f, 0.1f, NULL, "minecraft:netherite"};
// Java: ARMADILLO_SCUTE = (4, makeDefense(3, 6, 8, 3, 11), 10, ARMOR_EQUIP_WOLF, 0.0F, 0.0F, REPAIRS_WOLF_ARMOR, EquipmentAssets.ARMADILLO_SCUTE)
static LIBMATTI_MC_ArmorMaterial ARMADILLO_SCUTE = {4, MAKE_DEFENSE(3, 6, 8, 3, 11), 10, "minecraft:item.armor.equip_wolf", 0.0f, 0.0f, NULL, "minecraft:armadillo_scute"};

// The repairIngredient tags are interned lazily (TagKey interning is global).
static int tags_ready;

static void ensure_tags(void)
{
    if (tags_ready)
        return;
    LEATHER.repairIngredient = LIBMATTI_MC_TagKey_CreateItem("minecraft:repairs_leather_armor");
    COPPER.repairIngredient = LIBMATTI_MC_TagKey_CreateItem("minecraft:repairs_copper_armor");
    CHAINMAIL.repairIngredient = LIBMATTI_MC_TagKey_CreateItem("minecraft:repairs_chain_armor");
    IRON.repairIngredient = LIBMATTI_MC_TagKey_CreateItem("minecraft:repairs_iron_armor");
    GOLD.repairIngredient = LIBMATTI_MC_TagKey_CreateItem("minecraft:repairs_gold_armor");
    DIAMOND.repairIngredient = LIBMATTI_MC_TagKey_CreateItem("minecraft:repairs_diamond_armor");
    TURTLE_SCUTE.repairIngredient = LIBMATTI_MC_TagKey_CreateItem("minecraft:repairs_turtle_helmet");
    NETHERITE.repairIngredient = LIBMATTI_MC_TagKey_CreateItem("minecraft:repairs_netherite_armor");
    ARMADILLO_SCUTE.repairIngredient = LIBMATTI_MC_TagKey_CreateItem("minecraft:repairs_wolf_armor");
    tags_ready = 1;
}

const LIBMATTI_MC_ArmorMaterial *LIBMATTI_MC_ArmorMaterials_LEATHER(void)
{
    ensure_tags();
    return &LEATHER;
}

const LIBMATTI_MC_ArmorMaterial *LIBMATTI_MC_ArmorMaterials_COPPER(void)
{
    ensure_tags();
    return &COPPER;
}

const LIBMATTI_MC_ArmorMaterial *LIBMATTI_MC_ArmorMaterials_CHAINMAIL(void)
{
    ensure_tags();
    return &CHAINMAIL;
}

const LIBMATTI_MC_ArmorMaterial *LIBMATTI_MC_ArmorMaterials_IRON(void)
{
    ensure_tags();
    return &IRON;
}

const LIBMATTI_MC_ArmorMaterial *LIBMATTI_MC_ArmorMaterials_GOLD(void)
{
    ensure_tags();
    return &GOLD;
}

const LIBMATTI_MC_ArmorMaterial *LIBMATTI_MC_ArmorMaterials_DIAMOND(void)
{
    ensure_tags();
    return &DIAMOND;
}

const LIBMATTI_MC_ArmorMaterial *LIBMATTI_MC_ArmorMaterials_TURTLE_SCUTE(void)
{
    ensure_tags();
    return &TURTLE_SCUTE;
}

const LIBMATTI_MC_ArmorMaterial *LIBMATTI_MC_ArmorMaterials_NETHERITE(void)
{
    ensure_tags();
    return &NETHERITE;
}

const LIBMATTI_MC_ArmorMaterial *LIBMATTI_MC_ArmorMaterials_ARMADILLO_SCUTE(void)
{
    ensure_tags();
    return &ARMADILLO_SCUTE;
}
