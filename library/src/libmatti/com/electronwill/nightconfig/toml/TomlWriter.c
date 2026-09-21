// Java: public final class TomlWriter implements ConfigWriter - with the default settings
// (writeTableInlinePredicate = isEmpty, hideRedundantLevels = true, tab indent, system newline).
// write(config, file, WritingMode.REPLACE_ATOMIC) becomes a fopen/fclose pair; Java's Writer
// overload goes through LIBMATTI_NC_TomlWriter_WriteToString.

#include "libmatti/com/electronwill/nightconfig/toml/TomlWriter.h"

#include "libmatti/com/electronwill/nightconfig/core/io/CharsWrapper.h"
#include "libmatti/com/electronwill/nightconfig/toml/Toml.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// The output buffer (Java: CharacterOutput over a Writer)
// ---------------------------------------------------------------------------

typedef struct OutputTag
{
    char *data;
    size_t length;
    size_t capacity;
} Output;

// Java: final class TemporalWriter - forward declaration, used by ValueWriter_write
static void TemporalWriter_write(const LIBMATTI_JT_Temporal *temporal, Output *out);

static void output_init(Output *out)
{
    out->capacity = 256;
    out->data = malloc(out->capacity);
    out->data[0] = '\0';
    out->length = 0;
}

static void output_write(Output *out, const char *text, size_t length)
{
    if (out->length + length + 1 > out->capacity)
    {
        while (out->length + length + 1 > out->capacity) out->capacity *= 2;
        out->data = realloc(out->data, out->capacity);
    }
    memcpy(out->data + out->length, text, length);
    out->length += length;
    out->data[out->length] = '\0';
}

static void output_write_char(Output *out, char c)
{
    output_write(out, &c, 1);
}

static void output_write_str(Output *out, const char *text)
{
    output_write(out, text, strlen(text));
}

// ---------------------------------------------------------------------------
// StringWriter (Java: package com.electronwill.nightconfig.toml.StringWriter)
// ---------------------------------------------------------------------------

static void write_escaped_unicode(Output *out, int codePoint)
{
    char hexa[9];
    snprintf(hexa, sizeof(hexa), "\\u%04X", codePoint);
    output_write_str(out, hexa);
}

// Java: static void writeBasic(String str, CharacterOutput output)
static void StringWriter_writeBasic(const char *str, Output *out)
{
    output_write_char(out, '"');
    for (const unsigned char *p = (const unsigned char *) str; *p != '\0'; p++)
    {
        int c = *p;
        switch (c)
        {
        case '\\': output_write_str(out, "\\\\"); break;
        case '"': output_write_str(out, "\\\""); break;
        case '\b': output_write_str(out, "\\b"); break;
        case '\f': output_write_str(out, "\\f"); break;
        case '\n': output_write_str(out, "\\n"); break;
        case '\r': output_write_str(out, "\\r"); break;
        case '\t': output_write_str(out, "\\t"); break;
        default:
            if (LIBMATTI_NC_Toml_IsControlChar(c)) write_escaped_unicode(out, c);
            else output_write_char(out, (char) c);
            break;
        }
    }
    output_write_char(out, '"');
}

// ---------------------------------------------------------------------------
// ArrayWriter (Java: package com.electronwill.nightconfig.toml.ArrayWriter)
// ---------------------------------------------------------------------------

// Java: static void write(List<?> values, CharacterOutput output, TomlWriter writer) - no array
// indentation (writesIndented is false by default)
static void ArrayWriter_write(const LIBMATTI_NC_Value *values, size_t count, Output *out,
                              LIBMATTI_NC_TomlWriter *writer);

// ---------------------------------------------------------------------------
// ValueWriter (Java: package com.electronwill.nightconfig.toml.ValueWriter)
// ---------------------------------------------------------------------------

static void ValueWriter_write(const LIBMATTI_NC_Value *value, Output *out, LIBMATTI_NC_TomlWriter *writer);
static void writeKey(LIBMATTI_NC_TomlWriter *writer, const char *key, Output *out);

// Java: static void writeInline(UnmodifiableConfig config, CharacterOutput output, TomlWriter writer)
static void ValueWriter_writeInlineTable(const LIBMATTI_NC_Config *config, Output *out,
                                         LIBMATTI_NC_TomlWriter *writer)
{
    output_write_char(out, '{');
    for (size_t i = 0; i < config->count; i++)
    {
        // Comments aren't written in an inline table
        writeKey(writer, config->keys[i], out);
        output_write_str(out, " = ");
        ValueWriter_write(&config->values[i], out, writer);
        if (i + 1 < config->count) output_write_str(out, ", ");
    }
    output_write_char(out, '}');
}

