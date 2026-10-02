// Port of net.minecraft.world.level.chunk.storage.RegionFile (implementation)
// and the RegionBitmap sector allocator. All multi-byte fields are little
// endian on disk (Java's ByteBuffer.DEFAULT_ORDER); the pack/unpack helpers
// keep the shifts explicit like the Java source does.

#include "libmatti/net/minecraft/world/level/chunk/storage/RegionFile.h"

#include "libmatti/net/minecraft/world/level/chunk/storage/RegionFileVersion.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Java: SECTOR_INTS = SECTOR_BYTES / 4 - the header table length
#define SECTOR_INTS 1024
// Java: CHUNK_NOT_PRESENT - the empty offset entry
#define HEADER_SIZE_BYTES 8192

// ---------------------------------------------------------------------------
// RegionBitmap - Java: net.minecraft.world.level.chunk.storage.RegionBitmap
// ---------------------------------------------------------------------------

// Java: BitSet used - one bit per sector, 1024 sectors per region file
static void bitmap_force(LIBMATTI_MC_RegionFile *rf, int from, int count)
{
    for (int i = from; i < from + count && i < SECTOR_INTS; i++)
        rf->usedSectors[i / 8] |= (uint8_t) (1u << (i % 8));
}

static void bitmap_free(LIBMATTI_MC_RegionFile *rf, int from, int count)
{
    for (int i = from; i < from + count && i < SECTOR_INTS; i++)
        rf->usedSectors[i / 8] &= (uint8_t) ~(1u << (i % 8));
}

static bool bitmap_is_set(const LIBMATTI_MC_RegionFile *rf, int index)
{
    return (rf->usedSectors[index / 8] & (1u << (index % 8))) != 0;
}

// Java: allocate - the first run of `count` clear bits starting at the lowest
// index (nextClearBit/nextSetBit walk), the run forced used on success
static int bitmap_allocate(LIBMATTI_MC_RegionFile *rf, int count)
{
    int scan = 0;
    for (;;)
    {
        while (scan < SECTOR_INTS && bitmap_is_set(rf, scan))
            scan++;
        int runStart = scan;
        int run = 0;
        while (runStart + run < SECTOR_INTS && !bitmap_is_set(rf, runStart + run) && run < count)
            run++;
        if (run >= count)
        {
            bitmap_force(rf, runStart, count);
            return runStart;
        }
        if (runStart + run >= SECTOR_INTS)
            return -1; // Java: throws IOException("Ran out of space")
        scan = runStart + run;
        // skip the set run after the gap (Java: i = k)
        while (scan < SECTOR_INTS && bitmap_is_set(rf, scan))
            scan++;
        if (scan >= SECTOR_INTS)
            return -1;
    }
}

// ---------------------------------------------------------------------------
// big-endian helpers (Java: the header ByteBuffer defaults to BIG_ENDIAN, so
// the sector table and the per-chunk record length are big-endian on disk)
// ---------------------------------------------------------------------------

static uint32_t read_u32(const uint8_t *data)
{
    return ((uint32_t) data[0] << 24) | ((uint32_t) data[1] << 16) | ((uint32_t) data[2] << 8)
           | (uint32_t) data[3];
}

static void write_u32(uint8_t *data, uint32_t value)
{
    data[0] = (uint8_t) ((value >> 24) & 0xFF);
    data[1] = (uint8_t) ((value >> 16) & 0xFF);
    data[2] = (uint8_t) ((value >> 8) & 0xFF);
    data[3] = (uint8_t) (value & 0xFF);
}

// Java: getSectorNumber = offset >> 8 & 0xFFFFFF
static int sector_number(uint32_t offset)
{
    return (int) ((offset >> 8) & 0xFFFFFF);
}

// Java: getNumSectors = offset & 0xFF
static int num_sectors(uint32_t offset)
{
    return (int) (offset & 0xFF);
}

// Java: sizeToSectors = (size + 4095) / 4096
static int size_to_sectors(size_t size)
{
    return (int) ((size + LIBMATTI_MC_RegionFile_SECTOR_BYTES - 1) / LIBMATTI_MC_RegionFile_SECTOR_BYTES);
}

// Java: packSectorOffset = sector << 8 | count
static uint32_t pack_sector_offset(int sector, int count)
{
    return ((uint32_t) sector << 8) | (uint32_t) (count & 0xFF);
}

