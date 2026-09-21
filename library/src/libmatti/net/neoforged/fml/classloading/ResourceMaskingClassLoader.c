// Port of net.neoforged.fml.classloading.ResourceMaskingClassLoader.

#include "libmatti/net/neoforged/fml/classloading/ResourceMaskingClassLoader.h"

#include "libmatti/net/neoforged/fml/util/ClasspathResourceUtils.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_FML_ResourceMaskingClassLoader *LIBMATTI_FML_ResourceMaskingClassLoader_New(
    LIBMATTI_JL_ClassLoader *parent, const char **maskedClasspathElements, size_t maskedClasspathElementCount)
{
    LIBMATTI_FML_ResourceMaskingClassLoader *classLoader = calloc(1, sizeof(LIBMATTI_FML_ResourceMaskingClassLoader));
    // Java: super(Objects.requireNonNull(parent, "parent"))
    classLoader->base.parent = parent;

    // Java: private final Set<Path> maskedClasspathElements - the port takes ownership of a copy
    char **elements = calloc(maskedClasspathElementCount > 0 ? maskedClasspathElementCount : 1, sizeof(*elements));
    for (size_t i = 0; i < maskedClasspathElementCount; i++)
        elements[i] = strdup(maskedClasspathElements[i]);
    classLoader->maskedClasspathElements = (const char **) elements;
    classLoader->maskedClasspathElementCount = maskedClasspathElementCount;
    return classLoader;
}

void LIBMATTI_FML_ResourceMaskingClassLoader_Free(LIBMATTI_FML_ResourceMaskingClassLoader *classLoader)
{
    if (classLoader == NULL) return;
    for (size_t i = 0; i < classLoader->maskedClasspathElementCount; i++)
        free((void *) classLoader->maskedClasspathElements[i]);
    free((void *) classLoader->maskedClasspathElements);
    free(classLoader);
}

static int is_masked(const LIBMATTI_FML_ResourceMaskingClassLoader *classLoader, const char *root)
{
    if (root == NULL) return 0;
    for (size_t i = 0; i < classLoader->maskedClasspathElementCount; i++)
    {
        if (strcmp(classLoader->maskedClasspathElements[i], root) == 0) return 1;
    }
    return 0;
}

// Java: the FilteringEnumeration inner class - finds the elements not within a masked classpath element
static LIBMATTI_JN_URL **filter_masked(LIBMATTI_FML_ResourceMaskingClassLoader *classLoader, const char *relativePath,
                                      LIBMATTI_JN_URL **delegate, size_t delegateCount, size_t *count)
{
    LIBMATTI_JN_URL **result = malloc(sizeof(*result) * (delegateCount > 0 ? delegateCount : 1));
    *count = 0;
    for (size_t i = 0; i < delegateCount; i++)
    {
        // Java: Path root = ClasspathResourceUtils.getRootFromResourceUrl(relativePath, el);
        //       if (!maskedClasspathElements.contains(root)) nextElement = el;
        char *root = LIBMATTI_FML_ClasspathResourceUtils_GetRootFromResourceUrl(relativePath, delegate[i]);
        int masked = is_masked(classLoader, root);
        free(root);
        if (masked) continue;
        result[(*count)++] = delegate[i];
    }
    return result;
}

LIBMATTI_JN_URL **LIBMATTI_FML_ResourceMaskingClassLoader_GetResources(
    LIBMATTI_FML_ResourceMaskingClassLoader *classLoader, const char *name, size_t *count)
{
    // Java: return new FilteringEnumeration(super.getResources(name), name);
    size_t delegateCount = 0;
    LIBMATTI_JN_URL **delegate = LIBMATTI_JL_ClassLoader_FindResources(classLoader->base.parent, name,
                                                                       &delegateCount);
    LIBMATTI_JN_URL **result = filter_masked(classLoader, name, delegate, delegateCount, count);
    free(delegate);
    return result;
}

LIBMATTI_JN_URL *LIBMATTI_FML_ResourceMaskingClassLoader_GetResource(
    LIBMATTI_FML_ResourceMaskingClassLoader *classLoader, const char *name)
{
    // Java: var resource = getParent().getResource(name); if (resource == null) return null;
    LIBMATTI_JN_URL *resource = LIBMATTI_JL_ClassLoader_GetResource(classLoader->base.parent, name);
    if (resource == NULL) return NULL;

    // Java: var resourceRoot = ClasspathResourceUtils.getRootFromResourceUrl(name, resource);
    //       if (maskedClasspathElements.contains(resourceRoot)) { var resources = getResources(name);
    //           resource = resources.hasMoreElements() ? resources.nextElement() : null; }
    char *resourceRoot = LIBMATTI_FML_ClasspathResourceUtils_GetRootFromResourceUrl(name, resource);
    int masked = is_masked(classLoader, resourceRoot);
    free(resourceRoot);
    if (!masked) return resource;

    size_t count = 0;
    LIBMATTI_JN_URL **resources = LIBMATTI_FML_ResourceMaskingClassLoader_GetResources(classLoader, name, &count);
    LIBMATTI_JN_URL *next = count > 0 ? resources[0] : NULL;
    for (size_t i = 0; i < count; i++) free(resources[i]);
    free(resources);
    return next;
}
