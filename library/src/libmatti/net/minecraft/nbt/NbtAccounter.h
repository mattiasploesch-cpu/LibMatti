// Port of net.minecraft.nbt.NbtAccounter (the size/depth guard every binary NBT
// load runs through).

#ifndef MATTICRAFT_NET_MINECRAFT_NBT_NBTACCOUNTER_H
#define MATTICRAFT_NET_MINECRAFT_NBT_NBTACCOUNTER_H

#include <stddef.h>

// Java: public class NbtAccounter
typedef struct LIBMATTI_MC_NbtAccounter
{
    // Java: private final long quota - 0 means unlimited (unlimitedHeap())
    long long quota;
    // Java: private long usage
    long long usage;
    // Java: private final Deque<String> stack - the port tracks the depth only
    int depth;
} LIBMATTI_MC_NbtAccounter;

// Java: public static NbtAccounter unlimitedHeap()
LIBMATTI_MC_NbtAccounter LIBMATTI_MC_NbtAccounter_UnlimitedHeap(void);
// Java: public static NbtAccounter create(long quota)
LIBMATTI_MC_NbtAccounter LIBMATTI_MC_NbtAccounter_Create(long long quota);

// Java: public void pushDepth() - MAX_DEPTH guard
int LIBMATTI_MC_NbtAccounter_PushDepth(LIBMATTI_MC_NbtAccounter *accounter);
// Java: public void popDepth()
void LIBMATTI_MC_NbtAccounter_PopDepth(LIBMATTI_MC_NbtAccounter *accounter);
// Java: public void accountBytes(long bytes) - aborts over the quota
void LIBMATTI_MC_NbtAccounter_AccountBytes(LIBMATTI_MC_NbtAccounter *accounter, long long bytes);
// Java: public void accountBytes(long bytes, long factor)
void LIBMATTI_MC_NbtAccounter_AccountBytesFactor(LIBMATTI_MC_NbtAccounter *accounter, long long bytes, long long factor);

#endif //MATTICRAFT_NET_MINECRAFT_NBT_NBTACCOUNTER_H
