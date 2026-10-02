// Port of net.minecraft.world.level.chunk.storage.SerializableChunkData
// (implementation). The write path mirrors Java's record.write() key for key;
// the read path mirrors parse() + read() down to the heightmap bits formula
// (Mth.ceillog2(height + 1)) and the post-processing offset packing.

#include "libmatti/net/minecraft/world/level/chunk/storage/SerializableChunkData.h"

#include "libmatti/net/minecraft/SharedConstants.h"
#include "libmatti/net/minecraft/nbt/ListTag.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/world/level/Level.h"
#include "libmatti/net/minecraft/world/level/block/state/StateDefinition.h"
#include "libmatti/net/minecraft/world/level/block/state/properties/Property.h"
#include "libmatti/net/minecraft/world/level/chunk/LevelChunkSection.h"
#include "libmatti/net/minecraft/world/level/chunk/PalettedContainer.h"
#include "libmatti/net/minecraft/world/level/levelgen/Heightmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: the vanilla chunk status string the port writes and accepts
#define CHUNK_STATUS_FULL "minecraft:full"
// Java: the biome the port's sections carry (the biome model is worldgen content)
#define BIOME_PLAINS "minecraft:plains"

// ---------------------------------------------------------------------------
// small helpers
// ---------------------------------------------------------------------------

// Java: Mth.ceillog2 - the smallest bits with (1 << bits) >= value
static int ceil_log2(int value)
{
    int bits = 0;
    while ((1 << bits) < value)
        bits++;
    return bits;
}

// the heightmap serialization keys (Java: Types.getSerializationKey)
static const char *heightmap_key(int type)
{
    switch (type)
    {
        case LIBMATTI_MC_Heightmap_WORLD_SURFACE_WG:
            return "WORLD_SURFACE_WG";
        case LIBMATTI_MC_Heightmap_WORLD_SURFACE:
            return "WORLD_SURFACE";
        case LIBMATTI_MC_Heightmap_OCEAN_FLOOR_WG:
            return "OCEAN_FLOOR_WG";
        case LIBMATTI_MC_Heightmap_OCEAN_FLOOR:
            return "OCEAN_FLOOR";
        case LIBMATTI_MC_Heightmap_MOTION_BLOCKING:
            return "MOTION_BLOCKING";
        case LIBMATTI_MC_Heightmap_MOTION_BLOCKING_NO_LEAVES:
            return "MOTION_BLOCKING_NO_LEAVES";
        default:
            return NULL;
    }
}

static int heightmap_type_for_key(const char *key)
{
    for (int type = 0; type < LIBMATTI_MC_Heightmap_TYPES_COUNT; type++)
    {
        const char *name = heightmap_key(type);
        if (name != NULL && strcmp(name, key) == 0)
            return type;
    }
    return -1;
}

// ---------------------------------------------------------------------------
// the block-state string codec (Java: BlockStateCodec / Block.toString)
// ---------------------------------------------------------------------------

