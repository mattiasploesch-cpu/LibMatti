// Port of net.minecraft.world.inventory.AbstractContainerMenu - the menu core.
// The port keeps the slots/carried/data-slot state and the click semantics
// (the doClick branches over the ClickTypes the player can drive); the
// server-side synchronizer/remote-slot machinery collapses into the
// lastSlots snapshot broadcast (the single-player port has no wire yet).

#ifndef MATTICRAFT_MC_WORLD_INVENTORY_ABSTRACTCONTAINERMENU_H
#define MATTICRAFT_MC_WORLD_INVENTORY_ABSTRACTCONTAINERMENU_H

#include "libmatti/net/minecraft/world/inventory/ClickType.h"
#include "libmatti/net/minecraft/world/inventory/MenuType.h"
#include "libmatti/net/minecraft/world/inventory/Slot.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct LIBMATTI_MC_Player;
struct LIBMATTI_MC_Container;

// Java: the constants the click paths carry
#define LIBMATTI_MC_AbstractContainerMenu_SLOT_CLICKED_OUTSIDE -999
#define LIBMATTI_MC_AbstractContainerMenu_QUICKCRAFT_TYPE_CHARITABLE 0
#define LIBMATTI_MC_AbstractContainerMenu_QUICKCRAFT_TYPE_GREEDY 1
#define LIBMATTI_MC_AbstractContainerMenu_QUICKCRAFT_TYPE_CLONE 2
#define LIBMATTI_MC_AbstractContainerMenu_QUICKCRAFT_HEADER_START 0
#define LIBMATTI_MC_AbstractContainerMenu_QUICKCRAFT_HEADER_CONTINUE 1
#define LIBMATTI_MC_AbstractContainerMenu_QUICKCRAFT_HEADER_END 2
#define LIBMATTI_MC_AbstractContainerMenu_SLOTS_PER_ROW 9
#define LIBMATTI_MC_AbstractContainerMenu_SLOT_SIZE 18

// Java: public abstract class AbstractContainerMenu - the port's vtable carries
// the two abstracts (quickMoveStack/stillValid) the concrete menus implement
typedef struct LIBMATTI_MC_AbstractContainerMenu
{
    // the subclass hook table (the port's "abstract" methods + the overrides)
    // Java: public abstract ItemStack quickMoveStack(Player, int)
    LIBMATTI_MC_ItemStack *(*quickMoveStack)(struct LIBMATTI_MC_AbstractContainerMenu *menu,
                                             struct LIBMATTI_MC_Player *player, int index);
    // Java: public abstract boolean stillValid(Player)
    bool (*stillValid)(const struct LIBMATTI_MC_AbstractContainerMenu *menu, const struct LIBMATTI_MC_Player *player);
    // Java: public boolean canTakeItemForPickupAll - the InventoryMenu override
    bool (*canTakeItemForPickAll)(const struct LIBMATTI_MC_AbstractContainerMenu *menu,
                                  const LIBMATTI_MC_ItemStack *stack, const LIBMATTI_MC_Slot *slot);
    // Java: public boolean canDragTo(Slot) - default true
    bool (*canDragTo)(const struct LIBMATTI_MC_AbstractContainerMenu *menu, const LIBMATTI_MC_Slot *slot);

    // Java: public final NonNullList<Slot> slots
    LIBMATTI_MC_Slot **slots;
    int slotCount;
    int slotCapacity;
    // Java: private NonNullList<ItemStack> lastSlots (the listener snapshot)
    LIBMATTI_MC_ItemStack **lastSlots;
    int lastSlotCapacity;
    // Java: private final List<DataSlot> dataSlots
    LIBMATTI_MC_DataSlot *dataSlots;
    int dataSlotCount;
    int dataSlotCapacity;
    // Java: private ItemStack carried
    LIBMATTI_MC_ItemStack *carried;
    // Java: private int stateId
    int stateId;
    // Java: private final @Nullable MenuType<?> menuType / public final int containerId
    LIBMATTI_MC_MenuType *menuType;
    int containerId;
    // Java: private int quickcraftType / quickcraftStatus / Set<Slot> quickcraftSlots
    int quickcraftType;
    int quickcraftStatus;
    // the quickcraft drag set (the slot POINTERS ride; Java keys the Set by identity)
    LIBMATTI_MC_Slot **quickcraftSlots;
    int quickcraftSlotCount;
    int quickcraftSlotCapacity;
} LIBMATTI_MC_AbstractContainerMenu;

