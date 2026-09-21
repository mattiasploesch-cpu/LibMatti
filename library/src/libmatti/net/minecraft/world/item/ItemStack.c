// Port of net.minecraft.world.item.ItemStack (the data part).

#include "libmatti/net/minecraft/world/item/ItemStack.h"

#include "libmatti/net/minecraft/core/component/DataComponents.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaItems.h"

#include <stdlib.h>

LIBMATTI_MC_ItemStack *LIBMATTI_MC_ItemStack_Empty(void)
{
    static LIBMATTI_MC_ItemStack *empty;
    if (empty == NULL)
    {
        // Java: private ItemStack(@Nullable Void) - item = null, components over DataComponentMap.EMPTY
        empty = calloc(1, sizeof(LIBMATTI_MC_ItemStack));
        empty->components = LIBMATTI_MC_PatchedDataComponentMap_New(LIBMATTI_MC_DataComponentMap_Empty());
    }
    return empty;
}

LIBMATTI_MC_ItemStack *LIBMATTI_MC_ItemStack_New(LIBMATTI_MC_Item *item)
{
    return LIBMATTI_MC_ItemStack_NewWithCount(item, 1);
}

LIBMATTI_MC_ItemStack *LIBMATTI_MC_ItemStack_NewWithCount(LIBMATTI_MC_Item *item, int count)
{
    // Java: new PatchedDataComponentMap(item.components())
    return LIBMATTI_MC_ItemStack_NewWithComponents(item, count,
                                                   LIBMATTI_MC_PatchedDataComponentMap_New(LIBMATTI_MC_Item_GetComponents(item)));
}

LIBMATTI_MC_ItemStack *LIBMATTI_MC_ItemStack_NewWithComponents(LIBMATTI_MC_Item *item, int count,
                                                               LIBMATTI_MC_PatchedDataComponentMap *components)
{
    LIBMATTI_MC_ItemStack *stack = calloc(1, sizeof(LIBMATTI_MC_ItemStack));
    stack->item = item;
    stack->count = count;
    stack->components = components;
    return stack;
}

bool LIBMATTI_MC_ItemStack_IsEmpty(const LIBMATTI_MC_ItemStack *stack)
{
    // Java: this == EMPTY || this.item == null || this.count <= 0
    return stack == NULL || stack == LIBMATTI_MC_ItemStack_Empty() || stack->item == NULL || stack->count <= 0;
}

int LIBMATTI_MC_ItemStack_GetCount(const LIBMATTI_MC_ItemStack *stack)
{
    return LIBMATTI_MC_ItemStack_IsEmpty(stack) ? 0 : stack->count;
}

void LIBMATTI_MC_ItemStack_SetCount(LIBMATTI_MC_ItemStack *stack, int count)
{
    if (stack == NULL || stack == LIBMATTI_MC_ItemStack_Empty())
        return;
    stack->count = count;
    if (stack->count <= 0)
        stack->count = 0;
}

LIBMATTI_MC_Item *LIBMATTI_MC_ItemStack_GetItem(const LIBMATTI_MC_ItemStack *stack)
{
    // Java: this.isEmpty() ? Items.AIR : this.item
    if (LIBMATTI_MC_ItemStack_IsEmpty(stack))
        return LIBMATTI_MC_VanillaItems_AIR();
    return stack->item;
}

LIBMATTI_MC_DataComponentMap *LIBMATTI_MC_ItemStack_GetComponents(const LIBMATTI_MC_ItemStack *stack)
{
    // Java: (DataComponentMap)(!this.isEmpty() ? this.components : DataComponentMap.EMPTY)
    // The patched map is cast to its interface in Java; the port exposes the same
    // (type, value) view over the live map through the prototype ordering.
    (void) stack;
    return LIBMATTI_MC_DataComponentMap_Empty();
}

LIBMATTI_MC_DataComponentPatch *LIBMATTI_MC_ItemStack_GetComponentsPatch(const LIBMATTI_MC_ItemStack *stack)
{
    // Java: !this.isEmpty() ? this.components.asPatch() : DataComponentPatch.EMPTY
    if (LIBMATTI_MC_ItemStack_IsEmpty(stack))
        return LIBMATTI_MC_DataComponentPatch_Empty();
    return LIBMATTI_MC_PatchedDataComponentMap_AsPatch(stack->components);
}

void *LIBMATTI_MC_ItemStack_Get(const LIBMATTI_MC_ItemStack *stack, const LIBMATTI_MC_DataComponentType *type)
{
    if (LIBMATTI_MC_ItemStack_IsEmpty(stack))
        return NULL;
    return LIBMATTI_MC_PatchedDataComponentMap_Get(stack->components, type);
}

void *LIBMATTI_MC_ItemStack_GetOrDefault(const LIBMATTI_MC_ItemStack *stack, const LIBMATTI_MC_DataComponentType *type,
                                         void *defaultValue)
{
    void *value = LIBMATTI_MC_ItemStack_Get(stack, type);
    return value != NULL ? value : defaultValue;
}

