// The resource embed tool (build-time). Packs library/resources/ into the
// gzip blob the executable links in as the C array:
//
//   1. walk the resource tree (sorted per level, deterministic mtimes/owners)
//   2. build the tar in memory (POSIX ustar, the GNU ././@LongLink extension
//      for the >100-byte names)
//   3. gzip the tar in memory (RFC 1952, the bytes the gzip CLI writes)
//   4. write <out-dir>/EmbeddedPackResources.bin (the blob, for inspection)
//   5. emit EmbeddedPackResources_data.c/.h (the uint8_t array over the blob)
//
// Usage: embed_resources <resource-root> <out-dir>
// The generated files ride the matti library (CMake embeds them), so the
// executable carries the whole resource pack - no --assetsRoot, no pack dir
// on disk: the runtime inflates the blob into RAM (EmbeddedPackResources,
// the PackResources implementation Java's vanilla pack rides).

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <zlib.h>

// ---- the growable byte buffer ----------------------------------------------

typedef struct
{
    unsigned char *data;
    size_t size;
    size_t capacity;
} Buffer;

static void oom(void)
{
    fprintf(stderr, "embed_resources: out of memory\n");
    exit(1);
}

static void buf_reserve(Buffer *buf, size_t extra)
{
    if (buf->size + extra <= buf->capacity)
        return;
    size_t capacity = buf->capacity ? buf->capacity : 65536;
    while (capacity < buf->size + extra)
        capacity *= 2;
    buf->data = realloc(buf->data, capacity);
    if (buf->data == NULL)
        oom();
    buf->capacity = capacity;
}

static void buf_write(Buffer *buf, const void *data, size_t length)
{
    buf_reserve(buf, length);
    memcpy(buf->data + buf->size, data, length);
    buf->size += length;
}

// ---- the tar members (the walk output, sorted per level) -------------------

typedef struct
{
    char *relative; // the tar member name ("./assets/matticraft/...")
    char *absolute; // the on-disk path to read from
    long long size;
    int isDir;
} Entry;

static Entry *entries;
static size_t entryCount;
static size_t entryCapacity;

static void add_entry(const char *relative, const char *absolute, long long size, int isDir)
{
    if (entryCount == entryCapacity)
    {
        entryCapacity = entryCapacity ? entryCapacity * 2 : 1024;
        entries = realloc(entries, entryCapacity * sizeof(*entries));
        if (entries == NULL)
            oom();
    }
    Entry *entry = &entries[entryCount++];
    entry->relative = strdup(relative);
    entry->absolute = strdup(absolute);
    entry->size = size;
    entry->isDir = isDir;
    if (entry->relative == NULL || entry->absolute == NULL)
        oom();
}

// ---- the ustar header writer ------------------------------------------------

#define BLOCK_SIZE 512
#define TAR_NAME_MAX 100
#define GNU_LONGLINK_TYPE 'L'

// the octal writer the tar headers use (the NUL/space tail the readers
// accept: GNU tar writes the digits left-aligned with the trailing NUL, so
// the checksum reader sees "0001234\0" - the leading-zero form is valid too,
// but the digits must START at the field's first byte)
static void write_octal(char *out, size_t width, long long value)
{
    // width-2 digits + the trailing NUL (the tar convention)
    size_t digits = width - 1;
    for (size_t i = 0; i < digits; i++)
        out[i] = '0';
    out[digits] = '\0';
    // write the value least-significant first, right-aligned over the digits
    long long v = value;
    size_t position = digits;
    while (position > 0)
    {
        position--;
        out[position] = (char) ('0' + (v & 7));
        v >>= 3;
        if (v == 0)
            break;
    }
}

static void write_header(Buffer *tar, const char *name, long long size, int typeflag)
{
    char header[BLOCK_SIZE];
    memset(header, 0, sizeof(header));
    // the name field truncates at 100 - the long names ride @LongLink ahead
    strncpy(header, name, TAR_NAME_MAX);
    write_octal(header + 100, 8, 0644);                 // mode
    write_octal(header + 108, 8, 0);                    // uid
    write_octal(header + 116, 8, 0);                    // gid
    write_octal(header + 124, 12, typeflag == '0' ? size : 0); // size
    write_octal(header + 136, 12, 0);                   // mtime (the deterministic build)
    memset(header + 148, ' ', 8);                       // the checksum placeholder
    header[156] = (char) typeflag;
    memcpy(header + 257, "ustar  \0", 8);               // magic + version
    // the checksum over the header with the field spaced
    unsigned int checksum = 0;
    for (size_t i = 0; i < BLOCK_SIZE; i++)
        checksum += (unsigned char) header[i];
    write_octal(header + 148, 8, checksum);
    buf_write(tar, header, BLOCK_SIZE);
}

