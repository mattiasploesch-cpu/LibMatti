// Port of net.minecraft.world.item.ItemStack.
// Java's ItemStack is (count, item, PatchedDataComponentMap). The C port keeps
// exactly that; the behaviour methods (use, interact, ...) stay with the game
// port since they pull in Level/Player/Entity.

#ifndef MATTICRAFT_MC_WORLD_ITEM_ITEMSTACK_H
#define MATTICRAFT_MC_WORLD_ITEM_ITEMSTACK_H

#include "libmatti/net/minecraft/core/component/DataComponentMap.h"
#include "libmatti/net/minecraft/core/component/PatchedDataComponentMap.h"
#include "libmatti/net/minecraft/world/item/Item.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class ItemStack implements DataComponentHolder
typedef struct LIBMATTI_MC_ItemStack
{
    // Java: private int count
    int count;
    // Java: private int popTime
    int popTime;
    // Java: private final @Nullable Item item - NULL for EMPTY
    LIBMATTI_MC_Item *item;
    // Java: final PatchedDataComponentMap components
    LIBMATTI_MC_PatchedDataComponentMap *components;
} LIBMATTI_MC_ItemStack;

// Java: public static final ItemStack EMPTY
LIBMATTI_MC_ItemStack *LIBMATTI_MC_ItemStack_Empty(void);

// Java: public ItemStack(ItemLike) - count 1
LIBMATTI_MC_ItemStack *LIBMATTI_MC_ItemStack_New(LIBMATTI_MC_Item *item);
// Java: public ItemStack(ItemLike, int)
LIBMATTI_MC_ItemStack *LIBMATTI_MC_ItemStack_NewWithCount(LIBMATTI_MC_Item *item, int count);
// Java: private ItemStack(ItemLike, int, PatchedDataComponentMap)
LIBMATTI_MC_ItemStack *LIBMATTI_MC_ItemStack_NewWithComponents(LIBMATTI_MC_Item *item, int count,
                                                               LIBMATTI_MC_PatchedDataComponentMap *components);

// Java: public boolean isEmpty()
bool LIBMATTI_MC_ItemStack_IsEmpty(const LIBMATTI_MC_ItemStack *stack);
// Java: public int getCount() / setCount(int)
int LIBMATTI_MC_ItemStack_GetCount(const LIBMATTI_MC_ItemStack *stack);
void LIBMATTI_MC_ItemStack_SetCount(LIBMATTI_MC_ItemStack *stack, int count);
// Java: public Item getItem()
LIBMATTI_MC_Item *LIBMATTI_MC_ItemStack_GetItem(const LIBMATTI_MC_ItemStack *stack);

// Java: public DataComponentMap getComponents() - EMPTY for the empty stack
LIBMATTI_MC_DataComponentMap *LIBMATTI_MC_ItemStack_GetComponents(const LIBMATTI_MC_ItemStack *stack);
// Java: public DataComponentPatch getComponentsPatch()
LIBMATTI_MC_DataComponentPatch *LIBMATTI_MC_ItemStack_GetComponentsPatch(const LIBMATTI_MC_ItemStack *stack);

// Java: public <T> @Nullable T get(DataComponentType<? extends T>)
void *LIBMATTI_MC_ItemStack_Get(const LIBMATTI_MC_ItemStack *stack, const LIBMATTI_MC_DataComponentType *type);
// Java: public <T> T getOrDefault(DataComponentType<? super T>, T)
void *LIBMATTI_MC_ItemStack_GetOrDefault(const LIBMATTI_MC_ItemStack *stack, const LIBMATTI_MC_DataComponentType *type,
                                         void *defaultValue);
