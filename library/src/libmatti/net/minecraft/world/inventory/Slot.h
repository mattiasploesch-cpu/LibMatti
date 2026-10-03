// Port of net.minecraft.world.inventory.Slot.
// Java subclasses the Slot for the special behaviours (ResultSlot, ArmorSlot);
// the C port carries the override hooks as function pointers on the struct -
// NULL rides the Java default implementation.

#ifndef MATTICRAFT_MC_WORLD_INVENTORY_SLOT_H
#define MATTICRAFT_MC_WORLD_INVENTORY_SLOT_H

#include "libmatti/net/minecraft/world/item/ItemStack.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct LIBMATTI_MC_Container;
struct LIBMATTI_MC_Player;

// Java: public class Slot
typedef struct LIBMATTI_MC_Slot
{
    // Java: public final Container container
    struct LIBMATTI_MC_Container *container;
    // Java: private final int slot (the container index)
    int slot;
    // Java: public int index (the MENU index - set by addSlot)
    int index;
    // Java: public final int x / y (the screen position)
    int x;
    int y;

    // Java: the subclass overrides as hooks (NULL = the Java default)
    // Java: boolean mayPlace(ItemStack) - default true
    bool (*mayPlace)(const struct LIBMATTI_MC_Slot *slot, const LIBMATTI_MC_ItemStack *stack);
    // Java: boolean mayPickup(Player) - default true
    bool (*mayPickup)(const struct LIBMATTI_MC_Slot *slot, const struct LIBMATTI_MC_Player *player);
    // Java: void onTake(Player, ItemStack) - default setChanged()
    void (*onTake)(struct LIBMATTI_MC_Slot *slot, struct LIBMATTI_MC_Player *player, LIBMATTI_MC_ItemStack *stack);
    // Java: Identifier getNoItemIcon() - NULL = none
    const char *(*getNoItemIcon)(const struct LIBMATTI_MC_Slot *slot);
    // Java: boolean isActive() - default true
    bool (*isActive)(const struct LIBMATTI_MC_Slot *slot);
} LIBMATTI_MC_Slot;

// Java: public Slot(Container, int slot, int x, int y)
LIBMATTI_MC_Slot *LIBMATTI_MC_Slot_New(struct LIBMATTI_MC_Container *container, int containerSlot, int x, int y);
// the typed free (the menu frees its slots)
void LIBMATTI_MC_Slot_Free(LIBMATTI_MC_Slot *slot);

// Java: public ItemStack getItem()
LIBMATTI_MC_ItemStack *LIBMATTI_MC_Slot_GetItem(const LIBMATTI_MC_Slot *slot);
// Java: public boolean hasItem()
bool LIBMATTI_MC_Slot_HasItem(const LIBMATTI_MC_Slot *slot);
// Java: public void set(ItemStack) - the container takes the stack
void LIBMATTI_MC_Slot_Set(LIBMATTI_MC_Slot *slot, LIBMATTI_MC_ItemStack *stack);
// Java: public void setByPlayer(ItemStack) - the set with the onTake hooks
// (Java's setByPlayer(stack, old); the port keeps the two-arg default shape)
void LIBMATTI_MC_Slot_SetByPlayer(LIBMATTI_MC_Slot *slot, LIBMATTI_MC_ItemStack *stack, LIBMATTI_MC_ItemStack *oldStack);
// Java: public void setChanged()
void LIBMATTI_MC_Slot_SetChanged(LIBMATTI_MC_Slot *slot);
// Java: public ItemStack remove(int)
LIBMATTI_MC_ItemStack *LIBMATTI_MC_Slot_Remove(LIBMATTI_MC_Slot *slot, int amount);
// Java: public int getMaxStackSize() / getMaxStackSize(ItemStack)
int LIBMATTI_MC_Slot_GetMaxStackSize(const LIBMATTI_MC_Slot *slot);
int LIBMATTI_MC_Slot_GetMaxStackSizeFor(const LIBMATTI_MC_Slot *slot, const LIBMATTI_MC_ItemStack *stack);
// Java: public int getContainerSlot()
int LIBMATTI_MC_Slot_GetContainerSlot(const LIBMATTI_MC_Slot *slot);

// Java: public boolean mayPlace(ItemStack) / mayPickup(Player) / isActive()
bool LIBMATTI_MC_Slot_MayPlace(const LIBMATTI_MC_Slot *slot, const LIBMATTI_MC_ItemStack *stack);
bool LIBMATTI_MC_Slot_MayPickup(const LIBMATTI_MC_Slot *slot, const struct LIBMATTI_MC_Player *player);
bool LIBMATTI_MC_Slot_IsActive(const LIBMATTI_MC_Slot *slot);
// Java: public void onTake(Player, ItemStack)
void LIBMATTI_MC_Slot_OnTake(LIBMATTI_MC_Slot *slot, struct LIBMATTI_MC_Player *player, LIBMATTI_MC_ItemStack *stack);
// Java: public Identifier getNoItemIcon()
const char *LIBMATTI_MC_Slot_GetNoItemIcon(const LIBMATTI_MC_Slot *slot);

// Java: public Optional<ItemStack> tryRemove(int, int, Player) - the port
// answers the removed stack (EMPTY on the refusal paths)
LIBMATTI_MC_ItemStack *LIBMATTI_MC_Slot_TryRemove(LIBMATTI_MC_Slot *slot, int amount, int max,
                                                  struct LIBMATTI_MC_Player *player);
// Java: public ItemStack safeTake(int, int, Player)
LIBMATTI_MC_ItemStack *LIBMATTI_MC_Slot_SafeTake(LIBMATTI_MC_Slot *slot, int amount, int max,
                                                 struct LIBMATTI_MC_Player *player);
// Java: public ItemStack safeInsert(ItemStack) / safeInsert(ItemStack, int)
LIBMATTI_MC_ItemStack *LIBMATTI_MC_Slot_SafeInsert(LIBMATTI_MC_Slot *slot, LIBMATTI_MC_ItemStack *stack);
LIBMATTI_MC_ItemStack *LIBMATTI_MC_Slot_SafeInsertAmount(LIBMATTI_MC_Slot *slot, LIBMATTI_MC_ItemStack *stack, int amount);
// Java: public boolean allowModification(Player)
bool LIBMATTI_MC_Slot_AllowModification(LIBMATTI_MC_Slot *slot, struct LIBMATTI_MC_Player *player);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_INVENTORY_SLOT_H
