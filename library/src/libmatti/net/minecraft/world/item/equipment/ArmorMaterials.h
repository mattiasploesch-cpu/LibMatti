// Port of net.minecraft.world.item.equipment.ArmorMaterial and ArmorMaterials.

#ifndef MATTICRAFT_MC_WORLD_ITEM_EQUIPMENT_ARMORMATERIALS_H
#define MATTICRAFT_MC_WORLD_ITEM_EQUIPMENT_ARMORMATERIALS_H

#include "libmatti/net/minecraft/tags/TagKey.h"
#include "libmatti/net/minecraft/world/item/equipment/ArmorType.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public record ArmorMaterial(int durability, Map<ArmorType, Integer> defense,
// int enchantmentValue, Holder<SoundEvent> equipSound, float toughness,
// float knockbackResistance, TagKey<Item> repairIngredient, ResourceKey<EquipmentAsset> assetId)
typedef struct LIBMATTI_MC_ArmorMaterial
{
    int durability;
    // Java: Map<ArmorType, Integer> defense - indexed by the ArmorType ordinal
    int defense[5];
    int enchantmentValue;
    // Java: Holder<SoundEvent> equipSound - the sound location
    const char *equipSound;
    float toughness;
    float knockbackResistance;
    LIBMATTI_MC_TagKey *repairIngredient;
    const char *assetId;
} LIBMATTI_MC_ArmorMaterial;

// Java: the interface constants in ArmorMaterials (makeDefense order:
// boots, leggings, chestplate, helmet, body)
const LIBMATTI_MC_ArmorMaterial *LIBMATTI_MC_ArmorMaterials_LEATHER(void);
const LIBMATTI_MC_ArmorMaterial *LIBMATTI_MC_ArmorMaterials_COPPER(void);
const LIBMATTI_MC_ArmorMaterial *LIBMATTI_MC_ArmorMaterials_CHAINMAIL(void);
const LIBMATTI_MC_ArmorMaterial *LIBMATTI_MC_ArmorMaterials_IRON(void);
const LIBMATTI_MC_ArmorMaterial *LIBMATTI_MC_ArmorMaterials_GOLD(void);
const LIBMATTI_MC_ArmorMaterial *LIBMATTI_MC_ArmorMaterials_DIAMOND(void);
const LIBMATTI_MC_ArmorMaterial *LIBMATTI_MC_ArmorMaterials_TURTLE_SCUTE(void);
const LIBMATTI_MC_ArmorMaterial *LIBMATTI_MC_ArmorMaterials_NETHERITE(void);
const LIBMATTI_MC_ArmorMaterial *LIBMATTI_MC_ArmorMaterials_ARMADILLO_SCUTE(void);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_ITEM_EQUIPMENT_ARMORMATERIALS_H
