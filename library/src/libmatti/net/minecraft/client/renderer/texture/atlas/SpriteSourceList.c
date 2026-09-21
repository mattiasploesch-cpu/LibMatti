// Implementation of the atlas definition pipeline: the atlases/<name>.json
// "sources" array parses into the SpriteSource structs; Run() applies them
// exactly like SpriteSourceList.list: last writer wins, filter removes, the
// missing sprite is always appended first (Java: builder.add(p -> create())).

#include "libmatti/net/minecraft/client/renderer/texture/atlas/SpriteSourceList.h"

#include "libmatti/com/google/gson/Gson.h"
#include "libmatti/com/google/gson/JsonElement.h"
#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/minecraft/client/resources/metadata/TextureMetadataSections.h"
#include "libmatti/net/minecraft/client/renderer/texture/MissingTextureAtlasSprite.h"
#include "libmatti/net/minecraft/util/Mth.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOGGER() LIBMATTI_ML_LogManager_GetLogger()

// The DirectoryLister needs a collected list of (namespace, path) resources
// under a prefix - the ResourceOutput callback fills it.
typedef struct LIBMATTI_MC_ListedResource
{
    char *path;
} LIBMATTI_MC_ListedResource;

typedef struct DirListContext
{
    LIBMATTI_MC_ListedResource **outList;
    size_t *outCount;
    const char *filterPrefix;
} DirListContext;

static void dir_list_callback(void *userData, const char *ns, const char *path, int hasResource, size_t resourceLength)
{
    (void) ns;
    (void) hasResource;
    (void) resourceLength;
    DirListContext *context = userData;
    if (strncmp(path, context->filterPrefix, strlen(context->filterPrefix)) != 0)
        return;
    size_t count = *context->outCount;
    *context->outList = realloc(*context->outList, (count + 1) * sizeof(LIBMATTI_MC_ListedResource));
    (*context->outList)[count].path = strdup(path);
    (*context->outCount)++;
}

void LIBMATTI_MC_SpriteSource_Free(LIBMATTI_MC_SpriteSource *source)
{
    if (source == NULL)
        return;
    LIBMATTI_MC_Identifier_Free(source->resourceId);
    LIBMATTI_MC_Identifier_Free(source->spriteId);
    free(source->sourcePath);
    free(source->idPrefix);
    free(source->filterPattern);
    free(source);
}

// Java: Identifier.CODEC.fieldOf(...) - the port parses the string form
static LIBMATTI_MC_Identifier *parse_identifier_member(const LIBMATTI_GSON_JsonElement *object, const char *name)
{
    LIBMATTI_GSON_JsonElement *member = LIBMATTI_GSON_JsonElement_GetMember(object, name);
    if (member == NULL)
        return NULL;
    char *text = LIBMATTI_GSON_JsonElement_GetAsString(member);
    if (text == NULL)
        return NULL;
    LIBMATTI_MC_Identifier *identifier = LIBMATTI_MC_Identifier_TryParse(text);
    free(text);
    return identifier;
}

