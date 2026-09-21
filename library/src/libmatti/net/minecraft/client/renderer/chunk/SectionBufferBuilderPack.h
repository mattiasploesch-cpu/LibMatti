// Port of net.minecraft.client.renderer.SectionBufferBuilderPack - one
// ByteBufferBuilder per ChunkSectionLayer with the fixed byte sizes.

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_SECTIONBUFFERBUILDERPACK_H
#define MATTICRAFT_MC_CLIENT_RENDERER_SECTIONBUFFERBUILDERPACK_H

#include "libmatti/com/mojang/blaze3d/vertex/ByteBufferBuilder.h"
#include "libmatti/net/minecraft/client/renderer/chunk/ChunkSectionLayer.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class SectionBufferBuilderPack implements AutoCloseable
typedef struct LIBMATTI_MC_SectionBufferBuilderPack
{
    // Java: private final Map<ChunkSectionLayer, ByteBufferBuilder> buffers
    LIBMATTI_B3D_ByteBufferBuilder *buffers[LIBMATTI_MC_ChunkSectionLayer_COUNT];
    // Java: private static final long TOTAL_BYTES - the port tracks the sum
    // for the pack-limit bookkeeping.
    long long totalBytes;
    // Java: private boolean closed
    int closed;
} LIBMATTI_MC_SectionBufferBuilderPack;

// Java: public SectionBufferBuilderPack() - creates every layer's builder with
// its bufferSize.
LIBMATTI_MC_SectionBufferBuilderPack *LIBMATTI_MC_SectionBufferBuilderPack_New(void);
// Java: public void close() - frees every builder.
void LIBMATTI_MC_SectionBufferBuilderPack_Free(LIBMATTI_MC_SectionBufferBuilderPack *pack);

// Java: public ByteBufferBuilder buffer(ChunkSectionLayer)
LIBMATTI_B3D_ByteBufferBuilder *LIBMATTI_MC_SectionBufferBuilderPack_Buffer(
    LIBMATTI_MC_SectionBufferBuilderPack *pack, LIBMATTI_MC_ChunkSectionLayer layer);

// Java: public void clearAll() - reset every builder between compiles.
void LIBMATTI_MC_SectionBufferBuilderPack_ClearAll(LIBMATTI_MC_SectionBufferBuilderPack *pack);

// Java: public static int maxBytes() - the pack budget (Java: TOTAL_BYTES).
long long LIBMATTI_MC_SectionBufferBuilderPack_MaxBytes(void);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_SECTIONBUFFERBUILDERPACK_H