static const char zeros[BLOCK_SIZE];

static void write_long_name(Buffer *tar, const char *name)
{
    size_t length = strlen(name) + 1; // the NUL the extractors read
    write_header(tar, "././@LongLink", (long long) length, GNU_LONGLINK_TYPE);
    buf_write(tar, name, length);
    size_t padding = (BLOCK_SIZE - (length % BLOCK_SIZE)) % BLOCK_SIZE;
    if (padding)
        buf_write(tar, zeros, padding);
}

static void tar_entry(Buffer *tar, const Entry *entry)
{
    const char *name = entry->relative;
    if (strlen(name) >= TAR_NAME_MAX)
        write_long_name(tar, name);
    write_header(tar, name, entry->size, entry->isDir ? '5' : '0');
    if (entry->isDir)
        return;
    FILE *file = fopen(entry->absolute, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "embed_resources: cannot open %s\n", entry->absolute);
        exit(1);
    }
    char chunk[65536];
    long long remaining = entry->size;
    while (remaining > 0)
    {
        size_t read =
            fread(chunk, 1, remaining < (long long) sizeof(chunk) ? (size_t) remaining : sizeof(chunk), file);
        if (read == 0)
            break;
        buf_write(tar, chunk, read);
        remaining -= (long long) read;
    }
    fclose(file);
    size_t padding = (BLOCK_SIZE - (entry->size % BLOCK_SIZE)) % BLOCK_SIZE;
    if (padding)
        buf_write(tar, zeros, padding);
}

// ---- the deterministic tree walk --------------------------------------------

static void walk_tree(const char *root, const char *relative)
{
    char absolute[4096];
    if (relative[0] == '\0')
        snprintf(absolute, sizeof(absolute), "%s", root);
    else
        snprintf(absolute, sizeof(absolute), "%s/%s", root, relative);

    DIR *dir = opendir(absolute);
    if (dir == NULL)
        return;
    // the sorted per-level walk keeps the tar byte-deterministic (the
    // reproducible build: same tree, same blob)
    char **names = NULL;
    size_t count = 0, capacity = 0;
    struct dirent *item;
    while ((item = readdir(dir)) != NULL)
    {
        if (strcmp(item->d_name, ".") == 0 || strcmp(item->d_name, "..") == 0)
            continue;
        if (count == capacity)
        {
            capacity = capacity ? capacity * 2 : 64;
            names = realloc(names, capacity * sizeof(char *));
            if (names == NULL)
                oom();
        }
        names[count++] = strdup(item->d_name);
    }
    closedir(dir);
    for (size_t i = 0; i + 1 < count; i++)
        for (size_t j = i + 1; j < count; j++)
            if (strcmp(names[i], names[j]) > 0)
            {
                char *swap = names[i];
                names[i] = names[j];
                names[j] = swap;
            }

    for (size_t i = 0; i < count; i++)
    {
        char childRelative[4096];
        char childAbsolute[4096];
        if (relative[0] == '\0')
            snprintf(childRelative, sizeof(childRelative), "%s", names[i]);
        else
            snprintf(childRelative, sizeof(childRelative), "%s/%s", relative, names[i]);
        snprintf(childAbsolute, sizeof(childAbsolute), "%s/%s", root, childRelative);

        struct stat st;
        if (stat(childAbsolute, &st) != 0)
            continue;
        if (S_ISDIR(st.st_mode))
        {
            char tarName[4200];
            snprintf(tarName, sizeof(tarName), "./%s/", childRelative);
            add_entry(tarName, childAbsolute, 0, 1);
            walk_tree(root, childRelative);
        }
        else if (S_ISREG(st.st_mode))
        {
            char tarName[4200];
            snprintf(tarName, sizeof(tarName), "./%s", childRelative);
            add_entry(tarName, childAbsolute, (long long) st.st_size, 0);
        }
        free(names[i]);
    }
    free(names);
}

// ---- the generated C sources -------------------------------------------------

