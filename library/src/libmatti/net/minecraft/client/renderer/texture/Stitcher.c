// Implementation of net.minecraft.client.renderer.texture.Stitcher. The
// Region tree and the expansion heuristic follow the Java original line by
// line; the sort comparator (descending height, then width, then name) too.

#include "libmatti/net/minecraft/client/renderer/texture/Stitcher.h"

#include "libmatti/net/minecraft/util/Mth.h"

#include <stdlib.h>
#include <string.h>

// Java: record Holder<T>(T entry, int width, int height)
typedef struct Holder
{
    LIBMATTI_MC_Stitcher_Entry entry; // the width/height already include padding
    int width;
    int height;
} Holder;

// Java: public static class Region<T>
typedef struct Region
{
    int originX;
    int originY;
    int width;
    int height;
    struct Region **subSlots;
    size_t subSlotCount;
    Holder holder;   // Java: the holder reference - the port stores the value
    int hasHolder;   // (the stitch copy would dangle otherwise)
} Region;

struct LIBMATTI_MC_Stitcher
{
    int mipLevel;
    // Java: private final List<Holder<T>> texturesToBeStitched
    Holder *holders;
    size_t holderCount;
    size_t holderCapacity;
    // Java: private final List<Region<T>> storage
    Region **storage;
    size_t storageCount;
    size_t storageCapacity;
    int storageX;
    int storageY;
    int maxWidth;
    int maxHeight;
    int padding;
};

// Java: record Holder<T>(T entry, int width, int height) with the padded sizes
static Holder holder_new(const LIBMATTI_MC_Stitcher_Entry *entry, int width, int height)
{
    Holder holder;
    holder.entry = *entry;
    holder.width = width;
    holder.height = height;
    return holder;
}

// Java: private static int smallestFittingMinTexel(int size, int mipLevel)
static int smallest_fitting_min_texel(int size, int mipLevel)
{
    return ((size >> mipLevel) + (((size & ((1 << mipLevel) - 1)) == 0) ? 0 : 1)) << mipLevel;
}

LIBMATTI_MC_Stitcher *LIBMATTI_MC_Stitcher_New(int maxWidth, int maxHeight, int mipLevel, int anisotropy)
{
    LIBMATTI_MC_Stitcher *stitcher = calloc(1, sizeof(LIBMATTI_MC_Stitcher));
    stitcher->mipLevel = mipLevel;
    stitcher->maxWidth = maxWidth;
    stitcher->maxHeight = maxHeight;
    // Java: this.padding = 1 << mipLevel << Mth.clamp(anisotropy - 1, 0, 4)
    stitcher->padding = 1 << mipLevel << LIBMATTI_MC_Mth_ClampI(anisotropy - 1, 0, 4);
    return stitcher;
}

void LIBMATTI_MC_Stitcher_Free(LIBMATTI_MC_Stitcher *stitcher)
{
    if (stitcher == NULL)
        return;

    free(stitcher->holders);

    // The storage regions own their subSlot trees; the holders are copied by
    // value into the sorted array at stitch time (nothing extra to free).
    for (size_t i = 0; i < stitcher->storageCount; i++)
    {
        // walk the tree depth-first
        Region *stack[4096];
        int top = 0;
        stack[top++] = stitcher->storage[i];
        while (top > 0)
        {
            Region *region = stack[--top];
            for (size_t s = 0; s < region->subSlotCount; s++)
            {
                if (top < 4096)
                    stack[top++] = region->subSlots[s];
            }
            free(region->subSlots);
            free(region);
        }
    }
    free(stitcher->storage);
    free(stitcher);
}

// Java: public void registerSprite(T entry)
void LIBMATTI_MC_Stitcher_RegisterSprite(LIBMATTI_MC_Stitcher *stitcher, const LIBMATTI_MC_Stitcher_Entry *entry)
{
    if (stitcher->holderCount == stitcher->holderCapacity)
    {
        stitcher->holderCapacity = stitcher->holderCapacity == 0 ? 16 : stitcher->holderCapacity * 2;
        stitcher->holders = realloc(stitcher->holders, stitcher->holderCapacity * sizeof(Holder));
    }
    stitcher->holders[stitcher->holderCount++] = holder_new(
        entry,
        smallest_fitting_min_texel(entry->width + stitcher->padding * 2, stitcher->mipLevel),
        smallest_fitting_min_texel(entry->height + stitcher->padding * 2, stitcher->mipLevel));
}

