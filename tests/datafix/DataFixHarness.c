// Data fixer harness (P7.3).
//
// Two kinds of checks: parity against values produced by the Java sources (the
// addPadding / heightmap shift / biome expansion / section upgrade vectors come
// from /tmp/javaref/RefFix.java, the DataFix wrappers copied verbatim from the
// MCP-Reborn files), and end-to-end runs of a synthetic old chunk through the
// fix chain the storage path uses.

#include "libmatti/com/mojang/datafixers/DataFixer.h"
#include "libmatti/com/mojang/datafixers/DataFixUtils.h"
#include "libmatti/net/minecraft/nbt/CompoundTag.h"
#include "libmatti/net/minecraft/nbt/ListTag.h"
#include "libmatti/net/minecraft/nbt/Tag.h"
#include "libmatti/net/minecraft/util/datafix/DataFixers.h"
#include "libmatti/net/minecraft/util/datafix/DataFixTypes.h"
#include "libmatti/net/minecraft/util/datafix/fixes/BitStorageAlignFix.h"
#include "libmatti/net/minecraft/util/datafix/fixes/BlockStateData.h"
#include "libmatti/net/minecraft/util/datafix/fixes/BiomeIds.h"
#include "libmatti/net/minecraft/util/datafix/fixes/ChunkBiomeFix.h"
#include "libmatti/net/minecraft/util/datafix/fixes/ChunkHeightAndBiomeFix.h"
#include "libmatti/net/minecraft/util/datafix/fixes/ChunkPalettedStorageFix.h"
#include "libmatti/net/minecraft/util/datafix/fixes/ChunkStatusFix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks = 0;
static int failures = 0;

static void check(int condition, const char *what)
{
    checks++;
    if (!condition)
    {
        failures++;
        printf("FAIL: %s\n", what);
    }
}

// Java: RefFix.addPadding(256, 9, heightmap)
static const int64_t PADDING_HEIGHTMAP[37] = {
    394833811844858LL,  177630547426372LL,  213212592026332LL,  2397650293502888LL, 851354653652352LL,
    1196915061283360LL, 31139770142007168LL, 27375933739873152LL, 27177701426931456LL, 62929639336974848LL,
    324248037334905856LL, 1015850899016910848LL, 1949143394956165120LL, 4094014740031676416LL,
    9104056160272580608LL, 8288302056382464000LL, 1827786091537170433LL, 3911113682652758017LL,
    2377498070367862787LL, 7166817602363195392LL, 1431199926128214038LL, 3585757344104972340LL,
    2696263729246371917LL, 8934147268542464043LL, 5750823586193473548LL, 7320193541060165901LL,
    6747840956482979247LL, 3217418768397372937LL, 5901441160726971107LL, 1688567286849341875LL,
    249610087134425665LL, 7122363555654976085LL, 998897380302638000LL, 5530569472265475543LL,
    4686837019710719967LL, 5807496310538150181LL, 310045LL};

// Java: RefFix.addPadding(4096, 13, {0x1234, -1, 0x55, 7, -9}) - the array
// itself is 1024 longs, the first seven are non-zero
static const int64_t PADDING_SECTION[7] = {4660LL, 4503599627366400LL, 1442840575LL, 481036337152LL,
                                            1970324836974592LL, 4503599627370495LL, 255LL};

// Java: the input RefFix feeds getFixedHeightmap
static const int64_t HEIGHTMAP_INPUT[8] = {0LL,
                                           1LL,
                                           0x1FFLL,
                                           0x0FF0000LL,
                                           -1LL,
                                           0x0000FFFF0000LL,
                                           123456789LL,
                                           -987654321LL};

// Java: RefFix.getFixedHeightmap({0, 1, 0x1FF, 0x0FF0000, -1, 0x0000FFFF0000, 123456789, -987654321})
static const int64_t FIXED_HEIGHTMAP[8] = {0LL,
                                           65LL,
                                           511LL,
                                           33521664LL,
                                           9223372036854775807LL,
                                           12884869120LL,
                                           134217557LL,
                                           9223372036823455631LL};

