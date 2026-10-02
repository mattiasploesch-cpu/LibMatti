// Storage harness (P7.1): the anvil save format end to end -
//   - RegionFileVersion: the gzip/deflate/none codecs
//   - RegionFile: header/offset/sector bookkeeping, read-write round trips,
//     clear, the pad-to-sector close and the .mcc external spill
//   - RegionFileStorage: the region cache over the .mca folder
//   - SerializableChunkData: the chunk <-> NBT codec (sections, palettes,
//     heightmaps, post-processing, the block-state strings)
//   - LevelStorageSource: level.dat + the chunk save/load pass over a level

#include "libmatti/net/minecraft/SharedConstants.h"
#include "libmatti/net/minecraft/nbt/ListTag.h"
#include "libmatti/net/minecraft/nbt/NbtIo.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/world/level/Level.h"
#include "libmatti/net/minecraft/world/level/chunk/PalettedContainer.h"
#include "libmatti/net/minecraft/world/level/chunk/storage/RegionFileVersion.h"
#include "libmatti/net/minecraft/world/level/chunk/storage/RegionFile.h"
#include "libmatti/net/minecraft/world/level/chunk/storage/RegionFileStorage.h"
#include "libmatti/net/minecraft/world/level/chunk/storage/SerializableChunkData.h"
#include "libmatti/net/minecraft/world/level/levelgen/Heightmap.h"
#include "libmatti/net/minecraft/world/level/storage/LevelStorageSource.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int failures;
static int checks;

static void check(int condition, const char *what)
{
    checks++;
    if (!condition)
    {
        failures++;
        printf("FAIL: %s\n", what);
    }
}

// a deterministic byte pattern (the LCG keeps the oversized-chunk data
// incompressible so the .mcc spill triggers)
static void fill_pattern(uint8_t *data, size_t length, uint32_t seed)
{
    uint32_t state = seed;
    for (size_t i = 0; i < length; i++)
    {
        state = state * 1664525u + 1013904223u;
        data[i] = (uint8_t) (state >> 24);
    }
}

// ---------------------------------------------------------------------------
// RegionFileVersion
// ---------------------------------------------------------------------------

static void test_region_file_version(void)
{
    uint8_t payload[4096];
    fill_pattern(payload, sizeof(payload), 7);

    int ids[] = {LIBMATTI_MC_RegionFileVersion_GZIP, LIBMATTI_MC_RegionFileVersion_DEFLATE,
                 LIBMATTI_MC_RegionFileVersion_NONE};
    for (size_t i = 0; i < sizeof(ids) / sizeof(ids[0]); i++)
    {
        uint8_t *compressed = NULL;
        size_t compressedLength = 0;
        check(LIBMATTI_MC_RegionFileVersion_Compress(ids[i], payload, sizeof(payload), &compressed,
                                                     &compressedLength) == 0,
              "compress succeeds");
        uint8_t *restored = NULL;
        size_t restoredLength = 0;
        check(LIBMATTI_MC_RegionFileVersion_Decompress(ids[i], compressed, compressedLength, &restored,
                                                       &restoredLength) == 0,
              "decompress succeeds");
        check(restoredLength == sizeof(payload) && memcmp(restored, payload, sizeof(payload)) == 0,
              "codec round trip is byte exact");
        free(compressed);
        free(restored);
    }

    check(LIBMATTI_MC_RegionFileVersion_IsValidVersion(LIBMATTI_MC_RegionFileVersion_DEFLATE),
          "deflate is a valid version id");
    check(!LIBMATTI_MC_RegionFileVersion_IsValidVersion(127), "the custom id is rejected");
}

// ---------------------------------------------------------------------------
// RegionFile
// ---------------------------------------------------------------------------

