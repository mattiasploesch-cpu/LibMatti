#ifndef MATTICRAFT_JAVA_AWT_IMAGE_BASEMULTIRESOLUTIONIMAGE_H
#define MATTICRAFT_JAVA_AWT_IMAGE_BASEMULTIRESOLUTIONIMAGE_H

#include "libmatti/java/awt/Image.h"

#include <stddef.h>

// Java: public class BaseMultiResolutionImage extends AbstractMultiResolutionImage
typedef struct
{
    LIBMATTI_JAWT_Image base;
    LIBMATTI_JAWT_Image **resolutionVariants;
    size_t resolutionVariantCount;
} LIBMATTI_JAWT_IM_BaseMultiResolutionImage;

// Java: public BaseMultiResolutionImage(Image... resolutionVariants)
LIBMATTI_JAWT_IM_BaseMultiResolutionImage *LIBMATTI_JAWT_IM_BaseMultiResolutionImage_New(
    LIBMATTI_JAWT_Image **resolutionVariants, size_t count);
void LIBMATTI_JAWT_IM_BaseMultiResolutionImage_Free(LIBMATTI_JAWT_IM_BaseMultiResolutionImage *image);

#endif //MATTICRAFT_JAVA_AWT_IMAGE_BASEMULTIRESOLUTIONIMAGE_H