// Java: SpriteSources.FILE_CODEC - the "sources" list with the "type" dispatch
static LIBMATTI_MC_SpriteSource *parse_source(const LIBMATTI_GSON_JsonElement *object)
{
    LIBMATTI_MC_SpriteSource *source = calloc(1, sizeof(LIBMATTI_MC_SpriteSource));
    char *type = LIBMATTI_GSON_JsonElement_GetAsString(LIBMATTI_GSON_JsonElement_GetMember(object, "type"));

    if (type != NULL && strcmp(type, "minecraft:single") == 0)
    {
        source->kind = LIBMATTI_MC_SpriteSource_SINGLE_FILE;
        source->resourceId = parse_identifier_member(object, "resource");
        source->spriteId = parse_identifier_member(object, "sprite");
    }
    else if (type != NULL && strcmp(type, "minecraft:directory") == 0)
    {
        source->kind = LIBMATTI_MC_SpriteSource_DIRECTORY_LISTER;
        LIBMATTI_GSON_JsonElement *path = LIBMATTI_GSON_JsonElement_GetMember(object, "source");
        LIBMATTI_GSON_JsonElement *prefix = LIBMATTI_GSON_JsonElement_GetMember(object, "prefix");
        if (path != NULL)
            source->sourcePath = LIBMATTI_GSON_JsonElement_GetAsString(path);
        if (prefix != NULL)
            source->idPrefix = LIBMATTI_GSON_JsonElement_GetAsString(prefix);
    }
    else if (type != NULL && strcmp(type, "minecraft:filter") == 0)
    {
        source->kind = LIBMATTI_MC_SpriteSource_SOURCE_FILTER;
        LIBMATTI_GSON_JsonElement *pattern = LIBMATTI_GSON_JsonElement_GetMember(object, "pattern");
        if (pattern != NULL && LIBMATTI_GSON_JsonElement_IsJsonObject(pattern))
        {
            // Java: IdentifierPattern CODEC - the namespace/path globs
            LIBMATTI_GSON_JsonElement *ns = LIBMATTI_GSON_JsonElement_GetMember(pattern, "namespace");
            LIBMATTI_GSON_JsonElement *p = LIBMATTI_GSON_JsonElement_GetMember(pattern, "path");
            char *nsText = ns != NULL ? LIBMATTI_GSON_JsonElement_GetAsString(ns) : NULL;
            char *pText = p != NULL ? LIBMATTI_GSON_JsonElement_GetAsString(p) : NULL;
            size_t size = (nsText ? strlen(nsText) : 0) + (pText ? strlen(pText) : 0) + 4;
            source->filterPattern = malloc(size);
            snprintf(source->filterPattern, size, "%s:%s", nsText ? nsText : "*", pText ? pText : "*");
            free(nsText);
            free(pText);
        }
    }
    else
    {
        free(source);
        free(type);
        return NULL;
    }
    free(type);
    return source;
}

// Java: SpriteSourceList.load(ResourceManager, atlasId)
LIBMATTI_MC_SpriteSource **LIBMATTI_MC_SpriteSourceList_Load(
    const LIBMATTI_MC_MultiPackResourceManager *resourceManager, const LIBMATTI_MC_Identifier *atlasId,
    size_t *outCount)
{
    // Java: ATLAS_INFO_CONVERTER.idToFile(atlasId) - "atlases/" + path + ".json"
    char path[512];
    snprintf(path, sizeof(path), "atlases/%s.json", atlasId->path);

    // Java: getResourceStack(identifier) looks the atlas definition up in every
    // namespace (the FileToIdConverter walks them all); the port merges the
    // per-namespace stacks in pack order like Java's getResourceStack does.
    size_t namespaceCount = 0;
    char **namespaces = LIBMATTI_MC_MultiPackResourceManager_GetNamespaces(resourceManager, &namespaceCount);

    LIBMATTI_MC_SpriteSource **sources = NULL;
    size_t sourceCount = 0;
    size_t capacity = 0;

    for (size_t ns = 0; ns < namespaceCount; ns++)
    {
    size_t stackCount = 0;
    LIBMATTI_MC_Resource **stack =
        LIBMATTI_MC_MultiPackResourceManager_GetResourceStack(resourceManager, namespaces[ns], path, &stackCount);

    for (size_t i = 0; i < stackCount; i++)
    {
        size_t length = 0;
        unsigned char *bytes = LIBMATTI_MC_Resource_Open(stack[i], &length);
        if (bytes == NULL)
            continue;

        LIBMATTI_GSON_Gson *gson = LIBMATTI_GSON_Gson_New();
        LIBMATTI_GSON_JsonElement *root = LIBMATTI_GSON_Gson_FromJson(gson, (const char *) bytes, length);
        // Resource_Open hands out the owning pointer (no copy) - the bytes
        // stay with the resource (freed by Resource_Free below).
        if (root == NULL || !LIBMATTI_GSON_JsonElement_IsJsonObject(root))
        {
            LIBMATTI_GSON_JsonElement_Free(root);
            LIBMATTI_GSON_Gson_Free(gson);
            continue;
        }

        LIBMATTI_GSON_JsonElement *list = LIBMATTI_GSON_JsonElement_GetMember(root, "sources");
        if (list != NULL && LIBMATTI_GSON_JsonElement_IsJsonArray(list))
        {
            size_t elementCount = LIBMATTI_GSON_JsonElement_ElementCount(list);
            for (size_t e = 0; e < elementCount; e++)
            {
                LIBMATTI_GSON_JsonElement *element = LIBMATTI_GSON_JsonElement_ElementAt(list, e);
                LIBMATTI_MC_SpriteSource *source = parse_source(element);
                if (source != NULL)
                {
                    if (sourceCount == capacity)
                    {
                        capacity = capacity == 0 ? 8 : capacity * 2;
                        sources = realloc(sources, capacity * sizeof(LIBMATTI_MC_SpriteSource *));
                    }
                    sources[sourceCount++] = source;
                }
            }
        }
        LIBMATTI_GSON_JsonElement_Free(root);
        LIBMATTI_GSON_Gson_Free(gson);
    }

    for (size_t i = 0; i < stackCount; i++)
        LIBMATTI_MC_Resource_Free(stack[i]);
    free(stack);
    }

    for (size_t i = 0; i < namespaceCount; i++)
        free(namespaces[i]);
    free(namespaces);

    *outCount = sourceCount;
    return sources;
}