// Java: static void write(Object value, CharacterOutput output, TomlWriter writer)
static void ValueWriter_write(const LIBMATTI_NC_Value *value, Output *out, LIBMATTI_NC_TomlWriter *writer)
{
    switch (value->type)
    {
    case LIBMATTI_NC_VALUE_CONFIG:
        // Java: TableWriter.writeInline((Config) value, output, writer)
        ValueWriter_writeInlineTable(value->config, out, writer);
        break;
    case LIBMATTI_NC_VALUE_LIST:
        ArrayWriter_write(value->list, value->listCount, out, writer);
        break;
    case LIBMATTI_NC_VALUE_STRING:
        // Java: writesLiteral is false and the multiline predicate needs a newline in the middle;
        // plain single-line basic strings cover the strings the config layer writes.
        StringWriter_writeBasic(value->string, out);
        break;
    case LIBMATTI_NC_VALUE_BOOLEAN:
        output_write_str(out, value->boolean ? "true" : "false");
        break;
    case LIBMATTI_NC_VALUE_INT:
    {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%d", value->integer);
        output_write_str(out, buffer);
        break;
    }
    case LIBMATTI_NC_VALUE_LONG:
    {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%lld", value->longValue);
        output_write_str(out, buffer);
        break;
    }
    case LIBMATTI_NC_VALUE_TEMPORAL:
        // Java: TemporalWriter.write((Temporal) value, output)
        TemporalWriter_write(&value->temporal, out);
        break;
    case LIBMATTI_NC_VALUE_NULL:
    default:
        // Java: throw new WritingException("TOML doesn't support null values")
        break;
    }
}

// ---------------------------------------------------------------------------
// Java: final class TemporalWriter - writes the date/time value back in TOML form
// ---------------------------------------------------------------------------

// Java: private static void writePadded(int value, int numberOfDigits, CharacterOutput output)
static void write_padded(Output *out, int value, int numberOfDigits)
{
    char str[16];
    int length = snprintf(str, sizeof(str), "%d", value);
    for (int i = length; i < numberOfDigits; i++)
        output_write(out, "0", 1);
    output_write_str(out, str);
}

// Java: private static void writePaddedAndTrimmed(int value, int numberOfDigits, CharacterOutput output)
static void write_padded_and_trimmed(Output *out, int value, int numberOfDigits)
{
    char str[16];
    int length = snprintf(str, sizeof(str), "%d", value);
    for (int i = length; i < numberOfDigits; i++)
        output_write(out, "0", 1);
    // Java: trailing zeros are trimmed, at least one digit stays
    int trimmed = length;
    for (int i = length - 1; i >= 1; i--)
    {
        if (str[i] == '0') trimmed--;
        else break;
    }
    output_write(out, str, (size_t) trimmed);
}

// Java: private static void writeHour(Temporal temporal, CharacterOutput output)
static void TemporalWriter_writeHour(const LIBMATTI_JT_Temporal *temporal, Output *out)
{
    write_padded(out, temporal->hour, 2);
    output_write(out, ":", 1);
    write_padded(out, temporal->minute, 2);
    output_write(out, ":", 1);
    write_padded(out, temporal->second, 2);
    if (temporal->nano != 0)
    {
        output_write(out, ".", 1);
        write_padded_and_trimmed(out, temporal->nano, 9);
    }
}

// Java: private static void writeDate(Temporal temporal, CharacterOutput output)
static void TemporalWriter_writeDate(const LIBMATTI_JT_Temporal *temporal, Output *out)
{
    write_padded(out, temporal->year, 4);
    output_write(out, "-", 1);
    write_padded(out, temporal->month, 2);
    output_write(out, "-", 1);
    write_padded(out, temporal->dayOfMonth, 2);
}