// Java: protected AbstractContainerMenu(@Nullable MenuType<?>, int) - the base
// init (the vtable the concrete menu fills BEFORE its addSlot calls)
void LIBMATTI_MC_AbstractContainerMenu_Init(LIBMATTI_MC_AbstractContainerMenu *menu, LIBMATTI_MC_MenuType *menuType,
                                            int containerId);
// the typed free (frees the menu-owned slots + the carried stack; the
// containers the slots reference stay theirs)
void LIBMATTI_MC_AbstractContainerMenu_Free(LIBMATTI_MC_AbstractContainerMenu *menu);

// Java: protected Slot addSlot(Slot) - the menu takes the slot
LIBMATTI_MC_Slot *LIBMATTI_MC_AbstractContainerMenu_AddSlot(LIBMATTI_MC_AbstractContainerMenu *menu,
                                                            LIBMATTI_MC_Slot *slot);
// Java: protected DataSlot addDataSlot(DataSlot)
void LIBMATTI_MC_AbstractContainerMenu_AddDataSlot(LIBMATTI_MC_AbstractContainerMenu *menu, LIBMATTI_MC_DataSlot slot);
// Java: protected void addDataSlots(ContainerData) - the port takes the shared
// array (the ContainerData interface collapses into int[] + count)
void LIBMATTI_MC_AbstractContainerMenu_AddDataSlots(LIBMATTI_MC_AbstractContainerMenu *menu, int *data, int count);

// Java: protected void addInventoryHotbarSlots(Container, int, int)
void LIBMATTI_MC_AbstractContainerMenu_AddInventoryHotbarSlots(LIBMATTI_MC_AbstractContainerMenu *menu,
                                                               struct LIBMATTI_MC_Container *container, int x, int y);
// Java: protected void addInventoryExtendedSlots(Container, int, int)
void LIBMATTI_MC_AbstractContainerMenu_AddInventoryExtendedSlots(LIBMATTI_MC_AbstractContainerMenu *menu,
                                                                 struct LIBMATTI_MC_Container *container, int x, int y);
// Java: protected void addStandardInventorySlots(Container, int, int)
void LIBMATTI_MC_AbstractContainerMenu_AddStandardInventorySlots(LIBMATTI_MC_AbstractContainerMenu *menu,
                                                                 struct LIBMATTI_MC_Container *container, int x, int y);

// Java: public MenuType<?> getType() / public int containerId (the field)
LIBMATTI_MC_MenuType *LIBMATTI_MC_AbstractContainerMenu_GetType(const LIBMATTI_MC_AbstractContainerMenu *menu);
// Java: public Slot getSlot(int)
LIBMATTI_MC_Slot *LIBMATTI_MC_AbstractContainerMenu_GetSlot(const LIBMATTI_MC_AbstractContainerMenu *menu, int index);
// Java: public boolean isValidSlotIndex(int)
bool LIBMATTI_MC_AbstractContainerMenu_IsValidSlotIndex(const LIBMATTI_MC_AbstractContainerMenu *menu, int index);
// Java: public NonNullList<ItemStack> getItems() - the caller frees the list
// (not the stacks); the count rides the out-parameter
LIBMATTI_MC_ItemStack **LIBMATTI_MC_AbstractContainerMenu_GetItems(const LIBMATTI_MC_AbstractContainerMenu *menu,
                                                                   int *count);

// Java: public void setCarried(ItemStack) / public ItemStack getCarried()
void LIBMATTI_MC_AbstractContainerMenu_SetCarried(LIBMATTI_MC_AbstractContainerMenu *menu, LIBMATTI_MC_ItemStack *stack);
LIBMATTI_MC_ItemStack *LIBMATTI_MC_AbstractContainerMenu_GetCarried(const LIBMATTI_MC_AbstractContainerMenu *menu);

