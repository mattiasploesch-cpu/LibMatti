#ifndef MATTICRAFT_JAVAX_SWING_IMAGEICON_H
#define MATTICRAFT_JAVAX_SWING_IMAGEICON_H

#include "libmatti/java/awt/Image.h"

// Java: public class ImageIcon implements Icon
typedef struct
{
    // Java: private Image image
    LIBMATTI_JAWT_Image *image;
} LIBMATTI_JS_ImageIcon;

// Java: public ImageIcon(Image image)
LIBMATTI_JS_ImageIcon *LIBMATTI_JS_ImageIcon_New(LIBMATTI_JAWT_Image *image);
void LIBMATTI_JS_ImageIcon_Free(LIBMATTI_JS_ImageIcon *icon);

#endif //MATTICRAFT_JAVAX_SWING_IMAGEICON_H