// Java: static void write(Temporal temporal, CharacterOutput output)
static void TemporalWriter_write(const LIBMATTI_JT_Temporal *temporal, Output *out)
{
    if (LIBMATTI_JT_Temporal_SupportsYear(temporal))
    {
        TemporalWriter_writeDate(temporal, out);
        if (LIBMATTI_JT_Temporal_SupportsHour(temporal))
        {
            output_write(out, "T", 1);
            TemporalWriter_writeHour(temporal, out);
            if (LIBMATTI_JT_Temporal_SupportsOffsetSeconds(temporal))
            {
                // Java: output.write(ZoneOffset.ofTotalSeconds(offsetSeconds).getId()) - Z or +HH:MM
                int offsetSeconds = temporal->offsetSeconds;
                if (offsetSeconds == 0)
                {
                    output_write(out, "Z", 1);
                }
                else
                {
                    char sign = offsetSeconds < 0 ? '-' : '+';
                    if (offsetSeconds < 0) offsetSeconds = -offsetSeconds;
                    char offset[7];
                    snprintf(offset, sizeof(offset), "%c%02d:%02d", sign, offsetSeconds / 3600,
                             offsetSeconds % 3600 / 60);
                    output_write_str(out, offset);
                }
            }
        }
    }
    else if (LIBMATTI_JT_Temporal_SupportsHour(temporal))
    {
        TemporalWriter_writeHour(temporal, out);
    }
}

// Java: static void write(List<?> values, CharacterOutput output, TomlWriter writer)
static void ArrayWriter_write(const LIBMATTI_NC_Value *values, size_t count, Output *out,
                              LIBMATTI_NC_TomlWriter *writer)
{
    if (count == 0)
    {
        output_write_str(out, "[]");
        return;
    }

    output_write_char(out, '[');
    for (size_t i = 0; i < count; i++)
    {
        ValueWriter_write(&values[i], out, writer);
        if (i + 1 < count) output_write_str(out, ", ");
    }
    output_write_char(out, ']');
}

// ---------------------------------------------------------------------------
// ValueWriter (Java: package com.electronwill.nightconfig.toml.ValueWriter)
// ---------------------------------------------------------------------------
// (ValueWriter_write stays below writeKey, which it shares)

// ---------------------------------------------------------------------------
// TableWriter (Java: package com.electronwill.nightconfig.toml.TableWriter)
// ---------------------------------------------------------------------------

typedef struct
{
    // Java: List<Entry> simples, subTables, arraysOfTables - the port keeps the indexes instead
    size_t *simples;
    size_t simplesCount;
    size_t *subTables;
    size_t subTablesCount;
    size_t *arraysOfTables;
    size_t arraysOfTablesCount;
} OrganizedTable;

// Java: private static void writeIndent / writeNewline / writeIndentedComment / writeIndentedKey
static void writeIndent(LIBMATTI_NC_TomlWriter *writer, Output *out)
{
    for (int i = 0; i < writer->currentIndentLevel; i++) output_write_str(out, writer->indent);
}

static void writeNewline(LIBMATTI_NC_TomlWriter *writer, Output *out)
{
    output_write_str(out, writer->newline);
}

// Java: void writeIndentedComment(String commentString, CharacterOutput output)
static void writeIndentedComment(LIBMATTI_NC_TomlWriter *writer, const char *commentString, Output *out)
{
    if (commentString == NULL) return;

    // Java: List<String> comments = StringUtils.splitLines(commentString)
    const char *start = commentString;
    for (const char *p = commentString;; p++)
    {
        if (*p != '\n' && *p != '\0') continue;

        size_t length = (size_t) (p - start);
        if (length > 0 && start[length - 1] == '\r') length--;
        if (length > 0 || *p == '\0')
        {
            writeIndent(writer, out);
            output_write_char(out, '#');
            output_write(out, start, length);
            writeNewline(writer, out);
        }
        if (*p == '\0') break;
        start = p + 1;
    }
}

// Java: void writeKey(String key, CharacterOutput output)
static void writeKey(LIBMATTI_NC_TomlWriter *writer, const char *key, Output *out)
{
    LIBMATTI_NC_CharsWrapper wrapper = {(char *) key, strlen(key)};
    if (LIBMATTI_NC_Toml_IsValidBareKey(&wrapper, writer->lenientBareKeys))
        output_write_str(out, key);
    else
        StringWriter_writeBasic(key, out);
}

static void writeIndentedKey(LIBMATTI_NC_TomlWriter *writer, const char *key, Output *out)
{
    writeIndent(writer, out);
    writeKey(writer, key, out);
}

// Java: boolean writesInline(UnmodifiableConfig config) - the default predicate is isEmpty
static int writesInline(const LIBMATTI_NC_Config *config)
{
    return LIBMATTI_NC_Config_IsEmpty(config);
}