static void test_region_file(const char *dir)
{
    char path[1024];
    snprintf(path, sizeof(path), "%s/r.0.0.mca", dir);

    LIBMATTI_MC_RegionFile *rf = LIBMATTI_MC_RegionFile_New(path, dir, -1);
    check(rf != NULL, "region file opens (created)");

    // three chunks: (0,0), (5,7) and (31,31) - region-local index 0 / 7*32+5 / 31*32+31
    uint8_t small[300];
    fill_pattern(small, sizeof(small), 11);
    uint8_t medium[9000];
    fill_pattern(medium, sizeof(medium), 12);
    uint8_t big[50000];
    fill_pattern(big, sizeof(big), 13);

    LIBMATTI_MC_ChunkPos p0 = {0, 0};
    LIBMATTI_MC_ChunkPos p1 = {5, 7};
    LIBMATTI_MC_ChunkPos p2 = {31, 31};
    check(LIBMATTI_MC_RegionFile_WriteChunk(rf, &p0, small, sizeof(small)) == 0, "write chunk (0,0)");
    check(LIBMATTI_MC_RegionFile_WriteChunk(rf, &p1, medium, sizeof(medium)) == 0, "write chunk (5,7)");
    check(LIBMATTI_MC_RegionFile_WriteChunk(rf, &p2, big, sizeof(big)) == 0, "write chunk (31,31)");

    check(LIBMATTI_MC_RegionFile_HasChunk(rf, &p0), "hasChunk (0,0)");
    check(LIBMATTI_MC_RegionFile_DoesChunkExist(rf, &p1), "doesChunkExist (5,7)");
    LIBMATTI_MC_ChunkPos absent = {3, 3};
    check(!LIBMATTI_MC_RegionFile_HasChunk(rf, &absent), "chunk (3,3) is absent");

    // the header: the first chunk sits at sector 2 (the header sectors are 0/1)
    check((rf->offsets[0] >> 8) == 2, "chunk (0,0) starts at sector 2");
    check((rf->offsets[0] & 0xFF) == 1, "chunk (0,0) spans one sector");
    check(rf->timestamps[0] > 0, "the timestamp entry is set");

    // read back through the same open file
    size_t length = 0;
    uint8_t *restored = LIBMATTI_MC_RegionFile_ReadChunk(rf, &p1, &length);
    check(restored != NULL && length == sizeof(medium) && memcmp(restored, medium, sizeof(medium)) == 0,
          "chunk (5,7) reads back byte exact");
    free(restored);
    restored = LIBMATTI_MC_RegionFile_ReadChunk(rf, &p2, &length);
    check(restored != NULL && length == sizeof(big) && memcmp(restored, big, sizeof(big)) == 0,
          "chunk (31,31) reads back byte exact");
    free(restored);
    check(LIBMATTI_MC_RegionFile_ReadChunk(rf, &absent, &length) == NULL, "the absent chunk reads NULL");

    // clear frees the sectors and the entry
    check(LIBMATTI_MC_RegionFile_Clear(rf, &p1) == 0, "clear chunk (5,7)");
    check(!LIBMATTI_MC_RegionFile_HasChunk(rf, &p1), "cleared chunk is gone");

    check(LIBMATTI_MC_RegionFile_Close(rf) == 0, "region file closes (padded)");
    LIBMATTI_MC_RegionFile_Free(rf);

    struct stat st;
    check(stat(path, &st) == 0 && st.st_size % LIBMATTI_MC_RegionFile_SECTOR_BYTES == 0,
          "the closed file is a whole number of sectors");

    // re-open: the header persisted, the two chunks still read
    rf = LIBMATTI_MC_RegionFile_New(path, dir, -1);
    check(rf != NULL, "region file re-opens");
    restored = LIBMATTI_MC_RegionFile_ReadChunk(rf, &p0, &length);
    check(restored != NULL && length == sizeof(small) && memcmp(restored, small, sizeof(small)) == 0,
          "chunk (0,0) survives the reopen");
    free(restored);
    check(!LIBMATTI_MC_RegionFile_HasChunk(rf, &p1), "the cleared chunk stays gone");
    LIBMATTI_MC_RegionFile_Free(rf);
}

