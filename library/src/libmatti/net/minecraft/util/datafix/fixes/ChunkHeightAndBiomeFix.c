// Port of net.minecraft.util.datafix.fixes.ChunkHeightAndBiomeFix (P7.3).
//
// Java moves the 16 old sections (Y 0..15) into the 24 sections of the
// -64..320 world (index 0..23, Y -4..19), shifts every heightmap entry by the
// 64 block offset (the heightmaps store minY-relative values) and turns the
// flat 1024 entry biome array into the 64 entry per-section containers.

#include "libmatti/net/minecraft/util/datafix/fixes/ChunkHeightAndBiomeFix.h"
#include "libmatti/com/mojang/datafixers/DataFixUtils.h"
#include "libmatti/net/minecraft/nbt/CompoundTag.h"
#include "libmatti/net/minecraft/nbt/ListTag.h"
#include "libmatti/net/minecraft/util/datafix/fixes/BiomeIds.h"
#include "libmatti/net/minecraft/util/datafix/fixes/FixChunks.h"

#include <stdlib.h>
#include <string.h>

#define OLD_SECTION_COUNT 16
#define NEW_SECTION_COUNT 24
#define NEW_MIN_SECTION_Y (-4)
#define HEIGHTMAP_OFFSET 64

// Java: the heightmap keys the 1.17 chunks carried over
static const char *const HEIGHTMAP_KEYS[] = {
    "MOTION_BLOCKING", "MOTION_BLOCKING_NO_LEAVES", "OCEAN_FLOOR", "WORLD_SURFACE", "OCEAN_FLOOR_WG", "WORLD_SURFACE_WG",
};

// Java: getFixedHeightmap(Dynamic) - every 9 bit entry moves by 64, capped at
// the 9 bit maximum (511); a zero entry (nothing recorded) stays zero
static void shift_heightmap_word(int64_t word, int64_t *out)
{
    int64_t result = 0;
    for (int j = 0; j + 9 <= 64; j += 9)
    {
        int64_t value = (word >> j) & 511;
        int64_t shifted = value == 0 ? 0 : (value + HEIGHTMAP_OFFSET > 511 ? 511 : value + HEIGHTMAP_OFFSET);
        result |= shifted << j;
    }
    *out = result;
}

static void shift_heightmaps(LIBMATTI_MC_Nbt_CompoundTag *level)
{
    LIBMATTI_MC_Nbt_CompoundTag *heightmaps = NULL;
    if (!LIBMATTI_MC_Nbt_CompoundTag_GetCompound(level, "Heightmaps", &heightmaps) || heightmaps == NULL)
        return;
    for (size_t k = 0; k < sizeof(HEIGHTMAP_KEYS) / sizeof(HEIGHTMAP_KEYS[0]); k++)
    {
        const int64_t *raw = NULL;
        size_t rawLength = 0;
        if (!LIBMATTI_MC_Nbt_CompoundTag_GetLongArray(heightmaps, HEIGHTMAP_KEYS[k], &raw, &rawLength))
            continue;
        int64_t *shifted = malloc(rawLength * sizeof(int64_t));
        if (shifted == NULL)
            continue;
        for (size_t i = 0; i < rawLength; i++)
            shift_heightmap_word(raw[i], &shifted[i]);
        LIBMATTI_MC_Nbt_CompoundTag_PutLongArray(heightmaps, HEIGHTMAP_KEYS[k], shifted, rawLength);
        free(shifted);
    }
}

// Java: the output chunk schema nests the section storage under "block_states"
// with the fields "palette" and "data" - the pre-1.16.2 sections carry them
// directly as "Palette" and "BlockStates". Java gets that rename from the type
// tree, so the port does it while it rebuilds the sections.
static void nest_block_states(LIBMATTI_MC_Nbt_CompoundTag *section, LIBMATTI_MC_Nbt_CompoundTag *old)
{
    LIBMATTI_MC_Nbt_Tag *palette = LIBMATTI_MC_Nbt_CompoundTag_Get(old, "Palette");
    if (palette == NULL)
        return;
    LIBMATTI_MC_Nbt_Tag *data = LIBMATTI_MC_Nbt_CompoundTag_Get(old, "BlockStates");
    LIBMATTI_MC_Nbt_CompoundTag *blockStates = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_Put(blockStates, "palette", LIBMATTI_MC_Nbt_Tag_Copy(palette));
    if (data != NULL)
        LIBMATTI_MC_Nbt_CompoundTag_Put(blockStates, "data", LIBMATTI_MC_Nbt_Tag_Copy(data));
    LIBMATTI_MC_Nbt_CompoundTag_Put(section, "block_states", (LIBMATTI_MC_Nbt_Tag *) blockStates);
}

