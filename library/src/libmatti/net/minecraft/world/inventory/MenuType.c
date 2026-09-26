// Port of net.minecraft.world.inventory.MenuType over the MappedRegistry port.
// Java's static constants register into BuiltInRegistries.MENU on class use;
// the port registers lazily per accessor (idempotent like class init) and the
// registry owns every type (Java: the Registry holds the reference). The
// MenuSupplier factory rides the supplier the concrete menus hand over.

#include "libmatti/net/minecraft/world/inventory/MenuType.h"
#include "libmatti/net/minecraft/core/MappedRegistry.h"
#include "libmatti/net/minecraft/core/Registry.h"
#include "libmatti/net/minecraft/core/registries/BuiltInRegistries.h"
#include "libmatti/net/minecraft/resources/Identifier.h"

#include <stdlib.h>
#include <string.h>

// Java: private static <T> MenuType<T> register(String, MenuSupplier)
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_Register(const char *name, LIBMATTI_MC_MenuType_MenuSupplier constructor)
{
    LIBMATTI_MC_MenuType *type = calloc(1, sizeof(LIBMATTI_MC_MenuType));
    if (type == NULL)
        return NULL;
    type->name = strdup(name);
    if (type->name == NULL)
    {
        free(type);
        return NULL;
    }
    type->constructor = constructor;
    // Java: Registry.register(BuiltInRegistries.MENU, ResourceKey.create(
    // Registries.MENU, Identifier.withDefaultNamespace(name)), this)
    LIBMATTI_MC_MappedRegistry *registry = LIBMATTI_MC_BuiltInRegistries_MENU();
    LIBMATTI_MC_Identifier *identifier = LIBMATTI_MC_Identifier_Parse(name);
    if (registry == NULL || identifier == NULL)
    {
        free(type->name);
        free(type);
        LIBMATTI_MC_Identifier_Free(identifier);
        return NULL;
    }
    LIBMATTI_MC_Registry_RegisterWithIdentifier(registry, identifier, type);
    LIBMATTI_MC_Identifier_Free(identifier);
    return type;
}

// Java: public T create(int, Inventory) -> constructor.create(containerId, ...)
struct LIBMATTI_MC_AbstractContainerMenu *LIBMATTI_MC_MenuType_Create(const LIBMATTI_MC_MenuType *type,
                                                                     int containerId,
                                                                     const struct LIBMATTI_MC_Player *player)
{
    if (type == NULL || type->constructor == NULL)
        return NULL;
    return type->constructor(containerId, player);
}

// Java: BuiltInRegistries.MENU.getValue(Identifier.withDefaultNamespace(name))
LIBMATTI_MC_MenuType *LIBMATTI_MC_MenuType_GetByName(const char *name)
{
    LIBMATTI_MC_MappedRegistry *registry = LIBMATTI_MC_BuiltInRegistries_MENU();
    if (registry == NULL || name == NULL)
        return NULL;
    LIBMATTI_MC_Identifier *id = LIBMATTI_MC_Identifier_Parse(name);
    if (id == NULL)
        return NULL;
    LIBMATTI_MC_MenuType *type = (LIBMATTI_MC_MenuType *) LIBMATTI_MC_MappedRegistry_GetValue(registry, id);
    LIBMATTI_MC_Identifier_Free(id);
    return type;
}
