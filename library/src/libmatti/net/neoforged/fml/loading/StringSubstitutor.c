#include "libmatti/net/neoforged/fml/loading/StringSubstitutor.h"

#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"

#include <stdlib.h>
#include <string.h>

// Java: private static StrLookup<String> getStringLookup(ModFile file)
static char *lookup(const char *key, LIBMATTI_FML_ModFile *file)
{
    // Java: String[] parts = key.split("\\."); if (parts.length == 1) return key;
    const char *dot = strchr(key, '.');
    if (dot == NULL)
        return strdup(key);

    // Java: String pfx = parts[0];
    size_t pfxLength = (size_t) (dot - key);
    if (pfxLength == 4 && strncmp(key, "file", 4) == 0 && file != NULL)
    {
        // Java: return String.valueOf(file.getSubstitutionMap().get().get(parts[1]));
        char *name = strndup(key + 5, strcspn(key + 5, "."));
        const char *value = LIBMATTI_FML_ModFile_GetSubstitutionValue(file, name);
        free(name);
        // Java: String.valueOf(null) is the literal "null"
        return strdup(value != NULL ? value : "null");
    }

    return strdup(key);
}

// Java: public static String replace(String in, ModFile file)
char *LIBMATTI_FML_StringSubstitutor_Replace(const char *in, LIBMATTI_FML_ModFile *file)
{
    size_t length = strlen(in);
    char *result = malloc(length + 1);
    size_t written = 0;

    for (size_t i = 0; i < length;)
    {
        if (in[i] == '$' && i + 1 < length && in[i + 1] == '{')
        {
            const char *end = strchr(in + i + 2, '}');
            if (end != NULL)
            {
                char *key = strndup(in + i + 2, (size_t) (end - in - i - 2));
                char *value = lookup(key, file);
                free(key);
                result = realloc(result, written + strlen(value) + (length - i) + 1);
                strcpy(result + written, value);
                written += strlen(value);
                free(value);
                i = (size_t) (end - in) + 1;
                continue;
            }
        }

        result[written++] = in[i++];
    }

    result[written] = '\0';
    return result;
}
