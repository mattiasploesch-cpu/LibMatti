// Port of net.minecraft.server.packs.PathPackResources.

#include "libmatti/net/minecraft/server/packs/PathPackResources.h"

#include "libmatti/com/google/gson/Gson.h"
#include "libmatti/java/nio/file/Files.h"
#include "libmatti/net/minecraft/server/packs/PackType.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Java: hidden() -> a name starting with '.' is hidden
static int is_hidden(const char *name)
{
    return name[0] == '.';
}

static char *join_path(const char *a, const char *b)
{
    size_t length = strlen(a) + strlen(b) + 2;
    char *joined = malloc(length);
    snprintf(joined, length, "%s/%s", a, b);
    return joined;
}

static char *join_path3(const char *a, const char *b, const char *c)
{
    size_t length = strlen(a) + strlen(b) + strlen(c) + 3;
    char *joined = malloc(length);
    snprintf(joined, length, "%s/%s/%s", a, b, c);
    return joined;
}

// Java: public Path getRoot()
static const char *get_root(LIBMATTI_MC_PathPackResources *self)
{
    return self->source;
}

// Java: NeoForge's path resolution: DIRECTORY mode appends the namespace parts,
// LISTFILE reads the list file instead.
static char *resolve_directory(LIBMATTI_MC_PathPackResources *self, int type, const char *namespace,
                               const char *const *parts, size_t partCount)
{
    // Java: root().resolve(type.directory).resolve(...)
    const char *directory = LIBMATTI_MC_PackType_GetDirectory(type);
    if (directory == NULL) return NULL;

    if (self->mode == LIBMATTI_MC_PathPackResources_MODE_DIRECTORY)
    {
        char *result = join_path3(get_root(self), directory, namespace != NULL ? namespace : "");
        for (size_t i = 0; i < partCount; i++)
        {
            char *next = join_path(result, parts[i]);
            free(result);
            result = next;
        }
        return result;
    }

    // LISTFILE: the namespaces live in <root>/<type>/listfile
    char *list = join_path3(get_root(self), directory, "listfile");
    return list;
}

static unsigned char *read_file(const char *path, size_t *outLength)
{
    if (path == NULL) return NULL;
    return LIBMATTI_JNF_Files_ReadAllBytes(path, outLength);
}

static int file_exists(const char *path)
{
    if (path == NULL) return 0;
    return LIBMATTI_JNF_Files_Exists(path);
}

// ---------------------------------------------------------------------------
// VTable implementation
// ---------------------------------------------------------------------------

static unsigned char *path_get_root_resource(LIBMATTI_MC_PackResources *pack, const char *const *paths,
                                             size_t pathCount, size_t *outLength)
{
    LIBMATTI_MC_PathPackResources *self = (LIBMATTI_MC_PathPackResources *) pack;
    *outLength = 0;
    // Java: PathPackResources.getRootResource walks the parts under the root
    char *path = strdup(get_root(self));
    for (size_t i = 0; i < pathCount; i++)
    {
        char *next = join_path(path, paths[i]);
        free(path);
        path = next;
    }
    unsigned char *data = read_file(path, outLength);
    free(path);
    return data;
}

static unsigned char *path_open(LIBMATTI_MC_PackResources *pack, int type, const char *namespace, const char *path,
                                size_t *outLength)
{
    LIBMATTI_MC_PathPackResources *self = (LIBMATTI_MC_PathPackResources *) pack;
    *outLength = 0;
    const char *parts[1] = {path};
    char *full = resolve_directory(self, type, namespace, parts, 1);
    unsigned char *data = read_file(full, outLength);
    free(full);
    return data;
}

static unsigned char *path_open_resource(LIBMATTI_MC_PackResources *pack, int type, const char *namespace,
                                         const char *path, size_t *outLength)
{
    return path_open(pack, type, namespace, path, outLength);
}

