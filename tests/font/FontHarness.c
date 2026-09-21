// Font harness: drives the SimpleFont pack path over the real Monocraft.ttf
// (no GL context - texture and draw calls degrade to the binding's fallbacks)
// and checks the glyph metrics the measure/stringWidth paths report.

#include "libmatti/net/neoforged/fml/earlydisplay/SimpleFont.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks = 0;
static int failures = 0;
#define CHECK(cond)                                                              \
    do                                                                           \
    {                                                                            \
        checks++;                                                                \
        if (!(cond))                                                             \
        {                                                                        \
            failures++;                                                          \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);               \
        }                                                                        \
    } while (0)

static char *load_monocraft(long *outLength)
{
    // The harness runs with the tests/ directory as the working directory
    // (see add_test WORKING_DIRECTORY); the TTF is the source-tree resource.
    const char *path = "../vendor/FancyModLoader/earlydisplay/src/main/resources/"
                       "net/neoforged/fml/earlydisplay/theme/Monocraft.ttf";
    FILE *file = fopen(path, "rb");
    if (file == NULL)
        return NULL;
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *data = malloc((size_t) length);
    if (fread(data, 1, (size_t) length, file) != (size_t) length)
    {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *outLength = length;
    return data;
}

int main(void)
{
    long length = 0;
    char *ttf = load_monocraft(&length);
    CHECK(ttf != NULL);

    if (ttf != NULL)
    {
        // Java: new SimpleFont(themeResource, ...) - v-metrics at size 24, the
        // ASCII 32..126 pack, the GL texture (skipped without a context).
        LIBMATTI_FML_SimpleFont *font = LIBMATTI_FML_SimpleFont_New((const unsigned char *) ttf, (size_t) length);
        CHECK(font != NULL);

        if (font != NULL)
        {
            // Java: stbtt_GetScaledFontVMetrics at 24 -> lineSpacing > 0, descent <= 0
            CHECK(LIBMATTI_FML_SimpleFont_LineSpacing(font) > 0);
            CHECK(LIBMATTI_FML_SimpleFont_Descent(font) <= 0);

            // stringWidth = the sum of the glyph advances
            int wM = LIBMATTI_FML_SimpleFont_StringWidth(font, "M");
            CHECK(wM > 0);
            CHECK(LIBMATTI_FML_SimpleFont_StringWidth(font, "MM") == wM * 2);
            CHECK(LIBMATTI_FML_SimpleFont_StringWidth(font, "M\nM") == wM * 2); // \n counts 0

            // measureText: one line with content adds the line spacing
            float width = 0, height = 0;
            LIBMATTI_FML_SimpleFont_MeasureText(font, "M", &width, &height);
            CHECK(width == (float) wM);
            CHECK(height > 0);

            // out-of-range codepoints report 0 advance (skip missing)
            CHECK(LIBMATTI_FML_SimpleFont_StringWidth(font, "\x01") == 0);

            // draw without a GL context: the binding degrades to no-ops and the
            // batcher still returns the vertex bytes it packed
            LIBMATTI_FML_SimpleFont_DisplayText text = {"Matticraft", 0xFFFFFFFFu};
            size_t bytes = LIBMATTI_FML_SimpleFont_DrawTexts(font, 10.0f, 10.0f, &text, 1);
            CHECK(bytes > 0); // 10 glyphs * 4 vertices * 20 bytes

            LIBMATTI_FML_SimpleFont_Free(font);
        }
        free(ttf);
    }

    printf("font harness: %d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
