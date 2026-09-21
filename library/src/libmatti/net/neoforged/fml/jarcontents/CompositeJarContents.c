#include "libmatti/net/neoforged/fml/jarcontents/CompositeJarContents.h"

#include "libmatti/java/security/MessageDigest.h"
#include "libmatti/net/neoforged/fml/jarcontents/EmptyManifest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: JarFile.MANIFEST_NAME
static const char *MANIFEST_NAME = "META-INF/MANIFEST.MF";

typedef struct
{
    LIBMATTI_FML_JarContents base;
    LIBMATTI_FML_JarContents **delegates;
    LIBMATTI_FML_JarContents_PathFilter *filters;
    void **filterUserdata;
    size_t delegateCount;
    const char **contentRoots;
    size_t contentRootCount;
    // Java: private volatile Optional<String> checksum
    char *checksum;
    int checksumComputed;
} CompositeJarContents;

// Java: private boolean isMasked(int delegateIdx, String relativePath)
static int isMasked(CompositeJarContents *contents, size_t delegateIdx, const char *relativePath)
{
    if (contents->filters == NULL)
        return 0;
    LIBMATTI_FML_JarContents_PathFilter filter = contents->filters[delegateIdx];
    return filter != NULL && !filter(relativePath, contents->filterUserdata[delegateIdx]);
}

// Java: public boolean isFiltered()
int LIBMATTI_FML_CompositeJarContents_IsFiltered(const LIBMATTI_FML_JarContents *contents)
{
    CompositeJarContents *composite = contents->self;
    if (composite->filters == NULL)
        return 0;
    for (size_t i = 0; i < composite->delegateCount; i++)
        if (composite->filters[i] != NULL)
            return 1;
    return 0;
}

// Java: public Collection<Path> getContentRoots() { return contentRoots; }
static const char **getContentRoots(void *self, size_t *count)
{
    CompositeJarContents *contents = self;
    *count = contents->contentRootCount;
    return contents->contentRoots;
}

// Java: private Optional<String> computeChecksum()
static char *computeChecksum(CompositeJarContents *contents)
{
    // Java: if (isFiltered()) return Optional.empty();
    if (LIBMATTI_FML_CompositeJarContents_IsFiltered(&contents->base))
        return NULL;

    LIBMATTI_JS_MessageDigest *digest = LIBMATTI_JS_MessageDigest_GetInstance("SHA-256");

    // Java: for (var delegate : delegates) { var delegateChecksum = delegate.getChecksum(); if empty return empty;
    //             digest.update(delegateChecksum.get().getBytes()); }
    char *concatenated = NULL;
    size_t concatenatedLength = 0;

    for (size_t i = 0; i < contents->delegateCount; i++)
    {
        char *delegateChecksum = NULL;
        if (!LIBMATTI_FML_JarContents_GetChecksum(contents->delegates[i], &delegateChecksum))
        {
            free(concatenated);
            LIBMATTI_JS_MessageDigest_Free(digest);
            return NULL;
        }

        size_t length = strlen(delegateChecksum);
        concatenated = realloc(concatenated, concatenatedLength + length);
        memcpy(concatenated + concatenatedLength, delegateChecksum, length);
        concatenatedLength += length;
        free(delegateChecksum);
    }

    size_t hashLength = 0;
    unsigned char *hash = LIBMATTI_JS_MessageDigest_Digest(digest, (const unsigned char *)concatenated,
                                                           concatenatedLength, &hashLength);
    LIBMATTI_JS_MessageDigest_Free(digest);
    free(concatenated);

    char *hex = malloc(hashLength * 2 + 1);
    for (size_t i = 0; i < hashLength; i++)
        snprintf(hex + i * 2, 3, "%02x", hash[i]);
    free(hash);
    return hex;
}

// Java: public Optional<String> getChecksum()
static int getChecksum(void *self, char **out)
{
    CompositeJarContents *contents = self;

    if (!contents->checksumComputed)
    {
        contents->checksum = computeChecksum(contents);
        contents->checksumComputed = 1;
    }

    if (contents->checksum == NULL)
        return 0;

    *out = strdup(contents->checksum);
    return 1;
}

// Java: public Path getPrimaryPath() { return delegates[delegates.length - 1].getPrimaryPath(); }
static const char *getPrimaryPath(void *self)
{
    CompositeJarContents *contents = self;
    return LIBMATTI_FML_JarContents_GetPrimaryPath(contents->delegates[contents->delegateCount - 1]);
}

