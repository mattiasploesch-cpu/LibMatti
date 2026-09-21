#include "libmatti/net/neoforged/accesstransformer/parser/AtParser.h"

#include "libmatti/java/util/ArrayList.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// Java: parseModifier(String modifier, int line)
static int parse_modifier(const char *modifier, LIBMATTI_AT_Modifier *out)
{
    if (strcmp(modifier, "public") == 0)
    {
        *out = LIBMATTI_AT_Modifier_PUBLIC;
        return 1;
    }
    if (strcmp(modifier, "private") == 0)
    {
        *out = LIBMATTI_AT_Modifier_PRIVATE;
        return 1;
    }
    if (strcmp(modifier, "protected") == 0)
    {
        *out = LIBMATTI_AT_Modifier_PROTECTED;
        return 1;
    }
    if (strcmp(modifier, "default") == 0)
    {
        *out = LIBMATTI_AT_Modifier_DEFAULT;
        return 1;
    }
    return 0;
}

// Java: the class name validation - dot-separated java identifiers
static int is_valid_class_name(const char *name)
{
    int lastWasDot = 1; // start behaves like "after a dot"
    for (const char *c = name; *c != '\0'; c++)
    {
        if (lastWasDot)
        {
            if (!(isalpha(*c) || *c == '_' || *c == '$'))
                return 0;
        }
        else if (*c != '.' && !(isalnum(*c) || *c == '_' || *c == '$'))
            return 0;
        lastWasDot = *c == '.';
    }
    return !lastWasDot;
}

static int is_identifier_start(int c)
{
    return isalpha(c) || c == '_' || c == '$';
}

static int is_identifier_part(int c)
{
    return isalnum(c) || c == '_' || c == '$';
}

// Java: validateMethodDescriptor - "(...)..." with L...; forms
static int is_valid_method_descriptor(const char *desc)
{
    if (desc[0] != '(')
        return 0;
    const char *c = desc + 1;
    while (*c != ')' && *c != '\0')
    {
        if (strchr("BCDFIJSZ", *c) != NULL)
            c++;
        else if (*c == 'L')
        {
            while (*c != ';' && *c != '\0')
                c++;
            if (*c == '\0')
                return 0;
            c++;
        }
        else if (*c == '[')
            c++;
        else
            return 0;
    }
    return *c == ')';
}

