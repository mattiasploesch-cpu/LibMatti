// Inventory harness: the P6.1 core - the SimpleContainer storage (the
// get/set/remove/add item paths), the Slot semantics (safeInsert/safeTake/
// tryRemove with the mayPlace/mayPickup hooks), the MenuType registry and
// the AbstractContainerMenu click semantics (the doClick branches Java 1.21.11
// drives: PICKUP place/take/split/swap, QUICK_MOVE, CLONE, THROW, PICKUP_ALL,
// QUICK_CRAFT paint).

#include "libmatti/net/minecraft/Bootstrap.h"
#include "libmatti/net/minecraft/core/registries/BuiltInRegistries.h"
#include "libmatti/net/minecraft/world/Container.h"
#include "libmatti/net/minecraft/world/entity/VanillaEntities.h"
#include "libmatti/net/minecraft/world/inventory/AbstractContainerMenu.h"
#include "libmatti/net/minecraft/world/inventory/ChestMenu.h"
#include "libmatti/net/minecraft/world/inventory/InventoryMenu.h"
#include "libmatti/net/minecraft/world/inventory/MenuType.h"
#include "libmatti/net/minecraft/world/item/ItemStack.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaItems.h"

#include <stdio.h>
#include <string.h>

static int failures;
static int checks;

static void check(int condition, const char *what)
{
    checks++;
    if (!condition)
    {
        failures++;
        printf("FAIL: %s\n", what);
    }
}

// the click test needs a player (the CLONE instabuild gate); the port's
// Player_Create builds one over the entity base (no level needed for menus)
static LIBMATTI_MC_Player *make_player(void)
{
    return LIBMATTI_MC_Player_Create(NULL, "Tester");
}

// ---------------------------------------------------------------------------
// SimpleContainer (Java: SimpleContainer)
// ---------------------------------------------------------------------------

static void test_container(void)
{
    LIBMATTI_MC_Item *stone = LIBMATTI_MC_VanillaItems_GetByName("STONE");
    LIBMATTI_MC_Item *dirt = LIBMATTI_MC_VanillaItems_GetByName("DIRT");
    check(stone != NULL && dirt != NULL, "vanilla items resolve");

    LIBMATTI_MC_Container *container = LIBMATTI_MC_Container_New(27);
    check(container != NULL, "container allocates");
    if (container == NULL)
        return;

    check(LIBMATTI_MC_Container_GetContainerSize(container) == 27, "size 27");
    check(LIBMATTI_MC_Container_IsEmpty(container), "fresh container empty");
    check(LIBMATTI_MC_ItemStack_IsEmpty(LIBMATTI_MC_Container_GetItem(container, 0)), "fresh slot EMPTY");
    // Java: the out-of-bounds getItem answers EMPTY (not a crash)
    check(LIBMATTI_MC_ItemStack_IsEmpty(LIBMATTI_MC_Container_GetItem(container, -1)), "negative index EMPTY");
    check(LIBMATTI_MC_ItemStack_IsEmpty(LIBMATTI_MC_Container_GetItem(container, 27)), "past-end index EMPTY");

    // Java: setItem - the container takes the stack
    LIBMATTI_MC_Container_SetItem(container, 0, LIBMATTI_MC_ItemStack_NewWithCount(stone, 30));
    check(!LIBMATTI_MC_Container_IsEmpty(container), "stack stores");
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Container_GetItem(container, 0)) == 30, "count 30 stored");

    // Java: removeItem(0, 10) - the split leaves 20 behind
    LIBMATTI_MC_ItemStack *part = LIBMATTI_MC_Container_RemoveItem(container, 0, 10);
    check(LIBMATTI_MC_ItemStack_GetCount(part) == 10, "removeItem splits 10");
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Container_GetItem(container, 0)) == 20, "20 remain");
    LIBMATTI_MC_ItemStack_Free(part);

    // Java: removeItemNoUpdate - the whole stack out, EMPTY in
    LIBMATTI_MC_ItemStack *whole = LIBMATTI_MC_Container_RemoveItemNoUpdate(container, 0);
    check(LIBMATTI_MC_ItemStack_GetCount(whole) == 20, "removeItemNoUpdate takes the stack");
    check(LIBMATTI_MC_Container_IsEmpty(container), "slot empty after NoUpdate");
    LIBMATTI_MC_ItemStack_Free(whole);

    // Java: addItem - the copy merges into same-item slots up to the item's
    // max (stone: 64), the overflow rides the first empty slot
    LIBMATTI_MC_Container_SetItem(container, 3, LIBMATTI_MC_ItemStack_NewWithCount(stone, 60));
    LIBMATTI_MC_ItemStack *rest = LIBMATTI_MC_Container_AddItem(container, LIBMATTI_MC_ItemStack_NewWithCount(stone, 20));
    check(LIBMATTI_MC_ItemStack_IsEmpty(rest), "addItem places fully");
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Container_GetItem(container, 3)) == 64,
          "same-item merge fills slot 3 to the 64 cap");
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Container_GetItem(container, 0)) == 16,
          "the 16 overflow rides the first empty slot");

    // the next add merges into the overflow slot
    rest = LIBMATTI_MC_Container_AddItem(container, LIBMATTI_MC_ItemStack_NewWithCount(stone, 30));
    check(LIBMATTI_MC_ItemStack_IsEmpty(rest), "30 more merge in (16+30=46)");
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Container_GetItem(container, 0)) == 46, "slot 0 carries 46");

    // Java: removeItemType - the reverse scan pulls from the TAIL first
    // (the highest container index carries the 64-stack here)
    LIBMATTI_MC_ItemStack *pulled = LIBMATTI_MC_Container_RemoveItemType(container, stone, 12);
    check(LIBMATTI_MC_ItemStack_GetCount(pulled) == 12, "removeItemType pulls 12");
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Container_GetItem(container, 3)) == 52,
          "the tail slot (3) drains first, 52 remain");
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Container_GetItem(container, 0)) == 46,
          "the earlier slot rides untouched");
    LIBMATTI_MC_ItemStack_Free(pulled);

    LIBMATTI_MC_Container_ClearContent(container);
    check(LIBMATTI_MC_Container_IsEmpty(container), "clearContent empties");

    // Java: setItem clamps to min(container max, item max) = 64 for stone
    LIBMATTI_MC_Container_SetItem(container, 5, LIBMATTI_MC_ItemStack_NewWithCount(stone, 200));
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Container_GetItem(container, 5)) == 64,
          "setItem clamps to the item's 64");

    LIBMATTI_MC_Container_Free(container);
}

