// The embedded pack over the linked-in resource blob (implementation).
//
// Java: net.minecraft.server.packs.VanillaPackResources - the built-in pack
// whose bytes ride the classpath (the vanilla jar's assets/ + data/). The
// port's blob rides the .a: tools/embed_resources.c packs library/resources/
// into a deterministic tar + gzip at build time, the generated
// EmbeddedPackResources_data.c carries the bytes as a C array and this unit
// inflates them lazily into RAM on the first access. Every read answers from
// the process image - the pack never touches the filesystem (no --assetsRoot,
// no pack dir on disk, nothing extracted).

#include "libmatti/net/minecraft/server/packs/embedded/EmbeddedPackResources.h"

#include "libmatti/com/google/gson/Gson.h"
#include "libmatti/net/minecraft/server/packs/PackType.h"
#include "libmatti/net/minecraft/server/packs/embedded/EmbeddedPackResources_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

// ---------------------------------------------------------------------------
// The tar index (the blob is the deterministic tar the embed tool writes)
// ---------------------------------------------------------------------------

#define TAR_BLOCK 512

// Java: the member name ("./assets/matticraft/...") - the port keys the index
// by the tar member path exactly as stored (the embed tool's names).
typedef struct IndexBuild
{
    char **names;
    size_t *offsets;
    size_t *sizes;
    size_t count;
    size_t capacity;
} IndexBuild;

static void index_add(IndexBuild *index, const char *name, size_t offset, size_t size)
{
    if (index->count == index->capacity)
    {
        index->capacity = index->capacity ? index->capacity * 2 : 256;
        index->names = realloc(index->names, index->capacity * sizeof(char *));
        index->offsets = realloc(index->offsets, index->capacity * sizeof(size_t));
        index->sizes = realloc(index->sizes, index->capacity * sizeof(size_t));
        if (index->names == NULL || index->offsets == NULL || index->sizes == NULL)
            return;
    }
    index->names[index->count] = strdup(name);
    index->offsets[index->count] = offset;
    index->sizes[index->count] = size;
    if (index->names[index->count] != NULL)
        index->count++;
}

// the unsigned-octal reader the tar headers use (the size field)
static size_t read_octal(const char *field, size_t width)
{
    size_t value = 0;
    for (size_t i = 0; i < width; i++)
    {
        char c = field[i];
        if (c == '\0' || c == ' ')
            continue;
        if (c < '0' || c > '7')
            break;
        value = value * 8 + (size_t) (c - '0');
    }
    return value;
}

// the pending GNU @LongLink name (the L entry ahead of the real header)
static char *pendingLongName = NULL;

static void clear_pending_long_name(void)
{
    free(pendingLongName);
    pendingLongName = NULL;
}

// Walks the tar: indexes every member (files AND directories - the directory
// members carry the listResources structure), skipping the two end blocks.
static void index_tar(LIBMATTI_MC_EmbeddedPackResources *pack)
{
    IndexBuild index = {0};
    size_t offset = 0;
    while (offset + TAR_BLOCK <= pack->tarSize)
    {
        const unsigned char *block = pack->tarData + offset;
        // the end-of-archive marker (the zero block)
        int allZero = 1;
        for (size_t i = 0; i < TAR_BLOCK; i++)
        {
            if (block[i] != 0)
            {
                allZero = 0;
                break;
            }
        }
        if (allZero)
            break;

        char name[101];
        memcpy(name, block, 100);
        name[100] = '\0';
        unsigned int typeflag = block[156];
        size_t size = read_octal((const char *) block + 124, 12);

        // GNU: the long-name entry ahead of the real header
        if (typeflag == 'L')
        {
            clear_pending_long_name();
            pendingLongName = malloc(size + 1);
            if (pendingLongName != NULL)
            {
                memcpy(pendingLongName, block + TAR_BLOCK, size);
                pendingLongName[size] = '\0';
            }
            offset += TAR_BLOCK + ((size + TAR_BLOCK - 1) / TAR_BLOCK) * TAR_BLOCK;
            continue;
        }

        const char *finalName = pendingLongName != NULL ? pendingLongName : name;
        if (typeflag == '0' || typeflag == '5')
        {
            // Java: the pack indexes the raw member paths ("./assets/...")
            index_add(&index, finalName, offset + TAR_BLOCK, typeflag == '0' ? size : 0);
        }
        clear_pending_long_name();
        offset += TAR_BLOCK + ((size + TAR_BLOCK - 1) / TAR_BLOCK) * TAR_BLOCK;
    }

    pack->names = index.names;
    pack->offsets = index.offsets;
    pack->sizes = index.sizes;
    pack->entryCount = index.count;

    // Java: VanillaPackResourcesBuilder.withMetadataFilter - the pack.mcmeta
    // root resource (the metadata section the manager queries) rides the index
    // lookup at query time; nothing to pin here.
}

