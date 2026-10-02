// Port of net.minecraft.world.level.chunk.storage.RegionFile (the .mca container)
// plus the net.minecraft.world.level.chunk.storage.RegionBitmap sector allocator.
//
// The wire format (1:1 with Java):
//   - 8192-byte header: 1024 little-endian int32 offsets (sector<<8 | count)
//     then 1024 little-endian int32 timestamps (epoch seconds).
//   - chunk index = regionLocalX + regionLocalZ * 32.
//   - each chunk payload starts with a 5-byte record: int32 length-1 (the byte
//     count after the length field, +1 in Java's ChunkBuffer bookkeeping) and
//     the one-byte compression id; the rest is the compressed NBT.
//   - chunks that compress to 256+ sectors spill into c.<x>.<z>.mcc next to
//     the region file and the offset entry points at a 1-sector stub.

#ifndef MATTICRAFT_MC_WORLD_LEVEL_CHUNK_STORAGE_REGIONFILE_H
#define MATTICRAFT_MC_WORLD_LEVEL_CHUNK_STORAGE_REGIONFILE_H

#include "libmatti/net/minecraft/world/level/ChunkPos.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: private static final int SECTOR_BYTES / CHUNK_HEADER_SIZE / ...
#define LIBMATTI_MC_RegionFile_SECTOR_BYTES 4096
#define LIBMATTI_MC_RegionFile_CHUNK_HEADER_SIZE 5
#define LIBMATTI_MC_RegionFile_EXTERNAL_STREAM_FLAG 128
#define LIBMATTI_MC_RegionFile_EXTERNAL_CHUNK_THRESHOLD 256
#define LIBMATTI_MC_RegionFile_CHUNK_NOT_PRESENT 0
// Java: public static final String EXTERNAL_FILE_EXTENSION
#define LIBMATTI_MC_RegionFile_EXTERNAL_FILE_EXTENSION ".mcc"

// Java: public class RegionFile implements AutoCloseable
typedef struct LIBMATTI_MC_RegionFile
{
    char *path;            // the .mca file path (owned)
    char *externalFileDir; // Java: the externalFileDir (owned)
    int version;           // Java: the RegionFileVersion the file writes
    FILE *file;            // the open channel (Java: FileChannel)
    // Java: private final IntBuffer offsets/timestamps - the in-memory header
    uint32_t offsets[1024];
    uint32_t timestamps[1024];
    // Java: protected final RegionBitmap usedSectors
    uint8_t *usedSectors; // the bitmap (Java's BitSet), 1024 entries (64KB cap)
} LIBMATTI_MC_RegionFile;

// Java: public RegionFile(RegionStorageInfo, Path, Path, boolean) through
// RegionFileVersion.getSelected() - opens or creates the file, validates the
// header (invalid sector entries reset to CHUNK_NOT_PRESENT like Java) and
// rebuilds the sector bitmap. version < 0 means "the selected default".
LIBMATTI_MC_RegionFile *LIBMATTI_MC_RegionFile_New(const char *path, const char *externalFileDir, int version);
// Java: public synchronized DataInputStream getChunkDataInputStream(ChunkPos) -
// the decompressed NBT bytes (NULL when the chunk is not present); caller frees
uint8_t *LIBMATTI_MC_RegionFile_ReadChunk(LIBMATTI_MC_RegionFile *regionFile, const LIBMATTI_MC_ChunkPos *pos,
                                          size_t *outLength);
// Java: the ChunkBuffer close path (getChunkDataOutputStream().close()) - the
// compressed payload bytes are written, sectors allocated and the header flushed
int LIBMATTI_MC_RegionFile_WriteChunk(LIBMATTI_MC_RegionFile *regionFile, const LIBMATTI_MC_ChunkPos *pos,
                                      const uint8_t *data, size_t length);
// Java: public boolean doesChunkExist(ChunkPos) / hasChunk(ChunkPos)
bool LIBMATTI_MC_RegionFile_DoesChunkExist(LIBMATTI_MC_RegionFile *regionFile, const LIBMATTI_MC_ChunkPos *pos);
bool LIBMATTI_MC_RegionFile_HasChunk(LIBMATTI_MC_RegionFile *regionFile, const LIBMATTI_MC_ChunkPos *pos);
// Java: public void clear(ChunkPos) - the offset entry resets and the sectors free
int LIBMATTI_MC_RegionFile_Clear(LIBMATTI_MC_RegionFile *regionFile, const LIBMATTI_MC_ChunkPos *pos);
// Java: public void flush() - the file stream hits the disk
void LIBMATTI_MC_RegionFile_Flush(LIBMATTI_MC_RegionFile *regionFile);
// Java: public void close() - padToFullSector + flush; Free closes and frees
int LIBMATTI_MC_RegionFile_Close(LIBMATTI_MC_RegionFile *regionFile);
void LIBMATTI_MC_RegionFile_Free(LIBMATTI_MC_RegionFile *regionFile);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_CHUNK_STORAGE_REGIONFILE_H