static void test_add_padding(void)
{
    check(LIBMATTI_MC_DataFixUtils_CeilLog2(1) == 0, "ceillog2(1)");
    check(LIBMATTI_MC_DataFixUtils_CeilLog2(2) == 1, "ceillog2(2)");
    check(LIBMATTI_MC_DataFixUtils_CeilLog2(3) == 2, "ceillog2(3)");
    check(LIBMATTI_MC_DataFixUtils_CeilLog2(4) == 2, "ceillog2(4)");
    check(LIBMATTI_MC_DataFixUtils_CeilLog2(4096) == 12, "ceillog2(4096)");
    check(LIBMATTI_MC_DataFixUtils_GetVersion(LIBMATTI_MC_DataFixUtils_MakeKey(2832)) == 2832, "makeKey/getVersion");

    // the 9 bit heightmap re-pack: the reference builds its 36 word input with
    // the same LCG, so the port can rebuild it instead of embedding it
    int64_t input[36];
    int64_t seed = 0x0123456789ABCDEFLL;
    for (int i = 0; i < 36; i++)
    {
        seed = seed * 6364136223846793005LL + 1442695040888963407LL;
        input[i] = (seed >> 13) & 0x1FFFFFFFFFFFFLL;
    }
    size_t outLength = 0;
    int64_t *out = LIBMATTI_MC_BitStorageAlignFix_AddPadding(256, 9, input, 36, &outLength);
    check(out != NULL, "addPadding returns a buffer");
    check(outLength == 37, "addPadding(256,9) yields 37 longs");
    if (out != NULL && outLength == 37)
    {
        int equal = 1;
        for (size_t i = 0; i < 37; i++)
            if (out[i] != PADDING_HEIGHTMAP[i])
                equal = 0;
        check(equal, "addPadding(256,9) matches Java");
        free(out);
    }

    // a section-sized re-pack across the word boundaries (13 bits: four
    // entries per word, so 4096 entries need 1024 words)
    const int64_t sectionInput[5] = {0x1234LL, -1LL, 0x55LL, 7LL, -9LL};
    int64_t *sectionOut = LIBMATTI_MC_BitStorageAlignFix_AddPadding(4096, 13, sectionInput, 5, &outLength);
    check(sectionOut != NULL, "addPadding(4096,13) returns a buffer");
    check(outLength == 1024, "addPadding(4096,13) yields 1024 longs");
    if (sectionOut != NULL && outLength == 1024)
    {
        int equal = 1;
        for (size_t i = 0; i < 7; i++)
            if (sectionOut[i] != PADDING_SECTION[i])
                equal = 0;
        for (size_t i = 7; i < 1024; i++)
            if (sectionOut[i] != 0)
                equal = 0;
        check(equal, "addPadding(4096,13) matches Java");
        free(sectionOut);
    }

    // an empty input stays empty (Java returns the array as it was)
    int64_t *empty = LIBMATTI_MC_BitStorageAlignFix_AddPadding(256, 9, NULL, 0, &outLength);
    check(empty == NULL && outLength == 0, "addPadding of an empty array");
}

// Java: RefFix.getFixedHeightmap
static void test_heightmap_shift(void)
{
    LIBMATTI_MC_Nbt_CompoundTag *chunk = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag *heightmaps = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_PutLongArray(heightmaps, "WORLD_SURFACE", HEIGHTMAP_INPUT, 8);
    LIBMATTI_MC_Nbt_CompoundTag_Put(chunk, "Heightmaps", (LIBMATTI_MC_Nbt_Tag *) heightmaps);

    LIBMATTI_MC_ChunkHeightAndBiomeFix_Apply((LIBMATTI_MC_Nbt_Tag *) chunk);

    LIBMATTI_MC_Nbt_CompoundTag *after = NULL;
    check(LIBMATTI_MC_Nbt_CompoundTag_GetCompound(chunk, "Heightmaps", &after) && after != NULL, "heightmaps survive");
    const int64_t *shifted = NULL;
    size_t shiftedLength = 0;
    if (after != NULL)
    {
        check(LIBMATTI_MC_Nbt_CompoundTag_GetLongArray(after, "WORLD_SURFACE", &shifted, &shiftedLength),
              "shifted heightmap readable");
        check(shiftedLength == 8, "shifted heightmap length");
        int equal = shifted != NULL && shiftedLength == 8;
        for (size_t i = 0; equal && i < 8; i++)
            if (shifted[i] != FIXED_HEIGHTMAP[i])
                equal = 0;
        check(equal, "the +64 heightmap shift matches Java");
    }
    LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) chunk);
}