static void test_region_file_external(const char *dir)
{
    char path[1024];
    snprintf(path, sizeof(path), "%s/r.1.0.mca", dir);

    LIBMATTI_MC_RegionFile *rf = LIBMATTI_MC_RegionFile_New(path, dir, -1);
    check(rf != NULL, "region file for the external spill opens");

    // 1.2 MB of noise: compressed it stays above the 256-sector threshold
    size_t bigLength = 1200 * 1024;
    uint8_t *big = malloc(bigLength);
    fill_pattern(big, bigLength, 99);
    LIBMATTI_MC_ChunkPos p = {32, 0}; // region 1,0 -> local index 0
    check(LIBMATTI_MC_RegionFile_WriteChunk(rf, &p, big, bigLength) == 0, "oversized chunk writes");

    char externalPath[1024];
    snprintf(externalPath, sizeof(externalPath), "%s/c.32.0.mcc", dir);
    struct stat st;
    check(stat(externalPath, &st) == 0 && st.st_size > 0, "the .mcc external file exists");

    size_t length = 0;
    uint8_t *restored = LIBMATTI_MC_RegionFile_ReadChunk(rf, &p, &length);
    check(restored != NULL && length == bigLength && memcmp(restored, big, bigLength) == 0,
          "the external chunk reads back byte exact");
    free(restored);

    // clearing removes the external file
    check(LIBMATTI_MC_RegionFile_Clear(rf, &p) == 0, "clear the external chunk");
    check(stat(externalPath, &st) != 0, "the .mcc file is deleted with the clear");

    LIBMATTI_MC_RegionFile_Free(rf);
    free(big);
}

// ---------------------------------------------------------------------------
// RegionFileStorage
// ---------------------------------------------------------------------------

static void test_region_file_storage(const char *dir)
{
    char folder[1024];
    snprintf(folder, sizeof(folder), "%s/storage", dir);
    LIBMATTI_MC_RegionFileStorage *storage = LIBMATTI_MC_RegionFileStorage_New(folder, false);
    check(storage != NULL, "region storage opens");

    // two regions: (0,0) -> r.0.0.mca and (-1,0) -> r.-1.0.mca
    LIBMATTI_MC_ChunkPos posA = {0, 0};
    LIBMATTI_MC_ChunkPos posB = {-1, 0};
    LIBMATTI_MC_Nbt_CompoundTag *tagA = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(tagA, "marker", 111);
    LIBMATTI_MC_Nbt_CompoundTag *tagB = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(tagB, "marker", 222);

    check(LIBMATTI_MC_RegionFileStorage_Write(storage, &posA, tagA) == 0, "storage write (0,0)");
    check(LIBMATTI_MC_RegionFileStorage_Write(storage, &posB, tagB) == 0, "storage write (-1,0)");
    LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) tagA);
    LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) tagB);
    check(storage->cacheCount == 2, "two region files are cached");

    LIBMATTI_MC_Nbt_CompoundTag *readA = LIBMATTI_MC_RegionFileStorage_Read(storage, &posA);
    int32_t marker = 0;
    check(readA != NULL && LIBMATTI_MC_Nbt_CompoundTag_GetInt(readA, "marker", &marker) && marker == 111,
          "storage read (0,0) restores the tag");
    LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) readA);

    LIBMATTI_MC_Nbt_CompoundTag *readB = LIBMATTI_MC_RegionFileStorage_Read(storage, &posB);
    check(readB != NULL && LIBMATTI_MC_Nbt_CompoundTag_GetInt(readB, "marker", &marker) && marker == 222,
          "storage read (-1,0) restores the tag");
    LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) readB);

    LIBMATTI_MC_ChunkPos absent = {9, 9};
    check(LIBMATTI_MC_RegionFileStorage_Read(storage, &absent) == NULL, "an absent chunk reads NULL");

    char regionPath[1024];
    snprintf(regionPath, sizeof(regionPath), "%s/r.-1.0.mca", folder);
    struct stat st;
    check(stat(regionPath, &st) == 0, "the negative region file exists");

    check(LIBMATTI_MC_RegionFileStorage_Close(storage) == 0, "storage closes");
    LIBMATTI_MC_RegionFileStorage_Free(storage);
}

// ---------------------------------------------------------------------------
// the block-state string codec
// ---------------------------------------------------------------------------