// Java: public int getStateId() / incrementStateId()
int LIBMATTI_MC_AbstractContainerMenu_GetStateId(const LIBMATTI_MC_AbstractContainerMenu *menu);
int LIBMATTI_MC_AbstractContainerMenu_IncrementStateId(LIBMATTI_MC_AbstractContainerMenu *menu);

// Java: public void clicked(int, int, ClickType, Player) - the doClick dispatch
void LIBMATTI_MC_AbstractContainerMenu_Clicked(LIBMATTI_MC_AbstractContainerMenu *menu, int slotIndex, int button,
                                               LIBMATTI_MC_ClickType clickType, struct LIBMATTI_MC_Player *player);
// Java: protected abstract ItemStack quickMoveStack(Player, int) - the dispatch
// through the vtable (NULL vtable = the EMPTY stub)
LIBMATTI_MC_ItemStack *LIBMATTI_MC_AbstractContainerMenu_QuickMoveStack(LIBMATTI_MC_AbstractContainerMenu *menu,
                                                                        struct LIBMATTI_MC_Player *player, int index);
// Java: public abstract boolean stillValid(Player)
bool LIBMATTI_MC_AbstractContainerMenu_StillValid(const LIBMATTI_MC_AbstractContainerMenu *menu,
                                                  const struct LIBMATTI_MC_Player *player);

// Java: protected boolean moveItemStackTo(ItemStack, int, int, boolean) - the
// quick-move scan (the reverse flag rides fromIndex == end - 1 backwards)
bool LIBMATTI_MC_AbstractContainerMenu_MoveItemStackTo(LIBMATTI_MC_AbstractContainerMenu *menu,
                                                       LIBMATTI_MC_ItemStack *stack, int fromIndex, int endIndex,
                                                       bool reverse);

// Java: the quickcraft statics
int LIBMATTI_MC_AbstractContainerMenu_GetQuickcraftType(int headerValue);
int LIBMATTI_MC_AbstractContainerMenu_GetQuickcraftHeader(int headerValue);
int LIBMATTI_MC_AbstractContainerMenu_GetQuickcraftMask(int header, int type);
bool LIBMATTI_MC_AbstractContainerMenu_IsValidQuickcraftType(int quickcraftType, const struct LIBMATTI_MC_Player *player);
int LIBMATTI_MC_AbstractContainerMenu_GetQuickcraftPlaceCount(int quickcraftSlotCount, int quickcraftType,
                                                              const LIBMATTI_MC_ItemStack *stack);
// Java: public static boolean canItemQuickReplace(@Nullable Slot, ItemStack, boolean)
bool LIBMATTI_MC_AbstractContainerMenu_CanItemQuickReplace(const LIBMATTI_MC_Slot *slot,
                                                           const LIBMATTI_MC_ItemStack *stack, bool checkCount);
// Java: public OptionalInt findSlot(Container, int) - -1 on the miss
int LIBMATTI_MC_AbstractContainerMenu_FindSlot(const LIBMATTI_MC_AbstractContainerMenu *menu,
                                               const struct LIBMATTI_MC_Container *container, int containerSlot);
// Java: public void slotsChanged(Container)
void LIBMATTI_MC_AbstractContainerMenu_SlotsChanged(LIBMATTI_MC_AbstractContainerMenu *menu,
                                                    struct LIBMATTI_MC_Container *container);
// Java: public void broadcastChanges() - the lastSlots snapshot walk (the
// listener ping collapses into the stateId bump the client polls)
void LIBMATTI_MC_AbstractContainerMenu_BroadcastChanges(LIBMATTI_MC_AbstractContainerMenu *menu);
// Java: public void setItem(int, int, ItemStack)
void LIBMATTI_MC_AbstractContainerMenu_SetItem(LIBMATTI_MC_AbstractContainerMenu *menu, int slotIndex, int stateId,
                                               LIBMATTI_MC_ItemStack *stack);
// Java: public void removed(Player) - the default carried cleanup
void LIBMATTI_MC_AbstractContainerMenu_Removed(LIBMATTI_MC_AbstractContainerMenu *menu, struct LIBMATTI_MC_Player *player);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_INVENTORY_ABSTRACTCONTAINERMENU_H