// Java: RefFix.chunkBiomeFix over the identity biomes
static void test_biome_fix(void)
{
    LIBMATTI_MC_Nbt_CompoundTag *chunk = LIBMATTI_MC_Nbt_CompoundTag_New();
    int32_t biomes[256];
    for (int i = 0; i < 256; i++)
        biomes[i] = i;
    LIBMATTI_MC_Nbt_CompoundTag_PutIntArray(chunk, "Biomes", biomes, 256);

    check(LIBMATTI_MC_ChunkBiomeFix_Apply((LIBMATTI_MC_Nbt_Tag *) chunk), "ChunkBiomeFix applies");

    const int32_t *out = NULL;
    size_t outLength = 0;
    check(LIBMATTI_MC_Nbt_CompoundTag_GetIntArray(chunk, "Biomes", &out, &outLength), "Biomes readable");
    check(outLength == 1024, "Biomes expanded to 1024");
    if (out != NULL && outLength == 1024)
    {
        // Java: the 4x4 corners (2,2), (6,2), ... stamp the 16 entry rows
        static const int32_t EXPECTED_ROW[16] = {34, 38, 42, 46, 98, 102, 106, 110, 162, 166, 170, 174, 226, 230, 234, 238};
        int rowEqual = 1;
        for (int i = 0; i < 16; i++)
            if (out[i] != EXPECTED_ROW[i])
                rowEqual = 0;
        check(rowEqual, "the expanded biome row matches Java");
        check(out[5 * 16] == 34 && out[63 * 16] == 34, "every layer repeats the first row");
    }

    // a length that is not 256 leaves the array alone
    LIBMATTI_MC_Nbt_CompoundTag *other = LIBMATTI_MC_Nbt_CompoundTag_New();
    int32_t shortBiomes[16];
    for (int i = 0; i < 16; i++)
        shortBiomes[i] = i;
    LIBMATTI_MC_Nbt_CompoundTag_PutIntArray(other, "Biomes", shortBiomes, 16);
    check(!LIBMATTI_MC_ChunkBiomeFix_Apply((LIBMATTI_MC_Nbt_Tag *) other), "a short biome array is left alone");
    LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) other);
    LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) chunk);
}