void LIBMATTI_MC_SpriteSourceList_Free(LIBMATTI_MC_SpriteSource **sources, size_t count)
{
    for (size_t i = 0; i < count; i++)
        LIBMATTI_MC_SpriteSource_Free(sources[i]);
    free(sources);
}

// Java: the SpriteSource.Output map - last add wins (put replaces), filter removes
typedef struct SpriteCollector
{
    LIBMATTI_MC_SpriteResourceEntry *entries;
    size_t count;
    size_t capacity;
} SpriteCollector;

static int identifier_matches_pattern(const LIBMATTI_MC_Identifier *identifier, const char *pattern)
{
    // The port's IdentifierPattern: "ns:path" with * wildcards per segment.
    const char *colon = strchr(pattern, ':');
    if (colon == NULL)
        return 1;
    size_t nsLength = (size_t) (colon - pattern);
    const char *pathPattern = colon + 1;
    if (strncmp(pattern, "*", nsLength) != 0 && strncmp(pattern, identifier->namespace, nsLength) != 0)
        return 0;
    if (strcmp(pathPattern, "*") == 0)
        return 1;
    // Java: the path regex matches the whole path; the port keeps prefix/suffix globs
    size_t patternLength = strlen(pathPattern);
    size_t pathLength = strlen(identifier->path);
    const char *star = strchr(pathPattern, '*');
    if (star == NULL)
        return strcmp(identifier->path, pathPattern) == 0;
    size_t prefixLength = (size_t) (star - pathPattern);
    size_t suffixLength = patternLength - prefixLength - 1;
    if (pathLength < prefixLength + suffixLength)
        return 0;
    return strncmp(identifier->path, pathPattern, prefixLength) == 0
           && strcmp(identifier->path + pathLength - suffixLength, star + 1) == 0;
}

static void collector_add(SpriteCollector *collector, LIBMATTI_MC_Identifier *spriteId, LIBMATTI_MC_Resource *resource)
{
    char *key = malloc(strlen(spriteId->namespace) + 1 + strlen(spriteId->path) + 1);
    sprintf(key, "%s:%s", spriteId->namespace, spriteId->path);
    for (size_t i = 0; i < collector->count; i++)
    {
        char existing[512];
        snprintf(existing, sizeof(existing), "%s:%s", collector->entries[i].spriteId->namespace,
                 collector->entries[i].spriteId->path);
        if (strcmp(existing, key) == 0)
        {
            // Java: the previous loader is discarded (closed)
            LIBMATTI_MC_Resource_Free(collector->entries[i].resource);
            collector->entries[i].resource = resource;
            free(key);
            return;
        }
    }
    free(key);
    if (collector->count == collector->capacity)
    {
        collector->capacity = collector->capacity == 0 ? 16 : collector->capacity * 2;
        collector->entries = realloc(collector->entries, collector->capacity * sizeof(LIBMATTI_MC_SpriteResourceEntry));
    }
    collector->entries[collector->count].spriteId = spriteId;
    collector->entries[collector->count].resource = resource;
    collector->count++;
}