static void test_state_codec(void)
{
    LIBMATTI_MC_BlockState *stone = LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_GetByName("STONE"));
    char *text = LIBMATTI_MC_SerializableChunkData_StateToString(stone);
    check(text != NULL && strcmp(text, "minecraft:stone") == 0, "stone serializes as minecraft:stone");
    LIBMATTI_MC_BlockState *restored = LIBMATTI_MC_SerializableChunkData_StateFromString(text);
    check(restored == stone, "the stone string parses back to the same state pointer");
    free(text);

    // a state with properties: oak stairs carry facing/half/shape/waterlogged
    LIBMATTI_MC_Block *stairsBlock = LIBMATTI_MC_VanillaBlocks_GetByName("OAK_STAIRS");
    LIBMATTI_MC_BlockState *stairs = LIBMATTI_MC_Block_DefaultBlockState(stairsBlock);
    int propertyCount = 0;
    LIBMATTI_MC_Property **properties = LIBMATTI_MC_StateHolder_GetProperties(&stairs->holder, &propertyCount);
    check(propertyCount == 4, "oak stairs carry four properties");
    LIBMATTI_MC_Property *facing = properties[0];
    const LIBMATTI_MC_Property_Value *values = LIBMATTI_MC_Property_GetPossibleValues(facing, NULL);
    LIBMATTI_MC_BlockState *turned = LIBMATTI_MC_BlockState_SetValue(stairs, facing, values[2]);
    text = LIBMATTI_MC_SerializableChunkData_StateToString(turned);
    check(text != NULL && strncmp(text, "minecraft:oak_stairs[", 21) == 0, "the stairs string carries the property list");
    restored = LIBMATTI_MC_SerializableChunkData_StateFromString(text);
    check(restored == turned, "the stairs string round trips to the same state");
    free(text);

    check(LIBMATTI_MC_SerializableChunkData_StateFromString("minecraft:not_a_block") == NULL,
          "an unknown block id fails");
    check(LIBMATTI_MC_SerializableChunkData_StateFromString("minecraft:stone[not_a_prop=x]") == NULL,
          "an unknown property fails");
}

// ---------------------------------------------------------------------------
// the chunk codec (in memory)
// ---------------------------------------------------------------------------

// builds the demo platform: stone at y=64 over chunk (0,0) + a tower + a
// non-default state, and primes the heightmaps through setBlock
static LIBMATTI_MC_Level *build_demo_level(void)
{
    LIBMATTI_MC_Level *level = LIBMATTI_MC_Level_New(-64, 384, LIBMATTI_MC_Level_OVERWORLD, true);
    LIBMATTI_MC_BlockState *stone = LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_GetByName("STONE"));
    LIBMATTI_MC_BlockState *dirt = LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_GetByName("DIRT"));
    for (int x = 0; x < 16; x++)
    {
        for (int z = 0; z < 16; z++)
        {
            LIBMATTI_MC_BlockPos ground = {{x, 64, z}};
            LIBMATTI_MC_Level_SetBlock(level, &ground, stone, LIBMATTI_MC_Level_UPDATE_CLIENTS);
            LIBMATTI_MC_BlockPos fill = {{x, 63, z}};
            LIBMATTI_MC_Level_SetBlock(level, &fill, dirt, LIBMATTI_MC_Level_UPDATE_CLIENTS);
        }
    }
    for (int y = 65; y < 70; y++)
    {
        LIBMATTI_MC_BlockPos tower = {{4, y, 4}};
        LIBMATTI_MC_Level_SetBlock(level, &tower, stone, LIBMATTI_MC_Level_UPDATE_CLIENTS);
    }
    // an oak stair with a rotated facing - the property-bearing state path
    LIBMATTI_MC_BlockState *stairs = LIBMATTI_MC_Block_DefaultBlockState(LIBMATTI_MC_VanillaBlocks_GetByName("OAK_STAIRS"));
    int propertyCount = 0;
    LIBMATTI_MC_Property **properties = LIBMATTI_MC_StateHolder_GetProperties(&stairs->holder, &propertyCount);
    const LIBMATTI_MC_Property_Value *values = LIBMATTI_MC_Property_GetPossibleValues(properties[0], NULL);
    stairs = LIBMATTI_MC_BlockState_SetValue(stairs, properties[0], values[1]);
    LIBMATTI_MC_BlockPos stairPos = {{8, 64, 8}};
    LIBMATTI_MC_Level_SetBlock(level, &stairPos, stairs, LIBMATTI_MC_Level_UPDATE_CLIENTS);
    return level;
}