// ---------------------------------------------------------------------------
// Slot (Java: Slot) - the safeInsert/tryRemove/allowModification semantics
// ---------------------------------------------------------------------------

static bool slot_never_place(const LIBMATTI_MC_Slot *slot, const LIBMATTI_MC_ItemStack *stack)
{
    (void) slot;
    (void) stack;
    return false;
}

static void test_slot(void)
{
    LIBMATTI_MC_Item *stone = LIBMATTI_MC_VanillaItems_GetByName("STONE");
    LIBMATTI_MC_Item *dirt = LIBMATTI_MC_VanillaItems_GetByName("DIRT");

    LIBMATTI_MC_Container *container = LIBMATTI_MC_Container_New(9);
    LIBMATTI_MC_Slot *slot = LIBMATTI_MC_Slot_New(container, 0, 8, 8);
    check(slot != NULL, "slot allocates");
    if (slot == NULL)
    {
        LIBMATTI_MC_Container_Free(container);
        return;
    }
    check(slot->index == -1, "fresh menu index -1 (addSlot assigns)");
    check(!LIBMATTI_MC_Slot_HasItem(slot), "fresh slot empty");
    check(LIBMATTI_MC_Slot_MayPlace(slot, NULL), "default mayPlace true");
    check(LIBMATTI_MC_Slot_GetMaxStackSize(slot) == 99, "container max rides the slot");

    // Java: safeInsert - the empty slot takes the whole stack, EMPTY back
    LIBMATTI_MC_ItemStack *stack = LIBMATTI_MC_ItemStack_NewWithCount(stone, 30);
    LIBMATTI_MC_ItemStack *rest = LIBMATTI_MC_Slot_SafeInsert(slot, stack);
    check(rest == stack, "safeInsert returns the same stack (Java mutates)");
    check(LIBMATTI_MC_ItemStack_IsEmpty(rest), "insert into empty places all");
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Slot_GetItem(slot)) == 30, "slot carries 30");
    LIBMATTI_MC_ItemStack_Free(rest); // the drained input dies

    // Java: the same-item merge up to the item's max (stone: 64)
    LIBMATTI_MC_ItemStack *more = LIBMATTI_MC_ItemStack_NewWithCount(stone, 80);
    rest = LIBMATTI_MC_Slot_SafeInsert(slot, more);
    check(rest == more, "merge returns the same stack");
    check(LIBMATTI_MC_ItemStack_GetCount(rest) == 46, "insert caps at 64, 46 back");
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Slot_GetItem(slot)) == 64, "slot capped at 64");
    LIBMATTI_MC_ItemStack_Free(rest); // frees more (same object)

    // Java: the different-item insert stays a no-op (the same stack back)
    LIBMATTI_MC_ItemStack *dirtStack = LIBMATTI_MC_ItemStack_NewWithCount(dirt, 5);
    rest = LIBMATTI_MC_Slot_SafeInsert(slot, dirtStack);
    check(rest == dirtStack && LIBMATTI_MC_ItemStack_GetCount(rest) == 5, "different item not inserted");
    LIBMATTI_MC_ItemStack_Free(rest); // frees dirtStack (same object)

    // Java: tryRemove/safeTake
    LIBMATTI_MC_ItemStack *taken = LIBMATTI_MC_Slot_SafeTake(slot, 10, 99, NULL);
    check(LIBMATTI_MC_ItemStack_GetCount(taken) == 10, "safeTake splits 10");
    LIBMATTI_MC_ItemStack_Free(taken);
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Slot_GetItem(slot)) == 54, "54 remain after take");

    // Java: the mayPickup/mayPlace hooks gate the removal paths (the max <
    // count guard blocks the partial take when modification is off)
    slot->mayPlace = slot_never_place;
    LIBMATTI_MC_ItemStack *blocked = LIBMATTI_MC_Slot_TryRemove(slot, 5, 10, NULL);
    check(LIBMATTI_MC_ItemStack_IsEmpty(blocked), "allowModification false blocks tryRemove");
    LIBMATTI_MC_ItemStack_Free(blocked);

    LIBMATTI_MC_Slot_Free(slot);
    LIBMATTI_MC_Container_Free(container);
}