// Java: the lazy inflate - the blob is the gzip stream (RFC 1952, the bytes
// the gzip CLI writes); the output is the 60 MB tar in RAM, malloc'd once.
static int inflate_blob(LIBMATTI_MC_EmbeddedPackResources *pack)
{
    if (pack->inflated)
        return pack->tarData != NULL;
    pack->inflated = 1;

    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    // the blob is the gzip stream (RFC 1952, the deflate wrapper +16 selects)
    if (inflateInit2(&stream, 15 + 16) != Z_OK)
        return 0;
    stream.next_in = (unsigned char *) LIBMATTI_MC_EmbeddedPackResources_BLOB;
    stream.avail_in = (uInt) LIBMATTI_MC_EmbeddedPackResources_SIZE;

    size_t capacity = 1 << 20;
    pack->tarData = malloc(capacity);
    if (pack->tarData == NULL)
    {
        inflateEnd(&stream);
        return 0;
    }
    int status;
    do
    {
        if (pack->tarSize == capacity)
        {
            capacity *= 2;
            unsigned char *grown = realloc(pack->tarData, capacity);
            if (grown == NULL)
            {
                inflateEnd(&stream);
                return 0;
            }
            pack->tarData = grown;
        }
        stream.next_out = pack->tarData + pack->tarSize;
        stream.avail_out = (uInt) (capacity - pack->tarSize);
        status = inflate(&stream, Z_NO_FLUSH);
        pack->tarSize = capacity - stream.avail_out;
        if (status != Z_OK && status != Z_STREAM_END && status != Z_BUF_ERROR)
            break;
    } while (status != Z_STREAM_END);
    inflateEnd(&stream);

    index_tar(pack);
    return 1;
}

// ---------------------------------------------------------------------------
// The vTable (Java: VanillaPackResources' final methods over the bundle)
// ---------------------------------------------------------------------------

// The member lookup: exact match over the indexed names (the embed tool packs
// the sorted walk, so the index is sorted too - the linear scan is fine for
// the single-resource reads; listResources bisects over the range).
static long find_entry(const LIBMATTI_MC_EmbeddedPackResources *pack, const char *name)
{
    for (size_t i = 0; i < pack->entryCount; i++)
    {
        if (strcmp(pack->names[i], name) == 0)
            return (long) i;
    }
    return -1;
}

unsigned char *LIBMATTI_MC_EmbeddedPackResources_ReadFile(LIBMATTI_MC_EmbeddedPackResources *pack,
                                                          const char *name, size_t *outLength)
{
    *outLength = 0;
    if (pack == NULL || !inflate_blob(pack))
        return NULL;
    long index = find_entry(pack, name);
    if (index < 0)
        return NULL;
    size_t size = pack->sizes[index];
    if (size == 0)
        return NULL;
    // The NUL terminator keeps the JSON readers in-bounds (the length stays
    // the bytes' size; the terminator rides outside it like PathPackResources'
    // ReadAllBytes + 1). PNG consumers walk the length only.
    unsigned char *data = malloc(size + 1);
    if (data == NULL)
        return NULL;
    memcpy(data, pack->tarData + pack->offsets[index], size);
    data[size] = '\0';
    *outLength = size;
    return data;
}

static unsigned char *embedded_get_root_resource(LIBMATTI_MC_PackResources *self, const char *const *paths,
                                                 size_t pathCount, size_t *outLength)
{
    // Java: getRootResource walks the parts under the pack root - the port
    // joins the parts into the tar member name ("./pack.mcmeta").
    if (!inflate_blob((LIBMATTI_MC_EmbeddedPackResources *) self))
        return NULL;
    size_t total = 2;
    for (size_t i = 0; i < pathCount; i++)
        total += strlen(paths[i]) + 1;
    char *name = malloc(total);
    if (name == NULL)
        return NULL;
    size_t cursor = 0;
    name[cursor++] = '.';
    name[cursor++] = '/';
    for (size_t i = 0; i < pathCount; i++)
    {
        size_t length = strlen(paths[i]);
        memcpy(name + cursor, paths[i], length);
        cursor += length;
        if (i + 1 < pathCount)
            name[cursor++] = '/';
    }
    name[cursor] = '\0';
    unsigned char *data = LIBMATTI_MC_EmbeddedPackResources_ReadFile((LIBMATTI_MC_EmbeddedPackResources *) self,
                                                                     name, outLength);
    free(name);
    return data;
}