// Java: ResourceOutput callback for every entry
static void path_list_resources(LIBMATTI_MC_PackResources *pack, int type, const char *namespace, const char *prefix,
                                void *userData,
                                void (*onFile)(void *, const char *, const char *, int, size_t))
{
    LIBMATTI_MC_PathPackResources *self = (LIBMATTI_MC_PathPackResources *) pack;
    const char *directory = LIBMATTI_MC_PackType_GetDirectory(type);
    if (directory == NULL) return;

    // Java: resolve the directory <root>/<type>/<namespace>/<prefix...>
    char *base = join_path3(get_root(self), directory, namespace);
    char *walk = strdup(base); // the walk cursor (freed separately from base)
    if (prefix != NULL && prefix[0] != '\0')
    {
        // The prefix can be hierarchical ("textures/block")
        const char *p = prefix;
        while (*p != '\0')
        {
            const char *slash = strchr(p, '/');
            size_t segment = slash != NULL ? (size_t) (slash - p) : strlen(p);
            char *part = malloc(segment + 1);
            memcpy(part, p, segment);
            part[segment] = '\0';
            char *next = join_path(walk, part);
            free(walk);
            walk = next;
            free(part);
            if (slash == NULL) break;
            p = slash + 1;
        }
    }

    DIR *dir = opendir(walk);
    if (dir == NULL)
    {
        free(base);
        free(walk);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (is_hidden(entry->d_name)) continue;
        char *full = join_path(walk, entry->d_name);
        struct stat st;
        if (stat(full, &st) != 0)
        {
            free(full);
            continue;
        }
        if (S_ISDIR(st.st_mode))
        {
            // Java: directories are reported with a trailing slash, the path
            // relative to the namespace directory (prefix is only the walk root).
            char *relative = full + strlen(base) + 1;
            size_t locationLength = strlen(relative) + 2;
            char *location = malloc(locationLength);
            snprintf(location, locationLength, "%s/", relative);
            onFile(userData, namespace, location, 0, 0);
            free(location);
        }
        else if (S_ISREG(st.st_mode))
        {
            char *relative = full + strlen(base) + 1;
            char *location = strdup(relative);
            size_t length = (size_t) st.st_size;
            onFile(userData, namespace, location, 1, length);
            free(location);
        }
        free(full);
    }
    closedir(dir);
    free(base);
    free(walk);
}

static char **path_get_namespaces(LIBMATTI_MC_PackResources *pack, int type, size_t *outCount)
{
    LIBMATTI_MC_PathPackResources *self = (LIBMATTI_MC_PathPackResources *) pack;
    *outCount = 0;
    const char *directory = LIBMATTI_MC_PackType_GetDirectory(type);
    if (directory == NULL) return NULL;

    char **names = NULL;
    size_t count = 0;

    if (self->mode == LIBMATTI_MC_PathPackResources_MODE_DIRECTORY)
    {
        // Java: walk <root>/<type>/ and collect non-hidden directories
        char *base = join_path(get_root(self), directory);
        DIR *dir = opendir(base);
        if (dir != NULL)
        {
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL)
            {
                if (is_hidden(entry->d_name)) continue;
                char *full = join_path(base, entry->d_name);
                struct stat st;
                int isDir = stat(full, &st) == 0 && S_ISDIR(st.st_mode);
                free(full);
                if (!isDir) continue;
                names = realloc(names, (count + 1) * sizeof(char *));
                names[count] = strdup(entry->d_name);
                count++;
            }
            closedir(dir);
        }
        free(base);
    }
    else
    {
        // LISTFILE: each line is "<namespace>/<path>"; the namespaces are the first segments
        char *list = join_path3(get_root(self), directory, "listfile");
        if (file_exists(list))
        {
            size_t length = 0;
            unsigned char *data = read_file(list, &length);
            if (data != NULL)
            {
                const char *cursor = (const char *) data;
                const char *end = (const char *) data + length;
                while (cursor < end)
                {
                    const char *lineEnd = memchr(cursor, '\n', (size_t) (end - cursor));
                    size_t lineLength = lineEnd != NULL ? (size_t) (lineEnd - cursor) : (size_t) (end - cursor);
                    const char *slash = memchr(cursor, '/', lineLength);
                    if (slash != NULL)
                    {
                        size_t namespaceLength = (size_t) (slash - cursor);
                        char *namespace = malloc(namespaceLength + 1);
                        memcpy(namespace, cursor, namespaceLength);
                        namespace[namespaceLength] = '\0';
                        // deduplicate
                        int duplicate = 0;
                        for (size_t i = 0; i < count; i++)
                        {
                            if (strcmp(names[i], namespace) == 0)
                            {
                                duplicate = 1;
                                free(namespace);
                                break;
                            }
                        }
                        if (!duplicate)
                        {
                            names = realloc(names, (count + 1) * sizeof(char *));
                            names[count] = namespace;
                            count++;
                        }
                        else if (duplicate)
                        {
                            // namespace already owned by the array
                        }
                    }
                    if (lineEnd == NULL) break;
                    cursor = lineEnd + 1;
                }
                free(data);
            }
        }
        free(list);
    }

    names = realloc(names, (count + 1) * sizeof(char *));
    names[count] = NULL;
    *outCount = count;
    return names;
}