// ---------------------------------------------------------------------------
// MenuType registry (Java: MenuType + BuiltInRegistries.MENU)
// ---------------------------------------------------------------------------

static void test_menu_type(void)
{
    LIBMATTI_MC_MenuType *chest = LIBMATTI_MC_MenuType_GENERIC_9x3();
    check(chest != NULL, "GENERIC_9x3 registers");
    check(LIBMATTI_MC_MenuType_GENERIC_9x3() == chest, "accessor returns the same type (Java static)");

    LIBMATTI_MC_MenuType *lookup = LIBMATTI_MC_MenuType_GetByName("minecraft:generic_9x3");
    check(lookup == chest, "registry lookup resolves the type");
    check(LIBMATTI_MC_MenuType_GetByName("minecraft:not_a_menu") == NULL, "unknown name NULL");

    // the vanilla set rides the bootstrap - 25 constants, all distinct
    LIBMATTI_MC_MenuType *types[25] = {
        LIBMATTI_MC_MenuType_GENERIC_9x1(), LIBMATTI_MC_MenuType_GENERIC_9x2(),
        LIBMATTI_MC_MenuType_GENERIC_9x3(), LIBMATTI_MC_MenuType_GENERIC_9x4(),
        LIBMATTI_MC_MenuType_GENERIC_9x5(), LIBMATTI_MC_MenuType_GENERIC_9x6(),
        LIBMATTI_MC_MenuType_GENERIC_3x3(), LIBMATTI_MC_MenuType_CRAFTER_3x3(),
        LIBMATTI_MC_MenuType_ANVIL(), LIBMATTI_MC_MenuType_BEACON(),
        LIBMATTI_MC_MenuType_BLAST_FURNACE(), LIBMATTI_MC_MenuType_BREWING_STAND(),
        LIBMATTI_MC_MenuType_CRAFTING(), LIBMATTI_MC_MenuType_ENCHANTMENT(),
        LIBMATTI_MC_MenuType_FURNACE(), LIBMATTI_MC_MenuType_GRINDSTONE(),
        LIBMATTI_MC_MenuType_HOPPER(), LIBMATTI_MC_MenuType_LECTERN(),
        LIBMATTI_MC_MenuType_LOOM(), LIBMATTI_MC_MenuType_MERCHANT(),
        LIBMATTI_MC_MenuType_SHULKER_BOX(), LIBMATTI_MC_MenuType_SMITHING(),
        LIBMATTI_MC_MenuType_SMOKER(), LIBMATTI_MC_MenuType_CARTOGRAPHY_TABLE(),
        LIBMATTI_MC_MenuType_STONECUTTER()};
    for (int i = 0; i < 25; i++)
        check(types[i] != NULL, "vanilla MenuType constant non-null");
    for (int i = 0; i < 25; i++)
        for (int j = i + 1; j < 25; j++)
            if (types[i] == types[j])
            {
                check(0, "vanilla MenuTypes distinct");
                return;
            }
    check(1, "vanilla MenuTypes distinct");

    // Java: getType() throws on the typeless menu - the port answers NULL
    check(LIBMATTI_MC_AbstractContainerMenu_GetType(NULL) == NULL, "NULL menu type-safe");
}