static void test_chunk_codec(void)
{
    LIBMATTI_MC_Level *level = build_demo_level();
    LIBMATTI_MC_LevelChunk *chunk = LIBMATTI_MC_Level_GetChunk(level, 0, 0);
    check(chunk != NULL, "the demo chunk exists");

    LIBMATTI_MC_Nbt_CompoundTag *tag = LIBMATTI_MC_SerializableChunkData_Write(chunk);
    check(tag != NULL, "the chunk serializes");

    int32_t value = 0;
    check(LIBMATTI_MC_Nbt_CompoundTag_GetInt(tag, "xPos", &value) && value == 0, "xPos is written");
    check(LIBMATTI_MC_Nbt_CompoundTag_GetInt(tag, "zPos", &value) && value == 0, "zPos is written");
    check(LIBMATTI_MC_Nbt_CompoundTag_GetInt(tag, "yPos", &value) && value == -4, "yPos is the min section (-4)");
    check(LIBMATTI_MC_Nbt_CompoundTag_GetInt(tag, "DataVersion", &value)
              && value == LIBMATTI_MC_SharedConstants_GetDataVersion(),
          "DataVersion carries the world data version");
    check(LIBMATTI_MC_SerializableChunkData_IsFullChunk(tag), "the Status is full");

    LIBMATTI_MC_Nbt_ListTag *sections = NULL;
    check(LIBMATTI_MC_Nbt_CompoundTag_GetList(tag, "sections", &sections) && sections != NULL
              && LIBMATTI_MC_Nbt_ListTag_Size(sections) == 2,
          "two sections serialize (the y=63 dirt band and the y=64 platform band)");
    LIBMATTI_MC_Nbt_CompoundTag *sectionTag = NULL;
    check(LIBMATTI_MC_Nbt_ListTag_GetCompound(sections, 0, &sectionTag) && sectionTag != NULL, "the section is a compound");
    int8_t sectionY = 0;
    check(LIBMATTI_MC_Nbt_CompoundTag_GetByte(sectionTag, "Y", &sectionY) && sectionY == 3,
          "the first section Y is 3 (the y=63 dirt band)");
    check(LIBMATTI_MC_Nbt_ListTag_GetCompound(sections, 1, &sectionTag) && sectionTag != NULL, "the second section is a compound");
    check(LIBMATTI_MC_Nbt_CompoundTag_GetByte(sectionTag, "Y", &sectionY) && sectionY == 4,
          "the second section Y is 4 (the y=64 platform band)");
    LIBMATTI_MC_Nbt_CompoundTag *blockStates = NULL;
    check(LIBMATTI_MC_Nbt_CompoundTag_GetCompound(sectionTag, "block_states", &blockStates) && blockStates != NULL,
          "block_states is present");
    LIBMATTI_MC_Nbt_ListTag *palette = NULL;
    check(LIBMATTI_MC_Nbt_CompoundTag_GetList(blockStates, "palette", &palette) && palette != NULL
              && LIBMATTI_MC_Nbt_ListTag_Size(palette) == 3,
          "the platform palette holds air/stone/stairs");
    LIBMATTI_MC_Nbt_CompoundTag *heightmaps = NULL;
    check(LIBMATTI_MC_Nbt_CompoundTag_GetCompound(tag, "Heightmaps", &heightmaps) && heightmaps != NULL
              && LIBMATTI_MC_Nbt_CompoundTag_Contains(heightmaps, "MOTION_BLOCKING"),
          "the Heightmaps compound carries MOTION_BLOCKING");
    check(LIBMATTI_MC_Nbt_CompoundTag_Contains(tag, "PostProcessing"), "PostProcessing is written");
    // Java: isLightOn only when lightCorrect - the port's light engine is not
    // ported yet, so the flag stays false like Java's fresh ProtoChunk
    check(!LIBMATTI_MC_Nbt_CompoundTag_Contains(tag, "isLightOn"), "isLightOn is absent while the light flag is false");

    // the heightmap before the round trip (the platform surface at y=65)
    int surfaceBefore = LIBMATTI_MC_ChunkAccess_GetHeight(&chunk->base, LIBMATTI_MC_Heightmap_WORLD_SURFACE, 3, 3);

    // read the tag back into a second level
    LIBMATTI_MC_Level *loaded = LIBMATTI_MC_Level_New(-64, 384, LIBMATTI_MC_Level_OVERWORLD, true);
    LIBMATTI_MC_LevelChunk *restored = LIBMATTI_MC_SerializableChunkData_Read(loaded, tag);
    check(restored != NULL, "the chunk parses back");
    LIBMATTI_MC_Level_SetChunk(loaded, restored);

    LIBMATTI_MC_BlockPos ground = {{3, 64, 3}};
    LIBMATTI_MC_BlockPos fill = {{3, 63, 3}};
    LIBMATTI_MC_BlockPos tower = {{4, 67, 4}};
    LIBMATTI_MC_BlockPos stairPos = {{8, 64, 8}};
    check(LIBMATTI_MC_BlockState_GetBlock(LIBMATTI_MC_Level_GetBlockState(loaded, &ground))
              == (void *) LIBMATTI_MC_VanillaBlocks_GetByName("STONE"),
          "the platform stone restores");
    check(LIBMATTI_MC_BlockState_GetBlock(LIBMATTI_MC_Level_GetBlockState(loaded, &fill))
              == (void *) LIBMATTI_MC_VanillaBlocks_GetByName("DIRT"),
          "the dirt layer restores");
    check(LIBMATTI_MC_BlockState_GetBlock(LIBMATTI_MC_Level_GetBlockState(loaded, &tower))
              == (void *) LIBMATTI_MC_VanillaBlocks_GetByName("STONE"),
          "the tower restores");
    LIBMATTI_MC_BlockState *restoredStairs = LIBMATTI_MC_Level_GetBlockState(loaded, &stairPos);
    LIBMATTI_MC_BlockState *originalStairs = LIBMATTI_MC_Level_GetBlockState(level, &stairPos);
    check(restoredStairs == originalStairs, "the rotated stair state restores pointer-identical");
    check(LIBMATTI_MC_ChunkAccess_GetHeight(&restored->base, LIBMATTI_MC_Heightmap_WORLD_SURFACE, 3, 3)
              == surfaceBefore,
          "the WORLD_SURFACE heightmap value restores");
    check(LIBMATTI_MC_ChunkAccess_GetHeight(&restored->base, LIBMATTI_MC_Heightmap_MOTION_BLOCKING, 4, 4)
              == LIBMATTI_MC_ChunkAccess_GetHeight(&chunk->base, LIBMATTI_MC_Heightmap_MOTION_BLOCKING, 4, 4),
          "the MOTION_BLOCKING heightmap value restores");

    LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) tag);
    LIBMATTI_MC_Level_Free(level);
    LIBMATTI_MC_Level_Free(loaded);
}