static void *path_get_metadata_section(LIBMATTI_MC_PackResources *pack,
                                       const LIBMATTI_MC_MetadataSectionSerializer *serializer, char **outError)
{
    // Java: AbstractPackResources.getMetadataSection -> getRootResource("pack.mcmeta")
    size_t length = 0;
    unsigned char *data = LIBMATTI_MC_PackResources_GetMetadataSectionSupplierFromRoot(pack, "pack.mcmeta", &length);
    if (data == NULL)
    {
        *outError = NULL;
        return NULL;
    }
    // The port parses lazily; the pack.mcmeta JSON lives in MetadataSection parsing.
    // Caller chain: FilePackResources/PathPackResources both go through this helper.
    LIBMATTI_GSON_Gson *gson = LIBMATTI_GSON_Gson_New();
    LIBMATTI_GSON_JsonElement *element = LIBMATTI_GSON_Gson_FromJson(gson, (const char *) data, length);
    LIBMATTI_GSON_Gson_Free(gson);
    free(data);
    if (element == NULL) return NULL;
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

static const char *path_pack_id(LIBMATTI_MC_PackResources *pack)
{
    LIBMATTI_MC_PathPackResources *self = (LIBMATTI_MC_PathPackResources *) pack;
    return self->location->id;
}

static LIBMATTI_MC_PackLocationInfo *path_location(LIBMATTI_MC_PackResources *pack)
{
    LIBMATTI_MC_PathPackResources *self = (LIBMATTI_MC_PathPackResources *) pack;
    return self->location;
}

static void path_close(LIBMATTI_MC_PackResources *pack)
{
    LIBMATTI_MC_PathPackResources *self = (LIBMATTI_MC_PathPackResources *) pack;
    (void) self;
}

static const LIBMATTI_MC_PackResources_VTable PATH_VTABLE = {
    path_get_root_resource, path_open,           path_open_resource, path_list_resources,
    path_get_namespaces,    path_get_metadata_section, path_pack_id,  path_location,
    path_close,
};

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

LIBMATTI_MC_PathPackResources *LIBMATTI_MC_PathPackResources_NewWithMode(LIBMATTI_MC_PackLocationInfo *location,
                                                                         const char *source, int isBuiltin,
                                                                         LIBMATTI_MC_PathPackResources_Mode mode)
{
    LIBMATTI_MC_PathPackResources *self = calloc(1, sizeof(LIBMATTI_MC_PathPackResources));
    self->vtable = &PATH_VTABLE;
    self->location = location;
    self->source = strdup(source);
    self->isBuiltin = isBuiltin;
    self->mode = mode;
    return self;
}

LIBMATTI_MC_PathPackResources *LIBMATTI_MC_PathPackResources_New(LIBMATTI_MC_PackLocationInfo *location,
                                                                 const char *source, int isBuiltin)
{
    return LIBMATTI_MC_PathPackResources_NewWithMode(location, source, isBuiltin,
                                                     LIBMATTI_MC_PathPackResources_MODE_DIRECTORY);
}

void LIBMATTI_MC_PathPackResources_Free(LIBMATTI_MC_PathPackResources *pack)
{
    free(pack->source);
    LIBMATTI_MC_PackLocationInfo_Free(pack->location);
    free(pack);
}