// ---------------------------------------------------------------------------
// ChestMenu layout (Java: ChestMenu)
// ---------------------------------------------------------------------------

static void test_chest_menu(void)
{
    LIBMATTI_MC_Item *stone = LIBMATTI_MC_VanillaItems_GetByName("STONE");
    LIBMATTI_MC_Container *chest = LIBMATTI_MC_Container_New(27);
    LIBMATTI_MC_Container *inventory = LIBMATTI_MC_Container_New(36);
    LIBMATTI_MC_Player *player = make_player();

    LIBMATTI_MC_ChestMenu *menu = LIBMATTI_MC_ChestMenu_ThreeRows(7, inventory, chest);
    check(menu != NULL, "ChestMenu.threeRows builds");
    if (menu == NULL)
    {
        LIBMATTI_MC_Container_Free(chest);
        LIBMATTI_MC_Container_Free(inventory);
        return;
    }
    check(menu->base.containerId == 7, "containerId 7");
    check(menu->base.slotCount == 27 + 36, "27 chest + 36 inventory slots");
    check(LIBMATTI_MC_ChestMenu_GetRowCount(menu) == 3, "3 rows");
    check(LIBMATTI_MC_ChestMenu_GetContainer(menu) == chest, "chest container rides");
    // Java: the chest grid rides x=8, y=18 with the 18px pitch
    check(menu->base.slots[0]->x == 8 && menu->base.slots[0]->y == 18, "slot 0 at (8,18)");
    check(menu->base.slots[9]->x == 8 && menu->base.slots[9]->y == 36, "slot 9 at (8,36)");
    // Java: the inventory rides y = 18 + rows*18 + 13 = 85
    check(menu->base.slots[27]->y == 85, "inventory starts at y 85");

    // Java: the chest-slot index mapping j + i * 9
    check(LIBMATTI_MC_Slot_GetContainerSlot(menu->base.slots[5]) == 5, "chest slot 5 -> container 5");
    check(LIBMATTI_MC_Slot_GetContainerSlot(menu->base.slots[9]) == 9, "row 2 slot 0 -> container 9");
    // Java: the inventory mapping j + (i+1) * 9 (extended) then 0..8 (hotbar)
    check(LIBMATTI_MC_Slot_GetContainerSlot(menu->base.slots[27]) == 9, "inventory slot 27 -> container 9");
    check(LIBMATTI_MC_Slot_GetContainerSlot(menu->base.slots[26]) == 26, "chest last -> container 26");
    check(LIBMATTI_MC_Slot_GetContainerSlot(menu->base.slots[62]) == 8, "hotbar last -> container 8");

    // stillValid rides the container (SimpleContainer answers true)
    check(LIBMATTI_MC_AbstractContainerMenu_StillValid(&menu->base, NULL), "stillValid true");

    // the chest <-> inventory quick-move (the QUICK_MOVE click path)
    // Java: moveItemStackTo(..., true) scans REVERSE - the 40 land in the
    // hotbar's last slot (container slot 8, menu slot 62)
    LIBMATTI_MC_Container_SetItem(chest, 0, LIBMATTI_MC_ItemStack_NewWithCount(stone, 40));
    LIBMATTI_MC_AbstractContainerMenu_Clicked(&menu->base, 0, 0, LIBMATTI_MC_ClickType_QUICK_MOVE, player);
    check(LIBMATTI_MC_ItemStack_IsEmpty(LIBMATTI_MC_Container_GetItem(chest, 0)), "quick-move drains chest slot");
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Container_GetItem(inventory, 8)) == 40,
          "40 land in hotbar slot 8 (the reverse scan)");
    // and back: the stack rides menu slot 62
    LIBMATTI_MC_AbstractContainerMenu_Clicked(&menu->base, 62, 0, LIBMATTI_MC_ClickType_QUICK_MOVE, player);
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Container_GetItem(chest, 0)) == 40, "quick-move back to chest");

    LIBMATTI_MC_ChestMenu_Free(menu);
    LIBMATTI_MC_Player_Free(player);
    LIBMATTI_MC_Container_Free(chest);
    LIBMATTI_MC_Container_Free(inventory);
}

