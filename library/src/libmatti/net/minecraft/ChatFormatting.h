// Port of net.minecraft.ChatFormatting.

#ifndef MATTICRAFT_MC_CHATFORMATTING_H
#define MATTICRAFT_MC_CHATFORMATTING_H

#ifdef __cplusplus
extern "C" {
#endif

// Java: public enum ChatFormatting - id is the enum ordinal, the int value the
// colour (or -1 for RESET, which has no colour in Java either)
typedef enum LIBMATTI_MC_ChatFormatting
{
    LIBMATTI_MC_ChatFormatting_BLACK = 0,
    LIBMATTI_MC_ChatFormatting_DARK_BLUE = 1,
    LIBMATTI_MC_ChatFormatting_DARK_GREEN = 2,
    LIBMATTI_MC_ChatFormatting_DARK_AQUA = 3,
    LIBMATTI_MC_ChatFormatting_DARK_RED = 4,
    LIBMATTI_MC_ChatFormatting_DARK_PURPLE = 5,
    LIBMATTI_MC_ChatFormatting_GOLD = 6,
    LIBMATTI_MC_ChatFormatting_GRAY = 7,
    LIBMATTI_MC_ChatFormatting_DARK_GRAY = 8,
    LIBMATTI_MC_ChatFormatting_BLUE = 9,
    LIBMATTI_MC_ChatFormatting_GREEN = 10,
    LIBMATTI_MC_ChatFormatting_AQUA = 11,
    LIBMATTI_MC_ChatFormatting_RED = 12,
    LIBMATTI_MC_ChatFormatting_LIGHT_PURPLE = 13,
    LIBMATTI_MC_ChatFormatting_YELLOW = 14,
    LIBMATTI_MC_ChatFormatting_WHITE = 15,
    LIBMATTI_MC_ChatFormatting_OBFUSCATED = 16,
    LIBMATTI_MC_ChatFormatting_BOLD = 17,
    LIBMATTI_MC_ChatFormatting_STRIKETHROUGH = 18,
    LIBMATTI_MC_ChatFormatting_UNDERLINE = 19,
    LIBMATTI_MC_ChatFormatting_ITALIC = 20,
    LIBMATTI_MC_ChatFormatting_RESET = 21
} LIBMATTI_MC_ChatFormatting;

// Java: public String getName() - the enum name
const char *LIBMATTI_MC_ChatFormatting_GetName(LIBMATTI_MC_ChatFormatting formatting);
// Java: public char getCode() - the formatting code after §
char LIBMATTI_MC_ChatFormatting_GetCode(LIBMATTI_MC_ChatFormatting formatting);
// Java: public Integer getColor() - the RGB colour, -1 when absent
int LIBMATTI_MC_ChatFormatting_GetColor(LIBMATTI_MC_ChatFormatting formatting);
// Java: public boolean isColor()
int LIBMATTI_MC_ChatFormatting_IsColor(LIBMATTI_MC_ChatFormatting formatting);
// Java: public boolean isFormat()
int LIBMATTI_MC_ChatFormatting_IsFormat(LIBMATTI_MC_ChatFormatting formatting);
// Java: public String toString() - § + code
const char *LIBMATTI_MC_ChatFormatting_ToString(LIBMATTI_MC_ChatFormatting formatting);
// Java: public static ChatFormatting getByName(String cleanName) - NULL when unknown
int LIBMATTI_MC_ChatFormatting_GetByName(const char *name, LIBMATTI_MC_ChatFormatting *out);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_CHATFORMATTING_H
