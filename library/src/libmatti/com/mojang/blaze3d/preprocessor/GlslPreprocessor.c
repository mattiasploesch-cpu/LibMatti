#include "libmatti/com/mojang/blaze3d/preprocessor/GlslPreprocessor.h"

#include "libmatti/cpw/modlauncher/LogManager.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The import grammar: #moj_import "path" or #moj_import <path>, with optional
// whitespace (Java's regex also allows comments inside the directive; the port
// handles the plain whitespace cases the vanilla shaders use).
static const char *find_import(const char *cursor, const char **outPath, size_t *outPathLength,
                               int *outQuoted, const char **outStart, const char **outEnd)
{
    for (;;)
    {
        cursor = strstr(cursor, "#");
        if (cursor == NULL)
            return NULL;
        const char *p = cursor + 1;
        while (isspace((unsigned char) *p))
            p++;
        if (strncmp(p, "moj_import", 10) != 0)
        {
            cursor = p;
            continue;
        }
        p += 10;
        while (isspace((unsigned char) *p))
            p++;
        if (*p == '"')
        {
            *outQuoted = 1;
            p++;
        }
        else if (*p == '<')
        {
            *outQuoted = 0;
            p++;
        }
        else
        {
            cursor = p;
            continue;
        }
        const char *pathStart = p;
        while (*p != '\0' && *p != '"' && *p != '>' && *p != '\n')
            p++;
        if (*p == '\0' || *p == '\n')
        {
            cursor = p;
            continue;
        }
        *outPath = pathStart;
        *outPathLength = (size_t) (p - pathStart);
        *outStart = cursor;
        *outEnd = p + 1;
        return cursor;
    }
}

// Java: Context.sourceId - every import becomes a numbered source for #line.
typedef struct PreprocessContext
{
    int sourceId;
    int glslVersion;
} PreprocessContext;

// Java: processVersions - comments out the #version directive after lifting
// the context version to the maximum seen so far.
static void process_versions(char *line, size_t length, PreprocessContext *context)
{
    char *cursor = strstr(line, "version");
    if (cursor == NULL)
        return;
    // Walk back to the '#'.
    char *hash = NULL;
    for (char *p = cursor; p >= line; p--)
    {
        if (*p == '#')
        {
            hash = p;
            break;
        }
        if (!isspace((unsigned char) *p) && *p != '/')
            return;
    }
    if (hash == NULL)
        return;
    char *digits = cursor + 7;
    while (isspace((unsigned char) *digits))
        digits++;
    if (!isdigit((unsigned char) *digits))
        return;
    int version = atoi(digits);
    if (version > context->glslVersion)
        context->glslVersion = version;
    // Comment the directive out in place: "#version 330" -> "/*#version 330*/".
    memmove(digits + strlen(digits) + 2, digits + strlen(digits), 0);
    char *end = digits;
    while (isdigit((unsigned char) *end))
        end++;
    size_t directiveLength = (size_t) (end - hash);
    // Shift the rest right by 3 ("/*" before, "*/" after).
    memmove(end + 2, end, strlen(end) + 1);
    memmove(hash + 2, hash, directiveLength);
    hash[0] = '/';
    hash[1] = '*';
    (void) length;
}

// Java: processImports - recursive import expansion with #line bookkeeping.
// Memory rules: every chunk text this frame mallocs is freed by this frame;
// the returned buffer is malloc'ed fresh and owned by the caller.
static char *process_imports(const char *source, const char *prefix, PreprocessContext *context,
                             LIBMATTI_B3D_GlslPreprocessor_ApplyImport applyImport, void *userdata,
                             size_t *outSize);

typedef struct Chunk
{
    char *text;
    int owned;  // 1 when this frame malloc'ed the text and must free it
    struct Chunk *next;
} Chunk;

static void chunks_append(Chunk **head, Chunk **tail, char *text, int owned)
{
    Chunk *chunk = malloc(sizeof(Chunk));
    chunk->text = text;
    chunk->owned = owned;
    chunk->next = NULL;
    if (*tail != NULL)
        (*tail)->next = chunk;
    else
        *head = chunk;
    *tail = chunk;
}