// ---------------------------------------------------------------------------
// InventoryMenu layout (Java: InventoryMenu)
// ---------------------------------------------------------------------------

static void test_inventory_menu(void)
{
    LIBMATTI_MC_Container *inventory = LIBMATTI_MC_Container_New(41); // 36 + armor 39..40 range + offhand 40
    LIBMATTI_MC_Player *player = make_player();

    LIBMATTI_MC_InventoryMenu *menu = LIBMATTI_MC_InventoryMenu_New(inventory, true, player);
    check(menu != NULL, "InventoryMenu builds");
    if (menu == NULL)
    {
        LIBMATTI_MC_Container_Free(inventory);
        LIBMATTI_MC_Player_Free(player);
        return;
    }
    check(menu->base.containerId == 0, "CONTAINER_ID 0");
    check(menu->base.menuType == NULL, "typeless (Java: super(null, 0, 2, 2))");
    check(menu->base.slotCount == 46, "1 result + 4 craft + 4 armor + 36 inv + 1 offhand");
    check(LIBMATTI_MC_InventoryMenu_IsHotbarSlot(36) && LIBMATTI_MC_InventoryMenu_IsHotbarSlot(45),
          "36..45 hotbar range");
    check(!LIBMATTI_MC_InventoryMenu_IsHotbarSlot(35) && !LIBMATTI_MC_InventoryMenu_IsHotbarSlot(46),
          "35/46 outside the hotbar range");
    check(LIBMATTI_MC_InventoryMenu_GetResultSlot(menu) == menu->base.slots[0], "result slot = menu slot 0");
    check(LIBMATTI_MC_Slot_GetContainerSlot(menu->base.slots[5]) == 39, "armor head -> container 39");
    check(LIBMATTI_MC_Slot_GetContainerSlot(menu->base.slots[8]) == 36, "armor feet -> container 36");
    check(LIBMATTI_MC_Slot_GetContainerSlot(menu->base.slots[45]) == 40, "offhand -> container 40");

    // the result slot refuses inserts (Java: ResultSlot.mayPlace -> false)
    LIBMATTI_MC_Item *stone = LIBMATTI_MC_VanillaItems_GetByName("STONE");
    check(!LIBMATTI_MC_Slot_MayPlace(menu->base.slots[0], NULL), "result slot refuses placement");

    // the craft grid stages over the craft container
    LIBMATTI_MC_Container_SetItem(LIBMATTI_MC_InventoryMenu_GetCraftSlots(menu), 0,
                                  LIBMATTI_MC_ItemStack_NewWithCount(stone, 1));
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Slot_GetItem(menu->base.slots[1])) == 1,
          "craft slot 1 reads the craft container");

    LIBMATTI_MC_InventoryMenu_Free(menu);
    LIBMATTI_MC_Container_Free(inventory);
    LIBMATTI_MC_Player_Free(player);
}

// ---------------------------------------------------------------------------
// AbstractContainerMenu click semantics (Java: doClick)
// ---------------------------------------------------------------------------

// a minimal test menu: one 6-slot chest grid over the test container
typedef struct TestMenu
{
    LIBMATTI_MC_AbstractContainerMenu base;
} TestMenu;