// Java: public boolean has(DataComponentType<?>)
bool LIBMATTI_MC_ItemStack_Has(const LIBMATTI_MC_ItemStack *stack, const LIBMATTI_MC_DataComponentType *type);
// Java: public <T> @Nullable T set(DataComponentType<T>, @Nullable T)
void *LIBMATTI_MC_ItemStack_Set(LIBMATTI_MC_ItemStack *stack, const LIBMATTI_MC_DataComponentType *type, void *value);
// Java: public <T> @Nullable T remove(DataComponentType<? extends T>)
void *LIBMATTI_MC_ItemStack_Remove(LIBMATTI_MC_ItemStack *stack, const LIBMATTI_MC_DataComponentType *type);
// Java: public boolean hasNonDefault(DataComponentType<?>)
bool LIBMATTI_MC_ItemStack_HasNonDefault(const LIBMATTI_MC_ItemStack *stack, const LIBMATTI_MC_DataComponentType *type);

// Java: public int getMaxStackSize() - getOrDefault(MAX_STACK_SIZE, 1)
int LIBMATTI_MC_ItemStack_GetMaxStackSize(const LIBMATTI_MC_ItemStack *stack);
// Java: public boolean isStackable()
bool LIBMATTI_MC_ItemStack_IsStackable(const LIBMATTI_MC_ItemStack *stack);
// Java: public boolean isDamageableItem() - has(MAX_DAMAGE) && MAX_STACK_SIZE == 1
bool LIBMATTI_MC_ItemStack_IsDamageableItem(const LIBMATTI_MC_ItemStack *stack);
// Java: public boolean isDamaged() / getDamageValue() / setDamageValue(int)
bool LIBMATTI_MC_ItemStack_IsDamaged(const LIBMATTI_MC_ItemStack *stack);
int LIBMATTI_MC_ItemStack_GetDamageValue(const LIBMATTI_MC_ItemStack *stack);
void LIBMATTI_MC_ItemStack_SetDamageValue(LIBMATTI_MC_ItemStack *stack, int damage);
// Java: public int getMaxDamage()
int LIBMATTI_MC_ItemStack_GetMaxDamage(const LIBMATTI_MC_ItemStack *stack);

// Java: public ItemStack split(int) - removes the amount, returns the removed part
LIBMATTI_MC_ItemStack *LIBMATTI_MC_ItemStack_Split(LIBMATTI_MC_ItemStack *stack, int amount);
// Java: public ItemStack copy()
LIBMATTI_MC_ItemStack *LIBMATTI_MC_ItemStack_Copy(const LIBMATTI_MC_ItemStack *stack);
// Java: public ItemStack copyWithCount(int)
LIBMATTI_MC_ItemStack *LIBMATTI_MC_ItemStack_CopyWithCount(const LIBMATTI_MC_ItemStack *stack, int count);
// Java: public void grow(int) / shrink(int)
void LIBMATTI_MC_ItemStack_Grow(LIBMATTI_MC_ItemStack *stack, int amount);
void LIBMATTI_MC_ItemStack_Shrink(LIBMATTI_MC_ItemStack *stack, int amount);

// Java: public int getPopTime() / setPopTime(int)
int LIBMATTI_MC_ItemStack_GetPopTime(const LIBMATTI_MC_ItemStack *stack);
void LIBMATTI_MC_ItemStack_SetPopTime(LIBMATTI_MC_ItemStack *stack, int popTime);

// Java: public static boolean matches(ItemStack, ItemStack)
bool LIBMATTI_MC_ItemStack_Matches(const LIBMATTI_MC_ItemStack *a, const LIBMATTI_MC_ItemStack *b);
// Java: public static boolean isSameItem(ItemStack, ItemStack)
bool LIBMATTI_MC_ItemStack_IsSameItem(const LIBMATTI_MC_ItemStack *a, const LIBMATTI_MC_ItemStack *b);
// Java: public static boolean isSameItemSameComponents(ItemStack, ItemStack)
bool LIBMATTI_MC_ItemStack_IsSameItemSameComponents(const LIBMATTI_MC_ItemStack *a, const LIBMATTI_MC_ItemStack *b);

// Java: public boolean is(Item)
bool LIBMATTI_MC_ItemStack_Is(const LIBMATTI_MC_ItemStack *stack, const LIBMATTI_MC_Item *item);

void LIBMATTI_MC_ItemStack_Free(LIBMATTI_MC_ItemStack *stack);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_ITEM_ITEMSTACK_H