// Java: RefFix.upgradeSection - the section ids are the 12 bit
// Add << 12 | Blocks << 4 | Data values, so with Data and Add at zero the
// Blocks nibble picks MAP[Blocks << 4]: 1 -> stone, 2 -> grass_block, 3 -> dirt
static void test_section_upgrade(void)
{
    LIBMATTI_MC_Nbt_CompoundTag *section = LIBMATTI_MC_Nbt_CompoundTag_New();
    int8_t blocks[4096];
    int8_t data[2048];
    memset(blocks, 0, sizeof(blocks));
    memset(data, 0, sizeof(data));
    blocks[0] = 1;    // x0 y0 z0 -> stone
    blocks[1024] = 2; // x0 y4 z0 -> grass_block
    blocks[2048] = 3; // x0 y8 z0 -> dirt
    LIBMATTI_MC_Nbt_CompoundTag_PutByteArray(section, "Blocks", blocks, 4096);
    LIBMATTI_MC_Nbt_CompoundTag_PutByteArray(section, "Data", data, 2048);

    LIBMATTI_MC_Nbt_CompoundTag *chunk = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_ListTag *sections = LIBMATTI_MC_Nbt_ListTag_New();
    LIBMATTI_MC_Nbt_ListTag_Add(sections, (LIBMATTI_MC_Nbt_Tag *) section);
    LIBMATTI_MC_Nbt_CompoundTag_Put(chunk, "Sections", (LIBMATTI_MC_Nbt_Tag *) sections);

    check(LIBMATTI_MC_ChunkPalettedStorageFix_Apply((LIBMATTI_MC_Nbt_Tag *) chunk), "ChunkPalettedStorageFix applies");

    LIBMATTI_MC_Nbt_ListTag *palette = NULL;
    check(LIBMATTI_MC_Nbt_CompoundTag_GetList(section, "Palette", &palette) && palette != NULL, "palette written");
    check(palette != NULL && LIBMATTI_MC_Nbt_ListTag_Size(palette) == 4, "palette holds air + the three blocks");
    if (palette != NULL && LIBMATTI_MC_Nbt_ListTag_Size(palette) == 4)
    {
        // the palette entries are compounds {Name, Properties} - read them out
        static const char *const EXPECTED[4] = {"minecraft:air", "minecraft:stone", "minecraft:grass_block",
                                                "minecraft:dirt"};
        for (int i = 0; i < 4; i++)
        {
            LIBMATTI_MC_Nbt_CompoundTag *entry = NULL;
            if (!LIBMATTI_MC_Nbt_ListTag_GetCompound(palette, i, &entry) || entry == NULL)
            {
                check(0, "palette entry readable");
                continue;
            }
            const char *name = LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(entry, "Name", "?");
            check(strcmp(name, EXPECTED[i]) == 0, "palette entry name");
        }
        // the grass block carries the snowy property, the others do not
        LIBMATTI_MC_Nbt_CompoundTag *grass = NULL;
        if (LIBMATTI_MC_Nbt_ListTag_GetCompound(palette, 2, &grass) && grass != NULL)
        {
            LIBMATTI_MC_Nbt_CompoundTag *properties = NULL;
            check(LIBMATTI_MC_Nbt_CompoundTag_GetCompound(grass, "Properties", &properties) && properties != NULL,
                  "grass_block carries properties");
            if (properties != NULL)
                check(strcmp(LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(properties, "snowy", "?"), "false") == 0,
                      "the snowy property value");
        }
    }

    // Java: the packed longs - bits = max(4, ceillog2(3)) = 4
    const int64_t *packed = NULL;
    size_t packedLength = 0;
    check(LIBMATTI_MC_Nbt_CompoundTag_GetLongArray(section, "BlockStates", &packed, &packedLength), "BlockStates written");
    check(packedLength == 256, "BlockStates holds 256 longs at 4 bits");
    if (packed != NULL && packedLength == 256)
    {
        // the three set entries resolve to their palette index: stone 1,
        // grass_block 2, dirt 3 - the word/entry split is 16 entries per long
        check(((packed[0] >> 0) & 15) == 1, "entry 0 resolves to the stone index");
        check(((packed[4 * 256 / 16] >> ((4 * 256 % 16) * 4)) & 15) == 2, "entry 4*256 resolves to the grass index");
        check(((packed[8 * 256 / 16] >> ((8 * 256 % 16) * 4)) & 15) == 3, "entry 8*256 resolves to the dirt index");
        check(packed[0] == 1, "the first word matches Java");
    }
    check(!LIBMATTI_MC_Nbt_CompoundTag_Contains(section, "Blocks"), "Blocks removed");
    check(!LIBMATTI_MC_Nbt_CompoundTag_Contains(section, "Data"), "Data removed");
    check(!LIBMATTI_MC_Nbt_CompoundTag_Contains(section, "Add"), "Add removed");

    LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) chunk);
}