static void test_pickup_clicks(void)
{
    LIBMATTI_MC_Item *stone = LIBMATTI_MC_VanillaItems_GetByName("STONE");
    LIBMATTI_MC_Item *dirt = LIBMATTI_MC_VanillaItems_GetByName("DIRT");
    LIBMATTI_MC_Player *player = make_player();

    LIBMATTI_MC_Container *container = LIBMATTI_MC_Container_New(9); // rows=1 needs 9 (checkContainerSize)
    LIBMATTI_MC_ChestMenu *menu = LIBMATTI_MC_ChestMenu_New(LIBMATTI_MC_MenuType_GENERIC_9x1(), 1, NULL, container, 1);
    check(menu != NULL && menu->base.slotCount == 45, "9x1 menu builds (9 chest + 36 inventory)");
    if (menu == NULL)
    {
        LIBMATTI_MC_Container_Free(container);
        LIBMATTI_MC_Player_Free(player);
        return;
    }
    LIBMATTI_MC_AbstractContainerMenu *base = &menu->base;

    // Java: PICKUP on an empty slot with a full carried -> the whole stack in
    LIBMATTI_MC_AbstractContainerMenu_SetCarried(base, LIBMATTI_MC_ItemStack_NewWithCount(stone, 30));
    LIBMATTI_MC_AbstractContainerMenu_Clicked(base, 0, 0, LIBMATTI_MC_ClickType_PICKUP, player);
    check(LIBMATTI_MC_ItemStack_IsEmpty(LIBMATTI_MC_AbstractContainerMenu_GetCarried(base)),
          "PICKUP places all (carried empty)");
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Slot_GetItem(base->slots[0])) == 30, "slot carries 30");

    // Java: PICKUP on an empty hand over a stack -> everything out
    LIBMATTI_MC_AbstractContainerMenu_Clicked(base, 0, 0, LIBMATTI_MC_ClickType_PICKUP, player);
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_AbstractContainerMenu_GetCarried(base)) == 30,
          "PICKUP takes all");
    check(LIBMATTI_MC_ItemStack_IsEmpty(LIBMATTI_MC_Slot_GetItem(base->slots[0])), "slot empty after the take");

    // drop the carried outside so the hand is empty again
    LIBMATTI_MC_AbstractContainerMenu_Clicked(base, LIBMATTI_MC_AbstractContainerMenu_SLOT_CLICKED_OUTSIDE, 0,
                                              LIBMATTI_MC_ClickType_PICKUP, player);
    check(LIBMATTI_MC_ItemStack_IsEmpty(LIBMATTI_MC_AbstractContainerMenu_GetCarried(base)), "hand emptied outside");

    // stage a fresh 30-stack for the half/one-over/swap sequence (the slot is
    // empty after the take-all)
    LIBMATTI_MC_Container_SetItem(container, 0, LIBMATTI_MC_ItemStack_NewWithCount(stone, 30));

    // Java: secondary PICKUP on empty hand -> the half
    LIBMATTI_MC_AbstractContainerMenu_Clicked(base, 0, 1, LIBMATTI_MC_ClickType_PICKUP, player);
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_AbstractContainerMenu_GetCarried(base)) == 15,
          "secondary PICKUP takes the half (15)");

    // Java: secondary PICKUP with carried onto the same-item slot -> one over
    LIBMATTI_MC_AbstractContainerMenu_Clicked(base, 0, 1, LIBMATTI_MC_ClickType_PICKUP, player);
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_AbstractContainerMenu_GetCarried(base)) == 14,
          "one placed back (14 carried)");
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Slot_GetItem(base->slots[0])) == 16, "16 in the slot");

    // Java: primary PICKUP with the different item -> the swap
    LIBMATTI_MC_ItemStack *carriedDirt = LIBMATTI_MC_ItemStack_NewWithCount(dirt, 7);
    LIBMATTI_MC_AbstractContainerMenu_SetCarried(base, carriedDirt);
    LIBMATTI_MC_AbstractContainerMenu_Clicked(base, 0, 0, LIBMATTI_MC_ClickType_PICKUP, player);
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Slot_GetItem(base->slots[0])) == 7
              && LIBMATTI_MC_ItemStack_Is(LIBMATTI_MC_Slot_GetItem(base->slots[0]), dirt),
          "swap places the carried dirt");
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_AbstractContainerMenu_GetCarried(base)) == 16,
          "swap picks the stone 16 up");

    // Java: SLOT_CLICKED_OUTSIDE primary -> the carried drops
    LIBMATTI_MC_AbstractContainerMenu_Clicked(base, LIBMATTI_MC_AbstractContainerMenu_SLOT_CLICKED_OUTSIDE, 0,
                                              LIBMATTI_MC_ClickType_PICKUP, player);
    check(LIBMATTI_MC_ItemStack_IsEmpty(LIBMATTI_MC_AbstractContainerMenu_GetCarried(base)),
          "drop outside empties the carried");

    LIBMATTI_MC_ChestMenu_Free(menu);
    LIBMATTI_MC_Container_Free(container);
    LIBMATTI_MC_Player_Free(player);
}

