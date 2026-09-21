// Port of net.minecraft.client.renderer.texture.Stitcher - the region tree
// bin-packer the SpriteLoader uses to lay the atlas out. 1:1 with the Java
// original (HOLDER_COMPARATOR sort, expand growth, recursive Region.add).

#ifndef MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_STITCHER_H
#define MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_STITCHER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Java: interface Stitcher.Entry { int width(); int height(); Identifier name(); }
typedef struct LIBMATTI_MC_Stitcher_Entry
{
    int width;
    int height;
    const char *name; // borrowed
} LIBMATTI_MC_Stitcher_Entry;

// Java: interface SpriteLoader<T> { void load(T entry, int originX, int originY, int padding); }
typedef void (*LIBMATTI_MC_Stitcher_SpriteLoader)(void *userData, const LIBMATTI_MC_Stitcher_Entry *entry,
                                                  int originX, int originY, int padding);

// Java: public class Stitcher<T extends Stitcher.Entry>
typedef struct LIBMATTI_MC_Stitcher LIBMATTI_MC_Stitcher;

// Java: public Stitcher(int maxWidth, int maxHeight, int mipLevel, int anisotropy)
LIBMATTI_MC_Stitcher *LIBMATTI_MC_Stitcher_New(int maxWidth, int maxHeight, int mipLevel, int anisotropy);
void LIBMATTI_MC_Stitcher_Free(LIBMATTI_MC_Stitcher *stitcher);

// Java: public void registerSprite(T entry)
void LIBMATTI_MC_Stitcher_RegisterSprite(LIBMATTI_MC_Stitcher *stitcher, const LIBMATTI_MC_Stitcher_Entry *entry);
// Java: public void stitch() - returns 0 and leaves the message when the
// sprites do not fit (Java throws StitcherException)
int LIBMATTI_MC_Stitcher_Stitch(LIBMATTI_MC_Stitcher *stitcher);
// Java: public int getWidth() / getHeight()
int LIBMATTI_MC_Stitcher_GetWidth(const LIBMATTI_MC_Stitcher *stitcher);
int LIBMATTI_MC_Stitcher_GetHeight(const LIBMATTI_MC_Stitcher *stitcher);
// Java: public void gatherSprites(SpriteLoader)
void LIBMATTI_MC_Stitcher_GatherSprites(const LIBMATTI_MC_Stitcher *stitcher, void *userData,
                                        LIBMATTI_MC_Stitcher_SpriteLoader loader);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CLIENT_RENDERER_TEXTURE_STITCHER_H
