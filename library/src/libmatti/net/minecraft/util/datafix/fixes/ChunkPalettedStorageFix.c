// Port of net.minecraft.util.datafix.fixes.ChunkPalettedStorageFix (P7.3).
//
// Java: Section.upgrade() reads the three 2048-byte layers, rebuilds the 12 bit
// id per entry (Add << 12 | Blocks << 4 | Data) and writes the section back as
// a palette plus a PackedBitStorage "BlockStates" long array.
//
// Java addresses the block entities through the schema (the output chunk type
// names the field "block_entities" and nests the position as a "pos" int
// array); the port does that rename directly, because the fix runs straight on
// the NBT tree rather than through the type tree.

#include "libmatti/net/minecraft/util/datafix/fixes/ChunkPalettedStorageFix.h"
#include "libmatti/com/mojang/datafixers/DataFixUtils.h"
#include "libmatti/net/minecraft/nbt/CompoundTag.h"
#include "libmatti/net/minecraft/nbt/ListTag.h"
#include "libmatti/net/minecraft/util/datafix/fixes/BlockStateData.h"
#include "libmatti/net/minecraft/util/datafix/fixes/FixChunks.h"

#include <stdlib.h>
#include <string.h>

// Java: DataLayer.get(int x, int y, int z) - position = y << 8 | z << 4 | x,
// the nibble sitting in the low half of the byte when the position is even
static int data_layer_get(const int8_t *data, int x, int y, int z)
{
    int position = y << 8 | z << 4 | x;
    uint8_t byte = (uint8_t) data[position >> 1];
    return (position & 1) == 0 ? (int) (byte & 15) : (int) ((byte >> 4) & 15);
}

// Java: the CrudeIncrementalIntIdentityHashBiMap - Java compares the tag
// objects by identity, the port compares name + properties
typedef struct
{
    char *name;
    LIBMATTI_MC_Nbt_CompoundTag *properties; // owned
    int hasProperties;
} palette_entry;

typedef struct
{
    palette_entry entries[4096];
    int size;
} palette;

static int palette_find(const palette *p, const char *name, const LIBMATTI_MC_Nbt_CompoundTag *properties)
{
    for (int i = 0; i < p->size; i++)
    {
        if (strcmp(p->entries[i].name, name) != 0)
            continue;
        if (p->entries[i].hasProperties != (properties != NULL))
            continue;
        if (properties == NULL)
            return i;
        LIBMATTI_MC_Nbt_CompoundTag *other = p->entries[i].properties;
        if (LIBMATTI_MC_Nbt_CompoundTag_Size(other) != LIBMATTI_MC_Nbt_CompoundTag_Size(properties))
            continue;
        int equal = 1;
        size_t keyCount = 0;
        // Java: updateMapValues over the heightmap map - KeySet hands out
        // the compound's internal array, which the caller must not free
        char **keys = LIBMATTI_MC_Nbt_CompoundTag_KeySet(other, &keyCount);
        for (size_t k = 0; k < keyCount; k++)
        {
            const char *a = LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(other, keys[k], NULL);
            const char *b = a != NULL ? LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(properties, keys[k], NULL) : NULL;
            if (a == NULL || b == NULL || strcmp(a, b) != 0)
            {
                equal = 0;
                break;
            }
        }
        if (equal)
            return i;
    }
    return -1;
}

static int palette_add(palette *p, const char *name, const LIBMATTI_MC_Nbt_CompoundTag *properties)
{
    int at = palette_find(p, name, properties);
    if (at >= 0)
        return at;
    palette_entry *entry = &p->entries[p->size++];
    entry->name = strdup(name);
    entry->properties =
        properties != NULL ? (LIBMATTI_MC_Nbt_CompoundTag *) LIBMATTI_MC_Nbt_Tag_Copy((const LIBMATTI_MC_Nbt_Tag *) properties)
                           : NULL;
    entry->hasProperties = properties != NULL ? 1 : 0;
    return p->size - 1;
}

static void palette_free(palette *p)
{
    for (int i = 0; i < p->size; i++)
    {
        free(p->entries[i].name);
        LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) p->entries[i].properties);
    }
    p->size = 0;
}

// Java: the tag BlockStateData.getTag(id) carries
static LIBMATTI_MC_Nbt_CompoundTag *state_tag(const LIBMATTI_MC_BlockStateData *data)
{
    LIBMATTI_MC_Nbt_CompoundTag *tag = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_PutString(tag, "Name", data->name);
    if (data->propCount > 0)
    {
        LIBMATTI_MC_Nbt_CompoundTag *properties = LIBMATTI_MC_Nbt_CompoundTag_New();
        for (int i = 0; i < data->propCount; i++)
            LIBMATTI_MC_Nbt_CompoundTag_PutString(properties, data->props[i * 2], data->props[i * 2 + 1]);
        LIBMATTI_MC_Nbt_CompoundTag_Put(tag, "Properties", (LIBMATTI_MC_Nbt_Tag *) properties);
    }
    return tag;
}

