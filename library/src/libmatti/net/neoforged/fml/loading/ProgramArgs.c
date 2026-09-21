#include "libmatti/net/neoforged/fml/loading/ProgramArgs.h"

#include "libmatti/cpw/modlauncher/LogManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final Logger LOGGER = LogUtils.getLogger();
static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: private static class EntryValue implements Supplier<String[]>
typedef struct
{
    int raw;
    char *rawValue;
    char *prefix;
    char *key;
    int split;
    char *value;
} EntryValue;

// Java: public final class ProgramArgs
struct LIBMATTI_FML_ProgramArgs
{
    EntryValue *entries;
    size_t count;
};

static void entry_free(EntryValue *entry)
{
    free(entry->rawValue);
    free(entry->prefix);
    free(entry->key);
    free(entry->value);
}

// Java: private final Map<String, EntryValue> values - lookup of the first entry for a key
static EntryValue *find_entry(LIBMATTI_FML_ProgramArgs *programArgs, const char *key)
{
    for (size_t i = 0; i < programArgs->count; i++)
    {
        if (programArgs->entries[i].raw) continue;
        if (strcmp(programArgs->entries[i].key, key) == 0) return &programArgs->entries[i];
    }

    return NULL;
}

static void append_entry(LIBMATTI_FML_ProgramArgs *programArgs, EntryValue entry)
{
    programArgs->entries = realloc(programArgs->entries, sizeof(*programArgs->entries) * (programArgs->count + 1));
    programArgs->entries[programArgs->count++] = entry;
}

static void append_argument(char ***arguments, size_t *count, const char *value)
{
    *arguments = realloc(*arguments, sizeof(**arguments) * (*count + 1));
    (*arguments)[(*count)++] = strdup(value);
}

static void append_joined(char ***arguments, size_t *count, const char *left, const char *right, const char *separator)
{
    size_t length = strlen(left) + strlen(separator) + strlen(right) + 1;
    char *joined = malloc(length);
    snprintf(joined, length, "%s%s%s", left, separator, right);

    *arguments = realloc(*arguments, sizeof(**arguments) * (*count + 1));
    (*arguments)[(*count)++] = joined;
}

// Java: public static ProgramArgs from(String... args)
LIBMATTI_FML_ProgramArgs *LIBMATTI_FML_ProgramArgs_From(int argc, char *argv[])
{
    LIBMATTI_FML_ProgramArgs *programArgs = calloc(1, sizeof(LIBMATTI_FML_ProgramArgs));

    int ended = 0;
    for (int x = 0; x < argc; x++)
    {
        if (!ended)
        {
            // Java: if ("--".equals(args[x])) ended = true;
            if (strcmp("--", argv[x]) == 0)
            {
                ended = 1;
            }
            else if (strcmp("-", argv[x]) == 0)
            {
                LIBMATTI_FML_ProgramArgs_AddRaw(programArgs, argv[x]);
            }
            else if (argv[x][0] == '-')
            {
                char *equals = strchr(argv[x], '=');
                char *key = equals == NULL ? strdup(argv[x]) : strndup(argv[x], (size_t) (equals - argv[x]));
                const char *value = equals == NULL ? NULL : equals[1] == '\0' ? "" : equals + 1;

                // Java: if (idx == -1 && x + 1 < args.length && !args[x + 1].startsWith("-"))
                if (equals == NULL && x + 1 < argc && argv[x + 1][0] != '-')
                {
                    LIBMATTI_FML_ProgramArgs_AddArg(programArgs, 1, key, argv[x + 1]);
                    x++;
                }
                else
                {
                    LIBMATTI_FML_ProgramArgs_AddArg(programArgs, 0, key, value);
                }

                free(key);
            }
            else
            {
                LIBMATTI_FML_ProgramArgs_AddRaw(programArgs, argv[x]);
            }
        }
        else
        {
            LIBMATTI_FML_ProgramArgs_AddRaw(programArgs, argv[x]);
        }
    }

    return programArgs;
}

void LIBMATTI_FML_ProgramArgs_Free(LIBMATTI_FML_ProgramArgs *programArgs)
{
    if (programArgs == NULL) return;

    for (size_t i = 0; i < programArgs->count; i++)
        entry_free(&programArgs->entries[i]);
    free(programArgs->entries);
    free(programArgs);
}

// Java: public void addRaw(String arg)
void LIBMATTI_FML_ProgramArgs_AddRaw(LIBMATTI_FML_ProgramArgs *programArgs, const char *arg)
{
    // Java: entries.add(() -> new String[] { arg });
    EntryValue entry = {0};
    entry.raw = 1;
    entry.rawValue = strdup(arg);
    append_entry(programArgs, entry);
}

