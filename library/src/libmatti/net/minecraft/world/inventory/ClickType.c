// Port of net.minecraft.world.inventory.ClickType (the enum) + DataSlot
// (the abstract int slot over the three storage flavours). The ClickType id
// IS the enum constant (Java's wire codec maps the ids 0..6); the DataSlot
// storage rides the tagged union the header declares.

#include "libmatti/net/minecraft/world/inventory/ClickType.h"

#include <string.h>

int LIBMATTI_MC_ClickType_Id(LIBMATTI_MC_ClickType type)
{
    return (int) type; // Java: the enum ids 0..6 in declaration order
}

LIBMATTI_MC_DataSlot LIBMATTI_MC_DataSlot_Standalone(void)
{
    // Java: standalone() - the private int value impl
    LIBMATTI_MC_DataSlot slot;
    memset(&slot, 0, sizeof(slot));
    slot.kind = LIBMATTI_MC_DataSlot_STANDALONE;
    return slot;
}

LIBMATTI_MC_DataSlot LIBMATTI_MC_DataSlot_Shared(int *array, int index)
{
    // Java: shared(int[], int) - the array element impl
    LIBMATTI_MC_DataSlot slot;
    memset(&slot, 0, sizeof(slot));
    slot.kind = LIBMATTI_MC_DataSlot_SHARED;
    slot.sharedArray = array;
    slot.sharedIndex = index;
    return slot;
}

int LIBMATTI_MC_DataSlot_Get(const LIBMATTI_MC_DataSlot *slot)
{
    if (slot == NULL)
        return 0;
    switch (slot->kind)
    {
    case LIBMATTI_MC_DataSlot_SHARED:
        return slot->sharedArray != NULL ? slot->sharedArray[slot->sharedIndex] : 0;
    case LIBMATTI_MC_DataSlot_STANDALONE:
    case LIBMATTI_MC_DataSlot_CONTAINER:
    default:
        return slot->value;
    }
}

void LIBMATTI_MC_DataSlot_Set(LIBMATTI_MC_DataSlot *slot, int value)
{
    if (slot == NULL)
        return;
    switch (slot->kind)
    {
    case LIBMATTI_MC_DataSlot_SHARED:
        if (slot->sharedArray != NULL)
            slot->sharedArray[slot->sharedIndex] = value;
        return;
    case LIBMATTI_MC_DataSlot_STANDALONE:
    case LIBMATTI_MC_DataSlot_CONTAINER:
    default:
        slot->value = value;
        return;
    }
}

bool LIBMATTI_MC_DataSlot_CheckAndClearUpdateFlag(LIBMATTI_MC_DataSlot *slot)
{
    if (slot == NULL)
        return false;
    int current = LIBMATTI_MC_DataSlot_Get(slot);
    bool changed = current != slot->prevValue;
    slot->prevValue = current;
    return changed;
}
