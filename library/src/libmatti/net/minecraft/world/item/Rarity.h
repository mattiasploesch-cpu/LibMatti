// Port of net.minecraft.world.item.Rarity.

#ifndef MATTICRAFT_MC_WORLD_ITEM_RARITY_H
#define MATTICRAFT_MC_WORLD_ITEM_RARITY_H

#ifdef __cplusplus
extern "C" {
#endif

// Java: public enum Rarity implements StringRepresentable
typedef enum LIBMATTI_MC_Rarity
{
    LIBMATTI_MC_Rarity_COMMON = 0,
    LIBMATTI_MC_Rarity_UNCOMMON = 1,
    LIBMATTI_MC_Rarity_RARE = 2,
    LIBMATTI_MC_Rarity_EPIC = 3
} LIBMATTI_MC_Rarity;

// Java: public String getSerializedName()
const char *LIBMATTI_MC_Rarity_GetSerializedName(LIBMATTI_MC_Rarity rarity);
// Java: public ChatFormatting color()
int LIBMATTI_MC_Rarity_Color(LIBMATTI_MC_Rarity rarity);
// Java: public static Rarity byId(int)
LIBMATTI_MC_Rarity LIBMATTI_MC_Rarity_ById(int id);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_ITEM_RARITY_H