// Java: static OrganizedTable prepareTable(UnmodifiableCommentedConfig config, String comment, TomlWriter writer)
static void prepareTable(const LIBMATTI_NC_Config *config, OrganizedTable *table)
{
    memset(table, 0, sizeof(*table));
    size_t simplesCapacity = 0, tablesCapacity = 0, arraysCapacity = 0;

    for (size_t i = 0; i < config->count; i++)
    {
        const LIBMATTI_NC_Value *value = &config->values[i];
        if (value->type == LIBMATTI_NC_VALUE_CONFIG)
        {
            if (writesInline(value->config))
            {
                if (simplesCapacity == table->simplesCount)
                    table->simples = realloc(table->simples, sizeof(size_t) * (simplesCapacity += 4));
                table->simples[table->simplesCount++] = i;
            }
            else
            {
                if (tablesCapacity == table->subTablesCount)
                    table->subTables = realloc(table->subTables, sizeof(size_t) * (tablesCapacity += 4));
                table->subTables[table->subTablesCount++] = i;
            }
        }
        else if (value->type == LIBMATTI_NC_VALUE_LIST && value->listCount > 0)
        {
            // Java: list.stream().allMatch(UnmodifiableConfig.class::isInstance)
            int allConfigs = 1;
            for (size_t j = 0; j < value->listCount; j++)
                if (value->list[j].type != LIBMATTI_NC_VALUE_CONFIG) allConfigs = 0;

            if (allConfigs)
            {
                if (arraysCapacity == table->arraysOfTablesCount)
                    table->arraysOfTables = realloc(table->arraysOfTables, sizeof(size_t) * (arraysCapacity += 4));
                table->arraysOfTables[table->arraysOfTablesCount++] = i;
            }
            else
            {
                if (simplesCapacity == table->simplesCount)
                    table->simples = realloc(table->simples, sizeof(size_t) * (simplesCapacity += 4));
                table->simples[table->simplesCount++] = i;
            }
        }
        else
        {
            if (simplesCapacity == table->simplesCount)
                table->simples = realloc(table->simples, sizeof(size_t) * (simplesCapacity += 4));
            table->simples[table->simplesCount++] = i;
        }
    }
}

// Java: OrganizedTable.canBeSkipped() - the port never has comments (the parser does not read them),
// so the check reduces to no simples and at least one sub table/array of tables.
static int canBeSkipped(const OrganizedTable *table)
{
    return table->simplesCount == 0 &&
           (!table->subTablesCount || !table->arraysOfTablesCount) &&
           (table->subTablesCount > 0 || table->arraysOfTablesCount > 0);
}

typedef struct
{
    // Java: List<String> configPath
    const char **parts;
    size_t count;
    size_t capacity;
} ConfigPath;

static void configPath_push(ConfigPath *path, const char *part)
{
    if (path->capacity == path->count) path->parts = realloc(path->parts, sizeof(char *) * (path->capacity += 8));
    path->parts[path->count++] = part;
}

static void configPath_pop(ConfigPath *path)
{
    path->count--;
}

// Java: private static void writeTableName(List<String> name, CharacterOutput output, TomlWriter writer, char[] begin, char[] end)
static void writeTableName(LIBMATTI_NC_TomlWriter *writer, const ConfigPath *name, Output *out,
                           const char *begin, const char *end)
{
    // Java: if (name.isEmpty()) throw new WritingException("Invalid empty table name.")
    writeIndent(writer, out);
    output_write_str(out, begin);
    for (size_t i = 0; i < name->count; i++)
    {
        if (i > 0) output_write_char(out, '.');
        writeKey(writer, name->parts[i], out);
    }
    output_write_str(out, end);
}

