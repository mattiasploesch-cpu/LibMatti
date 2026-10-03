// Port of net.minecraft.world.level.chunk.storage.RegionFileVersion.
//
// Java wraps the per-chunk streams in a compression codec chosen by id byte;
// the C port runs the same ids over zlib (the gzip variant rides
// deflateInit2's gzip window like java.util.zip.GZIPOutputStream).

#ifndef MATTICRAFT_MC_WORLD_LEVEL_CHUNK_STORAGE_REGIONFILEVERSION_H
#define MATTICRAFT_MC_WORLD_LEVEL_CHUNK_STORAGE_REGIONFILEVERSION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: the registered versions (ids are wire-stable)
#define LIBMATTI_MC_RegionFileVersion_GZIP 1
#define LIBMATTI_MC_RegionFileVersion_DEFLATE 2
#define LIBMATTI_MC_RegionFileVersion_NONE 3

// Java: public static final RegionFileVersion DEFAULT = VERSION_DEFLATE
#define LIBMATTI_MC_RegionFileVersion_DEFAULT LIBMATTI_MC_RegionFileVersion_DEFLATE

// Java: public static boolean isValidVersion(int)
bool LIBMATTI_MC_RegionFileVersion_IsValidVersion(int id);

// Java: version.wrap(OutputStream) on the write side - the caller frees *outData.
// Returns 0 on success, -1 on an unknown id or a zlib error.
int LIBMATTI_MC_RegionFileVersion_Compress(int id, const uint8_t *data, size_t length,
                                           uint8_t **outData, size_t *outLength);

// Java: version.wrap(InputStream) on the read side - the caller frees *outData.
// Returns 0 on success, -1 on an unknown id or a zlib error.
int LIBMATTI_MC_RegionFileVersion_Decompress(int id, const uint8_t *data, size_t length,
                                             uint8_t **outData, size_t *outLength);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_LEVEL_CHUNK_STORAGE_REGIONFILEVERSION_H
