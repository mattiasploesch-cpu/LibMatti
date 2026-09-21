#include "PlainTextRenderer.h"

static void plain_render(LIBMATTI_L4J_TextRenderer *self, const char *input, LIBMATTI_JL_StringBuilder *output,
                         const char *styleName)
{
    (void)self;
    (void)styleName;
    LIBMATTI_JL_StringBuilder_Append(output, input);
}

static void plain_render_buffer(LIBMATTI_L4J_TextRenderer *self, LIBMATTI_JL_StringBuilder *input,
                                LIBMATTI_JL_StringBuilder *output)
{
    (void)self;
    LIBMATTI_JL_StringBuilder_Append(output, LIBMATTI_JL_StringBuilder_ToString(input));
}

static LIBMATTI_L4J_TextRenderer plainInstance = {plain_render, plain_render_buffer};

LIBMATTI_L4J_TextRenderer *LIBMATTI_L4J_PlainTextRenderer_GetInstance(void)
{
    return &plainInstance;
}
