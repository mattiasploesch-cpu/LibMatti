// Port of net.minecraft.world.inventory.ClickType + DataSlot.
// ClickType is the pure enum (the network codec rides the netty port);
// DataSlot collapses Java's abstract class into one struct over the three
// storage flavours (standalone / shared array / container-backed).

#ifndef MATTICRAFT_MC_WORLD_INVENTORY_CLICKTYPE_H
#define MATTICRAFT_MC_WORLD_INVENTORY_CLICKTYPE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public enum ClickType
typedef enum LIBMATTI_MC_ClickType
{
    LIBMATTI_MC_ClickType_PICKUP = 0,
    LIBMATTI_MC_ClickType_QUICK_MOVE = 1,
    LIBMATTI_MC_ClickType_SWAP = 2,
    LIBMATTI_MC_ClickType_CLONE = 3,
    LIBMATTI_MC_ClickType_THROW = 4,
    LIBMATTI_MC_ClickType_QUICK_CRAFT = 5,
    LIBMATTI_MC_ClickType_PICKUP_ALL = 6,
} LIBMATTI_MC_ClickType;

// Java: ClickType.id() - the wire id (the enum constant order)
int LIBMATTI_MC_ClickType_Id(LIBMATTI_MC_ClickType type);

// Java: public abstract class DataSlot - the int slot with the update flag
typedef struct LIBMATTI_MC_DataSlot
{
    // the storage flavour (the tagged union over Java's three anonymous impls)
    enum
    {
        LIBMATTI_MC_DataSlot_STANDALONE,
        LIBMATTI_MC_DataSlot_SHARED,
        LIBMATTI_MC_DataSlot_CONTAINER,
    } kind;
    // STANDALONE: private int value
    int value;
    // SHARED: DataSlot.shared(int[], int)
    int *sharedArray;
    int sharedIndex;
    // Java: private int prevValue (the checkAndClearUpdateFlag state)
    int prevValue;
} LIBMATTI_MC_DataSlot;

// Java: public static DataSlot standalone()
LIBMATTI_MC_DataSlot LIBMATTI_MC_DataSlot_Standalone(void);
// Java: public static DataSlot shared(int[], int)
LIBMATTI_MC_DataSlot LIBMATTI_MC_DataSlot_Shared(int *array, int index);
// Java: public abstract int get()
int LIBMATTI_MC_DataSlot_Get(const LIBMATTI_MC_DataSlot *slot);
// Java: public abstract void set(int)
void LIBMATTI_MC_DataSlot_Set(LIBMATTI_MC_DataSlot *slot, int value);
// Java: public boolean checkAndClearUpdateFlag()
bool LIBMATTI_MC_DataSlot_CheckAndClearUpdateFlag(LIBMATTI_MC_DataSlot *slot);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_INVENTORY_CLICKTYPE_H