char *LIBMATTI_MC_SerializableChunkData_StateToString(const LIBMATTI_MC_BlockState *state)
{
    if (state == NULL)
        return NULL;
    LIBMATTI_MC_Block *block = LIBMATTI_MC_BlockState_GetBlock(state);
    const char *id = LIBMATTI_MC_VanillaBlocks_IdOf(block);
    if (id == NULL)
        return NULL;

    int propertyCount = 0;
    LIBMATTI_MC_Property **properties = LIBMATTI_MC_StateHolder_GetProperties(&state->holder, &propertyCount);
    if (properties == NULL || propertyCount == 0)
    {
        char *out = malloc(strlen(id) + 11);
        if (out != NULL)
            sprintf(out, "minecraft:%s", id);
        return out;
    }

    // Java: the TreeMap order - the properties print sorted by name
    int *order = malloc((size_t) propertyCount * sizeof(int));
    if (order == NULL)
        return NULL;
    for (int i = 0; i < propertyCount; i++)
        order[i] = i;
    for (int i = 1; i < propertyCount; i++)
    {
        int current = order[i];
        int j = i - 1;
        while (j >= 0 && strcmp(LIBMATTI_MC_Property_GetName(properties[order[j]]),
                                LIBMATTI_MC_Property_GetName(properties[current])) > 0)
        {
            order[j + 1] = order[j];
            j--;
        }
        order[j + 1] = current;
    }

    // the exact size first, then the format pass
    size_t length = strlen(id) + 10 + 2; // "minecraft:" + "[...]"
    for (int i = 0; i < propertyCount; i++)
    {
        LIBMATTI_MC_Property *property = properties[order[i]];
        LIBMATTI_MC_Property_Value value = LIBMATTI_MC_StateHolder_GetValue(&state->holder, property);
        length += strlen(LIBMATTI_MC_Property_GetName(property)) + 1
                  + strlen(LIBMATTI_MC_Property_ValueName(property, value)) + 1;
    }

    char *out = malloc(length);
    if (out == NULL)
    {
        free(order);
        return NULL;
    }
    char *cursor = out;
    cursor += sprintf(cursor, "minecraft:%s[", id);
    for (int i = 0; i < propertyCount; i++)
    {
        LIBMATTI_MC_Property *property = properties[order[i]];
        LIBMATTI_MC_Property_Value value = LIBMATTI_MC_StateHolder_GetValue(&state->holder, property);
        cursor += sprintf(cursor, "%s%s=%s", i == 0 ? "" : ",",
                          LIBMATTI_MC_Property_GetName(property),
                          LIBMATTI_MC_Property_ValueName(property, value));
    }
    strcpy(cursor, "]");
    free(order);
    return out;
}

// the id -> block lookup (the port's registry exposes the path ids through
// VanillaBlocks_IdOf; the scan is cached on the last hit like the Java map read
// pattern in the hot path)
static LIBMATTI_MC_Block *block_by_id(const char *id)
{
    LIBMATTI_MC_Block **all = LIBMATTI_MC_VanillaBlocks_All();
    if (all == NULL)
        return NULL;
    for (int i = 0; i < LIBMATTI_MC_VanillaBlocks_COUNT; i++)
    {
        const char *candidate = LIBMATTI_MC_VanillaBlocks_IdOf(all[i]);
        if (candidate != NULL && strcmp(candidate, id) == 0)
            return all[i];
    }
    return NULL;
}

LIBMATTI_MC_BlockState *LIBMATTI_MC_SerializableChunkData_StateFromString(const char *string)
{
    if (string == NULL)
        return NULL;

    // Java: Identifier.parse + the block registry lookup; the namespace
    // defaults to minecraft like Identifier.withDefaultNamespace
    const char *id = string;
    if (strncmp(id, "minecraft:", 10) == 0)
        id += 10;
    else if (strchr(id, ':') != NULL)
        return NULL; // a foreign namespace the port's registry cannot hold

    char nameBuffer[256];
    size_t nameLength = 0;
    while (id[nameLength] != '\0' && id[nameLength] != '[' && nameLength < sizeof(nameBuffer) - 1)
        nameLength++;
    memcpy(nameBuffer, id, nameLength);
    nameBuffer[nameLength] = '\0';

    LIBMATTI_MC_Block *block = block_by_id(nameBuffer);
    if (block == NULL)
        return NULL;
    LIBMATTI_MC_BlockState *state = LIBMATTI_MC_Block_DefaultBlockState(block);
    if (state == NULL)
        return NULL;

    if (id[nameLength] != '[')
        return state; // the default state

    // the property list "k=v,k2=v2"
    const char *cursor = id + nameLength + 1;
    if (*cursor == ']')
        return state;
    while (*cursor != '\0' && *cursor != ']')
    {
        const char *equals = strchr(cursor, '=');
        if (equals == NULL)
            return NULL;
        size_t keyLength = (size_t) (equals - cursor);
        const char *valueEnd = strchr(equals + 1, ',');
        if (valueEnd == NULL)
            valueEnd = strchr(equals + 1, ']');
        if (valueEnd == NULL)
            return NULL;

        char keyBuffer[128];
        char valueBuffer[128];
        if (keyLength >= sizeof(keyBuffer) || (size_t) (valueEnd - equals - 1) >= sizeof(valueBuffer))
            return NULL;
        memcpy(keyBuffer, cursor, keyLength);
        keyBuffer[keyLength] = '\0';
        size_t valueLength = (size_t) (valueEnd - equals - 1);
        memcpy(valueBuffer, equals + 1, valueLength);
        valueBuffer[valueLength] = '\0';

        // Java: the property lookup on the block's state definition
        int propertyCount = 0;
        LIBMATTI_MC_Property **properties = LIBMATTI_MC_StateHolder_GetProperties(&state->holder, &propertyCount);
        LIBMATTI_MC_Property *property = NULL;
        for (int i = 0; i < propertyCount; i++)
        {
            if (strcmp(LIBMATTI_MC_Property_GetName(properties[i]), keyBuffer) == 0)
            {
                property = properties[i];
                break;
            }
        }
        if (property == NULL)
            return NULL;
        int found = 0;
        LIBMATTI_MC_Property_Value value = LIBMATTI_MC_Property_ParseValue(property, valueBuffer, &found);
        if (found == 0)
            return NULL;
        state = LIBMATTI_MC_BlockState_SetValue(state, property, value);
        if (state == NULL)
            return NULL;

        if (*valueEnd == ',')
            cursor = valueEnd + 1;
        else
            break;
    }
    return state;
}