// Java: the HOLDER_COMPARATOR: -height, then -width, then name
static int holder_compare(const void *a, const void *b)
{
    const Holder *ha = a;
    const Holder *hb = b;
    if (ha->height != hb->height)
        return hb->height - ha->height;
    if (ha->width != hb->width)
        return hb->width - ha->width;
    return strcmp(ha->entry.name, hb->entry.name);
}

// Java: Region.add(Holder) - the recursive slot fill
static int region_add(Region *region, Holder *holder)
{
    if (region->hasHolder)
        return 0;

    int width = holder->width;
    int height = holder->height;
    if (width > region->width || height > region->height)
        return 0;

    if (width == region->width && height == region->height)
    {
        region->holder = *holder;
        region->hasHolder = 1;
        return 1;
    }

    // Java: if (this.subSlots == null) { ... split ... }
    if (region->subSlots == NULL)
    {
        region->subSlotCount = 1;
        region->subSlots = malloc(sizeof(Region *));
        region->subSlots[0] = calloc(1, sizeof(Region));
        *region->subSlots[0] = (Region){region->originX, region->originY, width, height, {0}, 0};

        int k = region->width - width;  // remaining right
        int l = region->height - height; // remaining below
        if (l > 0 && k > 0)
        {
            int i1 = l > k ? l : k; // Java: Math.max(this.height... no - max(height, k) with height=l? keep Java
            // Java: int i1 = Math.max(this.height, k); int j1 = Math.max(this.width, l);
            i1 = region->height > k ? region->height : k;
            int j1 = region->width > l ? region->width : l;
            region->subSlotCount = 3;
            region->subSlots = realloc(region->subSlots, 3 * sizeof(Region *));
            if (i1 >= j1)
            {
                // below, then right
                region->subSlots[1] = calloc(1, sizeof(Region));
                *region->subSlots[1] = (Region){region->originX, region->originY + height, width, l, {0}, 0};
                region->subSlots[2] = calloc(1, sizeof(Region));
                *region->subSlots[2] = (Region){region->originX + width, region->originY, k, region->height, {0}, 0};
            }
            else
            {
                // right, then below
                region->subSlots[1] = calloc(1, sizeof(Region));
                *region->subSlots[1] = (Region){region->originX + width, region->originY, k, height, {0}, 0};
                region->subSlots[2] = calloc(1, sizeof(Region));
                *region->subSlots[2] = (Region){region->originX, region->originY + height, region->width, l, {0}, 0};
            }
        }
        else if (k == 0)
        {
            region->subSlotCount = 2;
            region->subSlots = realloc(region->subSlots, 2 * sizeof(Region *));
            region->subSlots[1] = calloc(1, sizeof(Region));
            *region->subSlots[1] = (Region){region->originX, region->originY + height, width, l, {0}, 0};
        }
        else if (l == 0)
        {
            region->subSlotCount = 2;
            region->subSlots = realloc(region->subSlots, 2 * sizeof(Region *));
            region->subSlots[1] = calloc(1, sizeof(Region));
            *region->subSlots[1] = (Region){region->originX + width, region->originY, k, height, {0}, 0};
        }
    }

    for (size_t i = 0; i < region->subSlotCount; i++)
    {
        if (region_add(region->subSlots[i], holder))
            return 1;
    }
    return 0;
}