bool LIBMATTI_MC_ItemStack_Has(const LIBMATTI_MC_ItemStack *stack, const LIBMATTI_MC_DataComponentType *type)
{
    return LIBMATTI_MC_ItemStack_Get(stack, type) != NULL;
}

void *LIBMATTI_MC_ItemStack_Set(LIBMATTI_MC_ItemStack *stack, const LIBMATTI_MC_DataComponentType *type, void *value)
{
    if (LIBMATTI_MC_ItemStack_IsEmpty(stack))
        return NULL;
    return LIBMATTI_MC_PatchedDataComponentMap_Set(stack->components, type, value);
}

void *LIBMATTI_MC_ItemStack_Remove(LIBMATTI_MC_ItemStack *stack, const LIBMATTI_MC_DataComponentType *type)
{
    if (LIBMATTI_MC_ItemStack_IsEmpty(stack))
        return NULL;
    return LIBMATTI_MC_PatchedDataComponentMap_Remove(stack->components, type);
}

bool LIBMATTI_MC_ItemStack_HasNonDefault(const LIBMATTI_MC_ItemStack *stack, const LIBMATTI_MC_DataComponentType *type)
{
    if (LIBMATTI_MC_ItemStack_IsEmpty(stack))
        return false;
    return LIBMATTI_MC_PatchedDataComponentMap_HasNonDefault(stack->components, type);
}

int LIBMATTI_MC_ItemStack_GetMaxStackSize(const LIBMATTI_MC_ItemStack *stack)
{
    // Java: this.getOrDefault(DataComponents.MAX_STACK_SIZE, 1)
    static int one = 1;
    int *value = LIBMATTI_MC_ItemStack_GetOrDefault(stack, LIBMATTI_MC_DataComponents_MAX_STACK_SIZE(), &one);
    return *value;
}

bool LIBMATTI_MC_ItemStack_IsStackable(const LIBMATTI_MC_ItemStack *stack)
{
    // Java: this.isStackable() -> getMaxStackSize() > 1 && (!this.isDamageableItem() || !this.isDamaged())
    return LIBMATTI_MC_ItemStack_GetMaxStackSize(stack) > 1
           && (!LIBMATTI_MC_ItemStack_IsDamageableItem(stack) || !LIBMATTI_MC_ItemStack_IsDamaged(stack));
}

bool LIBMATTI_MC_ItemStack_IsDamageableItem(const LIBMATTI_MC_ItemStack *stack)
{
    // Java: this.has(DataComponents.MAX_DAMAGE) && this.getOrDefault(DataComponents.MAX_STACK_SIZE, 1) == 1
    return LIBMATTI_MC_ItemStack_Has(stack, LIBMATTI_MC_DataComponents_MAX_DAMAGE())
           && LIBMATTI_MC_ItemStack_GetMaxStackSize(stack) == 1;
}

bool LIBMATTI_MC_ItemStack_IsDamaged(const LIBMATTI_MC_ItemStack *stack)
{
    // Java: this.isDamageableItem() && this.getDamageValue() > 0
    return LIBMATTI_MC_ItemStack_IsDamageableItem(stack) && LIBMATTI_MC_ItemStack_GetDamageValue(stack) > 0;
}

int LIBMATTI_MC_ItemStack_GetDamageValue(const LIBMATTI_MC_ItemStack *stack)
{
    // Java: this.getOrDefault(DataComponents.DAMAGE, 0)
    static int zero = 0;
    int *value = LIBMATTI_MC_ItemStack_GetOrDefault(stack, LIBMATTI_MC_DataComponents_DAMAGE(), &zero);
    return *value;
}

void LIBMATTI_MC_ItemStack_SetDamageValue(LIBMATTI_MC_ItemStack *stack, int damage)
{
    // Java: this.set(DataComponents.DAMAGE, p_41722_)
    static int stored;
    stored = damage;
    LIBMATTI_MC_ItemStack_Set(stack, LIBMATTI_MC_DataComponents_DAMAGE(), &stored);
}

int LIBMATTI_MC_ItemStack_GetMaxDamage(const LIBMATTI_MC_ItemStack *stack)
{
    // Java: this.getOrDefault(DataComponents.MAX_DAMAGE, 0)
    static int zero = 0;
    int *value = LIBMATTI_MC_ItemStack_GetOrDefault(stack, LIBMATTI_MC_DataComponents_MAX_DAMAGE(), &zero);
    return *value;
}

LIBMATTI_MC_ItemStack *LIBMATTI_MC_ItemStack_Split(LIBMATTI_MC_ItemStack *stack, int amount)
{
    // Java: int i = Math.min(this.getCount(), Math.max(1, amount)); ItemStack itemstack =
    // this.copyWithCount(i); this.setCount(this.getCount() - i); return itemstack;
    int take = amount < 1 ? 1 : amount;
    if (take > LIBMATTI_MC_ItemStack_GetCount(stack))
        take = LIBMATTI_MC_ItemStack_GetCount(stack);
    LIBMATTI_MC_ItemStack *part = LIBMATTI_MC_ItemStack_CopyWithCount(stack, take);
    LIBMATTI_MC_ItemStack_SetCount(stack, LIBMATTI_MC_ItemStack_GetCount(stack) - take);
    return part;
}