// Java: Section.upgrade() - the palette + BlockStates rewrite of one section
static bool upgrade_section(LIBMATTI_MC_Nbt_CompoundTag *section)
{
    // Java: this.hasData = p_15195_.get("Blocks").result().isPresent()
    // the Blocks array holds one byte per entry (4096), while Data and Add are
    // 2048-byte nibble layers
    const int8_t *blocks = NULL;
    size_t blocksLength = 0;
    if (!LIBMATTI_MC_Nbt_CompoundTag_GetByteArray(section, "Blocks", &blocks, &blocksLength))
        return false;
    if (blocksLength < 4096)
        return false;

    // Java: the Data and Add layers default to an all-zero DataLayer
    static const int8_t ZERO_LAYER[2048] = {0};
    const int8_t *dataBytes = ZERO_LAYER;
    const int8_t *addBytes = ZERO_LAYER;
    size_t dataLength = 0;
    size_t addLength = 0;
    if (!LIBMATTI_MC_Nbt_CompoundTag_GetByteArray(section, "Data", &dataBytes, &dataLength) || dataLength < 2048)
        dataBytes = ZERO_LAYER;
    if (!LIBMATTI_MC_Nbt_CompoundTag_GetByteArray(section, "Add", &addBytes, &addLength) || addLength < 2048)
        addBytes = ZERO_LAYER;

    palette pal;
    memset(&pal, 0, sizeof(pal));
    uint16_t buffer[4096];

    // Java: this.seen.add(AIR); idFor(palette, AIR); listTag.add(AIR)
    palette_add(&pal, "minecraft:air", NULL);

    for (int i = 0; i < 4096; i++)
    {
        int x = i & 15;
        int y = (i >> 4) & 15;
        int z = (i >> 8) & 15;
        int id = data_layer_get(addBytes, x, y, z) << 12 | (int) ((uint8_t) blocks[i]) << 4 | data_layer_get(dataBytes, x, y, z);

        // Java: BlockStateData.getTag(i1) - MAP[] is indexed by the whole 12 bit
        // id, not by the block nibble, so add/blocks/data all take part
        LIBMATTI_MC_BlockStateData data;
        LIBMATTI_MC_BlockStateData_Of(id, &data);
        LIBMATTI_MC_Nbt_CompoundTag *tag = state_tag(&data);
        LIBMATTI_MC_Nbt_CompoundTag *properties = NULL;
        if (!LIBMATTI_MC_Nbt_CompoundTag_GetCompound(tag, "Properties", &properties))
            properties = NULL; // the state has no properties
        int index = palette_add(&pal, data.name, properties);
        LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) tag);
        buffer[i] = (uint16_t) index;
    }

    // Java: int i = Math.max(4, ceillog2(this.seen.size()))
    int bits = LIBMATTI_MC_DataFixUtils_CeilLog2(pal.size);
    if (bits < 4)
        bits = 4;
    int perLong = 64 / bits;
    size_t longCount = ((size_t) 4096 + (size_t) perLong - 1) / (size_t) perLong;
    int64_t *packed = calloc(longCount, sizeof(int64_t));
    if (packed == NULL)
    {
        palette_free(&pal);
        return false;
    }
    for (int i = 0; i < 4096; i++)
        packed[i / perLong] |= (int64_t) ((uint64_t) buffer[i] << ((i % perLong) * bits));

    // Java: dynamic.set("Palette", list).set("BlockStates", longs)
    LIBMATTI_MC_Nbt_ListTag *paletteTag = LIBMATTI_MC_Nbt_ListTag_New();
    for (int i = 0; i < pal.size; i++)
    {
        LIBMATTI_MC_Nbt_CompoundTag *entry = LIBMATTI_MC_Nbt_CompoundTag_New();
        LIBMATTI_MC_Nbt_CompoundTag_PutString(entry, "Name", pal.entries[i].name);
        if (pal.entries[i].properties != NULL)
        {
            // the palette owns the entry's properties, so the tag gets a copy
            // (Java shares the immutable Dynamic instead)
            LIBMATTI_MC_Nbt_CompoundTag_Put(entry, "Properties",
                                            LIBMATTI_MC_Nbt_Tag_Copy((const LIBMATTI_MC_Nbt_Tag *) pal.entries[i].properties));
        }
        LIBMATTI_MC_Nbt_ListTag_Add(paletteTag, (LIBMATTI_MC_Nbt_Tag *) entry);
    }
    LIBMATTI_MC_Nbt_CompoundTag_Put(section, "Palette", (LIBMATTI_MC_Nbt_Tag *) paletteTag);
    LIBMATTI_MC_Nbt_CompoundTag_PutLongArray(section, "BlockStates", packed, longCount);
    free(packed);

    // Java: remove("Blocks").remove("Data").remove("Add")
    LIBMATTI_MC_Nbt_CompoundTag_Remove(section, "Blocks");
    LIBMATTI_MC_Nbt_CompoundTag_Remove(section, "Data");
    LIBMATTI_MC_Nbt_CompoundTag_Remove(section, "Add");

    palette_free(&pal);
    return true;
}