// Java: open(PackType, ResourceLocation) - the member under
// "./<assets|data>/<namespace>/<path>".
static unsigned char *embedded_open(LIBMATTI_MC_PackResources *self, int type, const char *namespace,
                                    const char *path, size_t *outLength)
{
    if (!inflate_blob((LIBMATTI_MC_EmbeddedPackResources *) self))
        return NULL;
    const char *directory = LIBMATTI_MC_PackType_GetDirectory((LIBMATTI_MC_PackType) type);
    if (directory == NULL)
        return NULL;
    char name[4096];
    snprintf(name, sizeof(name), "./%s/%s/%s", directory, namespace, path);
    return LIBMATTI_MC_EmbeddedPackResources_ReadFile((LIBMATTI_MC_EmbeddedPackResources *) self, name, outLength);
}

static unsigned char *embedded_open_resource(LIBMATTI_MC_PackResources *self, int type, const char *namespace,
                                             const char *path, size_t *outLength)
{
    // Java: openResource re-opens by the recorded location - the port's
    // Resource carries the namespace/path, so this is open().
    return embedded_open(self, type, namespace, path, outLength);
}

// Java: listResources(PackType, String namespace, String prefix, ResourceOutput)
// - the walk over the members under "./<dir>/<namespace>/<prefix...>". Like
// PathPackResources, the reported path is RELATIVE TO THE NAMESPACE ROOT
// ("blockstates/acacia_button.json", "textures/block/stone.png") - the
// prefix rides INSIDE the reported path (the atlas sources rely on it).
static void embedded_list_resources(LIBMATTI_MC_PackResources *self, int type, const char *namespace,
                                    const char *prefix, void *userData,
                                    void (*onFile)(void *, const char *, const char *, int, size_t))
{
    LIBMATTI_MC_EmbeddedPackResources *pack = (LIBMATTI_MC_EmbeddedPackResources *) self;
    if (!inflate_blob(pack))
        return;
    const char *directory = LIBMATTI_MC_PackType_GetDirectory((LIBMATTI_MC_PackType) type);
    if (directory == NULL)
        return;

    // the namespace root the reported paths are relative to
    char namespaceBase[2048];
    snprintf(namespaceBase, sizeof(namespaceBase), "./%s/%s", directory, namespace);
    size_t namespaceLength = strlen(namespaceBase);

    // the member prefix the walk covers (namespace base + the walk prefix)
    char base[4096];
    snprintf(base, sizeof(base), "%s", namespaceBase);
    if (prefix != NULL && prefix[0] != '\0')
    {
        size_t length = strlen(base);
        snprintf(base + length, sizeof(base) - length, "/%s", prefix);
    }
    size_t baseLength = strlen(base);

    for (size_t i = 0; i < pack->entryCount; i++)
    {
        const char *name = pack->names[i];
        if (strncmp(name, base, baseLength) != 0)
            continue;
        const char *tail = name + baseLength;
        // the prefix boundary: the next char must be '/' (a deeper entry) or
        // the walk root itself reports nothing ("block" must not match
        // "blockstates")
        if (*tail != '\0' && *tail != '/')
            continue;
        if (*tail == '\0')
            continue;
        const char *relative = name + namespaceLength + 1;
        int isDir = name[strlen(name) - 1] == '/';
        if (isDir)
            onFile(userData, namespace, relative, 0, 0);
        else
            onFile(userData, namespace, relative, 1, pack->sizes[i]);
    }
}

// Java: getNamespaces(PackType) - the first path segments under
// "./<dir>/" (the directory members make this the direct child read).
static char **embedded_get_namespaces(LIBMATTI_MC_PackResources *self, int type, size_t *outCount)
{
    LIBMATTI_MC_EmbeddedPackResources *pack = (LIBMATTI_MC_EmbeddedPackResources *) self;
    *outCount = 0;
    if (!inflate_blob(pack))
        return NULL;
    const char *directory = LIBMATTI_MC_PackType_GetDirectory((LIBMATTI_MC_PackType) type);
    if (directory == NULL)
        return NULL;

    char **names = NULL;
    size_t count = 0;
    size_t directoryLength = strlen(directory);
    for (size_t i = 0; i < pack->entryCount; i++)
    {
        const char *name = pack->names[i];
        // the member must sit directly under "./<dir>/<namespace>/..."
        if (strncmp(name, "./", 2) != 0)
            continue;
        const char *rest = name + 2;
        if (strncmp(rest, directory, directoryLength) != 0 || rest[directoryLength] != '/')
            continue;
        const char *after = rest + directoryLength + 1;
        const char *slash = strchr(after, '/');
        if (slash == NULL || slash == after)
            continue;
        size_t namespaceLength = (size_t) (slash - after);
        char namespace[256];
        if (namespaceLength >= sizeof(namespace))
            continue;
        memcpy(namespace, after, namespaceLength);
        namespace[namespaceLength] = '\0';
        int duplicate = 0;
        for (size_t j = 0; j < count; j++)
        {
            if (strcmp(names[j], namespace) == 0)
            {
                duplicate = 1;
                break;
            }
        }
        if (duplicate)
            continue;
        names = realloc(names, (count + 1) * sizeof(char *));
        if (names == NULL)
            break;
        names[count] = strdup(namespace);
        count++;
    }
    names = realloc(names, (count + 1) * sizeof(char *));
    if (names != NULL)
        names[count] = NULL;
    *outCount = count;
    return names;
}