// Java: dynamic2 - the single entry air container every section without
// block data gets, both for the carried over sections and for the eight the
// rule adds around the old range
static LIBMATTI_MC_Nbt_CompoundTag *make_air_container(void)
{
    LIBMATTI_MC_Nbt_CompoundTag *blockStates = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_ListTag *palette = LIBMATTI_MC_Nbt_ListTag_New();
    LIBMATTI_MC_Nbt_CompoundTag *air = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_PutString(air, "Name", "minecraft:air");
    LIBMATTI_MC_Nbt_ListTag_Add(palette, (LIBMATTI_MC_Nbt_Tag *) air);
    LIBMATTI_MC_Nbt_CompoundTag_Put(blockStates, "palette", (LIBMATTI_MC_Nbt_Tag *) palette);
    return blockStates;
}

// Java: makeBiomeContainer(Dynamic, Int2IntFunction) - a palette over the 64
// biome ids of one section layer plus the packed data (omitted for a single id).
//
// "index" is the index into the 24 container array, i.e. the section Y plus 4:
// getBiomeContainers fills dynamic[j] from the old layer j - 4, so the four
// containers below the old range and the four above it fall back to the border
// layers (Java: makeBiomeContainer(..., k % 16) and (..., k % 16 + 1008)).
static LIBMATTI_MC_Nbt_CompoundTag *make_biome_container(const int32_t *biomes, size_t biomeCount, int index)
{
    int ids[64];
    int order[64];
    int distinct = 0;

    int base;
    int modulo;
    if (biomeCount == 1536)
    {
        base = index * 64;
        modulo = 0;
    }
    else if (index < 4)
    {
        base = 0; // the lowest old layer
        modulo = 16;
    }
    else if (index >= 20)
    {
        base = 1008; // the highest old layer
        modulo = 16;
    }
    else
    {
        base = (index - 4) * 64; // the old layer this section grew into
        modulo = 0;
    }

    for (int i = 0; i < 64; i++)
    {
        int at = base + (modulo != 0 ? i % modulo : i);
        if (at >= (int) biomeCount)
            at = 0;
        ids[i] = biomes[at] & 0xFF;
        int seen = -1;
        for (int k = 0; k < distinct; k++)
            if (order[k] == ids[i])
            {
                seen = k;
                break;
            }
        if (seen < 0)
            order[distinct++] = ids[i];
    }

    LIBMATTI_MC_Nbt_CompoundTag *container = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_ListTag *palette = LIBMATTI_MC_Nbt_ListTag_New();
    for (int i = 0; i < distinct; i++)
        LIBMATTI_MC_Nbt_ListTag_Add(palette, LIBMATTI_MC_Nbt_StringTag_Of(LIBMATTI_MC_BiomeIds_NameOf(order[i])));
    LIBMATTI_MC_Nbt_CompoundTag_Put(container, "palette", (LIBMATTI_MC_Nbt_Tag *) palette);

    // Java: int i2 = ceillog2(palette.size()); a single id needs no data
    int bits = LIBMATTI_MC_DataFixUtils_CeilLog2(distinct);
    if (bits == 0)
        return container;

    int perLong = 64 / bits;
    size_t longCount = ((size_t) 64 + (size_t) perLong - 1) / (size_t) perLong;
    int64_t *packed = calloc(longCount, sizeof(int64_t));
    if (packed == NULL)
        return container;
    int outIndex = 0;
    int shift = 0;
    for (int i = 0; i < 64; i++)
    {
        int index = 0;
        for (int k = 0; k < distinct; k++)
            if (order[k] == ids[i])
                index = k;
        packed[outIndex] |= (int64_t) ((uint64_t) index << shift);
        shift += bits;
        if (shift + bits > 64)
        {
            outIndex++;
            shift = 0;
        }
    }
    LIBMATTI_MC_Nbt_CompoundTag_PutLongArray(container, "data", packed, longCount);
    free(packed);
    return container;
}