// Java: getOffsetIndex(ChunkPos) = regionLocalX + regionLocalZ * 32
static int offset_index(const LIBMATTI_MC_ChunkPos *pos)
{
    return LIBMATTI_MC_ChunkPos_GetRegionLocalX(pos) + LIBMATTI_MC_ChunkPos_GetRegionLocalZ(pos) * 32;
}

// Java: getTimestamp = epoch seconds
static uint32_t current_timestamp(void)
{
    return (uint32_t) time(NULL);
}

// Java: getExternalChunkPath - c.<x>.<z>.mcc in the externalFileDir
static void external_chunk_path(const LIBMATTI_MC_RegionFile *rf, const LIBMATTI_MC_ChunkPos *pos, char *out,
                                size_t outSize)
{
    snprintf(out, outSize, "%s/c.%d.%d%s", rf->externalFileDir, pos->x, pos->z,
             LIBMATTI_MC_RegionFile_EXTERNAL_FILE_EXTENSION);
}

// Java: isExternalStreamChunk = (b & 128) != 0
static bool is_external_stream_chunk(uint8_t b)
{
    return (b & LIBMATTI_MC_RegionFile_EXTERNAL_STREAM_FLAG) != 0;
}

// Java: getExternalChunkVersion = b & ~128
static uint8_t external_chunk_version(uint8_t b)
{
    return (uint8_t) (b & (uint8_t) ~LIBMATTI_MC_RegionFile_EXTERNAL_STREAM_FLAG);
}

// the header write (Java: writeHeader - the 8KB block at offset 0)
static int write_header(LIBMATTI_MC_RegionFile *rf)
{
    uint8_t header[HEADER_SIZE_BYTES];
    memset(header, 0, sizeof(header));
    for (int i = 0; i < SECTOR_INTS; i++)
    {
        write_u32(header + i * 4, rf->offsets[i]);
        write_u32(header + 4096 + i * 4, rf->timestamps[i]);
    }
    if (fseek(rf->file, 0, SEEK_SET) != 0)
        return -1;
    if (fwrite(header, 1, sizeof(header), rf->file) != sizeof(header))
        return -1;
    return 0;
}

LIBMATTI_MC_RegionFile *LIBMATTI_MC_RegionFile_New(const char *path, const char *externalFileDir, int version)
{
    if (path == NULL || externalFileDir == NULL)
        return NULL;

    LIBMATTI_MC_RegionFile *rf = calloc(1, sizeof(LIBMATTI_MC_RegionFile));
    if (rf == NULL)
        return NULL;
    rf->path = strdup(path);
    rf->externalFileDir = strdup(externalFileDir);
    rf->version = version >= 0 ? version : LIBMATTI_MC_RegionFileVersion_DEFAULT;
    rf->usedSectors = calloc(SECTOR_INTS / 8, 1);
    if (rf->path == NULL || rf->externalFileDir == NULL || rf->usedSectors == NULL)
    {
        LIBMATTI_MC_RegionFile_Free(rf);
        return NULL;
    }

    // Java: usedSectors.force(0, 2) - the header sectors are always taken
    bitmap_force(rf, 0, 2);

    // Java: FileChannel.open(CREATE, READ, WRITE)
    rf->file = fopen(path, "r+b");
    if (rf->file == NULL)
        rf->file = fopen(path, "w+b");
    if (rf->file == NULL)
    {
        LIBMATTI_MC_RegionFile_Free(rf);
        return NULL;
    }

    // Java: file.read(header, 0) + the sector-table walk
    uint8_t header[HEADER_SIZE_BYTES];
    memset(header, 0, sizeof(header));
    fseek(rf->file, 0, SEEK_END);
    long fileSize = ftell(rf->file);
    fseek(rf->file, 0, SEEK_SET);
    size_t read = fread(header, 1, sizeof(header), rf->file);
    if (read != HEADER_SIZE_BYTES && fileSize > 0)
    {
        // Java: LOGGER.warn "Region file ... has truncated header" - the zero
        // rest of the buffer keeps the offsets empty, the file is valid
    }
    if (read > 0 || fileSize == 0)
    {
        for (int i = 0; i < SECTOR_INTS; i++)
        {
            uint32_t value = read_u32(header + i * 4);
            rf->offsets[i] = value;
            rf->timestamps[i] = read_u32(header + 4096 + i * 4);
            if (value != 0)
            {
                int sector = sector_number(value);
                int count = num_sectors(value);
                // Java: the three validation branches reset the entry to 0 -
                // header overlap, zero size, out of bounds
                if (sector < 2 || count == 0 || (long) sector * LIBMATTI_MC_RegionFile_SECTOR_BYTES > fileSize)
                    rf->offsets[i] = 0;
                else
                    bitmap_force(rf, sector, count);
            }
        }
    }
    return rf;
}

