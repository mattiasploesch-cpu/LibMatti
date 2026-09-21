// Port of net.minecraft.server.packs.FilePackResources (the ZIP pack).

#include "libmatti/net/minecraft/server/packs/FilePackResources.h"

#include <stdio.h>

#include "libmatti/com/google/gson/Gson.h"
#include "libmatti/java/nio/file/Files.h"
#include "libmatti/net/minecraft/server/packs/PackType.h"

#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// FileBackup (Java: an AutoCloseable holding the ZipFile)
// ---------------------------------------------------------------------------

static LIBMATTI_MC_FilePackResources_FileBackup *backup_open(const char *source)
{
    LIBMATTI_JU_ZipFile *zip = LIBMATTI_JU_ZipFile_Open(source);
    if (zip == NULL) return NULL;
    LIBMATTI_MC_FilePackResources_FileBackup *backup = malloc(sizeof(LIBMATTI_MC_FilePackResources_FileBackup));
    backup->zip = zip;
    backup->opened = 1;
    return backup;
}

static void backup_free(LIBMATTI_MC_FilePackResources_FileBackup *backup)
{
    if (backup == NULL) return;
    if (backup->opened) LIBMATTI_JU_ZipFile_Free(backup->zip);
    free(backup);
}

// ---------------------------------------------------------------------------
// Path helpers (Java: the "prefix + relative path" resolution)
// ---------------------------------------------------------------------------

// Java: private String getPath(PackType type, ResourceLocation location)
static char *get_pack_path(int type, const char *namespace, const char *path)
{
    const char *directory = LIBMATTI_MC_PackType_GetDirectory(type);
    size_t length = strlen(directory) + strlen(namespace) + strlen(path) + 3;
    char *result = malloc(length);
    snprintf(result, length, "%s/%s/%s", directory, namespace, path);
    return result;
}

// Java: private ZipFile get() / ensureOpen
static LIBMATTI_JU_ZipFile *ensure_zip(LIBMATTI_MC_FilePackResources *self)
{
    if (self->backup == NULL) self->backup = backup_open(self->source);
    return self->backup != NULL ? self->backup->zip : NULL;
}

// ---------------------------------------------------------------------------
// VTable implementation
// ---------------------------------------------------------------------------

static unsigned char *file_get_root_resource(LIBMATTI_MC_PackResources *pack, const char *const *paths,
                                             size_t pathCount, size_t *outLength)
{
    LIBMATTI_MC_FilePackResources *self = (LIBMATTI_MC_FilePackResources *) pack;
    *outLength = 0;
    LIBMATTI_JU_ZipFile *zip = ensure_zip(self);
    if (zip == NULL) return NULL;

    // Java: getRootResource joins the parts with '/' ("pack.mcmeta" is a single part)
    size_t joinedLength = 1;
    for (size_t i = 0; i < pathCount; i++) joinedLength += strlen(paths[i]) + 1;
    char *joined = malloc(joinedLength);
    joined[0] = '\0';
    for (size_t i = 0; i < pathCount; i++)
    {
        if (i > 0) strcat(joined, "/");
        strcat(joined, paths[i]);
    }

    const LIBMATTI_JU_ZipEntry *entry = LIBMATTI_JU_ZipFile_GetEntry(zip, joined);
    free(joined);
    if (entry == NULL || entry->isDirectory) return NULL;
    return LIBMATTI_JU_ZipFile_Read(zip, entry, outLength);
}

static unsigned char *file_open(LIBMATTI_MC_PackResources *pack, int type, const char *namespace, const char *path,
                                size_t *outLength)
{
    LIBMATTI_MC_FilePackResources *self = (LIBMATTI_MC_FilePackResources *) pack;
    *outLength = 0;
    LIBMATTI_JU_ZipFile *zip = ensure_zip(self);
    if (zip == NULL) return NULL;

    char *entryName = get_pack_path(type, namespace, path);
    const LIBMATTI_JU_ZipEntry *entry = LIBMATTI_JU_ZipFile_GetEntry(zip, entryName);
    free(entryName);
    if (entry == NULL || entry->isDirectory) return NULL;
    return LIBMATTI_JU_ZipFile_Read(zip, entry, outLength);
}

static unsigned char *file_open_resource(LIBMATTI_MC_PackResources *pack, int type, const char *namespace,
                                         const char *path, size_t *outLength)
{
    return file_open(pack, type, namespace, path, outLength);
}