static void emit_generated_c(const char *outDir, const unsigned char *data, size_t size)
{
    // the C rides the out-dir flat; the include path resolves through the
    // generated include dir (the header lands in its libmatti/... slot)
    char path[4096];
    snprintf(path, sizeof(path), "%s/EmbeddedPackResources_data.c", outDir);
    FILE *out = fopen(path, "wb");
    if (out == NULL)
    {
        fprintf(stderr, "embed_resources: cannot write %s\n", path);
        exit(1);
    }
    fprintf(out, "// Generated by embed_resources - the resource pack blob as a C array.\n");
    fprintf(out, "// Packed from library/resources (deterministic tar + gzip); regenerate\n");
    fprintf(out, "// through the build (the embed_resources step), do not edit.\n\n");
    fprintf(out, "#include \"libmatti/net/minecraft/server/packs/embedded/EmbeddedPackResources.h\"\n\n");
    fprintf(out, "const unsigned char LIBMATTI_MC_EmbeddedPackResources_BLOB[%llu] =\n{\n", (unsigned long long) size);
    for (size_t i = 0; i < size; i++)
    {
        if (i % 20 == 0)
            fprintf(out, "    ");
        fprintf(out, "0x%02x,", data[i]);
        if ((i % 20) == 19 || i + 1 == size)
            fprintf(out, "\n");
    }
    fprintf(out, "};\n");
    fprintf(out, "const size_t LIBMATTI_MC_EmbeddedPackResources_SIZE = %llu;\n", (unsigned long long) size);
    fclose(out);
}

static void emit_generated_h(const char *outDir, size_t size)
{
    // the header lives in its libmatti path slot so the include the C carries
    // ("libmatti/net/minecraft/server/packs/embedded/...") resolves directly
    char path[4096];
    snprintf(path, sizeof(path), "%s/libmatti/net/minecraft/server/packs/embedded/EmbeddedPackResources_data.h", outDir);
    FILE *out = fopen(path, "wb");
    if (out == NULL)
    {
        fprintf(stderr, "embed_resources: cannot write %s\n", path);
        exit(1);
    }
    fprintf(out, "// Generated by embed_resources - the blob declaration the data C rides.\n\n");
    fprintf(out, "#ifndef MATTICRAFT_MC_PACKS_EMBEDDED_EMBEDDEDPACKRESOURCES_DATA_H\n");
    fprintf(out, "#define MATTICRAFT_MC_PACKS_EMBEDDED_EMBEDDEDPACKRESOURCES_DATA_H\n\n");
    fprintf(out, "#include <stddef.h>\n\n");
    fprintf(out, "extern const unsigned char LIBMATTI_MC_EmbeddedPackResources_BLOB[%llu];\n",
            (unsigned long long) size);
    fprintf(out, "extern const size_t LIBMATTI_MC_EmbeddedPackResources_SIZE;\n\n");
    fprintf(out, "#endif //MATTICRAFT_MC_PACKS_EMBEDDED_EMBEDDEDPACKRESOURCES_DATA_H\n");
    fclose(out);
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "usage: embed_resources <resource-root> <out-dir>\n");
        return 1;
    }
    const char *root = argv[1];
    const char *outDir = argv[2];

    // 1+2) the deterministic tar (the root dir entry, then the sorted walk)
    Buffer tar = {0};
    add_entry("./", root, 0, 1);
    walk_tree(root, "");
    for (size_t i = 0; i < entryCount; i++)
        tar_entry(&tar, &entries[i]);
    buf_write(&tar, zeros, BLOCK_SIZE * 2); // the end-of-archive marker

    // 3) the gzip stream (RFC 1952 - the bytes the gzip CLI writes)
    Buffer gz = {0};
    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    if (deflateInit2(&stream, 9, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
    {
        fprintf(stderr, "embed_resources: deflateInit2 failed\n");
        return 1;
    }
    stream.next_in = tar.data;
    stream.avail_in = (uInt) tar.size;
    int status;
    do
    {
        buf_reserve(&gz, 65536);
        stream.next_out = gz.data + gz.size;
        stream.avail_out = 65536;
        status = deflate(&stream, Z_FINISH);
        gz.size += 65536 - stream.avail_out;
    } while (status != Z_STREAM_END);
    deflateEnd(&stream);

    // 4+5) the outputs: the blob and the generated C sources
    char path[4096];
    snprintf(path, sizeof(path), "%s/EmbeddedPackResources.bin", outDir);
    FILE *blob = fopen(path, "wb");
    if (blob == NULL)
    {
        fprintf(stderr, "embed_resources: cannot write %s\n", path);
        return 1;
    }
    fwrite(gz.data, 1, gz.size, blob);
    fclose(blob);
    emit_generated_c(outDir, gz.data, gz.size);
    emit_generated_h(outDir, gz.size);

    size_t fileCount = 0;
    for (size_t i = 0; i < entryCount; i++)
        fileCount += entries[i].isDir ? 0 : 1;
    fprintf(stderr, "embed_resources: %zu files, tar %zu bytes, gzip %zu bytes\n",
            fileCount, tar.size, gz.size);
    return 0;
}
