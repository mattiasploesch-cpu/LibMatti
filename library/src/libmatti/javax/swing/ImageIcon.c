#include "libmatti/javax/swing/ImageIcon.h"

#include <stdlib.h>

// Java: public ImageIcon(Image image)
LIBMATTI_JS_ImageIcon *LIBMATTI_JS_ImageIcon_New(LIBMATTI_JAWT_Image *image)
{
    LIBMATTI_JS_ImageIcon *icon = calloc(1, sizeof(LIBMATTI_JS_ImageIcon));
    icon->image = image;
    return icon;
}

void LIBMATTI_JS_ImageIcon_Free(LIBMATTI_JS_ImageIcon *icon)
{
    free(icon);
}
