// Port of net.minecraft.ChatFormatting.

#include "libmatti/net/minecraft/ChatFormatting.h"

#include <string.h>

typedef struct ChatFormattingData
{
    const char *name;
    char code;
    int color;
} ChatFormattingData;

// Java: private final String name, char code, @Nullable Integer color - the
// table is indexed by the enum ordinal; colours 0-15, formats without a colour
// (Java stores null, the port -1) and RESET last.
static const ChatFormattingData DATA[] = {
    {"BLACK", '0', 0},
    {"DARK_BLUE", '1', 1},
    {"DARK_GREEN", '2', 2},
    {"DARK_AQUA", '3', 3},
    {"DARK_RED", '4', 4},
    {"DARK_PURPLE", '5', 5},
    {"GOLD", '6', 6},
    {"GRAY", '7', 7},
    {"DARK_GRAY", '8', 8},
    {"BLUE", '9', 9},
    {"GREEN", 'a', 10},
    {"AQUA", 'b', 11},
    {"RED", 'c', 12},
    {"LIGHT_PURPLE", 'd', 13},
    {"YELLOW", 'e', 14},
    {"WHITE", 'f', 15},
    {"OBFUSCATED", 'k', -1},
    {"BOLD", 'l', -1},
    {"STRIKETHROUGH", 'm', -1},
    {"UNDERLINE", 'n', -1},
    {"ITALIC", 'o', -1},
    {"RESET", 'r', -1},
};

#define COUNT (sizeof(DATA) / sizeof(DATA[0]))

const char *LIBMATTI_MC_ChatFormatting_GetName(LIBMATTI_MC_ChatFormatting formatting)
{
    return DATA[formatting].name;
}

char LIBMATTI_MC_ChatFormatting_GetCode(LIBMATTI_MC_ChatFormatting formatting)
{
    return DATA[formatting].code;
}

int LIBMATTI_MC_ChatFormatting_GetColor(LIBMATTI_MC_ChatFormatting formatting)
{
    return DATA[formatting].color;
}

int LIBMATTI_MC_ChatFormatting_IsColor(LIBMATTI_MC_ChatFormatting formatting)
{
    return formatting <= LIBMATTI_MC_ChatFormatting_WHITE;
}

int LIBMATTI_MC_ChatFormatting_IsFormat(LIBMATTI_MC_ChatFormatting formatting)
{
    return formatting >= LIBMATTI_MC_ChatFormatting_OBFUSCATED;
}

const char *LIBMATTI_MC_ChatFormatting_ToString(LIBMATTI_MC_ChatFormatting formatting)
{
    // Java: "\u00a7" + code - one static buffer per value keeps the pointers stable
    static char strings[COUNT][3];
    static int ready;
    if (!ready)
    {
        for (size_t i = 0; i < COUNT; i++)
        {
            strings[i][0] = '\xa7';
            strings[i][1] = DATA[i].code;
            strings[i][2] = '\0';
        }
        ready = 1;
    }
    return strings[formatting];
}

int LIBMATTI_MC_ChatFormatting_GetByName(const char *name, LIBMATTI_MC_ChatFormatting *out)
{
    for (size_t i = 0; i < COUNT; i++)
    {
        if (strcmp(DATA[i].name, name) == 0)
        {
            *out = (LIBMATTI_MC_ChatFormatting) i;
            return 0;
        }
    }
    return -1;
}