// Java: private static void writeWithHeader(UnmodifiableCommentedConfig config, String tableComment,
//         boolean inArrayOfTables, boolean tableHeader, List<String> configPath, CharacterOutput output, TomlWriter writer)
static void writeWithHeader(LIBMATTI_NC_TomlWriter *writer, const LIBMATTI_NC_Config *config,
                            int inArrayOfTables, int tableHeader, ConfigPath *configPath, Output *out)
{
    OrganizedTable table;
    prepareTable(config, &table);
    int hasSubTables = table.subTablesCount > 0;

    if (canBeSkipped(&table) && writer->hideRedundantLevels)
    {
        writer->currentIndentLevel++;

        // subtables, but first header if in array of tables
        if (inArrayOfTables)
        {
            writeTableName(writer, configPath, out, "[[", "]]");
            writeNewline(writer, out);
            writer->currentIndentLevel++;
        }
        for (size_t i = 0; i < table.subTablesCount; i++)
        {
            size_t index = table.subTables[i];
            configPath_push(configPath, config->keys[index]);
            writeWithHeader(writer, config->values[index].config, 0, 1, configPath, out);
            configPath_pop(configPath);

            if (i + 1 < table.subTablesCount) writeNewline(writer, out);
        }
        if (inArrayOfTables) writer->currentIndentLevel--;

        // sub arrays of tables
        for (size_t i = 0; i < table.arraysOfTablesCount; i++)
        {
            size_t index = table.arraysOfTables[i];
            configPath_push(configPath, config->keys[index]);
            const LIBMATTI_NC_Value *array = &config->values[index];
            for (size_t j = 0; j < array->listCount; j++)
                writeWithHeader(writer, array->list[j].config, 1, 1, configPath, out);
            configPath_pop(configPath);

            if (i + 1 < table.arraysOfTablesCount) writeNewline(writer, out);
        }
        writer->currentIndentLevel--;
    }
    else
    {
        // header
        if (inArrayOfTables)
        {
            writeTableName(writer, configPath, out, "[[", "]]");
            writeNewline(writer, out);
        }
        else if (tableHeader)
        {
            writeTableName(writer, configPath, out, "[", "]");
            writeNewline(writer, out);
        }

        // body
        writer->currentIndentLevel++;
        for (size_t i = 0; i < table.simplesCount; i++)
        {
            size_t index = table.simples[i];
            writeIndentedKey(writer, config->keys[index], out);
            output_write_str(out, " = ");
            ValueWriter_write(&config->values[index], out, writer);
            writeNewline(writer, out);
        }

        if (hasSubTables) writeNewline(writer, out);
        for (size_t i = 0; i < table.subTablesCount; i++)
        {
            size_t index = table.subTables[i];
            configPath_push(configPath, config->keys[index]);
            writeWithHeader(writer, config->values[index].config, 0, 1, configPath, out);
            configPath_pop(configPath);

            if (table.arraysOfTablesCount > 0 || i + 1 < table.subTablesCount) writeNewline(writer, out);
        }
        for (size_t i = 0; i < table.arraysOfTablesCount; i++)
        {
            size_t index = table.arraysOfTables[i];
            configPath_push(configPath, config->keys[index]);
            const LIBMATTI_NC_Value *array = &config->values[index];
            for (size_t j = 0; j < array->listCount; j++)
                writeWithHeader(writer, array->list[j].config, 1, 1, configPath, out);
            configPath_pop(configPath);

            if (i + 1 < table.arraysOfTablesCount) writeNewline(writer, out);
        }
        writer->currentIndentLevel--;
        // end of body
    }

    free(table.simples);
    free(table.subTables);
    free(table.arraysOfTables);
}

// ---------------------------------------------------------------------------
// TomlWriter
// ---------------------------------------------------------------------------

LIBMATTI_NC_TomlWriter *LIBMATTI_NC_TomlWriter_New(void)
{
    LIBMATTI_NC_TomlWriter *writer = calloc(1, sizeof(*writer));
    writer->lenientBareKeys = 0;
    writer->hideRedundantLevels = 1;
    writer->indent = "\t";
    writer->newline = "\n"; // Java: NewlineStyle.system()
    writer->currentIndentLevel = 0;
    return writer;
}

void LIBMATTI_NC_TomlWriter_Free(LIBMATTI_NC_TomlWriter *writer)
{
    free(writer);
}

char *LIBMATTI_NC_TomlWriter_WriteToString(LIBMATTI_NC_TomlWriter *writer, const LIBMATTI_NC_Config *config)
{
    Output out;
    output_init(&out);

    // Java: currentIndentLevel = -1 to make the root entries not indented; the port never indents
    // the root level (level 0 entries go through writeWithHeader without a header), so 0 is enough.
    writer->currentIndentLevel = 0;

    ConfigPath configPath = {0};
    writeWithHeader(writer, config, 0, 0, &configPath, &out);
    free(configPath.parts);

    return out.data;
}

int LIBMATTI_NC_TomlWriter_WriteFile(LIBMATTI_NC_TomlWriter *writer, const LIBMATTI_NC_Config *config,
                                     const char *file)
{
    char *data = LIBMATTI_NC_TomlWriter_WriteToString(writer, config);

    // Java: WritingMode.REPLACE_ATOMIC writes to a temp file and moves it; the port writes in place.
    FILE *stream = fopen(file, "wb");
    if (stream == NULL)
    {
        free(data);
        return 0;
    }
    size_t length = strlen(data);
    size_t written = fwrite(data, 1, length, stream);
    fclose(stream);
    free(data);

    return written == length;
}