// the global-palette path: 20 distinct states push the section past the
// 16-entry linear palette (all positions inside chunk (0,0), section 10)
static void test_chunk_codec_global_palette(void)
{
    LIBMATTI_MC_Level *level = LIBMATTI_MC_Level_New(-64, 384, LIBMATTI_MC_Level_OVERWORLD, true);
    LIBMATTI_MC_Block **all = LIBMATTI_MC_VanillaBlocks_All();
    for (int i = 0; i < 20; i++)
    {
        LIBMATTI_MC_BlockState *state = LIBMATTI_MC_Block_DefaultBlockState(all[i + 1]);
        LIBMATTI_MC_BlockPos pos = {{i % 16, 100, i / 16}};
        LIBMATTI_MC_Level_SetBlock(level, &pos, state, LIBMATTI_MC_Level_UPDATE_CLIENTS);
    }
    LIBMATTI_MC_LevelChunk *chunk = LIBMATTI_MC_Level_GetChunk(level, 0, 0);
    LIBMATTI_MC_LevelChunkSection *section = LIBMATTI_MC_ChunkAccess_GetSection(&chunk->base, 10); // y 96..111
    check(LIBMATTI_MC_PalettedContainer_GetMode(LIBMATTI_MC_LevelChunkSection_GetStates(section))
              == LIBMATTI_MC_PalettedContainer_Mode_GLOBAL,
          "the 20-state section switched to the global palette");

    LIBMATTI_MC_Nbt_CompoundTag *tag = LIBMATTI_MC_SerializableChunkData_Write(chunk);
    LIBMATTI_MC_Level *loaded = LIBMATTI_MC_Level_New(-64, 384, LIBMATTI_MC_Level_OVERWORLD, true);
    LIBMATTI_MC_LevelChunk *restored = LIBMATTI_MC_SerializableChunkData_Read(loaded, tag);
    check(restored != NULL, "the global-palette chunk parses back");
    LIBMATTI_MC_Level_SetChunk(loaded, restored);

    int mismatches = 0;
    for (int i = 0; i < 20; i++)
    {
        LIBMATTI_MC_BlockPos pos = {{i % 16, 100, i / 16}};
        if (LIBMATTI_MC_Level_GetBlockState(loaded, &pos) != LIBMATTI_MC_Level_GetBlockState(level, &pos))
            mismatches++;
    }
    check(mismatches == 0, "all 20 distinct states restore");

    LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) tag);
    LIBMATTI_MC_Level_Free(level);
    LIBMATTI_MC_Level_Free(loaded);
}