// ---------------------------------------------------------------------------
// the heightmap packing (Java: Heightmap.pack / setRawData over SimpleBitStorage)
// ---------------------------------------------------------------------------

// Java: the ctor - bits = Mth.ceillog2(height + 1)
static int heightmap_bits(int chunkHeight)
{
    return ceil_log2(chunkHeight + 1);
}

// the port's one-entry-per-column array -> the packed long[] (the caller frees)
static int64_t *pack_heightmap(const LIBMATTI_MC_Heightmap *heightmap, int chunkHeight, size_t *outCount)
{
    int bits = heightmap_bits(chunkHeight);
    int valuesPerLong = 64 / bits;
    size_t longCount = (256 + (size_t) valuesPerLong - 1) / (size_t) valuesPerLong;
    int64_t *out = calloc(longCount, sizeof(int64_t));
    if (out == NULL)
        return NULL;
    for (int i = 0; i < 256; i++)
    {
        uint64_t value = (uint64_t) heightmap->data[i] & ((1ULL << bits) - 1);
        out[i / valuesPerLong] |= (int64_t) (value << ((i % valuesPerLong) * bits));
    }
    if (outCount != NULL)
        *outCount = longCount;
    return out;
}

// the packed long[] -> the port's one-entry-per-column array
static int unpack_heightmap(LIBMATTI_MC_Heightmap *heightmap, int chunkHeight, const int64_t *data, size_t count)
{
    int bits = heightmap_bits(chunkHeight);
    int valuesPerLong = 64 / bits;
    size_t expected = (256 + (size_t) valuesPerLong - 1) / (size_t) valuesPerLong;
    if (count != expected)
        return -1; // Java: the size-mismatch warn + primeHeightmaps fallback
    uint64_t mask = (1ULL << bits) - 1;
    for (int i = 0; i < 256; i++)
    {
        uint64_t word = (uint64_t) data[i / valuesPerLong];
        heightmap->data[i] = (uint16_t) ((word >> ((i % valuesPerLong) * bits)) & mask);
    }
    return 0;
}

// ---------------------------------------------------------------------------
// the write path (Java: copyOf(...).write())
// ---------------------------------------------------------------------------

