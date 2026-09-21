// Port of net.minecraft.world.item.equipment.ArmorType.

#include "libmatti/net/minecraft/world/item/equipment/ArmorType.h"

// Java: HELMET(HEAD, 11, "helmet"), CHESTPLATE(CHEST, 16, "chestplate"),
// LEGGINGS(LEGS, 15, "leggings"), BOOTS(FEET, 13, "boots"), BODY(BODY, 16, "body")
static const struct
{
    int slot;
    int unitDurability;
    const char *name;
} DATA[] = {
    {0, 11, "helmet"},
    {1, 16, "chestplate"},
    {2, 15, "leggings"},
    {3, 13, "boots"},
    {4, 16, "body"},
};

int LIBMATTI_MC_ArmorType_GetDurability(LIBMATTI_MC_ArmorType type, int multiplier)
{
    return DATA[type].unitDurability * multiplier;
}

int LIBMATTI_MC_ArmorType_GetSlot(LIBMATTI_MC_ArmorType type)
{
    return DATA[type].slot;
}

const char *LIBMATTI_MC_ArmorType_GetName(LIBMATTI_MC_ArmorType type)
{
    return DATA[type].name;
}
