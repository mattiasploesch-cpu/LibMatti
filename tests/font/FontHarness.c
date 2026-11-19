




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



            int wM = LIBMATTI_FML_SimpleFont_StringWidth(font, "M");
            CHECK(wM > 0);
            CHECK(LIBMATTI_FML_SimpleFont_StringWidth(font, "MM") == wM * 2);
            CHECK(LIBMATTI_FML_SimpleFont_StringWidth(font, "M\nM") == wM * 2);




            float width = 0, height = 0;
            LIBMATTI_FML_SimpleFont_MeasureText(font, "M", &width, &height);
            CHECK(width == (float) wM);
            CHECK(height > 0);



            CHECK(LIBMATTI_FML_SimpleFont_StringWidth(font, "\x01") == 0);




            LIBMATTI_FML_SimpleFont_DisplayText text = {"Matticraft", 0xFFFFFFFFu};
            size_t bytes = LIBMATTI_FML_SimpleFont_DrawTexts(font, 10.0f, 10.0f, &text, 1);
            CHECK(bytes > 0);


            LIBMATTI_FML_SimpleFont_Free(font);
        }
        free(ttf);
    }

    printf("font harness: %d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
