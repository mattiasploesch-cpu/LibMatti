// ItemStack harness: the component round trip (prototype -> patch -> get/set/remove),
// the count mechanics (split/copy/grow/shrink), the stackable/damageable predicates and
// the EMPTY stack semantics.

#include "libmatti/net/minecraft/core/component/DataComponents.h"
#include "libmatti/net/minecraft/Bootstrap.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaItems.h"
#include "libmatti/net/minecraft/world/item/Item.h"
#include "libmatti/net/minecraft/world/item/ItemStack.h"

#include <stdio.h>
#include <string.h>

static int checks = 0;

#define CHECK(cond)                                                          \
    do                                                                       \
    {                                                                        \
        checks++;                                                            \
        if (!(cond))                                                         \
        {                                                                    \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);           \
            return 1;                                                        \
        }                                                                    \
    } while (0)

int main(void)
{
    LIBMATTI_MC_Bootstrap_BootStrap();

    // EMPTY stack: item resolves to AIR, every component access is inert
    LIBMATTI_MC_ItemStack *empty = LIBMATTI_MC_ItemStack_Empty();
    CHECK(LIBMATTI_MC_ItemStack_IsEmpty(empty));
    CHECK(LIBMATTI_MC_ItemStack_GetItem(empty) == LIBMATTI_MC_VanillaItems_AIR());
    CHECK(LIBMATTI_MC_ItemStack_Get(empty, LIBMATTI_MC_DataComponents_MAX_STACK_SIZE()) == NULL);

    // Prototype components come through: every vanilla item starts at MAX_STACK_SIZE 64
    LIBMATTI_MC_Item *stone = LIBMATTI_MC_VanillaItems_GetByName("STONE");
    CHECK(stone != NULL);
    LIBMATTI_MC_ItemStack *stack = LIBMATTI_MC_ItemStack_New(stone);
    CHECK(!LIBMATTI_MC_ItemStack_IsEmpty(stack));
    CHECK(LIBMATTI_MC_ItemStack_GetCount(stack) == 1);
    CHECK(LIBMATTI_MC_ItemStack_GetItem(stack) == stone);
    int *maxStack = LIBMATTI_MC_ItemStack_Get(stack, LIBMATTI_MC_DataComponents_MAX_STACK_SIZE());
    CHECK(maxStack != NULL && *maxStack == 64);
    CHECK(LIBMATTI_MC_ItemStack_GetMaxStackSize(stack) == 64);
    CHECK(LIBMATTI_MC_ItemStack_IsStackable(stack));

    // The patch overrides the prototype and remove falls back
    static int five = 5;
    LIBMATTI_MC_ItemStack_Set(stack, LIBMATTI_MC_DataComponents_MAX_STACK_SIZE(), &five);
    int *patched = LIBMATTI_MC_ItemStack_Get(stack, LIBMATTI_MC_DataComponents_MAX_STACK_SIZE());
    CHECK(patched != NULL && *patched == 5);
    CHECK(LIBMATTI_MC_ItemStack_HasNonDefault(stack, LIBMATTI_MC_DataComponents_MAX_STACK_SIZE()));
    LIBMATTI_MC_ItemStack_Remove(stack, LIBMATTI_MC_DataComponents_MAX_STACK_SIZE());
    // Java: the empty Optional in the patch shadows the prototype - get returns null,
    // getOrDefault falls back to the caller's default (that is why getMaxStackSize
    // reads getOrDefault(MAX_STACK_SIZE, 1))
    CHECK(LIBMATTI_MC_ItemStack_Get(stack, LIBMATTI_MC_DataComponents_MAX_STACK_SIZE()) == NULL);
    static int one = 1;
    CHECK(LIBMATTI_MC_ItemStack_GetOrDefault(stack, LIBMATTI_MC_DataComponents_MAX_STACK_SIZE(), &one) == &one);

    // Count mechanics: split takes from the tail, grow/shrink wrap setCount
    LIBMATTI_MC_ItemStack_SetCount(stack, 30);
    LIBMATTI_MC_ItemStack *part = LIBMATTI_MC_ItemStack_Split(stack, 12);
    CHECK(LIBMATTI_MC_ItemStack_GetCount(part) == 12);
    CHECK(LIBMATTI_MC_ItemStack_GetCount(stack) == 18);
    LIBMATTI_MC_ItemStack_Grow(stack, 7);
    CHECK(LIBMATTI_MC_ItemStack_GetCount(stack) == 25);
    LIBMATTI_MC_ItemStack_Shrink(stack, 5);
    CHECK(LIBMATTI_MC_ItemStack_GetCount(stack) == 20);

    // copy keeps item + components, copyWithCount clamps through setCount
    LIBMATTI_MC_ItemStack *copy = LIBMATTI_MC_ItemStack_Copy(stack);
    CHECK(LIBMATTI_MC_ItemStack_Matches(copy, stack));
    CHECK(LIBMATTI_MC_ItemStack_IsSameItemSameComponents(copy, stack));
    LIBMATTI_MC_ItemStack *four = LIBMATTI_MC_ItemStack_CopyWithCount(stack, 4);
    CHECK(LIBMATTI_MC_ItemStack_GetCount(four) == 4);
    CHECK(!LIBMATTI_MC_ItemStack_Matches(four, stack));

    // durability() flips the item to unstackable + damageable (Java: MAX_DAMAGE + MAX_STACK_SIZE 1)
    LIBMATTI_MC_ItemProperties *props = LIBMATTI_MC_ItemProperties_New();
    LIBMATTI_MC_ItemProperties_Durability(props, 250);
    LIBMATTI_MC_Item *sword = LIBMATTI_MC_Item_New(props);
    LIBMATTI_MC_ItemStack *worn = LIBMATTI_MC_ItemStack_New(sword);
    CHECK(LIBMATTI_MC_ItemStack_IsDamageableItem(worn));
    CHECK(!LIBMATTI_MC_ItemStack_IsStackable(worn));
    CHECK(LIBMATTI_MC_ItemStack_GetMaxDamage(worn) == 250);
    LIBMATTI_MC_ItemStack_SetDamageValue(worn, 10);
    CHECK(LIBMATTI_MC_ItemStack_IsDamaged(worn));
    CHECK(LIBMATTI_MC_ItemStack_GetDamageValue(worn) == 10);

    printf("itemstack: %d checks ok\n", checks);
    return 0;
}