// Java: private boolean expand(Holder)
static int expand(LIBMATTI_MC_Stitcher *stitcher, Holder *holder)
{
    int i = LIBMATTI_MC_Mth_SmallestEncompassingPowerOfTwo(stitcher->storageX);
    int j = LIBMATTI_MC_Mth_SmallestEncompassingPowerOfTwo(stitcher->storageY);
    int k = LIBMATTI_MC_Mth_SmallestEncompassingPowerOfTwo(stitcher->storageX + holder->width);
    int l = LIBMATTI_MC_Mth_SmallestEncompassingPowerOfTwo(stitcher->storageY + holder->height);
    int flag1 = k <= stitcher->maxWidth;
    int flag2 = l <= stitcher->maxHeight;
    if (!flag1 && !flag2)
        return 0;

    // Java: boolean flag3 = flag1 && i != k; flag4 = flag2 && j != l;
    int flag3 = flag1 && i != k;
    int flag4 = flag2 && j != l;
    int flag;
    if (flag3 ^ flag4)
        flag = flag3;
    else
        flag = flag1 && i <= j;

    Region *region = calloc(1, sizeof(Region));
    if (flag)
    {
        if (stitcher->storageY == 0)
            stitcher->storageY = l;
        *region = (Region){stitcher->storageX, 0, k - stitcher->storageX, stitcher->storageY, {0}, 0};
        stitcher->storageX = k;
    }
    else
    {
        *region = (Region){0, stitcher->storageY, stitcher->storageX, l - stitcher->storageY, {0}, 0};
        stitcher->storageY = l;
    }

    region_add(region, holder);

    if (stitcher->storageCount == stitcher->storageCapacity)
    {
        stitcher->storageCapacity = stitcher->storageCapacity == 0 ? 8 : stitcher->storageCapacity * 2;
        stitcher->storage = realloc(stitcher->storage, stitcher->storageCapacity * sizeof(Region *));
    }
    stitcher->storage[stitcher->storageCount++] = region;
    return 1;
}

// Java: private boolean addToStorage(Holder)
static int add_to_storage(LIBMATTI_MC_Stitcher *stitcher, Holder *holder)
{
    for (size_t i = 0; i < stitcher->storageCount; i++)
    {
        if (region_add(stitcher->storage[i], holder))
            return 1;
    }
    return expand(stitcher, holder);
}

// Java: public void stitch() - the port reports the failure through the return
int LIBMATTI_MC_Stitcher_Stitch(LIBMATTI_MC_Stitcher *stitcher)
{
    // Java: List<Holder> list = new ArrayList<>(texturesToBeStitched); list.sort(...)
    Holder *sorted = malloc(stitcher->holderCount * sizeof(Holder));
    memcpy(sorted, stitcher->holders, stitcher->holderCount * sizeof(Holder));
    qsort(sorted, stitcher->holderCount, sizeof(Holder), holder_compare);

    for (size_t i = 0; i < stitcher->holderCount; i++)
    {
        if (!add_to_storage(stitcher, &sorted[i]))
        {
            free(sorted);
            return 0;
        }
    }
    free(sorted);
    return 1;
}

int LIBMATTI_MC_Stitcher_GetWidth(const LIBMATTI_MC_Stitcher *stitcher)
{
    return stitcher->storageX;
}

int LIBMATTI_MC_Stitcher_GetHeight(const LIBMATTI_MC_Stitcher *stitcher)
{
    return stitcher->storageY;
}

// Java: Region.walk(SpriteLoader, padding)
static void region_walk(const Region *region, void *userData, LIBMATTI_MC_Stitcher_SpriteLoader loader, int padding)
{
    if (region->hasHolder)
    {
        loader(userData, &region->holder.entry, region->originX, region->originY, padding);
    }
    else if (region->subSlots != NULL)
    {
        for (size_t i = 0; i < region->subSlotCount; i++)
            region_walk(region->subSlots[i], userData, loader, padding);
    }
}

// Java: public void gatherSprites(SpriteLoader)
void LIBMATTI_MC_Stitcher_GatherSprites(const LIBMATTI_MC_Stitcher *stitcher, void *userData,
                                        LIBMATTI_MC_Stitcher_SpriteLoader loader)
{
    for (size_t i = 0; i < stitcher->storageCount; i++)
        region_walk(stitcher->storage[i], userData, loader, stitcher->padding);
}
