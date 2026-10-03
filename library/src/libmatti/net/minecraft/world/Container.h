// Port of net.minecraft.world.Container (the interface) + SimpleContainer +
// ContainerHelper's list helpers. Java's interface becomes the C free-function
// surface over the SimpleContainer storage (the port collapses the interface
// into the one in-memory implementation the menus drive; the BlockEntity
// containers ride the same struct when the block entities land).

#ifndef MATTICRAFT_MC_WORLD_CONTAINER_H
#define MATTICRAFT_MC_WORLD_CONTAINER_H

#include "libmatti/net/minecraft/world/item/ItemStack.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct LIBMATTI_MC_Player;

// Java: public interface Container - the port keeps the ONE default
// getMaxStackSize() = 99 (the stacks clamp their own max on insert).
#define LIBMATTI_MC_Container_DEFAULT_MAX_STACK_SIZE 99

// Java: public class SimpleContainer implements Container
typedef struct LIBMATTI_MC_Container
{
    // Java: private final int size
    int size;
    // Java: private final NonNullList<ItemStack> items (EMPTY-filled)
    LIBMATTI_MC_ItemStack **items;
    // Java: private @Nullable List<ContainerListener> listeners - the port
    // carries one listener slot (the menu owns it; Java keeps a list)
    void *listener; // LIBMATTI_MC_ContainerListener (opaque to avoid the cycle)
} LIBMATTI_MC_Container;

// Java: public SimpleContainer(int)
LIBMATTI_MC_Container *LIBMATTI_MC_Container_New(int size);
// the typed free (frees the stacks the container owns)
void LIBMATTI_MC_Container_Free(LIBMATTI_MC_Container *container);

// Java: ItemStack getItem(int) - EMPTY outside the bounds
LIBMATTI_MC_ItemStack *LIBMATTI_MC_Container_GetItem(const LIBMATTI_MC_Container *container, int index);
// Java: ItemStack removeItem(int, int) - ContainerHelper.removeItem (the split)
LIBMATTI_MC_ItemStack *LIBMATTI_MC_Container_RemoveItem(LIBMATTI_MC_Container *container, int index, int amount);
// Java: ItemStack removeItemNoUpdate(int) - the whole stack out, EMPTY in
LIBMATTI_MC_ItemStack *LIBMATTI_MC_Container_RemoveItemNoUpdate(LIBMATTI_MC_Container *container, int index);
// Java: void setItem(int, ItemStack) - the container TAKES the stack
void LIBMATTI_MC_Container_SetItem(LIBMATTI_MC_Container *container, int index, LIBMATTI_MC_ItemStack *stack);
// Java: int getContainerSize() / boolean isEmpty()
int LIBMATTI_MC_Container_GetContainerSize(const LIBMATTI_MC_Container *container);
bool LIBMATTI_MC_Container_IsEmpty(const LIBMATTI_MC_Container *container);
// Java: int getMaxStackSize() - the 99 default
int LIBMATTI_MC_Container_GetMaxStackSize(const LIBMATTI_MC_Container *container);
// Java: void setChanged() - the listener ping
void LIBMATTI_MC_Container_SetChanged(LIBMATTI_MC_Container *container);
// Java: boolean stillValid(Player) - SimpleContainer answers true (the port
// has no block-entity position to range-check yet)
bool LIBMATTI_MC_Container_StillValid(const LIBMATTI_MC_Container *container, const struct LIBMATTI_MC_Player *player);
// Java: void clearContent()
void LIBMATTI_MC_Container_ClearContent(LIBMATTI_MC_Container *container);

// Java: public ItemStack addItem(ItemStack) - the copy rides occupied same-item
// slots first, then empty slots; the REST (non-empty) returns
LIBMATTI_MC_ItemStack *LIBMATTI_MC_Container_AddItem(LIBMATTI_MC_Container *container, LIBMATTI_MC_ItemStack *stack);
// Java: public boolean canAddItem(ItemStack)
bool LIBMATTI_MC_Container_CanAddItem(const LIBMATTI_MC_Container *container, const LIBMATTI_MC_ItemStack *stack);
// Java: public ItemStack removeItemType(Item, int) - the remainder stack
LIBMATTI_MC_ItemStack *LIBMATTI_MC_Container_RemoveItemType(LIBMATTI_MC_Container *container,
                                                            const LIBMATTI_MC_Item *item, int amount);
// Java: public void addListener(ContainerListener)
void LIBMATTI_MC_Container_AddListener(LIBMATTI_MC_Container *container, void *listener);
void LIBMATTI_MC_Container_RemoveListener(LIBMATTI_MC_Container *container, void *listener);

// ---------------------------------------------------------------------------
// ContainerHelper (the list helpers the menus ride)
// ---------------------------------------------------------------------------

// Java: ContainerHelper.removeItem(List<ItemStack>, int, int) - split or EMPTY
// (works on the caller's stack array - the crafting grid rides it)
LIBMATTI_MC_ItemStack *LIBMATTI_MC_ContainerHelper_RemoveItem(LIBMATTI_MC_ItemStack **items, int size,
                                                              int index, int amount);
// Java: ContainerHelper.takeItem(List<ItemStack>, int) - the whole stack out
LIBMATTI_MC_ItemStack *LIBMATTI_MC_ContainerHelper_TakeItem(LIBMATTI_MC_ItemStack **items, int size, int index);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_CONTAINER_H