// Java: the registry walk - DataFixers.addFixers order and the update chain
static void test_fix_chain(void)
{
    const LIBMATTI_MC_DataFixer *fixer = LIBMATTI_MC_DataFixers_GetDataFixer();
    check(fixer != NULL, "the fix registry builds");
    if (fixer == NULL)
        return;
    check(fixer->dataVersion == 4671, "the registry targets DataVersion 4671");
    check(fixer->fixCount == 12, "twelve rules registered");

    // a synthetic 1.16 chunk: the numeric section storage, the legacy block
    // entity names, the old status and the light flag
    LIBMATTI_MC_Nbt_CompoundTag *chunk = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(chunk, "DataVersion", 1451);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(chunk, "xPos", 0);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(chunk, "zPos", 0);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(chunk, "yPos", 0);
    LIBMATTI_MC_Nbt_CompoundTag_PutString(chunk, "Status", "fullchunk");
    LIBMATTI_MC_Nbt_CompoundTag_PutBoolean(chunk, "isLightOn", 1);

    int8_t blocks[4096];
    int8_t dataLayer[2048];
    memset(blocks, 0, sizeof(blocks));
    memset(dataLayer, 0, sizeof(dataLayer));
    blocks[0] = 1;  // MAP[1 << 4] = stone
    blocks[256] = 3; // MAP[3 << 4] = dirt
    LIBMATTI_MC_Nbt_CompoundTag *section = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_PutByte(section, "Y", (int8_t) 4);
    LIBMATTI_MC_Nbt_CompoundTag_PutByteArray(section, "Blocks", blocks, 4096);
    LIBMATTI_MC_Nbt_CompoundTag_PutByteArray(section, "Data", dataLayer, 2048);
    LIBMATTI_MC_Nbt_ListTag *sections = LIBMATTI_MC_Nbt_ListTag_New();
    LIBMATTI_MC_Nbt_ListTag_Add(sections, (LIBMATTI_MC_Nbt_Tag *) section);
    LIBMATTI_MC_Nbt_CompoundTag_Put(chunk, "Sections", (LIBMATTI_MC_Nbt_Tag *) sections);

    LIBMATTI_MC_Nbt_ListTag *tileEntities = LIBMATTI_MC_Nbt_ListTag_New();
    LIBMATTI_MC_Nbt_CompoundTag *chest = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_PutString(chest, "id", "Chest");
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(chest, "x", 3);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(chest, "y", 65);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(chest, "z", 4);
    LIBMATTI_MC_Nbt_ListTag_Add(tileEntities, (LIBMATTI_MC_Nbt_Tag *) chest);
    LIBMATTI_MC_Nbt_CompoundTag_Put(chunk, "TileEntities", (LIBMATTI_MC_Nbt_Tag *) tileEntities);

    int32_t biomes[1024];
    for (int i = 0; i < 1024; i++)
        biomes[i] = 1; // plains
    LIBMATTI_MC_Nbt_CompoundTag_PutIntArray(chunk, "Biomes", biomes, 1024);

    LIBMATTI_MC_Nbt_CompoundTag *heightmaps = LIBMATTI_MC_Nbt_CompoundTag_New();
    int64_t word[8] = {0, 1, 0x1FF, 0x0FF0000, -1, 0x0000FFFF0000, 123456789, -987654321};
    LIBMATTI_MC_Nbt_CompoundTag_PutLongArray(heightmaps, "WORLD_SURFACE", word, 8);
    LIBMATTI_MC_Nbt_CompoundTag_Put(chunk, "Heightmaps", (LIBMATTI_MC_Nbt_Tag *) heightmaps);

    LIBMATTI_MC_Nbt_Tag *result = LIBMATTI_MC_DataFixer_UpdateToCurrentVersion(
        fixer, LIBMATTI_MC_DataFixTypes_CHUNK, (LIBMATTI_MC_Nbt_Tag *) chunk, 1451);
    check(result == (LIBMATTI_MC_Nbt_Tag *) chunk, "the update runs in place");
    check(fixer->lastAppliedCount > 0, "rules were applied");
    int ranPaletted = 0;
    int ranStatus = 0;
    int ranHeight = 0;
    for (int i = 0; i < fixer->lastAppliedCount; i++)
    {
        if (strcmp(fixer->lastApplied[i], "ChunkPalettedStorageFix") == 0)
            ranPaletted = 1;
        if (strcmp(fixer->lastApplied[i], "ChunkStatusFix2") == 0)
            ranStatus = 1;
        if (strcmp(fixer->lastApplied[i], "ChunkHeightAndBiomeFix") == 0)
            ranHeight = 1;
    }
    check(ranPaletted, "ChunkPalettedStorageFix ran");
    check(ranStatus, "ChunkStatusFix2 ran");
    check(ranHeight, "ChunkHeightAndBiomeFix ran");

    // the numeric storage became a palette ...
    LIBMATTI_MC_Nbt_ListTag *palette = NULL;
    check(LIBMATTI_MC_Nbt_CompoundTag_GetList(section, "Palette", &palette) && palette != NULL, "palette written");
    check(palette != NULL && LIBMATTI_MC_Nbt_ListTag_Size(palette) == 3, "palette holds air + stone + dirt");
    // ... the block entity id became modern and carries a pos array ...
    LIBMATTI_MC_Nbt_ListTag *blockEntities = NULL;
    check(LIBMATTI_MC_Nbt_CompoundTag_GetList(chunk, "block_entities", &blockEntities) && blockEntities != NULL,
          "block_entities written");
    if (blockEntities != NULL)
    {
        LIBMATTI_MC_Nbt_CompoundTag *entry = NULL;
        check(LIBMATTI_MC_Nbt_ListTag_GetCompound(blockEntities, 0, &entry) && entry != NULL, "block entity readable");
        if (entry != NULL)
        {
            check(strcmp(LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(entry, "id", "?"), "minecraft:chest") == 0,
                  "the chest id was mapped");
            const int32_t *pos = NULL;
            size_t posLength = 0;
            check(LIBMATTI_MC_Nbt_CompoundTag_GetIntArray(entry, "pos", &pos, &posLength) && posLength == 3,
                  "the position moved into a pos array");
        }
    }
    check(!LIBMATTI_MC_Nbt_CompoundTag_Contains(chunk, "TileEntities"), "TileEntities consumed");
    // no legacy flags here, so ChunkToProtoChunkFix leaves the status and
    // ChunkStatusFix2 carries "fullchunk" to the current "full"
    check(strcmp(LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(chunk, "Status", "?"), "full") == 0,
          "the status reaches the current name");
    // the light flag is gone
    check(!LIBMATTI_MC_Nbt_CompoundTag_Contains(chunk, "isLightOn"), "isLightOn removed");
    // the sections moved into the 24 section world with their biome container
    LIBMATTI_MC_Nbt_ListTag *newSections = NULL;
    check(LIBMATTI_MC_Nbt_CompoundTag_GetList(chunk, "sections", &newSections) && newSections != NULL,
          "the section list is flat and lowercase");
    if (newSections != NULL)
    {
        check(newSections != NULL && LIBMATTI_MC_Nbt_ListTag_Size(newSections) == 24, "24 sections");
        LIBMATTI_MC_Nbt_CompoundTag *first = NULL;
        if (LIBMATTI_MC_Nbt_ListTag_GetCompound(newSections, 0, &first) && first != NULL)
        {
            int8_t y = 0;
            LIBMATTI_MC_Nbt_CompoundTag_GetByte(first, "Y", &y);
            check(y == -4, "the first section is Y -4");
        }
        LIBMATTI_MC_Nbt_CompoundTag *last = NULL;
        if (LIBMATTI_MC_Nbt_ListTag_GetCompound(newSections, 23, &last) && last != NULL)
        {
            int8_t y = 0;
            LIBMATTI_MC_Nbt_CompoundTag_GetByte(last, "Y", &y);
            check(y == 19, "the last section is Y 19");
        }
        // the old section Y 4 now sits at index 8 and carries the blocks
        LIBMATTI_MC_Nbt_CompoundTag *moved = NULL;
        if (LIBMATTI_MC_Nbt_ListTag_GetCompound(newSections, 8, &moved) && moved != NULL)
        {
            LIBMATTI_MC_Nbt_CompoundTag *blockStates = NULL;
            check(LIBMATTI_MC_Nbt_CompoundTag_GetCompound(moved, "block_states", &blockStates) && blockStates != NULL,
                  "the converted section kept its block storage");
            if (blockStates != NULL)
            {
                LIBMATTI_MC_Nbt_ListTag *movedPalette = NULL;
                check(LIBMATTI_MC_Nbt_CompoundTag_GetList(blockStates, "palette", &movedPalette)
                          && movedPalette != NULL && LIBMATTI_MC_Nbt_ListTag_Size(movedPalette) == 3,
                      "the palette moved into block_states");
            }
            LIBMATTI_MC_Nbt_CompoundTag *biome = NULL;
            check(LIBMATTI_MC_Nbt_CompoundTag_GetCompound(moved, "biomes", &biome) && biome != NULL,
                  "the section carries a biome container");
        }
    }
    check(!LIBMATTI_MC_Nbt_CompoundTag_Contains(chunk, "Biomes"), "the flat biome array is consumed");
    // the heightmap values moved up by the 64 block world offset
    LIBMATTI_MC_Nbt_CompoundTag *after = NULL;
    if (LIBMATTI_MC_Nbt_CompoundTag_GetCompound(chunk, "Heightmaps", &after) && after != NULL)
    {
        const int64_t *shifted = NULL;
        size_t shiftedLength = 0;
        if (LIBMATTI_MC_Nbt_CompoundTag_GetLongArray(after, "WORLD_SURFACE", &shifted, &shiftedLength)
            && shiftedLength == 8)
        {
            check(shifted[1] == FIXED_HEIGHTMAP[1] && shifted[4] == FIXED_HEIGHTMAP[4], "the heightmap shifted");
        }
    }
    LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) chunk);

    // a chunk that is already current runs no rule at all
    LIBMATTI_MC_Nbt_CompoundTag *current = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(current, "DataVersion", 4671);
    LIBMATTI_MC_Nbt_CompoundTag_PutString(current, "Status", "minecraft:full");
    LIBMATTI_MC_Nbt_CompoundTag *before = NULL;
    LIBMATTI_MC_Nbt_CompoundTag_GetCompound(current, "sections", &before);
    LIBMATTI_MC_DataFixer_UpdateToCurrentVersion(fixer, LIBMATTI_MC_DataFixTypes_CHUNK, (LIBMATTI_MC_Nbt_Tag *) current,
                                                 4671);
    check(fixer->lastAppliedCount == 0, "a current chunk runs no rule");
    check(strcmp(LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(current, "Status", "?"), "minecraft:full") == 0,
          "a current chunk is untouched");
    LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) current);

    // a rule registered for another type never runs
    LIBMATTI_MC_Nbt_CompoundTag *player = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(player, "DataVersion", 704);
    LIBMATTI_MC_DataFixer_UpdateToCurrentVersion(fixer, LIBMATTI_MC_DataFixTypes_PLAYER, (LIBMATTI_MC_Nbt_Tag *) player,
                                                 704);
    check(fixer->lastAppliedCount == 0, "a chunk rule stays off a player tag");
    LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) player);
}