// Java: AbstractPackResources.getMetadataSection -> getRootResource(pack.mcmeta)
static void *embedded_get_metadata_section(LIBMATTI_MC_PackResources *self,
                                           const LIBMATTI_MC_MetadataSectionSerializer *serializer, char **outError)
{
    if (serializer == NULL)
    {
        *outError = NULL;
        return NULL; // the registry never registered this serializer's name
    }
    size_t length = 0;
    unsigned char *data = LIBMATTI_MC_PackResources_GetMetadataSectionSupplierFromRoot(self, "pack.mcmeta", &length);
    if (data == NULL)
    {
        *outError = NULL;
        return NULL;
    }
    // The port parses through the Gson layer like PathPackResources does.
    LIBMATTI_GSON_Gson *gson = LIBMATTI_GSON_Gson_New();
    LIBMATTI_GSON_JsonElement *element = LIBMATTI_GSON_Gson_FromJson(gson, (const char *) data, length);
    LIBMATTI_GSON_Gson_Free(gson);
    free(data);
    if (element == NULL)
        return NULL;
    LIBMATTI_GSON_JsonElement *sectionElement = LIBMATTI_GSON_JsonElement_GetMember(element, serializer->name);
    if (sectionElement == NULL)
    {
        LIBMATTI_GSON_JsonElement_Free(element);
        return NULL;
    }
    void *section = serializer->fromJson(serializer, sectionElement, outError);
    LIBMATTI_GSON_JsonElement_Free(element);
    return section;
}

static const char *embedded_pack_id(LIBMATTI_MC_PackResources *self)
{
    LIBMATTI_MC_EmbeddedPackResources *pack = (LIBMATTI_MC_EmbeddedPackResources *) self;
    return pack->location->id;
}

static LIBMATTI_MC_PackLocationInfo *embedded_location(LIBMATTI_MC_PackResources *self)
{
    LIBMATTI_MC_EmbeddedPackResources *pack = (LIBMATTI_MC_EmbeddedPackResources *) self;
    return pack->location;
}

static void embedded_close(LIBMATTI_MC_PackResources *self)
{
    (void) self; // the blob lives in the process image; nothing to close
}

static const LIBMATTI_MC_PackResources_VTable EMBEDDED_VTABLE = {
    embedded_get_root_resource, embedded_open,           embedded_open_resource, embedded_list_resources,
    embedded_get_namespaces,    embedded_get_metadata_section, embedded_pack_id,  embedded_location,
    embedded_close,
};

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

LIBMATTI_MC_EmbeddedPackResources *LIBMATTI_MC_EmbeddedPackResources_NewWithLocation(
    LIBMATTI_MC_PackLocationInfo *location)
{
    LIBMATTI_MC_EmbeddedPackResources *pack = calloc(1, sizeof(*pack));
    if (pack == NULL)
        return NULL;
    pack->vtable = &EMBEDDED_VTABLE;
    pack->location = location;
    // Java: the vanilla pack builds eagerly; the port inflates lazily (the
    // first resource read pays the ~100 ms decompress, the boots stay fast).
    return pack;
}

LIBMATTI_MC_EmbeddedPackResources *LIBMATTI_MC_EmbeddedPackResources_New(void)
{
    LIBMATTI_MC_PackLocationInfo *location =
        LIBMATTI_MC_PackLocationInfo_New("matticraft/embedded", "Matticraft embedded resources",
                                         LIBMATTI_MC_PackSource_BuiltIn());
    if (location == NULL)
        return NULL;
    return LIBMATTI_MC_EmbeddedPackResources_NewWithLocation(location);
}

void LIBMATTI_MC_EmbeddedPackResources_Free(LIBMATTI_MC_EmbeddedPackResources *pack)
{
    if (pack == NULL)
        return;
    for (size_t i = 0; i < pack->entryCount; i++)
        free(pack->names[i]);
    free(pack->names);
    free(pack->offsets);
    free(pack->sizes);
    free(pack->tarData);
    LIBMATTI_MC_PackLocationInfo_Free(pack->location);
    free(pack);
}

size_t LIBMATTI_MC_EmbeddedPackResources_EntryCount(const LIBMATTI_MC_EmbeddedPackResources *pack)
{
    return pack != NULL ? pack->entryCount : 0;
}