static void test_clone_throw_pickup_all(void)
{
    LIBMATTI_MC_Item *stone = LIBMATTI_MC_VanillaItems_GetByName("STONE");
    LIBMATTI_MC_Player *player = make_player();
    check(player != NULL, "player builds");
    if (player == NULL)
        return;
    // Java: CLONE rides hasInfiniteMaterials -> the creative instabuild ability
    LIBMATTI_MC_Player_GetAbilities(player)->instabuild = true;

    LIBMATTI_MC_Container *container = LIBMATTI_MC_Container_New(9);
    LIBMATTI_MC_ChestMenu *menu = LIBMATTI_MC_ChestMenu_New(LIBMATTI_MC_MenuType_GENERIC_9x1(), 1, NULL, container, 1);
    LIBMATTI_MC_AbstractContainerMenu *base = &menu->base;

    // Java: CLONE (creative middle click) - the instabuild ability copies to max
    LIBMATTI_MC_Container_SetItem(container, 2, LIBMATTI_MC_ItemStack_NewWithCount(stone, 3));
    LIBMATTI_MC_AbstractContainerMenu_Clicked(base, 2, 2, LIBMATTI_MC_ClickType_CLONE, player);
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_AbstractContainerMenu_GetCarried(base)) == 64,
          "CLONE copies the max stack (stone: 64, creative instabuild)");

    // Java: PICKUP_ALL (double click on an EMPTY cell) - the same-item stacks
    // gather into the carried
    LIBMATTI_MC_AbstractContainerMenu_SetCarried(base, LIBMATTI_MC_ItemStack_NewWithCount(stone, 1));
    LIBMATTI_MC_Container_SetItem(container, 5, LIBMATTI_MC_ItemStack_NewWithCount(stone, 20));
    LIBMATTI_MC_AbstractContainerMenu_Clicked(base, 1, 0, LIBMATTI_MC_ClickType_PICKUP_ALL, player);
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_AbstractContainerMenu_GetCarried(base)) == 24,
          "PICKUP_ALL gathers 1 + 3 (slot 2) + 20 (slot 5)");
    check(LIBMATTI_MC_ItemStack_IsEmpty(LIBMATTI_MC_Container_GetItem(container, 2))
              && LIBMATTI_MC_ItemStack_IsEmpty(LIBMATTI_MC_Container_GetItem(container, 5)),
          "gathered slots empty");

    // Java: THROW - the whole stack out of the menu (carried must be empty)
    LIBMATTI_MC_AbstractContainerMenu_SetCarried(base, LIBMATTI_MC_ItemStack_Empty());
    LIBMATTI_MC_Container_SetItem(container, 6, LIBMATTI_MC_ItemStack_NewWithCount(stone, 10));
    LIBMATTI_MC_AbstractContainerMenu_Clicked(base, 6, 1, LIBMATTI_MC_ClickType_THROW, player);
    check(LIBMATTI_MC_ItemStack_IsEmpty(LIBMATTI_MC_Container_GetItem(container, 6)), "THROW clears the slot");

    LIBMATTI_MC_ChestMenu_Free(menu);
    LIBMATTI_MC_Container_Free(container);
    LIBMATTI_MC_Player_Free(player);
}

