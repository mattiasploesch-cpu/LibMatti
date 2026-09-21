// Port of com.mojang.brigadier.StringReader (and ImmutableStringReader).
// Java throws CommandSyntaxException; the C port returns the exception through
// an out-parameter and reads through it on the next call.

#ifndef MATTICRAFT_BR_STRINGREADER_H
#define MATTICRAFT_BR_STRINGREADER_H

#include <stdint.h>

typedef struct LIBMATTI_BR_CommandSyntaxException LIBMATTI_BR_CommandSyntaxException;

// Java: private static final char constants
#define LIBMATTI_BR_StringReader_SYNTAX_ESCAPE '\\'
#define LIBMATTI_BR_StringReader_SYNTAX_DOUBLE_QUOTE '"'
#define LIBMATTI_BR_StringReader_SYNTAX_SINGLE_QUOTE '\''

// Java: public class StringReader implements ImmutableStringReader
typedef struct LIBMATTI_BR_StringReader
{
    // Java: private final String string (borrowed, not owned)
    const char *string;
    int cursor;
} LIBMATTI_BR_StringReader;

// Java: public StringReader(final String string) - the string is borrowed
LIBMATTI_BR_StringReader LIBMATTI_BR_StringReader_Of(const char *string);
// Java: public StringReader(final StringReader other)
LIBMATTI_BR_StringReader LIBMATTI_BR_StringReader_Copy(const LIBMATTI_BR_StringReader *other);

// Java: public String getString()
const char *LIBMATTI_BR_StringReader_GetString(const LIBMATTI_BR_StringReader *reader);
// Java: public void setCursor(final int cursor)
void LIBMATTI_BR_StringReader_SetCursor(LIBMATTI_BR_StringReader *reader, int cursor);
// Java: public int getRemainingLength()
int LIBMATTI_BR_StringReader_GetRemainingLength(const LIBMATTI_BR_StringReader *reader);
// Java: public int getTotalLength()
int LIBMATTI_BR_StringReader_GetTotalLength(const LIBMATTI_BR_StringReader *reader);
// Java: public int getCursor()
int LIBMATTI_BR_StringReader_GetCursor(const LIBMATTI_BR_StringReader *reader);
// Java: public String getRead() - the caller frees
char *LIBMATTI_BR_StringReader_GetRead(const LIBMATTI_BR_StringReader *reader);
// Java: public String getRemaining() - the caller frees
char *LIBMATTI_BR_StringReader_GetRemaining(const LIBMATTI_BR_StringReader *reader);
// Java: public boolean canRead(final int length)
int LIBMATTI_BR_StringReader_CanReadLength(const LIBMATTI_BR_StringReader *reader, int length);
// Java: public boolean canRead()
int LIBMATTI_BR_StringReader_CanRead(const LIBMATTI_BR_StringReader *reader);
// Java: public char peek()
char LIBMATTI_BR_StringReader_Peek(const LIBMATTI_BR_StringReader *reader);
// Java: public char peek(final int offset)
char LIBMATTI_BR_StringReader_PeekOffset(const LIBMATTI_BR_StringReader *reader, int offset);
// Java: public char read()
char LIBMATTI_BR_StringReader_Read(LIBMATTI_BR_StringReader *reader);
// Java: public void skip()
void LIBMATTI_BR_StringReader_Skip(LIBMATTI_BR_StringReader *reader);

// Java: public static boolean isAllowedNumber(final char c)
int LIBMATTI_BR_StringReader_IsAllowedNumber(char c);
// Java: public static boolean isQuotedStringStart(char c)
int LIBMATTI_BR_StringReader_IsQuotedStringStart(char c);
// Java: public void skipWhitespace()
void LIBMATTI_BR_StringReader_SkipWhitespace(LIBMATTI_BR_StringReader *reader);

// Java: public int readInt() throws - sets *outException on failure
int LIBMATTI_BR_StringReader_ReadInt(LIBMATTI_BR_StringReader *reader,
                                     LIBMATTI_BR_CommandSyntaxException **outException);
// Java: public long readLong() throws
long long LIBMATTI_BR_StringReader_ReadLong(LIBMATTI_BR_StringReader *reader,
                                            LIBMATTI_BR_CommandSyntaxException **outException);
// Java: public double readDouble() throws
double LIBMATTI_BR_StringReader_ReadDouble(LIBMATTI_BR_StringReader *reader,
                                           LIBMATTI_BR_CommandSyntaxException **outException);
// Java: public float readFloat() throws
float LIBMATTI_BR_StringReader_ReadFloat(LIBMATTI_BR_StringReader *reader,
                                         LIBMATTI_BR_CommandSyntaxException **outException);

// Java: public static boolean isAllowedInUnquotedString(final char c)
int LIBMATTI_BR_StringReader_IsAllowedInUnquotedString(char c);
// Java: public String readUnquotedString() - the caller frees
char *LIBMATTI_BR_StringReader_ReadUnquotedString(LIBMATTI_BR_StringReader *reader);
// Java: public String readQuotedString() throws - the caller frees
char *LIBMATTI_BR_StringReader_ReadQuotedString(LIBMATTI_BR_StringReader *reader,
                                                LIBMATTI_BR_CommandSyntaxException **outException);
// Java: public String readStringUntil(char terminator) throws - the caller frees
char *LIBMATTI_BR_StringReader_ReadStringUntil(LIBMATTI_BR_StringReader *reader, char terminator,
                                               LIBMATTI_BR_CommandSyntaxException **outException);
// Java: public String readString() throws - the caller frees
char *LIBMATTI_BR_StringReader_ReadString(LIBMATTI_BR_StringReader *reader,
                                          LIBMATTI_BR_CommandSyntaxException **outException);
// Java: public boolean readBoolean() throws
int LIBMATTI_BR_StringReader_ReadBoolean(LIBMATTI_BR_StringReader *reader,
                                         LIBMATTI_BR_CommandSyntaxException **outException);
// Java: public void expect(final char c) throws
void LIBMATTI_BR_StringReader_Expect(LIBMATTI_BR_StringReader *reader, char c,
                                     LIBMATTI_BR_CommandSyntaxException **outException);

#endif