uint8_t *LIBMATTI_MC_RegionFile_ReadChunk(LIBMATTI_MC_RegionFile *rf, const LIBMATTI_MC_ChunkPos *pos,
                                          size_t *outLength)
{
    if (outLength != NULL)
        *outLength = 0;
    int index = offset_index(pos);
    uint32_t offset = rf->offsets[index];
    if (offset == 0)
        return NULL; // Java: CHUNK_NOT_PRESENT -> null stream

    int sector = sector_number(offset);
    int count = num_sectors(offset);
    size_t payloadSize = (size_t) count * LIBMATTI_MC_RegionFile_SECTOR_BYTES;
    uint8_t *payload = malloc(payloadSize);
    if (payload == NULL)
        return NULL;
    if (fseek(rf->file, (long) sector * LIBMATTI_MC_RegionFile_SECTOR_BYTES, SEEK_SET) != 0
        || fread(payload, 1, payloadSize, rf->file) != payloadSize)
    {
        free(payload);
        return NULL;
    }

    // Java: the 5-byte record (length-1 int32, compression byte)
    if (payloadSize < LIBMATTI_MC_RegionFile_CHUNK_HEADER_SIZE)
    {
        free(payload);
        return NULL;
    }
    uint32_t declared = read_u32(payload);
    uint8_t compressionId = payload[4];
    if (declared == 0)
    {
        // Java: "Chunk is allocated, but stream is missing"
        free(payload);
        return NULL;
    }

    // Java: the external-stream branch - the payload lives in the .mcc file
    if (is_external_stream_chunk(compressionId))
    {
        free(payload);
        compressionId = external_chunk_version(compressionId);
        char externalPath[1024];
        external_chunk_path(rf, pos, externalPath, sizeof(externalPath));
        FILE *external = fopen(externalPath, "rb");
        if (external == NULL)
            return NULL;
        fseek(external, 0, SEEK_END);
        long externalSize = ftell(external);
        fseek(external, 0, SEEK_SET);
        uint8_t *externalData = malloc(externalSize > 0 ? (size_t) externalSize : 1);
        if (externalData == NULL || fread(externalData, 1, (size_t) externalSize, external) != (size_t) externalSize)
        {
            free(externalData);
            fclose(external);
            return NULL;
        }
        fclose(external);
        uint8_t *decompressed = NULL;
        size_t decompressedLength = 0;
        int status = LIBMATTI_MC_RegionFileVersion_Decompress(compressionId, externalData, (size_t) externalSize,
                                                              &decompressed, &decompressedLength);
        free(externalData);
        if (status != 0)
            return NULL;
        if (outLength != NULL)
            *outLength = decompressedLength;
        return decompressed;
    }

    // Java: j1 = i1 - 1 - the byte count after the length field, truncated
    // records fail like Java's "stream is truncated"
    size_t streamLength = (size_t) declared - 1;
    if (streamLength > payloadSize - LIBMATTI_MC_RegionFile_CHUNK_HEADER_SIZE)
    {
        free(payload);
        return NULL;
    }

    uint8_t *decompressed = NULL;
    size_t decompressedLength = 0;
    int status = LIBMATTI_MC_RegionFileVersion_Decompress(compressionId, payload + 5, streamLength, &decompressed,
                                                          &decompressedLength);
    free(payload);
    if (status != 0)
        return NULL;
    if (outLength != NULL)
        *outLength = decompressedLength;
    return decompressed;
}