LIBMATTI_AT_AtEntry **LIBMATTI_AT_AtParser_Parse(const char *text, const char *originName, size_t *count,
                                                 int *errorLine)
{
    LIBMATTI_JU_ArrayList *entries = LIBMATTI_JU_ArrayList_New();
    int lineNumber = 0;
    *errorLine = 0;

    const char *cursor = text;
    while (*cursor != '\0')
    {
        // Java: LineNumberReader.readLine()
        size_t lineLength = strcspn(cursor, "\n");
        char *line = malloc(lineLength + 1);
        memcpy(line, cursor, lineLength);
        line[lineLength] = '\0';
        cursor += lineLength + (cursor[lineLength] == '\n' ? 1 : 0);
        lineNumber++;

        // Java: strip comments ('#') and split on whitespace runs
        char *parts[8];
        size_t partCount = 0;
        char *builder = malloc(lineLength + 1);
        size_t builderLength = 0;

        for (char *c = line; *c != '\0' && partCount < 8; c++)
        {
            if (isspace((unsigned char) *c))
            {
                if (builderLength != 0)
                {
                    builder[builderLength] = '\0';
                    parts[partCount++] = strdup(builder);
                    builderLength = 0;
                }
            }
            else if (*c == '#')
            {
                break;
            }
            else
            {
                builder[builderLength++] = *c;
            }
        }
        if (builderLength != 0 && partCount < 8)
        {
            builder[builderLength] = '\0';
            parts[partCount++] = strdup(builder);
        }
        free(builder);
        free(line);

        if (partCount == 0)
            continue; // empty or comment-only line

        if (partCount < 2)
        {
            *errorLine = lineNumber;
            return NULL;
        }

        // Java: the -f / +f suffixes on the modifier
        char *modifierString = parts[0];
        LIBMATTI_AT_FinalState finalState = LIBMATTI_AT_FinalState_LEAVE;
        size_t modifierLength = strlen(modifierString);
        if (modifierLength > 2 && strcmp(modifierString + modifierLength - 2, "-f") == 0)
        {
            finalState = LIBMATTI_AT_FinalState_REMOVEFINAL;
            modifierString[modifierLength - 2] = '\0';
        }
        else if (modifierLength > 2 && strcmp(modifierString + modifierLength - 2, "+f") == 0)
        {
            finalState = LIBMATTI_AT_FinalState_MAKEFINAL;
            modifierString[modifierLength - 2] = '\0';
        }

        LIBMATTI_AT_Modifier modifier;
        if (!parse_modifier(modifierString, &modifier))
        {
            *errorLine = lineNumber;
            return NULL;
        }

        char *className = parts[1];
        if (!is_valid_class_name(className))
        {
            *errorLine = lineNumber;
            return NULL;
        }

        LIBMATTI_AT_Transformation *transformation =
            LIBMATTI_AT_Transformation_New(modifier, finalState, originName, lineNumber);

        LIBMATTI_AT_Target *target;
        if (partCount < 3)
        {
            target = LIBMATTI_AT_Target_Class(className);
        }
        else
        {
            char *member = parts[2];
            if (strcmp(member, "*") == 0)
                target = LIBMATTI_AT_Target_WildcardField(className);
            else if (strcmp(member, "*()") == 0)
                target = LIBMATTI_AT_Target_WildcardMethod(className);
            else if (strchr(member, '(') != NULL)
            {
                // Java: name = before '(', desc = from '(' with '.' replaced by '/'
                char *paren = strchr(member, '(');
                size_t nameLength = (size_t) (paren - member);
                char name[256];
                if (nameLength >= sizeof(name))
                    nameLength = sizeof(name) - 1;
                memcpy(name, member, nameLength);
                name[nameLength] = '\0';

                char *desc = strdup(paren);
                for (char *d = desc; *d != '\0'; d++)
                    if (*d == '.')
                        *d = '/';
                if (!is_valid_method_descriptor(desc))
                {
                    *errorLine = lineNumber;
                    return NULL;
                }
                if (strcmp(name, "<init>") != 0)
                {
                    for (const char *n = name; *n != '\0'; n++)
                    {
                        if ((n == name && !is_identifier_start((unsigned char) *n)) ||
                            (n != name && !is_identifier_part((unsigned char) *n)))
                        {
                            *errorLine = lineNumber;
                            return NULL;
                        }
                    }
                }
                target = LIBMATTI_AT_Target_Method(className, name, desc);
                free(desc);
            }
            else
            {
                // Java: validateIdentifier(member, "field", line)
                for (const char *n = member; *n != '\0'; n++)
                {
                    if ((n == member && !is_identifier_start((unsigned char) *n)) ||
                        (n != member && !is_identifier_part((unsigned char) *n)))
                    {
                        *errorLine = lineNumber;
                        return NULL;
                    }
                }
                target = LIBMATTI_AT_Target_Field(className, member);
            }
        }

        LIBMATTI_AT_AtEntry *entry = malloc(sizeof(LIBMATTI_AT_AtEntry));
        entry->target = target;
        entry->transformation = transformation;
        LIBMATTI_JU_ArrayList_Add(entries, entry);

        for (size_t i = 0; i < partCount; i++)
            free(parts[i]);
    }

    LIBMATTI_AT_AtEntry **result = (LIBMATTI_AT_AtEntry **) entries->elements;
    *count = entries->size;
    free(entries->elements); // the array moves to the caller
    free(entries);
    return result;
}

void LIBMATTI_AT_AtParser_FreeEntries(LIBMATTI_AT_AtEntry **entries, size_t count)
{
    if (entries == NULL)
        return;
    for (size_t i = 0; i < count; i++)
    {
        LIBMATTI_AT_Target_Free(entries[i]->target);
        LIBMATTI_AT_Transformation_Free(entries[i]->transformation);
        free(entries[i]);
    }
    free(entries);
}