// one section's block_states compound (Java: store("block_states", codec, states))
static void write_block_states(LIBMATTI_MC_Nbt_CompoundTag *sectionTag, LIBMATTI_MC_LevelChunkSection *section)
{
    LIBMATTI_MC_PalettedContainer *container = LIBMATTI_MC_LevelChunkSection_GetStates(section);
    LIBMATTI_MC_Nbt_CompoundTag *blockStates = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_ListTag *palette = LIBMATTI_MC_Nbt_ListTag_New();

    if (container == NULL)
    {
        LIBMATTI_MC_Nbt_ListTag_Add(palette, LIBMATTI_MC_Nbt_StringTag_Of("minecraft:air"));
        LIBMATTI_MC_Nbt_CompoundTag_Put(blockStates, "palette", (LIBMATTI_MC_Nbt_Tag *) palette);
        LIBMATTI_MC_Nbt_CompoundTag_Put(sectionTag, "block_states", (LIBMATTI_MC_Nbt_Tag *) blockStates);
        return;
    }

    if (LIBMATTI_MC_PalettedContainer_GetMode(container) == LIBMATTI_MC_PalettedContainer_Mode_SINGLE_VALUE)
    {
        char *stateString = LIBMATTI_MC_SerializableChunkData_StateToString(container->singleValue);
        LIBMATTI_MC_Nbt_ListTag_Add(palette, LIBMATTI_MC_Nbt_StringTag_Of(stateString != NULL ? stateString
                                                                                              : "minecraft:air"));
        free(stateString);
    }
    else
    {
        size_t paletteSize = LIBMATTI_MC_PalettedContainer_GetPaletteSize(container);
        for (size_t i = 0; i < paletteSize; i++)
        {
            char *stateString = LIBMATTI_MC_SerializableChunkData_StateToString(
                LIBMATTI_MC_PalettedContainer_GetPaletteEntry(container, i));
            LIBMATTI_MC_Nbt_ListTag_Add(palette, LIBMATTI_MC_Nbt_StringTag_Of(stateString != NULL ? stateString
                                                                                                  : "minecraft:air"));
            free(stateString);
        }
    }

    LIBMATTI_MC_Nbt_CompoundTag_Put(blockStates, "palette", (LIBMATTI_MC_Nbt_Tag *) palette);

    // Java: the codec writes the data array only when the storage exists
    size_t rawLength = 0;
    const uint64_t *raw = LIBMATTI_MC_PalettedContainer_GetRaw(container, &rawLength);
    if (raw != NULL && rawLength > 0)
    {
        LIBMATTI_MC_Nbt_CompoundTag_PutLongArray(blockStates, "data", (const int64_t *) raw, rawLength);
    }
    LIBMATTI_MC_Nbt_CompoundTag_Put(sectionTag, "block_states", (LIBMATTI_MC_Nbt_Tag *) blockStates);
}

LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_SerializableChunkData_Write(LIBMATTI_MC_LevelChunk *chunk)
{
    if (chunk == NULL)
        return NULL;
    LIBMATTI_MC_Nbt_CompoundTag *tag = LIBMATTI_MC_Nbt_CompoundTag_New();

    // Java: NbtUtils.addCurrentDataVersion(compoundtag)
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(tag, "DataVersion", LIBMATTI_MC_SharedConstants_GetDataVersion());
    const LIBMATTI_MC_ChunkPos *pos = LIBMATTI_MC_LevelChunk_GetPos(chunk);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(tag, LIBMATTI_MC_SerializableChunkData_X_POS_TAG, pos->x);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(tag, "yPos", chunk->base.levelHeightAccessor.minY / 16);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(tag, LIBMATTI_MC_SerializableChunkData_Z_POS_TAG, pos->z);
    // Java: this.lastUpdateTime = serverLevel.getGameTime()
    int64_t lastUpdate = chunk->level != NULL ? chunk->level->gameTime : 0;
    LIBMATTI_MC_Nbt_CompoundTag_PutLong(tag, "LastUpdate", lastUpdate);
    LIBMATTI_MC_Nbt_CompoundTag_PutLong(tag, "InhabitedTime", LIBMATTI_MC_ChunkAccess_GetInhabitedTime(&chunk->base));
    LIBMATTI_MC_Nbt_CompoundTag_PutString(tag, "Status", CHUNK_STATUS_FULL);

    // Java: the sections list - only-air sections are skipped like copyOf's null
    // SectionData entries
    LIBMATTI_MC_Nbt_ListTag *sections = LIBMATTI_MC_Nbt_ListTag_New();
    int minSectionY = chunk->base.levelHeightAccessor.minY / 16;
    for (int i = 0; i < chunk->base.sectionCount; i++)
    {
        LIBMATTI_MC_LevelChunkSection *section = chunk->base.sections[i];
        if (section == NULL || LIBMATTI_MC_LevelChunkSection_HasOnlyAir(section))
            continue;
        LIBMATTI_MC_Nbt_CompoundTag *sectionTag = LIBMATTI_MC_Nbt_CompoundTag_New();
        LIBMATTI_MC_Nbt_CompoundTag_PutByte(sectionTag, "Y", (int8_t) (minSectionY + i));
        write_block_states(sectionTag, section);
        // Java: store("biomes", biomeCodec, biomes) - the port writes the single
        // plains palette the biome model will replace
        LIBMATTI_MC_Nbt_CompoundTag *biomes = LIBMATTI_MC_Nbt_CompoundTag_New();
        LIBMATTI_MC_Nbt_ListTag *biomePalette = LIBMATTI_MC_Nbt_ListTag_New();
        LIBMATTI_MC_Nbt_ListTag_Add(biomePalette, LIBMATTI_MC_Nbt_StringTag_Of(BIOME_PLAINS));
        LIBMATTI_MC_Nbt_CompoundTag_Put(biomes, "palette", (LIBMATTI_MC_Nbt_Tag *) biomePalette);
        LIBMATTI_MC_Nbt_CompoundTag_Put(sectionTag, "biomes", (LIBMATTI_MC_Nbt_Tag *) biomes);
        LIBMATTI_MC_Nbt_ListTag_Add(sections, (LIBMATTI_MC_Nbt_Tag *) sectionTag);
    }
    LIBMATTI_MC_Nbt_CompoundTag_Put(tag, LIBMATTI_MC_SerializableChunkData_SECTIONS_TAG, (LIBMATTI_MC_Nbt_Tag *) sections);

    if (chunk->base.isLightCorrect)
        LIBMATTI_MC_Nbt_CompoundTag_PutBoolean(tag, LIBMATTI_MC_SerializableChunkData_IS_LIGHT_ON_TAG, 1);

    // Java: the block_entities list - the port saves the pending tags (the live
    // BlockEntity NBT writers are game-port content)
    LIBMATTI_MC_Nbt_ListTag *blockEntities = LIBMATTI_MC_Nbt_ListTag_New();
    for (int i = 0; i < chunk->base.pendingBlockEntityCount; i++)
    {
        LIBMATTI_MC_Nbt_Tag *pending = chunk->base.pendingBlockEntities[i].tag;
        if (pending != NULL && pending->id == LIBMATTI_MC_Nbt_TAG_COMPOUND)
            LIBMATTI_MC_Nbt_ListTag_Add(blockEntities, LIBMATTI_MC_Nbt_Tag_Copy(pending));
    }
    LIBMATTI_MC_Nbt_CompoundTag_Put(tag, "block_entities", (LIBMATTI_MC_Nbt_Tag *) blockEntities);

    // Java: saveTicks (empty until the tick list lands) + the post-processing
    // offsets (ShortList[] -> ListTag of ListTags of ShortTags)
    LIBMATTI_MC_Nbt_ListTag *postProcessing = LIBMATTI_MC_Nbt_ListTag_New();
    for (int i = 0; i < chunk->base.postProcessingCount; i++)
    {
        LIBMATTI_MC_PostProcessingList *list = &chunk->base.postProcessing[i];
        LIBMATTI_MC_Nbt_ListTag *sectionList = LIBMATTI_MC_Nbt_ListTag_New();
        for (int j = 0; j < list->count; j++)
        {
            LIBMATTI_MC_Nbt_Tag *shortTag = LIBMATTI_MC_Nbt_ShortTag_Of((int16_t) list->packed[j]);
            LIBMATTI_MC_Nbt_ListTag_Add(sectionList, shortTag);
        }
        LIBMATTI_MC_Nbt_ListTag_Add(postProcessing, (LIBMATTI_MC_Nbt_Tag *) sectionList);
    }
    LIBMATTI_MC_Nbt_CompoundTag_Put(tag, "PostProcessing", (LIBMATTI_MC_Nbt_Tag *) postProcessing);

    // Java: the Heightmaps compound - every primed heightmap packs
    LIBMATTI_MC_Nbt_CompoundTag *heightmaps = LIBMATTI_MC_Nbt_CompoundTag_New();
    int chunkHeight = chunk->base.levelHeightAccessor.height;
    for (int type = 0; type < LIBMATTI_MC_Heightmap_TYPES_COUNT; type++)
    {
        LIBMATTI_MC_Heightmap *heightmap = chunk->base.heightmaps[type];
        if (heightmap == NULL)
            continue;
        size_t count = 0;
        int64_t *packed = pack_heightmap(heightmap, chunkHeight, &count);
        if (packed == NULL)
            continue;
        LIBMATTI_MC_Nbt_CompoundTag_PutLongArray(heightmaps, heightmap_key(type), packed, count);
        free(packed);
    }
    LIBMATTI_MC_Nbt_CompoundTag_Put(tag, LIBMATTI_MC_SerializableChunkData_HEIGHTMAPS_TAG,
                                    (LIBMATTI_MC_Nbt_Tag *) heightmaps);

    // Java: compoundtag.put("structures", this.structureData) - the empty
    // structures compound (the structure model is game-port content)
    LIBMATTI_MC_Nbt_CompoundTag *structures = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Nbt_CompoundTag_Put(tag, "structures", (LIBMATTI_MC_Nbt_Tag *) structures);

    return tag;
}