// Java: public void addArg(boolean split, String raw, String value)
void LIBMATTI_FML_ProgramArgs_AddArg(LIBMATTI_FML_ProgramArgs *programArgs, int split, const char *raw,
                                     const char *value)
{
    // Java: int idx = raw.startsWith("--") ? 2 : 1;
    size_t index = raw[0] == '-' && raw[1] == '-' ? 2 : 1;

    EntryValue entry = {0};
    entry.prefix = strndup(raw, index);
    entry.key = strdup(raw + index);
    entry.split = split;
    entry.value = value != NULL ? strdup(value) : NULL;

    if (find_entry(programArgs, entry.key) != NULL)
    {
        // Java: LOGGER.info("Duplicate entries for {} Unindexable", key);
        LIBMATTI_ML_Logger_Info(LOGGER(), NULL, "Duplicate entries for {} Unindexable", entry.key);
    }

    // Java: entries.add(entry);
    append_entry(programArgs, entry);
}

// Java: public String[] getArguments()
char **LIBMATTI_FML_ProgramArgs_GetArguments(const LIBMATTI_FML_ProgramArgs *programArgs, size_t *count)
{
    char **arguments = NULL;
    *count = 0;

    for (size_t i = 0; i < programArgs->count; i++)
    {
        const EntryValue *entry = &programArgs->entries[i];

        if (entry->raw)
        {
            append_argument(&arguments, count, entry->rawValue);
            continue;
        }

        if (entry->value == NULL)
        {
            append_joined(&arguments, count, entry->prefix, entry->key, "");
            continue;
        }

        if (entry->split)
        {
            append_joined(&arguments, count, entry->prefix, entry->key, "");
            append_argument(&arguments, count, entry->value);
            continue;
        }

        // Java: return new String[] { prefix + getKey() + '=' + getValue() };
        size_t length = strlen(entry->prefix) + strlen(entry->key) + strlen(entry->value) + 2;
        char *joined = malloc(length);
        snprintf(joined, length, "%s%s=%s", entry->prefix, entry->key, entry->value);
        append_argument(&arguments, count, joined);
        free(joined);
    }

    return arguments;
}

// Java: public boolean hasValue(String key)
int LIBMATTI_FML_ProgramArgs_HasValue(const LIBMATTI_FML_ProgramArgs *programArgs, const char *key)
{
    // Java: return getOrDefault(key, null) != null;
    return LIBMATTI_FML_ProgramArgs_GetOrDefault(programArgs, key, NULL) != NULL;
}

// Java: public String get(String key)
const char *LIBMATTI_FML_ProgramArgs_Get(const LIBMATTI_FML_ProgramArgs *programArgs, const char *key)
{
    // Java: EntryValue ent = values.get(key); return ent == null ? null : ent.getValue();
    EntryValue *entry = find_entry((LIBMATTI_FML_ProgramArgs *) programArgs, key);
    return entry == NULL ? NULL : entry->value;
}

// Java: public String getOrDefault(String key, String value)
const char *LIBMATTI_FML_ProgramArgs_GetOrDefault(const LIBMATTI_FML_ProgramArgs *programArgs, const char *key,
                                                  const char *value)
{
    // Java: return ent == null ? value : ent.getValue() == null ? value : ent.getValue();
    EntryValue *entry = find_entry((LIBMATTI_FML_ProgramArgs *) programArgs, key);
    if (entry == NULL || entry->value == NULL) return value;
    return entry->value;
}

// Java: public void put(String key, String value)
void LIBMATTI_FML_ProgramArgs_Put(LIBMATTI_FML_ProgramArgs *programArgs, const char *key, const char *value)
{
    EntryValue *entry = find_entry(programArgs, key);

    if (entry == NULL)
    {
        // Java: entry = new EntryValue(true, "--", key, value); values.put(key, entry); entries.add(entry);
        EntryValue added = {0};
        added.prefix = strdup("--");
        added.key = strdup(key);
        added.split = 1;
        added.value = strdup(value);
        append_entry(programArgs, added);
        return;
    }

    // Java: entry.setValue(value);
    free(entry->value);
    entry->value = strdup(value);
}

// Java: public void putLazy(String key, String value)
void LIBMATTI_FML_ProgramArgs_PutLazy(LIBMATTI_FML_ProgramArgs *programArgs, const char *key, const char *value)
{
    EntryValue *entry = find_entry(programArgs, key);

    if (entry == NULL)
    {
        // Java: addArg(true, "--" + key, value);
        size_t length = strlen(key) + 3;
        char *raw = malloc(length);
        snprintf(raw, length, "--%s", key);
        LIBMATTI_FML_ProgramArgs_AddArg(programArgs, 1, raw, value);
        free(raw);
        return;
    }

    if (entry->value == NULL)
    {
        // Java: ent.setValue(value);
        entry->value = strdup(value);
    }
}

// Java: public String remove(String key)
char *LIBMATTI_FML_ProgramArgs_Remove(LIBMATTI_FML_ProgramArgs *programArgs, const char *key)
{
    // Java: EntryValue ent = values.remove(key); if (ent == null) return null; entries.remove(ent); return ent.getValue();
    EntryValue *entry = find_entry(programArgs, key);
    if (entry == NULL) return NULL;

    char *value = entry->value != NULL ? strdup(entry->value) : NULL;

    size_t index = (size_t) (entry - programArgs->entries);
    entry_free(entry);

    memmove(&programArgs->entries[index], &programArgs->entries[index + 1],
            sizeof(*programArgs->entries) * (programArgs->count - index - 1));
    programArgs->count--;

    return value;
}
