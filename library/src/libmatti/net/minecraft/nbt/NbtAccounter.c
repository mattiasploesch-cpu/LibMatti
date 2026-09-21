// Port of net.minecraft.nbt.NbtAccounter.

#include "libmatti/net/minecraft/nbt/NbtAccounter.h"

#include "libmatti/cpw/modlauncher/LogManager.h"

#include <stdio.h>
#include <stdlib.h>

// Java: public static final int MAX_DEPTH = 512 (declared on Tag)
#define LIBMATTI_MC_Nbt_MAX_DEPTH 512

// Java: public static NbtAccounter unlimitedHeap()
LIBMATTI_MC_NbtAccounter LIBMATTI_MC_NbtAccounter_UnlimitedHeap(void)
{
    LIBMATTI_MC_NbtAccounter accounter = {0, 0, 0};
    return accounter;
}

// Java: public static NbtAccounter create(long quota)
LIBMATTI_MC_NbtAccounter LIBMATTI_MC_NbtAccounter_Create(long long quota)
{
    LIBMATTI_MC_NbtAccounter accounter = {quota, 0, 0};
    return accounter;
}

// Java: public void pushDepth() - the guard is Java's throw, the port aborts
int LIBMATTI_MC_NbtAccounter_PushDepth(LIBMATTI_MC_NbtAccounter *accounter)
{
    if (++accounter->depth > LIBMATTI_MC_Nbt_MAX_DEPTH)
    {
        LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
        LIBMATTI_ML_Logger_Error(logger, NULL, "Tried to read NBT tag with too high complexity, depth > 512");
        exit(1);
    }
    return 1;
}

// Java: public void popDepth()
void LIBMATTI_MC_NbtAccounter_PopDepth(LIBMATTI_MC_NbtAccounter *accounter)
{
    if (accounter->depth > 0)
        accounter->depth--;
}

// Java: public void accountBytes(long bytes) - the guard is Java's NbtAccounterException
void LIBMATTI_MC_NbtAccounter_AccountBytes(LIBMATTI_MC_NbtAccounter *accounter, long long bytes)
{
    if (accounter->quota == 0)
        return;
    accounter->usage += bytes;
    if (accounter->usage > accounter->quota)
    {
        char detail[160];
        snprintf(detail, sizeof(detail), "%lld bytes, max allowed: %lld", accounter->usage, accounter->quota);
        LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
        LIBMATTI_ML_Logger_Error(logger, NULL, "Tried to read NBT tag that was too big; tried to allocate: {}", detail);
        exit(1);
    }
}

// Java: public void accountBytes(long bytes, long factor)
void LIBMATTI_MC_NbtAccounter_AccountBytesFactor(LIBMATTI_MC_NbtAccounter *accounter, long long bytes, long long factor)
{
    LIBMATTI_MC_NbtAccounter_AccountBytes(accounter, bytes * factor);
}