// Java: public Optional<URI> findFile(String relativePath)
static int findFile(void *self, const char *relativePath, LIBMATTI_JN_URI **out)
{
    CompositeJarContents *contents = self;
    for (size_t i = 0; i < contents->delegateCount; i++)
    {
        if (isMasked(contents, i, relativePath))
            continue;
        if (LIBMATTI_FML_JarContents_FindFile(contents->delegates[i], relativePath, out))
            return 1;
    }
    return 0;
}

// Java: public Manifest getManifest()
static LIBMATTI_JU_Manifest *getManifest(void *self)
{
    CompositeJarContents *contents = self;
    for (size_t i = 0; i < contents->delegateCount; i++)
    {
        if (isMasked(contents, i, MANIFEST_NAME))
            continue;

        LIBMATTI_JU_Manifest *manifest = LIBMATTI_FML_JarContents_GetManifest(contents->delegates[i]);
        // Java: empty manifests are considered "missing"
        if (manifest != NULL && (manifest->mainAttributes.count > 0 || manifest->sectionCount > 0))
            return manifest;
    }
    return LIBMATTI_FML_EmptyManifest_Instance();
}

// Java: public @Nullable JarResource get(String relativePath)
static LIBMATTI_FML_JarResource *get(void *self, const char *relativePath)
{
    CompositeJarContents *contents = self;
    for (size_t i = 0; i < contents->delegateCount; i++)
    {
        if (isMasked(contents, i, relativePath))
            continue;

        LIBMATTI_FML_JarResource *resource = LIBMATTI_FML_JarContents_Get(contents->delegates[i], relativePath);
        if (resource != NULL)
            return resource;
    }
    return NULL;
}

// Java: public boolean containsFile(String relativePath)
static int containsFile(void *self, const char *relativePath)
{
    CompositeJarContents *contents = self;
    for (size_t i = 0; i < contents->delegateCount; i++)
    {
        if (isMasked(contents, i, relativePath))
            continue;
        if (LIBMATTI_FML_JarContents_ContainsFile(contents->delegates[i], relativePath))
            return 1;
    }
    return 0;
}

// Java: public InputStream openFile(String relativePath) throws IOException
static unsigned char *openFile(void *self, const char *relativePath, size_t *outLength)
{
    CompositeJarContents *contents = self;
    for (size_t i = 0; i < contents->delegateCount; i++)
    {
        if (isMasked(contents, i, relativePath))
            continue;

        unsigned char *bytes = LIBMATTI_FML_JarContents_OpenFile(contents->delegates[i], relativePath, outLength);
        if (bytes != NULL)
            return bytes;
    }

    *outLength = 0;
    return NULL;
}

typedef struct
{
    LIBMATTI_FML_JarResourceVisitor visitor;
    void *userdata;
    CompositeJarContents *contents;
    size_t delegateIdx;
    // Java: final Set<String> pathsVisited
    char **pathsVisited;
    size_t visitedCount;
} DistinctVisitor;

// Java: the anonymous JarResourceVisitor that deduplicates across delegates
static void distinctVisit(const char *relativePath, LIBMATTI_FML_JarResource *resource, void *userdata)
{
    DistinctVisitor *distinct = userdata;

    if (isMasked(distinct->contents, distinct->delegateIdx, relativePath))
        return;

    for (size_t i = 0; i < distinct->visitedCount; i++)
        if (strcmp(distinct->pathsVisited[i], relativePath) == 0)
            return;

    distinct->pathsVisited = realloc(distinct->pathsVisited, sizeof(char *) * (distinct->visitedCount + 1));
    distinct->pathsVisited[distinct->visitedCount++] = strdup(relativePath);

    distinct->visitor(relativePath, resource, distinct->userdata);
}

// Java: public void visitContent(String startingFolder, JarResourceVisitor visitor)
static void visitContent(void *self, const char *startingFolder, LIBMATTI_FML_JarResourceVisitor visitor,
                         void *userdata)
{
    CompositeJarContents *contents = self;

    DistinctVisitor distinct = {visitor, userdata, contents, 0, NULL, 0};

    for (size_t i = 0; i < contents->delegateCount; i++)
    {
        distinct.delegateIdx = i;
        LIBMATTI_FML_JarContents_VisitContentFrom(contents->delegates[i], startingFolder, distinctVisit, &distinct);
    }

    for (size_t i = 0; i < distinct.visitedCount; i++)
        free(distinct.pathsVisited[i]);
    free(distinct.pathsVisited);
}

// Java: public void close() throws IOException
static void closeContents(void *self)
{
    CompositeJarContents *contents = self;

    for (size_t i = 0; i < contents->delegateCount; i++)
        LIBMATTI_FML_JarContents_Close(contents->delegates[i]);

    free(contents->delegates);
    free(contents->filters);
    free(contents->filterUserdata);
    free(contents->contentRoots);
    free(contents->checksum);
    free(contents);
}

