// Port of com.electronwill.nightconfig.toml.Toml.

#ifndef MATTICRAFT_NC_TOML_TOML_H
#define MATTICRAFT_NC_TOML_TOML_H

#include "libmatti/com/electronwill/nightconfig/core/io/CharacterInput.h"
#include "libmatti/com/electronwill/nightconfig/toml/TomlParser.h"

#include <stddef.h>

// Java: private static final char[] WHITESPACE_OR_NEWLINE = {'\t', ' ', '\n', '\r'}
#define LIBMATTI_NC_TOML_WHITESPACE_OR_NEWLINE "\t \n\r"
// Java: private static final char[] WHITESPACE = {'\t', ' '}
#define LIBMATTI_NC_TOML_WHITESPACE "\t "
// Java: private static final char[] FORBIDDEN_IN_ALL_BARE_KEYS = {'.', '[', ']', '#', '='}
#define LIBMATTI_NC_TOML_FORBIDDEN_IN_ALL_BARE_KEYS ".[]#="

// Java: static int readUseful(CharacterInput input, List<CharsWrapper> commentsList)
int LIBMATTI_NC_Toml_ReadUseful(LIBMATTI_NC_ReaderInput *input, LIBMATTI_NC_TomlParser *parser);
// Java: static char readUsefulChar(CharacterInput input)
int LIBMATTI_NC_Toml_ReadUsefulChar(LIBMATTI_NC_ReaderInput *input);
// Java: static char readNonSpaceChar(CharacterInput input, boolean skipNewlines)
int LIBMATTI_NC_Toml_ReadNonSpaceChar(LIBMATTI_NC_ReaderInput *input, int skipNewlines);
// Java: static int readNonSpace(CharacterInput input, boolean skipNewlines)
int LIBMATTI_NC_Toml_ReadNonSpace(LIBMATTI_NC_ReaderInput *input, int skipNewlines);
// Java: static CharsWrapper readLine(CharacterInput input)
LIBMATTI_NC_CharsWrapper LIBMATTI_NC_Toml_ReadLine(LIBMATTI_NC_ReaderInput *input);

// Java: static boolean isControlChar(int c)
int LIBMATTI_NC_Toml_IsControlChar(int c);
// Java: static boolean isValidCodePoint(int c)
int LIBMATTI_NC_Toml_IsValidCodePoint(int c);
// Java: static boolean isValidInBareKey(char c, boolean lenient)
int LIBMATTI_NC_Toml_IsValidInBareKey(char c, int lenient);
// Java: static boolean isValidBareKey(CharSequence csq, boolean lenient)
int LIBMATTI_NC_Toml_IsValidBareKey(const LIBMATTI_NC_CharsWrapper *wrapper, int lenient);
// Java: static boolean isKeyValueSeparator(char c, boolean lenient)
int LIBMATTI_NC_Toml_IsKeyValueSeparator(char c, int lenient);

#endif //MATTICRAFT_NC_TOML_TOML_H
