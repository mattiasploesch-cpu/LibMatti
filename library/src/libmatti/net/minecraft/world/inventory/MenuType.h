// Port of net.minecraft.world.inventory.MenuType - the registry-data part.
// Java's MenuType carries the MenuSupplier constructor reference; the port
// keeps a factory function pointer the create() call resolves. The constants
// register into BuiltInRegistries.MENU like Java's static block.

#ifndef MATTICRAFT_MC_WORLD_INVENTORY_MENUTYPE_H
#define MATTICRAFT_MC_WORLD_INVENTORY_MENUTYPE_H

#include "libmatti/net/minecraft/core/registries/BuiltInRegistries.h"

#ifdef __cplusplus
extern "C" {
#endif

struct LIBMATTI_MC_AbstractContainerMenu;
struct LIBMATTI_MC_Player;

// Java: MenuType.MenuSupplier<T> - (containerId, inventory) -> menu
typedef struct LIBMATTI_MC_AbstractContainerMenu *(*LIBMATTI_MC_MenuType_MenuSupplier)(
    int containerId, const struct LIBMATTI_MC_Player *player);

// Java: public class MenuType<T extends AbstractContainerMenu>
typedef struct LIBMATTI_MC_MenuType
{
    // the registered key (setId in Java's register() - the port keeps the name)
    char *name;
    // Java: private final MenuType.MenuSupplier<T> constructor
    LIBMATTI_MC_MenuType_MenuSupplier constructor;
} LIBMATTI_MC_MenuType;

// Java: private static MenuType register(String, MenuSupplier) - one call per
// vanilla constant; the returned type is registry-owned (never freed)
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_Register(const char *name, LIBMATTI_MC_MenuType_MenuSupplier constructor);

// Java: public T create(int, Inventory)
struct LIBMATTI_MC_AbstractContainerMenu *LIBMATTI_MC_MenuType_Create(const LIBMATTI_MC_MenuType *type,
                                                                     int containerId,
                                                                     const struct LIBMATTI_MC_Player *player);

// Java: the static constants in declaration order - one accessor per constant,
// each registering on first touch (Java's static block; idempotent like class init)
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_GENERIC_9x1(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_GENERIC_9x2(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_GENERIC_9x3(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_GENERIC_9x4(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_GENERIC_9x5(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_GENERIC_9x6(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_GENERIC_3x3(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_CRAFTER_3x3(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_ANVIL(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_BEACON(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_BLAST_FURNACE(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_BREWING_STAND(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_CRAFTING(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_ENCHANTMENT(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_FURNACE(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_GRINDSTONE(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_HOPPER(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_LECTERN(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_LOOM(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_MERCHANT(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_SHULKER_BOX(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_SMITHING(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_SMOKER(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_CARTOGRAPHY_TABLE(void);
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_STONECUTTER(void);

// The vanilla registration tail (VanillaBootstrap calls it after the entities;
// Java's MenuType static block runs on class use)
void LIBMATTI_MC_VanillaMenus_RegisterAll(void);
// Java: BuiltInRegistries.MENU.getValue(name) - the by-name lookup
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_GetByName(const char *name);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_INVENTORY_MENUTYPE_H
