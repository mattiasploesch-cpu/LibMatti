// Port of org.apache.commons.lang3.text.StrSubstitutor / ExtendedMessageFormat.

#include "libmatti/org/apache/commons/lang3/text/StrSubstitutor.h"

#include "libmatti/java/text/Text.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: StrSubstitutor.replace - "${name}" runs resolve through the lookup
char *LIBMATTI_CL3_StrSubstitutor_Replace(const char *source, LIBMATTI_CL3_StrLookup lookup, void *self)
{
    size_t capacity = strlen(source) + 64;
    char *out = malloc(capacity);
    size_t outIndex = 0;

    for (size_t i = 0; source[i] != '\0';)
    {
        // Java: the "${" prefix match
        if (source[i] == '$' && source[i + 1] == '{')
        {
            const char *end = strchr(source + i + 2, '}');
            if (end != NULL)
            {
                size_t nameLength = (size_t) (end - (source + i + 2));
                char *name = strndup(source + i + 2, nameLength);
                char *value = lookup != NULL ? lookup(name, self) : NULL;
                free(name);

                if (value != NULL)
                {
                    size_t valueLength = strlen(value);
                    if (outIndex + valueLength + 1 > capacity)
                    {
                        capacity = (outIndex + valueLength + 1) * 2;
                        out = realloc(out, capacity);
                    }
                    memcpy(out + outIndex, value, valueLength);
                    outIndex += valueLength;
                    free(value);
                }
                else
                {
                    // Java: an unresolved variable stays as-is
                    if (outIndex + nameLength + 4 > capacity)
                    {
                        capacity = (outIndex + nameLength + 4) * 2;
                        out = realloc(out, capacity);
                    }
                    outIndex += (size_t) sprintf(out + outIndex, "${%.*s}", (int) nameLength, source + i + 2);
                }
                i = (size_t) (end - source) + 1;
                continue;
            }
        }
        out[outIndex++] = source[i++];
    }
    out[outIndex] = '\0';
    return out;
}

// Java: ExtendedMessageFormat.format - the port folds into MessageFormat
char *LIBMATTI_CL3_ExtendedMessageFormat_Format(const char *pattern, const char *const *values, size_t count)
{
    return LIBMATTI_JT_MessageFormat_Format(pattern, values, count);
}

char *LIBMATTI_CL3_StringUtils_Join(const char *const *parts, size_t count, char separator)
{
    size_t capacity = 16;
    for (size_t i = 0; i < count; i++)
        capacity += strlen(parts[i]);
    char *out = malloc(capacity);
    size_t outIndex = 0;
    for (size_t i = 0; i < count; i++)
    {
        if (i > 0)
            out[outIndex++] = separator;
        strcpy(out + outIndex, parts[i]);
        outIndex += strlen(parts[i]);
    }
    out[outIndex] = '\0';
    return out;
}