// ---------------------------------------------------------------------------
// the read path (Java: parse(...).read())
// ---------------------------------------------------------------------------

bool LIBMATTI_MC_SerializableChunkData_IsFullChunk(const LIBMATTI_MC_Nbt_CompoundTag *tag)
{
    if (tag == NULL)
        return false;
    const char *status = LIBMATTI_MC_Nbt_CompoundTag_GetStringOr(tag, "Status", "");
    return strcmp(status, CHUNK_STATUS_FULL) == 0;
}

// one section's block_states compound -> the section's container
static int read_block_states(LIBMATTI_MC_LevelChunkSection *section, const LIBMATTI_MC_Nbt_CompoundTag *sectionTag,
                             int chunkHeight)
{
    LIBMATTI_MC_Nbt_CompoundTag *blockStates = NULL;
    if (!LIBMATTI_MC_Nbt_CompoundTag_GetCompound(sectionTag, "block_states", &blockStates) || blockStates == NULL)
        return -1;
    LIBMATTI_MC_Nbt_ListTag *palette = NULL;
    if (!LIBMATTI_MC_Nbt_CompoundTag_GetList(blockStates, "palette", &palette) || palette == NULL)
        return -1;
    int paletteSize = LIBMATTI_MC_Nbt_ListTag_Size(palette);
    if (paletteSize <= 0)
        return -1;

    LIBMATTI_MC_BlockState **states = malloc((size_t) paletteSize * sizeof(LIBMATTI_MC_BlockState *));
    if (states == NULL)
        return -1;
    for (int i = 0; i < paletteSize; i++)
    {
        const char *stateString = NULL;
        if (!LIBMATTI_MC_Nbt_ListTag_GetString(palette, i, &stateString))
        {
            free(states);
            return -1;
        }
        states[i] = LIBMATTI_MC_SerializableChunkData_StateFromString(stateString);
        if (states[i] == NULL)
        {
            free(states);
            return -1;
        }
    }

    const int64_t *data = NULL;
    size_t dataLength = 0;
    bool hasData = LIBMATTI_MC_Nbt_CompoundTag_GetLongArray(blockStates, "data", &data, &dataLength);
    int bits = 0;
    if (hasData && dataLength > 0)
    {
        // Java: the codec recomputes the bits from the palette size
        bits = ceil_log2(paletteSize);
        if (bits < LIBMATTI_MC_PalettedContainer_MIN_BITS)
            bits = LIBMATTI_MC_PalettedContainer_MIN_BITS;
        int valuesPerLong = 64 / bits;
        size_t expected = (LIBMATTI_MC_LevelChunkSection_SECTION_SIZE + (size_t) valuesPerLong - 1)
                          / (size_t) valuesPerLong;
        if (dataLength != expected)
        {
            free(states);
            return -1;
        }
    }

    LIBMATTI_MC_PalettedContainer *container = LIBMATTI_MC_PalettedContainer_ReadFromPalette(
        states, (size_t) paletteSize, hasData ? (const uint64_t *) data : NULL, dataLength, bits);
    free(states);
    if (container == NULL)
        return -1;

    // the fresh section's all-air container is replaced by the loaded one
    if (section->states != NULL)
        LIBMATTI_MC_PalettedContainer_Free(section->states);
    section->states = container;
    LIBMATTI_MC_LevelChunkSection_RecalcBlockCounts(section);
    (void) chunkHeight;
    return 0;
}