// Java: SpriteSourceList.list(ResourceManager)
LIBMATTI_MC_SpriteResourceEntry *LIBMATTI_MC_SpriteSourceList_Run(
    LIBMATTI_MC_SpriteSource **sources, size_t sourceCount,
    const LIBMATTI_MC_MultiPackResourceManager *resourceManager, size_t *outCount)
{
    SpriteCollector collector = {NULL, 0, 0};

    for (size_t i = 0; i < sourceCount; i++)
    {
        LIBMATTI_MC_SpriteSource *source = sources[i];
        size_t namespaceCount = 0;
        char **namespacesList = LIBMATTI_MC_MultiPackResourceManager_GetNamespaces(resourceManager, &namespaceCount);
        switch (source->kind)
        {
        case LIBMATTI_MC_SpriteSource_SINGLE_FILE:
        {
            // Java: identifier = TEXTURE_ID_CONVERTER.idToFile(resourceId)
            char path[512];
            snprintf(path, sizeof(path), "textures/%s.png", source->resourceId->path);
            LIBMATTI_MC_Resource *resource =
                LIBMATTI_MC_MultiPackResourceManager_GetResource(resourceManager, source->resourceId->namespace, path);
            if (resource != NULL)
            {
                // Java: spriteId.orElse(resourceId)
                LIBMATTI_MC_Identifier *spriteId = source->spriteId != NULL
                                                       ? LIBMATTI_MC_Identifier_FromNamespaceAndPath(
                                                             source->spriteId->namespace, source->spriteId->path)
                                                       : LIBMATTI_MC_Identifier_FromNamespaceAndPath(
                                                             source->resourceId->namespace, source->resourceId->path);
                collector_add(&collector, spriteId, resource);
            }
            else
            {
                // Java: LOGGER.warn("Missing sprite: {}", identifier)
                char message[600];
                snprintf(message, sizeof(message), "Missing sprite: %s:%s", source->resourceId->namespace, path);
                LIBMATTI_ML_Logger_Warn(LOGGER(), NULL, message);
            }
            break;
        }
        case LIBMATTI_MC_SpriteSource_DIRECTORY_LISTER:
        {
            // Java: FileToIdConverter("textures/" + sourcePath, ".png").listMatchingResources
            // walks every namespace and fires for the .png files below the prefix.
            typedef struct DirCollector
            {
                SpriteCollector *collector;
                const LIBMATTI_MC_MultiPackResourceManager *resourceManager;
                const char *idPrefix;
                const char *dirPrefix; // "textures/<sourcePath>/"
            } DirCollector;

            char dirPrefix[512];
            snprintf(dirPrefix, sizeof(dirPrefix), "textures/%s/", source->sourcePath);
            DirCollector dirCollector = {&collector, resourceManager,
                                         source->idPrefix != NULL ? source->idPrefix : "", dirPrefix};

            for (size_t ns = 0; ns < namespaceCount; ns++)
            {
                // The callback collects the file list per namespace.
                char searchPrefix[512];
                snprintf(searchPrefix, sizeof(searchPrefix), "textures/%s", source->sourcePath);

                // Java: listMatchingResources lists every resource under the
                // prefix with the .png extension - the port filters in the callback.
                size_t foundCount = 0;
                char **paths = NULL;
                LIBMATTI_MC_ResourceOutput onFile =
                    (LIBMATTI_MC_ResourceOutput)(void (*)(void *, const char *, const char *, int, size_t)) NULL;
                (void) onFile;

                // The port lists through the manager and collects ns/path pairs.
                LIBMATTI_MC_ListedResource *listed = NULL;
                DirListContext context = {&listed, &foundCount, searchPrefix};
                LIBMATTI_MC_MultiPackResourceManager_ListResources(resourceManager, searchPrefix, &context,
                                                                   dir_list_callback);
                (void) paths;

                for (size_t f = 0; f < foundCount; f++)
                {
                    const char *path = listed[f].path;
                    size_t dirPrefixLength = strlen(dirCollector.dirPrefix);
                    size_t pathLength = strlen(path);
                    if (pathLength <= dirPrefixLength + 4
                        || strncmp(path, dirCollector.dirPrefix, dirPrefixLength) != 0
                        || strcmp(path + pathLength - 4, ".png") != 0)
                        continue;

                    // Java: fileToId(location).withPrefix(idPrefix)
                    char spritePath[512];
                    snprintf(spritePath, sizeof(spritePath), "%s%.*s", dirCollector.idPrefix,
                             (int) (pathLength - dirPrefixLength - 4), path + dirPrefixLength);

                    LIBMATTI_MC_Resource *resource = LIBMATTI_MC_MultiPackResourceManager_GetResource(
                        resourceManager, namespacesList[ns], path);
                    if (resource == NULL)
                        continue;

                    LIBMATTI_MC_Identifier *spriteId =
                        LIBMATTI_MC_Identifier_FromNamespaceAndPath(namespacesList[ns], spritePath);
                    collector_add(&collector, spriteId, resource);
                }
                free(listed);
                listed = NULL;
                foundCount = 0;
            }
            break;
        }
        case LIBMATTI_MC_SpriteSource_SOURCE_FILTER:
        {
            // Java: output.removeAll(this.filter.locationPredicate())
            for (size_t e = 0; e < collector.count;)
            {
                if (identifier_matches_pattern(collector.entries[e].spriteId, source->filterPattern))
                {
                    LIBMATTI_MC_Resource_Free(collector.entries[e].resource);
                    LIBMATTI_MC_Identifier_Free(collector.entries[e].spriteId);
                    memmove(&collector.entries[e], &collector.entries[e + 1],
                            (collector.count - e - 1) * sizeof(LIBMATTI_MC_SpriteResourceEntry));
                    collector.count--;
                }
                else
                {
                    e++;
                }
            }
            break;
        }
        }
    }

    // The DirectoryLister port above needs the resource-manager prefix listing;
    // the simple case (no prefix listing available) yields nothing. The loader
    // takes the entries as-is.
    (void) resourceManager;

    *outCount = collector.count;
    return collector.entries;
}

