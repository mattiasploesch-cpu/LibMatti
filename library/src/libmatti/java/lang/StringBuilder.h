#ifndef MATTICRAFT_JAVA_LANG_STRINGBUILDER_H
#define MATTICRAFT_JAVA_LANG_STRINGBUILDER_H

#include <stddef.h>

typedef struct
{
    char *value;
    size_t length;
    size_t capacity;
} LIBMATTI_JL_StringBuilder;

LIBMATTI_JL_StringBuilder *LIBMATTI_JL_StringBuilder_New(void);
LIBMATTI_JL_StringBuilder *LIBMATTI_JL_StringBuilder_NewFromString(const char *value);
void LIBMATTI_JL_StringBuilder_Free(LIBMATTI_JL_StringBuilder *builder);

LIBMATTI_JL_StringBuilder *LIBMATTI_JL_StringBuilder_Append(LIBMATTI_JL_StringBuilder *builder, const char *value);
LIBMATTI_JL_StringBuilder *LIBMATTI_JL_StringBuilder_AppendChar(LIBMATTI_JL_StringBuilder *builder, char value);

size_t LIBMATTI_JL_StringBuilder_Length(const LIBMATTI_JL_StringBuilder *builder);
char LIBMATTI_JL_StringBuilder_CharAt(const LIBMATTI_JL_StringBuilder *builder, size_t index);
const char *LIBMATTI_JL_StringBuilder_ToString(const LIBMATTI_JL_StringBuilder *builder);

#endif //MATTICRAFT_JAVA_LANG_STRINGBUILDER_H