static void test_quick_craft(void)
{
    LIBMATTI_MC_Item *stone = LIBMATTI_MC_VanillaItems_GetByName("STONE");
    LIBMATTI_MC_Player *player = make_player();

    LIBMATTI_MC_Container *container = LIBMATTI_MC_Container_New(9);
    LIBMATTI_MC_ChestMenu *menu = LIBMATTI_MC_ChestMenu_New(LIBMATTI_MC_MenuType_GENERIC_9x1(), 1, NULL, container, 1);
    LIBMATTI_MC_AbstractContainerMenu *base = &menu->base;

    // Java: the drag paint - START(type CHARITABLE) -> CONTINUE x2 -> END
    LIBMATTI_MC_AbstractContainerMenu_SetCarried(base, LIBMATTI_MC_ItemStack_NewWithCount(stone, 20));
    int header = LIBMATTI_MC_AbstractContainerMenu_GetQuickcraftMask(
        LIBMATTI_MC_AbstractContainerMenu_QUICKCRAFT_HEADER_START,
        LIBMATTI_MC_AbstractContainerMenu_QUICKCRAFT_TYPE_CHARITABLE);
    LIBMATTI_MC_AbstractContainerMenu_Clicked(base, -1, header, LIBMATTI_MC_ClickType_QUICK_CRAFT, player);
    check(base->quickcraftStatus == 1, "drag START arms the paint");
    LIBMATTI_MC_AbstractContainerMenu_Clicked(base, 0, 1, LIBMATTI_MC_ClickType_QUICK_CRAFT, player);
    LIBMATTI_MC_AbstractContainerMenu_Clicked(base, 1, 1, LIBMATTI_MC_ClickType_QUICK_CRAFT, player);
    check(base->quickcraftSlotCount == 2, "two slots painted");
    LIBMATTI_MC_AbstractContainerMenu_Clicked(base, -1,
                                              LIBMATTI_MC_AbstractContainerMenu_QUICKCRAFT_HEADER_END,
                                              LIBMATTI_MC_ClickType_QUICK_CRAFT, player);
    // Java: CHARITABLE splits floor(20/2)=10 per slot
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Slot_GetItem(base->slots[0])) == 10, "paint slot 0 = 10");
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_Slot_GetItem(base->slots[1])) == 10, "paint slot 1 = 10");
    check(LIBMATTI_MC_ItemStack_GetCount(LIBMATTI_MC_AbstractContainerMenu_GetCarried(base)) == 0,
          "the carried drains");
    check(base->quickcraftStatus == 0 && base->quickcraftSlotCount == 0, "drag reset after END");

    LIBMATTI_MC_ChestMenu_Free(menu);
    LIBMATTI_MC_Container_Free(container);
    LIBMATTI_MC_Player_Free(player);
}

// ---------------------------------------------------------------------------
// DataSlot + the menu state (Java: DataSlot/incrementStateId)
// ---------------------------------------------------------------------------

static void test_data_slots(void)
{
    LIBMATTI_MC_DataSlot standalone = LIBMATTI_MC_DataSlot_Standalone();
    LIBMATTI_MC_DataSlot_Set(&standalone, 42);
    check(LIBMATTI_MC_DataSlot_Get(&standalone) == 42, "standalone stores");
    check(LIBMATTI_MC_DataSlot_CheckAndClearUpdateFlag(&standalone), "first check flags");
    check(!LIBMATTI_MC_DataSlot_CheckAndClearUpdateFlag(&standalone), "unchanged check clears");
    LIBMATTI_MC_DataSlot_Set(&standalone, 42);
    check(!LIBMATTI_MC_DataSlot_CheckAndClearUpdateFlag(&standalone), "same value no flag");

    int shared[3] = {0, 0, 0};
    LIBMATTI_MC_DataSlot slot = LIBMATTI_MC_DataSlot_Shared(shared, 1);
    LIBMATTI_MC_DataSlot_Set(&slot, 7);
    check(shared[1] == 7, "shared writes through");
    shared[1] = 9;
    check(LIBMATTI_MC_DataSlot_Get(&slot) == 9, "shared reads through");

    // Java: incrementStateId wraps at 32768
    LIBMATTI_MC_Container *container = LIBMATTI_MC_Container_New(9);
    LIBMATTI_MC_ChestMenu *menu = LIBMATTI_MC_ChestMenu_New(LIBMATTI_MC_MenuType_GENERIC_9x1(), 1, NULL, container, 1);
    menu->base.stateId = 32766;
    check(LIBMATTI_MC_AbstractContainerMenu_IncrementStateId(&menu->base) == 32767, "increment to 32767");
    check(LIBMATTI_MC_AbstractContainerMenu_IncrementStateId(&menu->base) == 0, "wrap to 0 (& 32767)");
    LIBMATTI_MC_ChestMenu_Free(menu);
    LIBMATTI_MC_Container_Free(container);
}

int main(void)
{
    LIBMATTI_MC_Bootstrap_BootStrap();
    test_container();
    test_slot();
    test_menu_type();
    test_chest_menu();
    test_inventory_menu();
    test_pickup_clicks();
    test_clone_throw_pickup_all();
    test_quick_craft();
    test_data_slots();
    printf("inventory: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}