void LIBMATTI_MC_SpriteResourceEntries_Free(LIBMATTI_MC_SpriteResourceEntry *entries, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        LIBMATTI_MC_Identifier_Free(entries[i].spriteId);
        LIBMATTI_MC_Resource_Free(entries[i].resource);
    }
    free(entries);
}

// Java: SpriteResourceLoader - decode + metadata + frame size
LIBMATTI_MC_SpriteContents *LIBMATTI_MC_SpriteResourceLoader_LoadSprite(const LIBMATTI_MC_Identifier *spriteId,
                                                                        LIBMATTI_MC_Resource *resource)
{
    size_t length = 0;
    unsigned char *bytes = LIBMATTI_MC_Resource_Open(resource, &length);
    if (bytes == NULL)
        return NULL;

    LIBMATTI_B3D_NativeImage *image = LIBMATTI_B3D_NativeImage_Read(bytes, length);
    // Resource_Open hands out the owning pointer - the resource keeps it.
    if (image == NULL)
    {
        // Java: LOGGER.error("Using missing texture, unable to load {}", ...); return null
        return NULL;
    }

    // Java: the animation metadata from the .mcmeta
    LIBMATTI_MC_AnimationMetadataSection *animation = NULL;
    size_t metadataLength = 0;
    const unsigned char *metadataBytes = LIBMATTI_MC_Resource_Metadata(resource, &metadataLength);
    if (metadataBytes != NULL && metadataLength > 0)
    {
        LIBMATTI_GSON_Gson *gson = LIBMATTI_GSON_Gson_New();
        LIBMATTI_GSON_JsonElement *root = LIBMATTI_GSON_Gson_FromJson(gson, (const char *) metadataBytes, metadataLength);
        if (root != NULL && LIBMATTI_GSON_JsonElement_IsJsonObject(root))
        {
            LIBMATTI_GSON_JsonElement *animObject = LIBMATTI_GSON_JsonElement_GetMember(root, "animation");
            if (animObject != NULL)
                animation = LIBMATTI_MC_AnimationMetadataSection_Parse(animObject, NULL);
        }
        LIBMATTI_GSON_JsonElement_Free(root);
        LIBMATTI_GSON_Gson_Free(gson);
    }

    // Java: framesize = animation.isPresent() ? calculateFrameSize(...) : new FrameSize(w, h)
    LIBMATTI_MC_FrameSize frameSize;
    if (animation != NULL)
        frameSize = LIBMATTI_MC_AnimationMetadataSection_CalculateFrameSize(animation, image->width, image->height);
    else
    {
        frameSize.width = image->width;
        frameSize.height = image->height;
    }

    // Java: the multiple-of check errors out (close + null)
    if (!LIBMATTI_MC_Mth_IsMultipleOf(image->width, frameSize.width)
        || !LIBMATTI_MC_Mth_IsMultipleOf(image->height, frameSize.height))
    {
        LIBMATTI_B3D_NativeImage_Free(image);
        LIBMATTI_MC_AnimationMetadataSection_Free(animation);
        return NULL;
    }

    LIBMATTI_MC_Identifier *name =
        LIBMATTI_MC_Identifier_FromNamespaceAndPath(spriteId->namespace, spriteId->path);
    return LIBMATTI_MC_SpriteContents_New(name, frameSize, image, animation);
}