// Java: the block entity map write - the schema rename plus the "pos" nesting
static void rename_block_entities(LIBMATTI_MC_Nbt_CompoundTag *level)
{
    LIBMATTI_MC_Nbt_ListTag *tileEntities = NULL;
    if (!LIBMATTI_MC_Nbt_CompoundTag_GetList(level, "TileEntities", &tileEntities) || tileEntities == NULL)
        return;

    int32_t baseX = LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(level, "xPos", 0) << 4;
    int32_t baseZ = LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(level, "zPos", 0) << 4;

    LIBMATTI_MC_Nbt_ListTag *out = LIBMATTI_MC_Nbt_ListTag_New();
    int count = LIBMATTI_MC_Nbt_ListTag_Size(tileEntities);
    for (int i = 0; i < count; i++)
    {
        LIBMATTI_MC_Nbt_CompoundTag *entity = NULL;
        if (!LIBMATTI_MC_Nbt_ListTag_GetCompound(tileEntities, i, &entity) || entity == NULL)
            continue;
        int32_t x = LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(entity, "x", 0);
        int32_t y = LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(entity, "y", 0);
        int32_t z = LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(entity, "z", 0);
        int32_t pos[3] = {x, y, z};

        LIBMATTI_MC_Nbt_CompoundTag *converted = LIBMATTI_MC_Nbt_CompoundTag_New();
        LIBMATTI_MC_Nbt_CompoundTag_PutIntArray(converted, "pos", pos, 3);
        LIBMATTI_MC_Nbt_CompoundTag_Merge(converted, entity);
        LIBMATTI_MC_Nbt_CompoundTag_Remove(converted, "x");
        LIBMATTI_MC_Nbt_CompoundTag_Remove(converted, "y");
        LIBMATTI_MC_Nbt_CompoundTag_Remove(converted, "z");
        (void) baseX;
        (void) baseZ;
        LIBMATTI_MC_Nbt_ListTag_Add(out, (LIBMATTI_MC_Nbt_Tag *) converted);
    }
    LIBMATTI_MC_Nbt_CompoundTag_Remove(level, "TileEntities");
    LIBMATTI_MC_Nbt_CompoundTag_Put(level, "block_entities", (LIBMATTI_MC_Nbt_Tag *) out);
}

bool LIBMATTI_MC_ChunkPalettedStorageFix_Apply(LIBMATTI_MC_Nbt_Tag *rootTag)
{
    if (rootTag == NULL || rootTag->id != LIBMATTI_MC_Nbt_TAG_COMPOUND)
        return false;
    LIBMATTI_MC_Nbt_CompoundTag *level = LIBMATTI_MC_FixChunks_LevelOf((LIBMATTI_MC_Nbt_CompoundTag *) rootTag);
    if (level == NULL)
        return false;

    // Java: fix(Dynamic) - without a Sections list the rule is a no-op
    LIBMATTI_MC_Nbt_ListTag *sections = NULL;
    if (!LIBMATTI_MC_Nbt_CompoundTag_GetList(level, "Sections", &sections) || sections == NULL)
        return false;

    bool changed = false;
    int count = LIBMATTI_MC_Nbt_ListTag_Size(sections);
    for (int i = 0; i < count; i++)
    {
        LIBMATTI_MC_Nbt_CompoundTag *section = NULL;
        if (!LIBMATTI_MC_Nbt_ListTag_GetCompound(sections, i, &section) || section == NULL)
            continue;
        if (upgrade_section(section))
            changed = true;
    }

    if (LIBMATTI_MC_Nbt_CompoundTag_Contains(level, "TileEntities"))
    {
        rename_block_entities(level);
        changed = true;
    }
    return changed;
}