int LIBMATTI_MC_RegionFile_WriteChunk(LIBMATTI_MC_RegionFile *rf, const LIBMATTI_MC_ChunkPos *pos,
                                      const uint8_t *data, size_t length)
{
    // Java: the ChunkBuffer packs the 5-byte record first, then RegionFile.write
    size_t streamLength = length + 1; // Java: i = this.count - 5 + 1
    if (streamLength > 0xFFFFFFFFu)
        return -1;

    uint8_t *compressed = NULL;
    size_t compressedLength = 0;
    if (LIBMATTI_MC_RegionFileVersion_Compress(rf->version, data, length, &compressed, &compressedLength) != 0)
        return -1;

    // Java: write() - the 5-byte record rides the sector stream
    size_t recordSize = LIBMATTI_MC_RegionFile_CHUNK_HEADER_SIZE + compressedLength;
    int sectors = size_to_sectors(recordSize);
    uint8_t *record = malloc(sectors * LIBMATTI_MC_RegionFile_SECTOR_BYTES);
    if (record == NULL)
    {
        free(compressed);
        return -1;
    }
    memset(record, 0, (size_t) sectors * LIBMATTI_MC_RegionFile_SECTOR_BYTES);
    write_u32(record, (uint32_t) (compressedLength + 1));
    record[4] = (uint8_t) rf->version;
    if (compressedLength > 0)
        memcpy(record + 5, compressed, compressedLength);
    free(compressed);

    int index = offset_index(pos);
    uint32_t oldOffset = rf->offsets[index];

    if (sectors >= LIBMATTI_MC_RegionFile_EXTERNAL_CHUNK_THRESHOLD)
    {
        // Java: the external .mcc spill - a temp file is written then renamed,
        // the offset entry points at a 1-sector stub carrying only the record
        char externalPath[1024];
        external_chunk_path(rf, pos, externalPath, sizeof(externalPath));
        char tempPath[1100];
        snprintf(tempPath, sizeof(tempPath), "%s.tmp", externalPath);
        FILE *temp = fopen(tempPath, "wb");
        if (temp == NULL)
        {
            free(record);
            return -1;
        }
        // the temp file carries the compressed stream only (Java: position(5))
        size_t written = fwrite(record + 5, 1, compressedLength, temp);
        fclose(temp);
        free(record);
        if (written != compressedLength)
            return -1;
        uint8_t *stub = calloc(1, LIBMATTI_MC_RegionFile_SECTOR_BYTES);
        if (stub == NULL)
            return -1;

        int stubSector = bitmap_allocate(rf, 1);
        if (stubSector < 0)
        {
            free(stub);
            return -1;
        }
        // Java: createExternalStub - length 1 + the version id | 128
        stub[0] = 0;
        stub[1] = 0;
        stub[2] = 0;
        stub[3] = 1;
        stub[4] = (uint8_t) (rf->version | LIBMATTI_MC_RegionFile_EXTERNAL_STREAM_FLAG);
        if (fseek(rf->file, (long) stubSector * LIBMATTI_MC_RegionFile_SECTOR_BYTES, SEEK_SET) != 0
            || fwrite(stub, 1, LIBMATTI_MC_RegionFile_SECTOR_BYTES, rf->file) != LIBMATTI_MC_RegionFile_SECTOR_BYTES)
        {
            free(stub);
            return -1;
        }
        free(stub);

        rf->offsets[index] = pack_sector_offset(stubSector, 1);
        rf->timestamps[index] = current_timestamp();
        if (write_header(rf) != 0)
            return -1;
        // Java: the commit op - the temp file replaces the external chunk
        if (rename(tempPath, externalPath) != 0)
            return -1;
        if (num_sectors(oldOffset) != 0)
            bitmap_free(rf, sector_number(oldOffset), num_sectors(oldOffset));
        return 0;
    }

    int sector = bitmap_allocate(rf, sectors);
    if (sector < 0)
    {
        free(record);
        return -1;
    }
    if (fseek(rf->file, (long) sector * LIBMATTI_MC_RegionFile_SECTOR_BYTES, SEEK_SET) != 0
        || fwrite(record, 1, (size_t) sectors * LIBMATTI_MC_RegionFile_SECTOR_BYTES, rf->file)
               != (size_t) sectors * LIBMATTI_MC_RegionFile_SECTOR_BYTES)
    {
        free(record);
        bitmap_free(rf, sector, sectors);
        return -1;
    }
    free(record);

    rf->offsets[index] = pack_sector_offset(sector, sectors);
    rf->timestamps[index] = current_timestamp();
    if (write_header(rf) != 0)
        return -1;
    // Java: the internal-path commit op - the stale external chunk is deleted
    char externalPath[1024];
    external_chunk_path(rf, pos, externalPath, sizeof(externalPath));
    remove(externalPath);
    if (num_sectors(oldOffset) != 0)
        bitmap_free(rf, sector_number(oldOffset), num_sectors(oldOffset));
    return 0;
}

