#ifndef MATTICRAFT_L4J_TEXTRENDERER_H
#define MATTICRAFT_L4J_TEXTRENDERER_H

#include "libmatti/java/lang/StringBuilder.h"

typedef struct LIBMATTI_L4J_TextRenderer
{
    void (*render)(struct LIBMATTI_L4J_TextRenderer *self, const char *input, LIBMATTI_JL_StringBuilder *output,
                   const char *styleName);
    void (*renderBuffer)(struct LIBMATTI_L4J_TextRenderer *self, LIBMATTI_JL_StringBuilder *input,
                         LIBMATTI_JL_StringBuilder *output);
} LIBMATTI_L4J_TextRenderer;

#endif //MATTICRAFT_L4J_TEXTRENDERER_H