// Java: a 1.12.2 chunk (DataVersion 1343) walks the whole chain - the block
// entity ids are the ones the 1.13 fix renames
static void test_legacy_chunk(void)
{
    const LIBMATTI_MC_DataFixer *fixer = LIBMATTI_MC_DataFixers_GetDataFixer();
    if (fixer == NULL)
        return;

    LIBMATTI_MC_Nbt_CompoundTag *chunk = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(chunk, "DataVersion", 1343);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(chunk, "xPos", 0);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(chunk, "zPos", 0);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(chunk, "yPos", 0);
    LIBMATTI_MC_Nbt_CompoundTag_PutBoolean(chunk, "TerrainPopulated", 1);
    LIBMATTI_MC_Nbt_CompoundTag_PutBoolean(chunk, "LightPopulated", 1);
    LIBMATTI_MC_Nbt_CompoundTag_PutBoolean(chunk, "isLightOn", 1);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(chunk, "InhabitedTime", 42);

    static int8_t blocks[4096];
    static int8_t low[2048];
    memset(blocks, 0, sizeof(blocks));
    memset(low, 0, sizeof(low));
    blocks[0] = 1;  // MAP[1 << 4] = stone
    blocks[4095] = 3; // MAP[3 << 4] = dirt
    LIBMATTI_MC_Nbt_CompoundTag *section = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_PutByte(section, "Y", (int8_t) 0);
    LIBMATTI_MC_Nbt_CompoundTag_PutByteArray(section, "Blocks", blocks, 4096);
    LIBMATTI_MC_Nbt_CompoundTag_PutByteArray(section, "Data", low, 2048);
    LIBMATTI_MC_Nbt_ListTag *sections = LIBMATTI_MC_Nbt_ListTag_New();
    LIBMATTI_MC_Nbt_ListTag_Add(sections, (LIBMATTI_MC_Nbt_Tag *) section);
    LIBMATTI_MC_Nbt_CompoundTag_Put(chunk, "Sections", (LIBMATTI_MC_Nbt_Tag *) sections);

    LIBMATTI_MC_Nbt_ListTag *tileEntities = LIBMATTI_MC_Nbt_ListTag_New();
    static const char *const OLD_IDS[3] = {"Chest", "Furnace", "Sign"};
    for (int i = 0; i < 3; i++)
    {
        LIBMATTI_MC_Nbt_CompoundTag *entity = LIBMATTI_MC_Nbt_CompoundTag_New();
        LIBMATTI_MC_Nbt_CompoundTag_PutString(entity, "id", OLD_IDS[i]);
        LIBMATTI_MC_Nbt_CompoundTag_PutInt(entity, "x", i);
        LIBMATTI_MC_Nbt_CompoundTag_PutInt(entity, "y", 64);
        LIBMATTI_MC_Nbt_CompoundTag_PutInt(entity, "z", 0);
        LIBMATTI_MC_Nbt_ListTag_Add(tileEntities, (LIBMATTI_MC_Nbt_Tag *) entity);
    }
    LIBMATTI_MC_Nbt_CompoundTag_Put(chunk, "TileEntities", (LIBMATTI_MC_Nbt_Tag *) tileEntities);

    LIBMATTI_MC_DataFixer_UpdateToCurrentVersion(fixer, LIBMATTI_MC_DataFixTypes_CHUNK,
                                                 (LIBMATTI_MC_Nbt_Tag *) chunk, 1343);

    int ranEntity = 0;
    for (int i = 0; i < fixer->lastAppliedCount; i++)
        if (strcmp(fixer->lastApplied[i], "BlockEntityIdFix") == 0)
            ranEntity = 1;
    check(ranEntity, "BlockEntityIdFix runs for a 1.12 chunk");

    LIBMATTI_MC_Nbt_ListTag *blockEntities = NULL;
    check(LIBMATTI_MC_Nbt_CompoundTag_GetList(chunk, "block_entities", &blockEntities) && blockEntities != NULL,
          "the block entities are converted");
    static const char *const NEW_IDS[3] = {"minecraft:chest", "minecraft:furnace", "minecraft:sign"};
    if (blockEntities != NULL && LIBMATTI_MC_Nbt_ListTag_Size(blockEntities) == 3)
    {
        int allMapped = 1;
        for (int i = 0; i < 3; i++)
        {
            LIBMATTI_MC_Nbt_CompoundTag *entity = NULL;
            if (!LIBMATTI_MC_Nbt_ListTag_GetCompound(blockEntities, i, &entity) || entity == NULL ||
                strcmp(LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(entity, "id", "?"), NEW_IDS[i]) != 0)
                allMapped = 0;
        }
        check(allMapped, "every block entity id became modern");
    }
    // both legacy flags were set, so ChunkToProtochunkFix answers
    // "mobs_spawned" and ChunkStatusFix2 carries that to "spawn"
    check(strcmp(LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(chunk, "Status", "?"), "spawn") == 0,
          "a fully populated chunk lands on the current status");
    check(LIBMATTI_MC_Nbt_CompoundTag_GetLongOr(chunk, "InhabitedTime", -1) == 42, "unrelated fields survive");

    // the converted section keeps its blocks
    LIBMATTI_MC_Nbt_ListTag *newSections = NULL;
    if (LIBMATTI_MC_Nbt_CompoundTag_GetList(chunk, "sections", &newSections) && newSections != NULL)
    {
        // the old section Y 0 is now new index 4
        LIBMATTI_MC_Nbt_CompoundTag *moved = NULL;
        check(LIBMATTI_MC_Nbt_ListTag_GetCompound(newSections, 4, &moved) && moved != NULL,
              "the old section 0 sits at new index 4");
    }
    LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) chunk);
}