LIBMATTI_MC_LevelChunk *LIBMATTI_MC_SerializableChunkData_Read(struct LIBMATTI_MC_Level *level,
                                                               const LIBMATTI_MC_Nbt_CompoundTag *tag)
{
    if (level == NULL || tag == NULL)
        return NULL;
    int32_t chunkX = 0;
    int32_t chunkZ = 0;
    if (!LIBMATTI_MC_Nbt_CompoundTag_GetInt(tag, LIBMATTI_MC_SerializableChunkData_X_POS_TAG, &chunkX))
        return NULL;
    if (!LIBMATTI_MC_Nbt_CompoundTag_GetInt(tag, LIBMATTI_MC_SerializableChunkData_Z_POS_TAG, &chunkZ))
        return NULL;

    LIBMATTI_MC_ChunkPos pos;
    pos.x = chunkX;
    pos.z = chunkZ;
    LIBMATTI_MC_LevelChunk *chunk = LIBMATTI_MC_LevelChunk_New(level, &pos);
    if (chunk == NULL)
        return NULL;

    int minSectionY = chunk->base.levelHeightAccessor.minY / 16;
    int chunkHeight = chunk->base.levelHeightAccessor.height;

    LIBMATTI_MC_Nbt_ListTag *sections = NULL;
    if (LIBMATTI_MC_Nbt_CompoundTag_GetList(tag, LIBMATTI_MC_SerializableChunkData_SECTIONS_TAG, &sections)
        && sections != NULL)
    {
        int sectionCount = LIBMATTI_MC_Nbt_ListTag_Size(sections);
        for (int i = 0; i < sectionCount; i++)
        {
            LIBMATTI_MC_Nbt_CompoundTag *sectionTag = NULL;
            if (!LIBMATTI_MC_Nbt_ListTag_GetCompound(sections, i, &sectionTag) || sectionTag == NULL)
                continue;
            int8_t sectionY = 0;
            if (!LIBMATTI_MC_Nbt_CompoundTag_GetByte(sectionTag, "Y", &sectionY))
                continue;
            int index = (int) sectionY - minSectionY;
            if (index < 0 || index >= chunk->base.sectionCount)
                continue;
            read_block_states(chunk->base.sections[index], sectionTag, chunkHeight);
        }
    }

    // Java: the Heightmaps compound - setRawData per key (unpacked)
    LIBMATTI_MC_Nbt_CompoundTag *heightmaps = NULL;
    if (LIBMATTI_MC_Nbt_CompoundTag_GetCompound(tag, LIBMATTI_MC_SerializableChunkData_HEIGHTMAPS_TAG, &heightmaps)
        && heightmaps != NULL)
    {
        size_t keyCount = 0;
        char **keys = LIBMATTI_MC_Nbt_CompoundTag_KeySet(heightmaps, &keyCount);
        for (size_t i = 0; i < keyCount; i++)
        {
            int type = heightmap_type_for_key(keys[i]);
            if (type < 0)
                continue;
            const int64_t *raw = NULL;
            size_t rawLength = 0;
            if (!LIBMATTI_MC_Nbt_CompoundTag_GetLongArray(heightmaps, keys[i], &raw, &rawLength))
                continue;
            LIBMATTI_MC_Heightmap *heightmap = LIBMATTI_MC_ChunkAccess_GetOrCreateHeightmapUnprimed(&chunk->base, type);
            if (heightmap != NULL)
                unpack_heightmap(heightmap, chunkHeight, raw, rawLength);
        }
    }

    // Java: the PostProcessing offsets - addPackedPostProcess per entry
    LIBMATTI_MC_Nbt_ListTag *postProcessing = NULL;
    if (LIBMATTI_MC_Nbt_CompoundTag_GetList(tag, "PostProcessing", &postProcessing) && postProcessing != NULL)
    {
        int listCount = LIBMATTI_MC_Nbt_ListTag_Size(postProcessing);
        for (int i = 0; i < listCount; i++)
        {
            LIBMATTI_MC_Nbt_ListTag *sectionList = NULL;
            if (!LIBMATTI_MC_Nbt_ListTag_GetList(postProcessing, i, &sectionList) || sectionList == NULL)
                continue;
            int entryCount = LIBMATTI_MC_Nbt_ListTag_Size(sectionList);
            for (int j = 0; j < entryCount; j++)
            {
                int16_t packed = 0;
                if (LIBMATTI_MC_Nbt_ListTag_GetShort(sectionList, j, &packed))
                    LIBMATTI_MC_ChunkAccess_AddPackedPostProcess(&chunk->base, i, (uint16_t) packed);
            }
        }
    }

    // Java: the block_entities list -> the pending NBT map (the post-load
    // processor promotes them once the block entity factory lands)
    LIBMATTI_MC_Nbt_ListTag *blockEntities = NULL;
    if (LIBMATTI_MC_Nbt_CompoundTag_GetList(tag, "block_entities", &blockEntities) && blockEntities != NULL)
    {
        int entityCount = LIBMATTI_MC_Nbt_ListTag_Size(blockEntities);
        for (int i = 0; i < entityCount; i++)
        {
            LIBMATTI_MC_Nbt_CompoundTag *entityTag = NULL;
            if (!LIBMATTI_MC_Nbt_ListTag_GetCompound(blockEntities, i, &entityTag) || entityTag == NULL)
                continue;
            const int32_t *posArray = NULL;
            size_t posLength = 0;
            if (!LIBMATTI_MC_Nbt_CompoundTag_GetIntArray(entityTag, "pos", &posArray, &posLength) || posLength != 3)
                continue;
            LIBMATTI_MC_BlockPos blockPos = {{posArray[0], posArray[1], posArray[2]}};
            LIBMATTI_MC_Nbt_Tag *copy = LIBMATTI_MC_Nbt_Tag_Copy((const LIBMATTI_MC_Nbt_Tag *) entityTag);
            if (copy != NULL)
                LIBMATTI_MC_ChunkAccess_SetBlockEntityNbt(&chunk->base, copy, &blockPos);
        }
    }

    int64_t inhabitedTime = 0;
    if (LIBMATTI_MC_Nbt_CompoundTag_GetLong(tag, "InhabitedTime", &inhabitedTime))
        LIBMATTI_MC_ChunkAccess_SetInhabitedTime(&chunk->base, inhabitedTime);
    chunk->base.isLightCorrect = LIBMATTI_MC_Nbt_CompoundTag_GetBooleanOr(tag,
                                                                          LIBMATTI_MC_SerializableChunkData_IS_LIGHT_ON_TAG,
                                                                          0) != 0;
    return chunk;
}
