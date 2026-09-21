// Port of net.minecraft.client.renderer.chunk.ChunkSectionLayer - the four
// terrain layers a section compiles into (Java's enum with the pipeline, the
// buffer size and the sortOnUpload flag).

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_CHUNKSECTIONLAYER_H
#define MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_CHUNKSECTIONLAYER_H

#ifdef __cplusplus
extern "C" {
#endif

// Java: public enum ChunkSectionLayer { SOLID, CUTOUT, TRANSLUCENT, TRIPWIRE }
// The RenderPipeline / bufferSize / sortOnUpload flags ride along as the
// port's layer tables (the pipeline slots stay the render port's part).
typedef enum LIBMATTI_MC_ChunkSectionLayer
{
    LIBMATTI_MC_ChunkSectionLayer_SOLID = 0,
    LIBMATTI_MC_ChunkSectionLayer_CUTOUT,
    LIBMATTI_MC_ChunkSectionLayer_TRANSLUCENT,
    LIBMATTI_MC_ChunkSectionLayer_TRIPWIRE,
    LIBMATTI_MC_ChunkSectionLayer_COUNT
} LIBMATTI_MC_ChunkSectionLayer;

// Java: public int bufferSize() - the pre-allocation per layer.
int LIBMATTI_MC_ChunkSectionLayer_BufferSize(LIBMATTI_MC_ChunkSectionLayer layer);
// Java: public boolean sortOnUpload()
int LIBMATTI_MC_ChunkSectionLayer_SortOnUpload(LIBMATTI_MC_ChunkSectionLayer layer);
// Java: public String label()
const char *LIBMATTI_MC_ChunkSectionLayer_Label(LIBMATTI_MC_ChunkSectionLayer layer);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_CHUNK_CHUNKSECTIONLAYER_H
