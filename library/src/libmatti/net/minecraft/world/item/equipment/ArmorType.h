// Port of net.minecraft.world.item.equipment.ArmorType.

#ifndef MATTICRAFT_MC_WORLD_ITEM_EQUIPMENT_ARMORTYPE_H
#define MATTICRAFT_MC_WORLD_ITEM_EQUIPMENT_ARMORTYPE_H

#ifdef __cplusplus
extern "C" {
#endif

// Java: public enum ArmorType implements StringRepresentable - the int is the
// unit durability every material multiplies with
typedef enum LIBMATTI_MC_ArmorType
{
    LIBMATTI_MC_ArmorType_HELMET = 0,
    LIBMATTI_MC_ArmorType_CHESTPLATE = 1,
    LIBMATTI_MC_ArmorType_LEGGINGS = 2,
    LIBMATTI_MC_ArmorType_BOOTS = 3,
    LIBMATTI_MC_ArmorType_BODY = 4
} LIBMATTI_MC_ArmorType;

// Java: public int getDurability(int multiplier) - unitDurability * multiplier
int LIBMATTI_MC_ArmorType_GetDurability(LIBMATTI_MC_ArmorType type, int multiplier);
// Java: public EquipmentSlot getSlot() - 0 HEAD, 1 CHEST, 2 LEGS, 3 FEET, 4 BODY
int LIBMATTI_MC_ArmorType_GetSlot(LIBMATTI_MC_ArmorType type);
// Java: public String getName() / getSerializedName()
const char *LIBMATTI_MC_ArmorType_GetName(LIBMATTI_MC_ArmorType type);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_ITEM_EQUIPMENT_ARMORTYPE_H