bool LIBMATTI_MC_ChunkHeightAndBiomeFix_Apply(LIBMATTI_MC_Nbt_Tag *rootTag)
{
    if (rootTag == NULL || rootTag->id != LIBMATTI_MC_Nbt_TAG_COMPOUND)
        return false;
    LIBMATTI_MC_Nbt_CompoundTag *level = LIBMATTI_MC_FixChunks_LevelOf((LIBMATTI_MC_Nbt_CompoundTag *) rootTag);
    if (level == NULL)
        return false;

    // Java: the flat biome array is consumed and dropped
    bool changed = false;
    const int32_t *biomes = NULL;
    size_t biomeCount = 0;
    bool hasBiomes = LIBMATTI_MC_Nbt_CompoundTag_GetIntArray(level, "Biomes", &biomes, &biomeCount) && biomeCount >= 1024;
    if (hasBiomes)
        LIBMATTI_MC_Nbt_CompoundTag_Remove(level, "Biomes");

    shift_heightmaps(level);
    changed = true;

    // Java: the rule is registered on the schema that introduced the 384 block
    // world, so it only ever sees the 16 section shape. The port's chain runs
    // the whole list, so it checks the shape itself: a chunk that already has
    // the flat 24 section list is converted.
    if (!LIBMATTI_MC_Nbt_CompoundTag_Contains(level, "Sections"))
        return false;
    LIBMATTI_MC_Nbt_ListTag *sections = NULL;
    if (!LIBMATTI_MC_Nbt_CompoundTag_GetList(level, "Sections", &sections) || sections == NULL)
        return false;

    // Java: UpgradeChunk's ctor does this.sections[section.y] = section - the old
    // sections are placed by their Y field, not by their list position
    int oldCount = LIBMATTI_MC_Nbt_ListTag_Size(sections);
    if (oldCount > OLD_SECTION_COUNT)
        oldCount = OLD_SECTION_COUNT;

    LIBMATTI_MC_Nbt_ListTag *newSections = LIBMATTI_MC_Nbt_ListTag_New();
    for (int i = 0; i < NEW_SECTION_COUNT; i++)
    {
        LIBMATTI_MC_Nbt_CompoundTag *section = LIBMATTI_MC_Nbt_CompoundTag_New();
        // Java: the new sections carry their absolute section Y
        LIBMATTI_MC_Nbt_CompoundTag_PutByte(section, "Y", (int8_t) (i + NEW_MIN_SECTION_Y));
        for (int at = 0; at < oldCount; at++)
        {
            LIBMATTI_MC_Nbt_CompoundTag *old = NULL;
            if (!LIBMATTI_MC_Nbt_ListTag_GetCompound(sections, at, &old) || old == NULL)
                continue;
            int8_t oldY = 0;
            if (!LIBMATTI_MC_Nbt_CompoundTag_GetByte(old, "Y", &oldY))
                oldY = (int8_t) at;
            if (oldY != i + NEW_MIN_SECTION_Y)
                continue;
            // Java: the old fields ride over into the new section
            LIBMATTI_MC_Nbt_Tag *blockStates = LIBMATTI_MC_Nbt_CompoundTag_Get(old, "block_states");
            if (blockStates != NULL)
                LIBMATTI_MC_Nbt_CompoundTag_Put(section, "block_states", LIBMATTI_MC_Nbt_Tag_Copy(blockStates));
            else
                nest_block_states(section, old);
            break;
        }
        // Java: the sections the rule adds around the old range carry the air
        // container, and so does every section the old list left out
        if (LIBMATTI_MC_Nbt_CompoundTag_Get(section, "block_states") == NULL)
            LIBMATTI_MC_Nbt_CompoundTag_Put(section, "block_states", (LIBMATTI_MC_Nbt_Tag *) make_air_container());
        if (hasBiomes)
        {
            // Java: dynamic1[i] - the container array is indexed by Y + 4
            LIBMATTI_MC_Nbt_CompoundTag *biome = make_biome_container(biomes, biomeCount, i);
            LIBMATTI_MC_Nbt_CompoundTag_Put(section, "biomes", (LIBMATTI_MC_Nbt_Tag *) biome);
        }
        else
        {
            // Java: Arrays.fill(dynamic, makePalettedContainer(createList(
            // createString("minecraft:plains"))))
            LIBMATTI_MC_Nbt_CompoundTag *biome = LIBMATTI_MC_Nbt_CompoundTag_New();
            LIBMATTI_MC_Nbt_ListTag *palette = LIBMATTI_MC_Nbt_ListTag_New();
            LIBMATTI_MC_Nbt_ListTag_Add(palette, LIBMATTI_MC_Nbt_StringTag_Of("minecraft:plains"));
            LIBMATTI_MC_Nbt_CompoundTag_Put(biome, "palette", (LIBMATTI_MC_Nbt_Tag *) palette);
            LIBMATTI_MC_Nbt_CompoundTag_Put(section, "biomes", (LIBMATTI_MC_Nbt_Tag *) biome);
        }
        LIBMATTI_MC_Nbt_ListTag_Add(newSections, (LIBMATTI_MC_Nbt_Tag *) section);
    }

    // Java: the section list is rebuilt under the flat (1.18) field name
    LIBMATTI_MC_Nbt_CompoundTag_Remove(level, "Sections");
    LIBMATTI_MC_Nbt_CompoundTag_Put(level, "sections", (LIBMATTI_MC_Nbt_Tag *) newSections);
    (void) changed;
    return true;
}