static char *process_imports(const char *source, const char *prefix, PreprocessContext *context,
                             LIBMATTI_B3D_GlslPreprocessor_ApplyImport applyImport, void *userdata,
                             size_t *outSize)
{
    Chunk *head = NULL, *tail = NULL;
    size_t total = 0;
    int sourceId = context->sourceId;
    const char *cursor = source;
    char lineDirective[64];
    lineDirective[0] = '\0';

    const char *path, *importStart, *importEnd;
    size_t pathLength;
    int quoted;
    while (find_import(cursor, &path, &pathLength, &quoted, &importStart, &importEnd) != NULL)
    {
        // The text before the directive stays in the output.
        if (importStart > cursor)
        {
            size_t length = (size_t) (importStart - cursor);
            char *before = malloc(length + 1);
            memcpy(before, cursor, length);
            before[length] = '\0';
            chunks_append(&head, &tail, before, 1);
            total += length;
        }

        char *location = malloc(pathLength + strlen(prefix) + 1);
        sprintf(location, "%s%.*s", prefix, (int) pathLength, path);
        // Java: applyImport(quoted, prefix + location) - quoted imports get the
        // containing file's directory as prefix.
        char *imported = applyImport(userdata, quoted ? prefix : "", location);
        free(location);

        if (imported != NULL && imported[0] != '\0')
        {
            // Java: sourceId++ and a "#line 0 <sourceId>" before the import body.
            context->sourceId++;
            int importedId = context->sourceId;
            size_t importedSize = 0;
            char *expanded = process_imports(imported, quoted ? "" : prefix, context, applyImport, userdata,
                                             &importedSize);
            // The import body is fully inlined into the expanded copy now;
            // imported is released after the if/else through the shared free.
            imported = NULL;
            char *lineHeader = malloc(48);
            sprintf(lineHeader, "#line 0 %d\n", importedId);
            chunks_append(&head, &tail, lineHeader, 1);
            total += strlen(lineHeader);
            if (expanded != NULL)
            {
                // The recursion result stays owned by the child frame; the
                // child tracks it through its own chunk list, so the parent
                // only borrows it for the join (owned = 0).
                chunks_append(&head, &tail, expanded, 0);
                total += importedSize;
            }
        }
        else
        {
            // Java: keeps the directive as a comment so GLSL ignores it.
            const char *template2 = quoted ? "/*#moj_import \"%.*s\"*/" : "/*#moj_import <%.*s>*/";
            char *comment = malloc(pathLength + 32);
            sprintf(comment, template2, (int) pathLength, path);
            chunks_append(&head, &tail, comment, 1);
            total += strlen(comment);
        }
        if (imported != NULL)
            free(imported);

        // Java: a "#line <line> <sourceId>" resets the line counter afterwards.
        sprintf(lineDirective, "#line %d %d\n", 1, sourceId);
        cursor = importEnd;
    }

    if (*cursor != '\0')
    {
        size_t length = strlen(cursor);
        char *rest = malloc(length + 1);
        strcpy(rest, cursor);
        chunks_append(&head, &tail, rest, 1);
        total += length;
    }

    // Join the chunks into one buffer. Ownership: this frame frees exactly
    // the texts it malloc'ed itself; the recursion frees its own result
    // before returning a fresh join to the caller. To keep that rule simple
    // the frame copies everything and frees only the texts it created
    // (before/comment/lineHeader), never the recursion results.
    char *result = malloc(total + 1);
    size_t offset = 0;
    for (Chunk *chunk = head; chunk != NULL; chunk = chunk->next)
    {
        if (chunk->text != NULL)
        {
            size_t length = strlen(chunk->text);
            memcpy(result + offset, chunk->text, length);
            offset += length;
        }
    }
    result[offset] = '\0';
    for (Chunk *chunk = head; chunk != NULL; chunk = chunk->next)
    {
        // chunk->owned != 0 marks texts this frame malloc'ed.
        if (chunk->owned && chunk->text != NULL)
            free(chunk->text);
        free(chunk);
    }

    // Java: setVersion - lift the remaining (uncommented) #version to max.
    {
        char *versionLine = strstr(result, "#version");
        if (versionLine != NULL)
        {
            char *digits = versionLine + 8;
            while (isspace((unsigned char) *digits))
                digits++;
            if (isdigit((unsigned char) *digits))
            {
                int version = atoi(digits);
                if (version > context->glslVersion)
                    context->glslVersion = version;
            }
        }
    }
    if (outSize != NULL)
        *outSize = total;
    return result;
}

char *LIBMATTI_B3D_GlslPreprocessor_Process(const char *source,
                                             LIBMATTI_B3D_GlslPreprocessor_ApplyImport applyImport,
                                             void *applyImportUserdata,
                                             size_t *outSize)
{
    PreprocessContext context = {0, 0};
    char *expanded = process_imports(source, "", &context, applyImport, applyImportUserdata, outSize);
    // Java: setVersion runs over the first output line; the port lifts any
    // uncommented #version in the final buffer to the context maximum.
    char *versionLine = strstr(expanded, "#version ");
    if (versionLine != NULL && context.glslVersion > 0)
    {
        char *digits = versionLine + 9;
        while (isspace((unsigned char) *digits))
            digits++;
        if (isdigit((unsigned char) *digits))
        {
            int declared = atoi(digits);
            if (declared < context.glslVersion)
            {
                char *end = digits;
                while (isdigit((unsigned char) *end))
                    end++;
                char number[16];
                sprintf(number, "%d", context.glslVersion);
                if (strlen(number) == (size_t) (end - digits))
                    memcpy(digits, number, strlen(number));
            }
        }
    }
    return expanded;
}

char *LIBMATTI_B3D_GlslPreprocessor_InjectDefines(const char *source, const char *const *defines)
{
    // Java: if (defines.isEmpty()) return source.
    if (defines == NULL || defines[0] == NULL)
        return strdup(source);

    const char *newline = strchr(source, '\n');
    size_t headLength = newline != NULL ? (size_t) (newline - source) + 1 : strlen(source);

    size_t definesLength = 0;
    for (const char *const *define = defines; *define != NULL; define++)
        definesLength += strlen(*define) + 1;
    // "#line 1 0\n"
    definesLength += 11;

    char *result = malloc(headLength + definesLength + strlen(source) - headLength + 1);
    memcpy(result, source, headLength);
    size_t offset = headLength;
    for (const char *const *define = defines; *define != NULL; define++)
    {
        size_t length = strlen(*define);
        memcpy(result + offset, *define, length);
        offset += length;
        result[offset++] = '\n';
    }
    memcpy(result + offset, "#line 1 0\n", 10);
    offset += 10;
    strcpy(result + offset, source + headLength);
    return result;
}
