// Port of net.minecraft.nbt.NbtIo - the binary named-root format. Java streams
// through DataInput/DataOutput (big-endian, modified UTF-8); the port reads from
// and writes to memory buffers, the gzip layer comes from zlib like Java's
// GZIPInputStream/GZIPOutputStream.

#ifndef MATTICRAFT_NET_MINECRAFT_NBT_NBTIO_H
#define MATTICRAFT_NET_MINECRAFT_NBT_NBTIO_H

#include "libmatti/net/minecraft/nbt/CompoundTag.h"
#include "libmatti/net/minecraft/nbt/NbtAccounter.h"

#include <stddef.h>
#include <stdint.h>

typedef LIBMATTI_MC_NbtAccounter LIBMATTI_MC_Nbt_AccounterView;

// Java: public static CompoundTag read(DataInput) - the root tag is a named compound
LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_Nbt_NbtIo_Read(const uint8_t *data, size_t length,
                                                        LIBMATTI_MC_Nbt_AccounterView *accounter);
// Java: public static CompoundTag readCompressed(InputStream, NbtAccounter) - gzip
LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_Nbt_NbtIo_ReadCompressed(const uint8_t *data, size_t length,
                                                                  LIBMATTI_MC_Nbt_AccounterView *accounter);
// Java: public static CompoundTag readCompressed(Path) - reads the file first
LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_Nbt_NbtIo_ReadCompressedFile(const char *path,
                                                                      LIBMATTI_MC_Nbt_AccounterView *accounter);
// Java: public static @Nullable CompoundTag read(Path)
LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_Nbt_NbtIo_ReadFile(const char *path,
                                                            LIBMATTI_MC_Nbt_AccounterView *accounter);

// Java: public static void write(CompoundTag, DataOutput) - out: malloc'd buffer, caller frees
int LIBMATTI_MC_Nbt_NbtIo_Write(LIBMATTI_MC_Nbt_CompoundTag *compound, uint8_t **outData, size_t *outLength);
// Java: public static void writeCompressed(CompoundTag, OutputStream) - gzip
int LIBMATTI_MC_Nbt_NbtIo_WriteCompressed(LIBMATTI_MC_Nbt_CompoundTag *compound, uint8_t **outData,
                                          size_t *outLength);
// Java: public static void writeCompressed(CompoundTag, Path)
int LIBMATTI_MC_Nbt_NbtIo_WriteCompressedFile(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *path);
// Java: public static void write(CompoundTag, Path)
int LIBMATTI_MC_Nbt_NbtIo_WriteFile(LIBMATTI_MC_Nbt_CompoundTag *compound, const char *path);

#endif //MATTICRAFT_NET_MINECRAFT_NBT_NBTIO_H