// the two status rules on their own
static void test_status_rules(void)
{
    LIBMATTI_MC_Nbt_CompoundTag *chunk = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_PutString(chunk, "Status", "postprocessed");
    check(LIBMATTI_MC_ChunkStatusFix_Apply((LIBMATTI_MC_Nbt_Tag *) chunk), "ChunkStatusFix applies");
    check(strcmp(LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(chunk, "Status", "?"), "fullchunk") == 0,
          "postprocessed became fullchunk");
    check(LIBMATTI_MC_ChunkStatusFix2_Apply((LIBMATTI_MC_Nbt_Tag *) chunk), "ChunkStatusFix2 applies");
    check(strcmp(LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(chunk, "Status", "?"), "full") == 0,
          "fullchunk became full");
    // a status the table does not name is newer than the rule - it stays
    LIBMATTI_MC_Nbt_CompoundTag_PutString(chunk, "Status", "minecraft:full");
    check(!LIBMATTI_MC_ChunkStatusFix2_Apply((LIBMATTI_MC_Nbt_Tag *) chunk), "a modern status is left alone");
    check(strcmp(LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(chunk, "Status", "?"), "minecraft:full") == 0,
          "a modern status survives");
    LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) chunk);
}

int main(void)
{
    printf("datafix harness\n");
    test_add_padding();
    test_heightmap_shift();
    test_biome_fix();
    test_section_upgrade();
    test_status_rules();
    test_legacy_chunk();
    test_fix_chain();
    printf("datafix harness: %d/%d checks ok\n", checks - failures, checks);
    return failures == 0 ? 0 : 1;
}