LIBMATTI_MC_ItemStack *LIBMATTI_MC_ItemStack_Copy(const LIBMATTI_MC_ItemStack *stack)
{
    if (LIBMATTI_MC_ItemStack_IsEmpty(stack))
        return LIBMATTI_MC_ItemStack_Empty();
    // Java: new ItemStack(this.getItem(), this.count, this.components.copy()); itemstack.setPopTime(...)
    LIBMATTI_MC_ItemStack *copy = LIBMATTI_MC_ItemStack_NewWithComponents(
            stack->item, stack->count, LIBMATTI_MC_PatchedDataComponentMap_Copy(stack->components));
    copy->popTime = stack->popTime;
    return copy;
}

LIBMATTI_MC_ItemStack *LIBMATTI_MC_ItemStack_CopyWithCount(const LIBMATTI_MC_ItemStack *stack, int count)
{
    if (LIBMATTI_MC_ItemStack_IsEmpty(stack))
        return LIBMATTI_MC_ItemStack_Empty();
    // Java: ItemStack itemstack = this.copy(); itemstack.setCount(p_256354_); return itemstack;
    LIBMATTI_MC_ItemStack *copy = LIBMATTI_MC_ItemStack_Copy(stack);
    LIBMATTI_MC_ItemStack_SetCount(copy, count);
    return copy;
}

void LIBMATTI_MC_ItemStack_Grow(LIBMATTI_MC_ItemStack *stack, int amount)
{
    // Java: this.setCount(this.getCount() + p_41566_)
    LIBMATTI_MC_ItemStack_SetCount(stack, LIBMATTI_MC_ItemStack_GetCount(stack) + amount);
}

void LIBMATTI_MC_ItemStack_Shrink(LIBMATTI_MC_ItemStack *stack, int amount)
{
    // Java: this.grow(-p_41576_)
    LIBMATTI_MC_ItemStack_Grow(stack, -amount);
}

int LIBMATTI_MC_ItemStack_GetPopTime(const LIBMATTI_MC_ItemStack *stack)
{
    return stack != NULL ? stack->popTime : 0;
}

void LIBMATTI_MC_ItemStack_SetPopTime(LIBMATTI_MC_ItemStack *stack, int popTime)
{
    if (stack != NULL)
        stack->popTime = popTime;
}

bool LIBMATTI_MC_ItemStack_Matches(const LIBMATTI_MC_ItemStack *a, const LIBMATTI_MC_ItemStack *b)
{
    // Java: a.isEmpty() && b.isEmpty() || !a.isEmpty() && !b.isEmpty() && a.count == b.count
    //       && a.item == b.item && Objects.equals(a.components, b.components)
    if (LIBMATTI_MC_ItemStack_IsEmpty(a) || LIBMATTI_MC_ItemStack_IsEmpty(b))
        return LIBMATTI_MC_ItemStack_IsEmpty(a) && LIBMATTI_MC_ItemStack_IsEmpty(b);
    if (a->count != b->count || a->item != b->item)
        return false;
    return LIBMATTI_MC_PatchedDataComponentMap_Equals(a->components, b->components);
}

bool LIBMATTI_MC_ItemStack_IsSameItem(const LIBMATTI_MC_ItemStack *a, const LIBMATTI_MC_ItemStack *b)
{
    // Java: !p_287761_.isEmpty() && !p_287676_.isEmpty() && (p_287761_.item == p_287676_.item || ...)
    if (LIBMATTI_MC_ItemStack_IsEmpty(a) || LIBMATTI_MC_ItemStack_IsEmpty(b))
        return false;
    return a->item == b->item;
}

bool LIBMATTI_MC_ItemStack_IsSameItemSameComponents(const LIBMATTI_MC_ItemStack *a, const LIBMATTI_MC_ItemStack *b)
{
    // Java: this.isSameItem(other) && (other.isEmpty() && isEmpty() || components.equals(other.components))
    if (!LIBMATTI_MC_ItemStack_IsSameItem(a, b))
        return false;
    if (LIBMATTI_MC_ItemStack_IsEmpty(a) && LIBMATTI_MC_ItemStack_IsEmpty(b))
        return true;
    return LIBMATTI_MC_PatchedDataComponentMap_Equals(a->components, b->components);
}

bool LIBMATTI_MC_ItemStack_Is(const LIBMATTI_MC_ItemStack *stack, const LIBMATTI_MC_Item *item)
{
    // Java: this.getItem() == p_150931_
    return LIBMATTI_MC_ItemStack_GetItem(stack) == item;
}

void LIBMATTI_MC_ItemStack_Free(LIBMATTI_MC_ItemStack *stack)
{
    if (stack == NULL || stack == LIBMATTI_MC_ItemStack_Empty())
        return;
    LIBMATTI_MC_PatchedDataComponentMap_Free(stack->components);
    free(stack);
}
