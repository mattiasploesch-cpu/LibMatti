// Java: MenuType's static block - the 25 vanilla constants in declaration
// order. Each accessor registers on first touch through MenuType_Register
// (Java: Registry.register in the static initialiser); the concrete menus the
// suppliers resolve arrive with their ports (the screens pass the containers
// explicitly - the registry create() path fills with the inventory port P6.3).

#include "libmatti/net/minecraft/world/inventory/MenuType.h"

#include <stdlib.h>

void LIBMATTI_MC_VanillaMenus_RegisterAll(void)
{
    // Java: the static constants in declaration order (MenuType.java 17-41)
    LIBMATTI_MC_MenuType_GENERIC_9x1();
    LIBMATTI_MC_MenuType_GENERIC_9x2();
    LIBMATTI_MC_MenuType_GENERIC_9x3();
    LIBMATTI_MC_MenuType_GENERIC_9x4();
    LIBMATTI_MC_MenuType_GENERIC_9x5();
    LIBMATTI_MC_MenuType_GENERIC_9x6();
    LIBMATTI_MC_MenuType_GENERIC_3x3();
    LIBMATTI_MC_MenuType_CRAFTER_3x3();
    LIBMATTI_MC_MenuType_ANVIL();
    LIBMATTI_MC_MenuType_BEACON();
    LIBMATTI_MC_MenuType_BLAST_FURNACE();
    LIBMATTI_MC_MenuType_BREWING_STAND();
    LIBMATTI_MC_MenuType_CRAFTING();
    LIBMATTI_MC_MenuType_ENCHANTMENT();
    LIBMATTI_MC_MenuType_FURNACE();
    LIBMATTI_MC_MenuType_GRINDSTONE();
    LIBMATTI_MC_MenuType_HOPPER();
    LIBMATTI_MC_MenuType_LECTERN();
    LIBMATTI_MC_MenuType_LOOM();
    LIBMATTI_MC_MenuType_MERCHANT();
    LIBMATTI_MC_MenuType_SHULKER_BOX();
    LIBMATTI_MC_MenuType_SMITHING();
    LIBMATTI_MC_MenuType_SMOKER();
    LIBMATTI_MC_MenuType_CARTOGRAPHY_TABLE();
    LIBMATTI_MC_MenuType_STONECUTTER();
}

// the lazy accessors - one per constant, in declaration order (Java: the
// static fields). supplier NULL = the typeless constants until their concrete
// menus land (the port's MenuType_Create answers NULL, like Java's
// getType() throw on the typeless InventoryMenu).
#define VANILLA_MENU_TYPE(accessor, name)                                                                          \
    LIBMATTI_MC_MenuType *accessor(void)                                                                           \
    {                                                                                                              \
        static LIBMATTI_MC_MenuType *type = NULL;                                                                  \
        if (type == NULL)                                                                                          \
            type = LIBMATTI_MC_MenuType_Register(name, NULL);                                                      \
        return type;                                                                                               \
    }

VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_GENERIC_9x1, "minecraft:generic_9x1")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_GENERIC_9x2, "minecraft:generic_9x2")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_GENERIC_9x3, "minecraft:generic_9x3")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_GENERIC_9x4, "minecraft:generic_9x4")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_GENERIC_9x5, "minecraft:generic_9x5")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_GENERIC_9x6, "minecraft:generic_9x6")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_GENERIC_3x3, "minecraft:generic_3x3")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_CRAFTER_3x3, "minecraft:crafter_3x3")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_ANVIL, "minecraft:anvil")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_BEACON, "minecraft:beacon")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_BLAST_FURNACE, "minecraft:blast_furnace")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_BREWING_STAND, "minecraft:brewing_stand")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_CRAFTING, "minecraft:crafting")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_ENCHANTMENT, "minecraft:enchantment")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_FURNACE, "minecraft:furnace")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_GRINDSTONE, "minecraft:grindstone")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_HOPPER, "minecraft:hopper")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_LECTERN, "minecraft:lectern")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_LOOM, "minecraft:loom")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_MERCHANT, "minecraft:merchant")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_SHULKER_BOX, "minecraft:shulker_box")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_SMITHING, "minecraft:smithing")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_SMOKER, "minecraft:smoker")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_CARTOGRAPHY_TABLE, "minecraft:cartography_table")
VANILLA_MENU_TYPE(LIBMATTI_MC_MenuType_STONECUTTER, "minecraft:stonecutter")