bool LIBMATTI_MC_RegionFile_HasChunk(LIBMATTI_MC_RegionFile *rf, const LIBMATTI_MC_ChunkPos *pos)
{
    return rf->offsets[offset_index(pos)] != 0;
}

bool LIBMATTI_MC_RegionFile_DoesChunkExist(LIBMATTI_MC_RegionFile *rf, const LIBMATTI_MC_ChunkPos *pos)
{
    // Java: doesChunkExist - the offset entry + the record validation
    uint32_t offset = rf->offsets[offset_index(pos)];
    if (offset == 0)
        return false;
    int sector = sector_number(offset);
    int count = num_sectors(offset);
    uint8_t record[LIBMATTI_MC_RegionFile_CHUNK_HEADER_SIZE];
    if (fseek(rf->file, (long) sector * LIBMATTI_MC_RegionFile_SECTOR_BYTES, SEEK_SET) != 0)
        return false;
    if (fread(record, 1, sizeof(record), rf->file) != sizeof(record))
        return false;
    uint32_t declared = read_u32(record);
    uint8_t compressionId = record[4];
    if (is_external_stream_chunk(compressionId))
    {
        if (!LIBMATTI_MC_RegionFileVersion_IsValidVersion(external_chunk_version(compressionId)))
            return false;
        char externalPath[1024];
        external_chunk_path(rf, pos, externalPath, sizeof(externalPath));
        FILE *external = fopen(externalPath, "rb");
        if (external == NULL)
            return false;
        fclose(external);
        return true;
    }
    if (!LIBMATTI_MC_RegionFileVersion_IsValidVersion(compressionId))
        return false;
    if (declared == 0)
        return false;
    size_t streamLength = (size_t) declared - 1;
    return streamLength <= (size_t) count * LIBMATTI_MC_RegionFile_SECTOR_BYTES;
}

int LIBMATTI_MC_RegionFile_Clear(LIBMATTI_MC_RegionFile *rf, const LIBMATTI_MC_ChunkPos *pos)
{
    // Java: clear - the entry resets, the header flushes, the external chunk
    // is deleted and the sectors free
    int index = offset_index(pos);
    uint32_t offset = rf->offsets[index];
    if (offset == 0)
        return 0;
    rf->offsets[index] = 0;
    rf->timestamps[index] = current_timestamp();
    if (write_header(rf) != 0)
        return -1;
    char externalPath[1024];
    external_chunk_path(rf, pos, externalPath, sizeof(externalPath));
    remove(externalPath);
    bitmap_free(rf, sector_number(offset), num_sectors(offset));
    return 0;
}

void LIBMATTI_MC_RegionFile_Flush(LIBMATTI_MC_RegionFile *rf)
{
    if (rf != NULL && rf->file != NULL)
        fflush(rf->file);
}

int LIBMATTI_MC_RegionFile_Close(LIBMATTI_MC_RegionFile *rf)
{
    if (rf == NULL || rf->file == NULL)
        return 0;
    // Java: padToFullSector - a single zero byte grows the file to the next
    // sector boundary so the next open reads whole sectors
    fseek(rf->file, 0, SEEK_END);
    long size = ftell(rf->file);
    long padded = (long) size_to_sectors((size_t) size) * LIBMATTI_MC_RegionFile_SECTOR_BYTES;
    int result = 0;
    if (size != padded)
    {
        const uint8_t zero = 0;
        if (fseek(rf->file, padded - 1, SEEK_SET) == 0 && fwrite(&zero, 1, 1, rf->file) != 1)
            result = -1;
    }
    if (fflush(rf->file) != 0)
        result = -1;
    fclose(rf->file);
    rf->file = NULL;
    return result;
}

void LIBMATTI_MC_RegionFile_Free(LIBMATTI_MC_RegionFile *rf)
{
    if (rf == NULL)
        return;
    if (rf->file != NULL)
    {
        LIBMATTI_MC_RegionFile_Close(rf);
    }
    free(rf->path);
    free(rf->externalFileDir);
    free(rf->usedSectors);
    free(rf);
}