static void file_list_resources(LIBMATTI_MC_PackResources *pack, int type, const char *namespace, const char *prefix,
                                void *userData,
                                void (*onFile)(void *, const char *, const char *, int, size_t))
{
    LIBMATTI_MC_FilePackResources *self = (LIBMATTI_MC_FilePackResources *) pack;
    LIBMATTI_JU_ZipFile *zip = ensure_zip(self);
    if (zip == NULL) return;

    // Java: the search prefix is "<directory>/<namespace>/<prefix>/"
    const char *directory = LIBMATTI_MC_PackType_GetDirectory(type);
    if (directory == NULL) return;
    size_t searchLength = strlen(directory) + strlen(namespace) + strlen(prefix) + 3;
    char *search = malloc(searchLength + 1);
    snprintf(search, searchLength + 1, "%s/%s/%s/", directory, namespace, prefix);

    size_t searchLen = strlen(search);
    size_t count = LIBMATTI_JU_ZipFile_EntryCount(zip);
    for (size_t i = 0; i < count; i++)
    {
        const LIBMATTI_JU_ZipEntry *entry = LIBMATTI_JU_ZipFile_EntryAt(zip, i);
        if (strncmp(entry->name, search, searchLen) != 0) continue;
        const char *relative = entry->name + searchLen;
        if (relative[0] == '\0') continue;
        char *location = malloc(strlen(prefix) + strlen(relative) + 1);
        snprintf(location, strlen(prefix) + strlen(relative) + 1, "%s%s", prefix, relative);
        // Java: the output receives the entry; a directory entry ends with '/'
        int isDirectory = entry->isDirectory;
        onFile(userData, namespace, location, isDirectory ? 0 : 1, entry->uncompressedSize);
        free(location);
    }
    free(search);
}

static char **file_get_namespaces(LIBMATTI_MC_PackResources *pack, int type, size_t *outCount)
{
    LIBMATTI_MC_FilePackResources *self = (LIBMATTI_MC_FilePackResources *) pack;
    *outCount = 0;
    LIBMATTI_JU_ZipFile *zip = ensure_zip(self);
    if (zip == NULL) return NULL;

    const char *directory = LIBMATTI_MC_PackType_GetDirectory(type);
    if (directory == NULL) return NULL;
    size_t directoryLength = strlen(directory);

    char **names = NULL;
    size_t count = 0;
    size_t capacity = 0;

    size_t entryCount = LIBMATTI_JU_ZipFile_EntryCount(zip);
    for (size_t i = 0; i < entryCount; i++)
    {
        const LIBMATTI_JU_ZipEntry *entry = LIBMATTI_JU_ZipFile_EntryAt(zip, i);
        // Java: entries "<dir>/<namespace>/..." contribute the namespace
        if (strncmp(entry->name, directory, directoryLength) != 0) continue;
        const char *rest = entry->name + directoryLength;
        if (rest[0] != '/') continue;
        rest++;
        const char *slash = strchr(rest, '/');
        if (slash == NULL) continue;
        size_t namespaceLength = (size_t) (slash - rest);
        if (namespaceLength == 0) continue;

        char *namespace = malloc(namespaceLength + 1);
        memcpy(namespace, rest, namespaceLength);
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
        {
            free(namespace);
            continue;
        }
        if (count == capacity)
        {
            capacity = capacity == 0 ? 8 : capacity * 2;
            names = realloc(names, capacity * sizeof(char *));
        }
        names[count] = namespace;
        count++;
    }

    names = realloc(names, (count + 1) * sizeof(char *));
    names[count] = NULL;
    *outCount = count;
    return names;
}

static void *file_get_metadata_section(LIBMATTI_MC_PackResources *pack,
                                       const LIBMATTI_MC_MetadataSectionSerializer *serializer, char **outError)
{
    size_t length = 0;
    unsigned char *data = LIBMATTI_MC_PackResources_GetMetadataSectionSupplierFromRoot(pack, "pack.mcmeta", &length);
    if (data == NULL)
    {
        *outError = NULL;
        return NULL;
    }
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

static const char *file_pack_id(LIBMATTI_MC_PackResources *pack)
{
    LIBMATTI_MC_FilePackResources *self = (LIBMATTI_MC_FilePackResources *) pack;
    return self->location->id;
}

static LIBMATTI_MC_PackLocationInfo *file_location(LIBMATTI_MC_PackResources *pack)
{
    LIBMATTI_MC_FilePackResources *self = (LIBMATTI_MC_FilePackResources *) pack;
    return self->location;
}

static void file_close(LIBMATTI_MC_PackResources *pack)
{
    LIBMATTI_MC_FilePackResources *self = (LIBMATTI_MC_FilePackResources *) pack;
    backup_free(self->backup);
    self->backup = NULL;
}

static const LIBMATTI_MC_PackResources_VTable FILE_VTABLE = {
    file_get_root_resource, file_open,           file_open_resource, file_list_resources,
    file_get_namespaces,    file_get_metadata_section, file_pack_id,  file_location,
    file_close,
};

LIBMATTI_MC_FilePackResources *LIBMATTI_MC_FilePackResources_New(LIBMATTI_MC_PackLocationInfo *location,
                                                                 const char *source, int isBuiltin)
{
    LIBMATTI_MC_FilePackResources *self = calloc(1, sizeof(LIBMATTI_MC_FilePackResources));
    self->vtable = &FILE_VTABLE;
    self->location = location;
    self->source = strdup(source);
    self->isBuiltin = isBuiltin;
    self->backup = NULL;
    return self;
}

void LIBMATTI_MC_FilePackResources_Free(LIBMATTI_MC_FilePackResources *pack)
{
    file_close((LIBMATTI_MC_PackResources *) pack);
    free(pack->source);
    LIBMATTI_MC_PackLocationInfo_Free(pack->location);
    free(pack);
}
