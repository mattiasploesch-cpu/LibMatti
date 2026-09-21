// Port of net.minecraft.world.item.Rarity.

#include "libmatti/net/minecraft/world/item/Rarity.h"

#include "libmatti/net/minecraft/ChatFormatting.h"

#include <stddef.h>

// Java: private static final Rarity[] - BY_ID uses a continuous out-of-bounds -> ZERO map
static const LIBMATTI_MC_Rarity VALUES[] = {
    LIBMATTI_MC_Rarity_COMMON,
    LIBMATTI_MC_Rarity_UNCOMMON,
    LIBMATTI_MC_Rarity_RARE,
    LIBMATTI_MC_Rarity_EPIC,
};

const char *LIBMATTI_MC_Rarity_GetSerializedName(LIBMATTI_MC_Rarity rarity)
{
    switch (rarity)
    {
        case LIBMATTI_MC_Rarity_COMMON: return "common";
        case LIBMATTI_MC_Rarity_UNCOMMON: return "uncommon";
        case LIBMATTI_MC_Rarity_RARE: return "rare";
        case LIBMATTI_MC_Rarity_EPIC: return "epic";
    }
    return "common";
}

int LIBMATTI_MC_Rarity_Color(LIBMATTI_MC_Rarity rarity)
{
    switch (rarity)
    {
        case LIBMATTI_MC_Rarity_COMMON: return LIBMATTI_MC_ChatFormatting_WHITE;
        case LIBMATTI_MC_Rarity_UNCOMMON: return LIBMATTI_MC_ChatFormatting_YELLOW;
        case LIBMATTI_MC_Rarity_RARE: return LIBMATTI_MC_ChatFormatting_AQUA;
        case LIBMATTI_MC_Rarity_EPIC: return LIBMATTI_MC_ChatFormatting_LIGHT_PURPLE;
    }
    return LIBMATTI_MC_ChatFormatting_WHITE;
}

LIBMATTI_MC_Rarity LIBMATTI_MC_Rarity_ById(int id)
{
    // Java: ByIdMap.continuous(..., OutOfBoundsStrategy.ZERO)
    if (id < 0 || (size_t) id >= sizeof(VALUES) / sizeof(VALUES[0]))
        return VALUES[0];
    return VALUES[id];
}
