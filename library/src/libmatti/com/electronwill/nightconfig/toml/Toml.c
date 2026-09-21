// Port of com.electronwill.nightconfig.toml.Toml.

#include "libmatti/com/electronwill/nightconfig/toml/Toml.h"

#include <string.h>

static const char NEWLINE[] = "\n";

// Java: static int readUseful(CharacterInput input, List<CharsWrapper> commentsList)
// The port skips the comment lines but does not store them (CommentedConfig comments are not read
// by FML), so the comments list has no counterpart.
int LIBMATTI_NC_Toml_ReadUseful(LIBMATTI_NC_ReaderInput *input, LIBMATTI_NC_TomlParser *parser)
{
    (void) parser;

    int next = LIBMATTI_NC_ReaderInput_ReadAndSkip(input, LIBMATTI_NC_TOML_WHITESPACE_OR_NEWLINE,
                                                   strlen(LIBMATTI_NC_TOML_WHITESPACE_OR_NEWLINE));
    while (next == '#')
    {
        LIBMATTI_NC_Toml_ReadLine(input);
        next = LIBMATTI_NC_ReaderInput_ReadAndSkip(input, LIBMATTI_NC_TOML_WHITESPACE_OR_NEWLINE,
                                                   strlen(LIBMATTI_NC_TOML_WHITESPACE_OR_NEWLINE));
    }
    return next;
}

int LIBMATTI_NC_Toml_ReadUsefulChar(LIBMATTI_NC_ReaderInput *input)
{
    int next = LIBMATTI_NC_ReaderInput_ReadCharAndSkip(input, LIBMATTI_NC_TOML_WHITESPACE_OR_NEWLINE,
                                                       strlen(LIBMATTI_NC_TOML_WHITESPACE_OR_NEWLINE));
    while (next == '#')
    {
        LIBMATTI_NC_Toml_ReadLine(input);
        next = LIBMATTI_NC_ReaderInput_ReadCharAndSkip(input, LIBMATTI_NC_TOML_WHITESPACE_OR_NEWLINE,
                                                       strlen(LIBMATTI_NC_TOML_WHITESPACE_OR_NEWLINE));
    }
    return next;
}

int LIBMATTI_NC_Toml_ReadNonSpaceChar(LIBMATTI_NC_ReaderInput *input, int skipNewlines)
{
    return skipNewlines
               ? LIBMATTI_NC_ReaderInput_ReadCharAndSkip(input, LIBMATTI_NC_TOML_WHITESPACE_OR_NEWLINE,
                                                         strlen(LIBMATTI_NC_TOML_WHITESPACE_OR_NEWLINE))
               : LIBMATTI_NC_ReaderInput_ReadCharAndSkip(input, LIBMATTI_NC_TOML_WHITESPACE,
                                                         strlen(LIBMATTI_NC_TOML_WHITESPACE));
}

int LIBMATTI_NC_Toml_ReadNonSpace(LIBMATTI_NC_ReaderInput *input, int skipNewlines)
{
    return LIBMATTI_NC_Toml_ReadNonSpaceChar(input, skipNewlines);
}

// Java: static CharsWrapper readLine(CharacterInput input)
LIBMATTI_NC_CharsWrapper LIBMATTI_NC_Toml_ReadLine(LIBMATTI_NC_ReaderInput *input)
{
    LIBMATTI_NC_CharsWrapper chars = LIBMATTI_NC_ReaderInput_ReadUntil(input, NEWLINE, 1);
    if (chars.length > 0 && chars.data[chars.length - 1] == '\r')
        return LIBMATTI_NC_CharsWrapper_SubView2(&chars, 0, chars.length - 1);
    return chars;
}

int LIBMATTI_NC_Toml_IsControlChar(int c)
{
    return c <= 0x1F || c == 0x7F;
}

int LIBMATTI_NC_Toml_IsValidCodePoint(int c)
{
    return c <= 0xD7FF || (c >= 0xE000 && c <= 0x10FFFF);
}

int LIBMATTI_NC_Toml_IsValidInBareKey(char c, int lenient)
{
    if (lenient)
        return c > ' ' && strchr(LIBMATTI_NC_TOML_FORBIDDEN_IN_ALL_BARE_KEYS, c) == NULL &&
               !LIBMATTI_NC_Toml_IsControlChar(c);

    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_';
}

int LIBMATTI_NC_Toml_IsValidBareKey(const LIBMATTI_NC_CharsWrapper *wrapper, int lenient)
{
    if (wrapper->length == 0) return 0;
    for (size_t i = 0; i < wrapper->length; i++)
        if (!LIBMATTI_NC_Toml_IsValidInBareKey(wrapper->data[i], lenient)) return 0;
    return 1;
}

int LIBMATTI_NC_Toml_IsKeyValueSeparator(char c, int lenient)
{
    return c == '=' || (lenient && c == ':');
}
