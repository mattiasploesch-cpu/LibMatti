// Implementation of com.mojang.blaze3d.platform.TextureUtil: the solidify
// breadth-first flood fill and the dark-colour fill for the mipmaps of
// cutout textures.

#include "libmatti/com/mojang/blaze3d/platform/TextureUtil.h"

#include "libmatti/net/minecraft/util/ARGB.h"

#include <limits.h>
#include <stdlib.h>

// Java: the pack/x/y helpers over the image width
#define PACK(x, y, width) ((x) + (y) * (width))
#define UNPACK_X(index, width) ((index) % (width))
#define UNPACK_Y(index, width) ((index) / (width))

// Java: private static final int[] DIRECTIONS - the 4 neighbours
static const int DIRECTIONS[4][2] = {
    {1, 0}, {-1, 0}, {0, 1}, {0, -1},
};

// Java: public static void solidify(NativeImage)
void LIBMATTI_B3D_TextureUtil_Solidify(LIBMATTI_B3D_NativeImage *image)
{
    int width = image->width;
    int height = image->height;
    int *dist = malloc((size_t) width * (size_t) height * sizeof(int));
    int *colour = malloc((size_t) width * (size_t) height * sizeof(int));
    int *queue = malloc((size_t) width * (size_t) height * sizeof(int));
    int head = 0;
    int tail = 0;

    // Java: Arrays.fill(aint1, Integer.MAX_VALUE)
    for (int i = 0; i < width * height; i++)
    {
        dist[i] = INT_MAX;
        colour[i] = 0;
    }

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            int pixel = LIBMATTI_B3D_NativeImage_GetPixel(image, x, y);
            if (LIBMATTI_MC_ARGB_ALPHA(pixel) != 0)
            {
                int index = PACK(x, y, width);
                dist[index] = 0;
                colour[index] = pixel;
                queue[tail++] = index;
            }
        }
    }

    // Java: the IntArrayFIFOQueue BFS
    while (head < tail)
    {
        int index = queue[head++];
        int x = UNPACK_X(index, width);
        int y = UNPACK_Y(index, width);

        for (int d = 0; d < 4; d++)
        {
            int nx = x + DIRECTIONS[d][0];
            int ny = y + DIRECTIONS[d][1];
            if (nx >= 0 && ny >= 0 && nx < width && ny < height)
            {
                int next = PACK(nx, ny, width);
                if (dist[next] > dist[index] + 1)
                {
                    dist[next] = dist[index] + 1;
                    colour[next] = colour[index];
                    queue[tail++] = next;
                }
            }
        }
    }

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            int pixel = LIBMATTI_B3D_NativeImage_GetPixel(image, x, y);
            if (LIBMATTI_MC_ARGB_ALPHA(pixel) == 0)
            {
                // Java: ARGB.color(0, aint[pack(x, y, i)]) - keep alpha 0
                LIBMATTI_B3D_NativeImage_SetPixel(image, x, y,
                                                  LIBMATTI_MC_ARGB_ColorAlpha(0, colour[PACK(x, y, width)] & 0xFFFFFF));
            }
        }
    }

    free(dist);
    free(colour);
    free(queue);
}

// Java: public static void fillEmptyAreasWithDarkColor(NativeImage)
void LIBMATTI_B3D_TextureUtil_FillEmptyAreasWithDarkColor(LIBMATTI_B3D_NativeImage *image)
{
    int width = image->width;
    int height = image->height;
    int darkest = -1;
    int darkestSum = INT_MAX;

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            int pixel = LIBMATTI_B3D_NativeImage_GetPixel(image, x, y);
            if (LIBMATTI_MC_ARGB_ALPHA(pixel) != 0)
            {
                int sum = LIBMATTI_MC_ARGB_RED(pixel) + LIBMATTI_MC_ARGB_GREEN(pixel) + LIBMATTI_MC_ARGB_BLUE(pixel);
                if (sum < darkestSum)
                {
                    darkestSum = sum;
                    darkest = pixel;
                }
            }
        }
    }

    // Java: 3 * red / 4 etc - the darkened copy
    int red = 3 * LIBMATTI_MC_ARGB_RED(darkest) / 4;
    int green = 3 * LIBMATTI_MC_ARGB_GREEN(darkest) / 4;
    int blue = 3 * LIBMATTI_MC_ARGB_BLUE(darkest) / 4;
    int dark = LIBMATTI_MC_ARGB_Color(0, red, green, blue);

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            if (LIBMATTI_MC_ARGB_ALPHA(LIBMATTI_B3D_NativeImage_GetPixel(image, x, y)) == 0)
                LIBMATTI_B3D_NativeImage_SetPixel(image, x, y, dark);
        }
    }
}
