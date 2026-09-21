#include "libmatti/java/awt/image/BaseMultiResolutionImage.h"

#include <stdlib.h>

// Java: public BaseMultiResolutionImage(Image... resolutionVariants)
LIBMATTI_JAWT_IM_BaseMultiResolutionImage *LIBMATTI_JAWT_IM_BaseMultiResolutionImage_New(
    LIBMATTI_JAWT_Image **resolutionVariants, size_t count)
{
    LIBMATTI_JAWT_IM_BaseMultiResolutionImage *image = calloc(1, sizeof(LIBMATTI_JAWT_IM_BaseMultiResolutionImage));
    image->resolutionVariants = resolutionVariants;
    image->resolutionVariantCount = count;

    // Java: getWidth(observer) / getHeight(observer) delegate to the base image
    // (a variant that could not be read stays null, as ImageIO.read returns null)
    for (size_t i = 0; i < count; i++)
    {
        if (resolutionVariants[i] == NULL) continue;
        image->base.width = resolutionVariants[i]->width;
        image->base.height = resolutionVariants[i]->height;
        break;
    }
    return image;
}

void LIBMATTI_JAWT_IM_BaseMultiResolutionImage_Free(LIBMATTI_JAWT_IM_BaseMultiResolutionImage *image)
{
    if (image == NULL) return;

    for (size_t i = 0; i < image->resolutionVariantCount; i++)
        LIBMATTI_JAWT_Image_Free(image->resolutionVariants[i]);
    free(image->resolutionVariants);
    free(image);
}