// Java: public String toString()
static char *toStringImpl(void *self)
{
    CompositeJarContents *contents = self;

    char *result = strdup("composite(");
    size_t length = strlen(result);

    // Java: in reverse order as that is how the user passed it
    for (size_t i = contents->delegateCount; i-- > 0;)
    {
        char *delegate = LIBMATTI_FML_JarContents_ToString(contents->delegates[i]);
        const char *prefix = "";
        const char *suffix = "";
        if (contents->filters != NULL && contents->filters[i] != NULL)
        {
            prefix = "filtered(";
            suffix = ")";
        }

        size_t addition = strlen(prefix) + strlen(delegate) + strlen(suffix) + 3;
        result = realloc(result, length + addition);
        snprintf(result + length, addition, "%s%s%s", prefix, delegate, suffix);
        length += strlen(prefix) + strlen(delegate) + strlen(suffix);

        free(delegate);

        if (i != 0)
        {
            result = realloc(result, length + 3);
            strcpy(result + length, ", ");
            length += 2;
        }
    }

    result = realloc(result, length + 2);
    strcpy(result + length, ")");
    return result;
}

// Java: public CompositeJarContents(List<JarContents> delegates, @Nullable List<@Nullable PathFilter> filters)
LIBMATTI_FML_JarContents *LIBMATTI_FML_CompositeJarContents_New(LIBMATTI_FML_JarContents **delegates,
                                                               LIBMATTI_FML_JarContents_PathFilter *filters,
                                                               void **filterUserdata, size_t delegateCount)
{
    // Java: throw new IllegalArgumentException(...) guards
    if (delegateCount == 0)
        return NULL;

    CompositeJarContents *contents = calloc(1, sizeof(CompositeJarContents));

    // Java: internally the first match gets returned, so the list is reversed
    contents->delegates = calloc(delegateCount, sizeof(LIBMATTI_FML_JarContents *));
    contents->delegateCount = delegateCount;
    for (size_t i = 0; i < delegateCount; i++)
        contents->delegates[i] = delegates[delegateCount - i - 1];

    if (filters != NULL)
    {
        contents->filters = calloc(delegateCount, sizeof(LIBMATTI_FML_JarContents_PathFilter));
        contents->filterUserdata = calloc(delegateCount, sizeof(void *));
        for (size_t i = 0; i < delegateCount; i++)
        {
            contents->filters[i] = filters[delegateCount - i - 1];
            contents->filterUserdata[i] = filterUserdata != NULL ? filterUserdata[delegateCount - i - 1] : NULL;
        }
    }

    // Java: collect all unique content roots of our delegates, then reverse
    const char **roots = NULL;
    size_t rootCount = 0;
    for (size_t i = 0; i < delegateCount; i++)
    {
        size_t count = 0;
        const char **delegateRoots = LIBMATTI_FML_JarContents_GetContentRoots(contents->delegates[i], &count);
        for (size_t r = 0; r < count; r++)
        {
            int duplicate = 0;
            for (size_t existing = 0; existing < rootCount; existing++)
            {
                if (strcmp(roots[existing], delegateRoots[r]) == 0)
                {
                    duplicate = 1;
                    break;
                }
            }
            if (duplicate)
                continue;

            roots = realloc(roots, sizeof(char *) * (rootCount + 1));
            roots[rootCount++] = delegateRoots[r];
        }
        free(delegateRoots);
    }

    contents->contentRoots = calloc(rootCount > 0 ? rootCount : 1, sizeof(char *));
    contents->contentRootCount = rootCount;
    for (size_t i = 0; i < rootCount; i++)
        contents->contentRoots[i] = roots[rootCount - i - 1];
    free(roots);

    contents->base.self = contents;
    contents->base.kind = LIBMATTI_FML_JARCONTENTS_KIND_COMPOSITE;
    contents->base.getChecksum = getChecksum;
    contents->base.getPrimaryPath = getPrimaryPath;
    contents->base.getContentRoots = getContentRoots;
    contents->base.get = get;
    contents->base.findFile = findFile;
    contents->base.openFile = openFile;
    contents->base.containsFile = containsFile;
    contents->base.getManifest = getManifest;
    contents->base.visitContent = visitContent;
    contents->base.close = closeContents;
    contents->base.toStringImpl = toStringImpl;

    return &contents->base;
}

// Java: public List<JarContents> getDelegates() { return List.of(delegates); }
LIBMATTI_FML_JarContents **LIBMATTI_FML_CompositeJarContents_GetDelegates(const LIBMATTI_FML_JarContents *contents,
                                                                         size_t *count)
{
    CompositeJarContents *composite = contents->self;
    *count = composite->delegateCount;
    return composite->delegates;
}