// ---------------------------------------------------------------------------
// LevelStorageSource - level.dat + the chunk save/load pass
// ---------------------------------------------------------------------------

static void test_level_storage(const char *dir)
{
    LIBMATTI_MC_LevelStorageSource *source = LIBMATTI_MC_LevelStorageSource_CreateDefault(dir);
    check(source != NULL, "level storage source opens");
    LIBMATTI_MC_LevelStorageAccess *access = LIBMATTI_MC_LevelStorageSource_CreateAccess(source, "HarnessWorld",
                                                                                          LIBMATTI_MC_Level_OVERWORLD);
    check(access != NULL, "the overworld access opens");

    // the nether access nests under dimensions/minecraft/the_nether
    LIBMATTI_MC_LevelStorageAccess *nether = LIBMATTI_MC_LevelStorageSource_CreateAccess(
        source, "HarnessWorld", LIBMATTI_MC_Level_NETHER);
    check(nether != NULL, "the nether access opens");
    char netherRegion[1024];
    snprintf(netherRegion, sizeof(netherRegion), "%s/HarnessWorld/dimensions/minecraft/the_nether/region", dir);
    struct stat st;
    check(stat(netherRegion, &st) == 0 && S_ISDIR(st.st_mode), "the nether region folder nests under dimensions/");
    LIBMATTI_MC_LevelStorageAccess_Free(nether);

    LIBMATTI_MC_Level *level = build_demo_level();
    LIBMATTI_MC_Level_SetGameTime(level, 12345);
    LIBMATTI_MC_Level_SetDayTime(level, 6000);
    LIBMATTI_MC_Level_SetRaining(level, true);

    // the level.dat write + the chunk save pass
    LIBMATTI_MC_Nbt_CompoundTag *data = LIBMATTI_MC_LevelStorage_BuildLevelData(level, "Harness World");
    check(data != NULL, "the level data compound builds");
    check(LIBMATTI_MC_LevelStorageAccess_WriteLevelData(access, data) == 0, "level.dat writes");
    LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) data);
    check(LIBMATTI_MC_LevelStorage_SaveChunks(access, level) == 1, "the demo chunk saves through the storage");

    char levelDataPath[1024];
    snprintf(levelDataPath, sizeof(levelDataPath), "%s/HarnessWorld/level.dat", dir);
    check(stat(levelDataPath, &st) == 0 && st.st_size > 0, "level.dat exists");
    char regionPath[1024];
    snprintf(regionPath, sizeof(regionPath), "%s/HarnessWorld/region/r.0.0.mca", dir);
    check(stat(regionPath, &st) == 0, "the overworld region file exists");

    LIBMATTI_MC_LevelStorageAccess_Free(access);
    LIBMATTI_MC_Level_Free(level);

    // the reload: a fresh source/access reads the level.dat and the chunk
    access = LIBMATTI_MC_LevelStorageSource_CreateAccess(source, "HarnessWorld", LIBMATTI_MC_Level_OVERWORLD);
    check(access != NULL, "the access re-opens");
    LIBMATTI_MC_Nbt_CompoundTag *restoredData = NULL;
    int readOk = LIBMATTI_MC_LevelStorageAccess_ReadLevelData(access, &restoredData);
    check(readOk == 1 && restoredData != NULL, "level.dat reads back");
    if (restoredData != NULL)
    {
        const char *levelName = LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(restoredData, "LevelName", "");
        check(strcmp(levelName, "Harness World") == 0, "the level name restores");
    }

    LIBMATTI_MC_Level *fresh = LIBMATTI_MC_Level_New(-64, 384, LIBMATTI_MC_Level_OVERWORLD, true);
    if (restoredData != NULL)
    {
        check(LIBMATTI_MC_LevelStorage_ApplyLevelData(fresh, restoredData) == 1, "the level data applies");
        check(LIBMATTI_MC_Level_GetGameTime(fresh) == 12345, "the game time restores");
        check(LIBMATTI_MC_Level_GetDayTime(fresh) == 6000, "the day time restores");
        check(LIBMATTI_MC_Level_IsRaining(fresh), "the rain flag restores");
        LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) restoredData);
    }

    LIBMATTI_MC_LevelChunk *loadedChunk = LIBMATTI_MC_LevelStorage_LoadChunk(access, fresh, 0, 0);
    check(loadedChunk != NULL, "the saved chunk loads");
    LIBMATTI_MC_BlockPos ground = {{3, 64, 3}};
    LIBMATTI_MC_BlockPos stairPos = {{8, 64, 8}};
    check(LIBMATTI_MC_BlockState_GetBlock(LIBMATTI_MC_Level_GetBlockState(fresh, &ground))
              == (void *) LIBMATTI_MC_VanillaBlocks_GetByName("STONE"),
          "the loaded chunk carries the platform");
    check(LIBMATTI_MC_Level_GetBlockState(fresh, &stairPos) != NULL
              && LIBMATTI_MC_BlockState_GetBlock(LIBMATTI_MC_Level_GetBlockState(fresh, &stairPos))
                     == (void *) LIBMATTI_MC_VanillaBlocks_GetByName("OAK_STAIRS"),
          "the loaded chunk carries the stair state");
    check(LIBMATTI_MC_LevelStorage_LoadChunk(access, fresh, 5, 5) == NULL, "an absent chunk loads NULL");
    check(LIBMATTI_MC_Level_GetChunkCount(fresh) == 1, "one chunk lives in the fresh level");

    LIBMATTI_MC_LevelStorageAccess_Free(access);
    LIBMATTI_MC_Level_Free(fresh);
    LIBMATTI_MC_LevelStorageSource_Free(source);
}

int main(void)
{
    char dirTemplate[] = "/tmp/matti_storage_XXXXXX";
    char *dir = mkdtemp(dirTemplate);
    if (dir == NULL)
    {
        printf("FAIL: the temp dir is unavailable\n");
        return 1;
    }

    test_region_file_version();
    test_region_file(dir);
    test_region_file_external(dir);
    test_region_file_storage(dir);
    test_state_codec();
    test_chunk_codec();
    test_chunk_codec_global_palette();
    test_level_storage(dir);

    printf("%d/%d checks ok\n", checks - failures, checks);
    if (failures > 0)
        printf("%d FAILURES\n", failures);
    return failures > 0 ? 1 : 0;
}
