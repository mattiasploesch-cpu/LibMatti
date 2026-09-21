#include "libmatti/com/mojang/blaze3d/opengl/VertexArrayCache.h"

#include "libmatti/com/mojang/blaze3d/opengl/GlStateManager.h"
#include "libmatti/com/mojang/blaze3d/systems/RenderSystem.h"
#include "libmatti/org/lwjgl/opengl/Constants.h"
#include "libmatti/org/lwjgl/opengl/GL.h"

#include <stdlib.h>
#include <string.h>

// Java: VertexArrayCache.VertexArray - id + format + lastVertexBuffer.
typedef struct VertexArray
{
    unsigned int id;
    const LIBMATTI_B3D_VertexFormat *format;
    const LIBMATTI_B3D_GpuBuffer *lastVertexBuffer;
    struct VertexArray *next;
} VertexArray;

struct LIBMATTI_B3D_VertexArrayCache
{
    int separate;
    int needsMesaWorkaround;
    VertexArray *head;
};

// Java: private final Map<VertexFormat, VertexArray> cache - the port is a
// linked list keyed by pointer identity (the formats are singletons).
static VertexArray *cache_find(LIBMATTI_B3D_VertexArrayCache *cache, const LIBMATTI_B3D_VertexFormat *format)
{
    for (VertexArray *entry = cache->head; entry != NULL; entry = entry->next)
    {
        if (entry->format == format)
            return entry;
    }
    return NULL;
}

static void cache_put(LIBMATTI_B3D_VertexArrayCache *cache, unsigned int id,
                      const LIBMATTI_B3D_VertexFormat *format, const LIBMATTI_B3D_GpuBuffer *vertexBuffer)
{
    VertexArray *entry = malloc(sizeof(VertexArray));
    entry->id = id;
    entry->format = format;
    entry->lastVertexBuffer = vertexBuffer;
    entry->next = cache->head;
    cache->head = entry;
}

LIBMATTI_B3D_VertexArrayCache *LIBMATTI_B3D_VertexArrayCache_Create(int useSeparate)
{
    LIBMATTI_B3D_VertexArrayCache *cache = calloc(1, sizeof(LIBMATTI_B3D_VertexArrayCache));
    cache->separate = useSeparate;
    // Java: Separate ctor - needsMesaWorkaround when the vendor is Mesa and the
    // version is one of 25.0.0/25.0.1/25.0.2.
    if (useSeparate)
    {
        const char *vendor = LIBMATTI_B3D_GlStateManager_GetString(7936 /* GL_VENDOR */);
        if (vendor != NULL && strcmp(vendor, "Mesa") == 0)
        {
            const char *version = LIBMATTI_B3D_GlStateManager_GetString(7938 /* GL_VERSION */);
            cache->needsMesaWorkaround = version != NULL &&
                                         (strstr(version, "25.0.0") != NULL ||
                                          strstr(version, "25.0.1") != NULL ||
                                          strstr(version, "25.0.2") != NULL);
        }
    }
    return cache;
}

void LIBMATTI_B3D_VertexArrayCache_Free(LIBMATTI_B3D_VertexArrayCache *cache)
{
    if (cache == NULL)
        return;
    VertexArray *entry = cache->head;
    while (entry != NULL)
    {
        VertexArray *next = entry->next;
        unsigned int id = entry->id;
        LIBMATTI_B3D_GlStateManager_DeleteVertexArrays(1, &id);
        free(entry);
        entry = next;
    }
    free(cache);
}

// Java: Emulated.setupCombinedAttributes - one combined vertex buffer.
static void setup_combined_attributes(const LIBMATTI_B3D_VertexFormat *format, int first)
{
    int vertexSize = LIBMATTI_B3D_VertexFormat_GetVertexSize(format);
    int count = LIBMATTI_B3D_VertexFormat_GetElementCount(format);
    for (int j = 0; j < count; j++)
    {
        const LIBMATTI_B3D_VertexFormatElement *element = LIBMATTI_B3D_VertexFormat_GetElement(format, j);
        if (first)
            LIBMATTI_B3D_GlStateManager_EnableVertexAttribArray((unsigned int) j);

        switch (element->usage)
        {
            case LIBMATTI_B3D_VFE_POSITION:
            case LIBMATTI_B3D_VFE_GENERIC:
            case LIBMATTI_B3D_VFE_UV:
                if (element->type == LIBMATTI_B3D_VFE_FLOAT)
                {
                    LIBMATTI_B3D_GlStateManager_VertexAttribPointer(
                        (unsigned int) j, element->count, LIBMATTI_B3D_VertexFormatElement_GlType(element),
                        0, vertexSize, (const void *) (size_t) LIBMATTI_B3D_VertexFormat_GetOffset(format, j));
                }
                else
                {
                    LIBMATTI_B3D_GlStateManager_VertexAttribIPointer(
                        (unsigned int) j, element->count, LIBMATTI_B3D_VertexFormatElement_GlType(element),
                        vertexSize, (const void *) (size_t) LIBMATTI_B3D_VertexFormat_GetOffset(format, j));
                }
                break;
            case LIBMATTI_B3D_VFE_NORMAL:
            case LIBMATTI_B3D_VFE_COLOR:
                LIBMATTI_B3D_GlStateManager_VertexAttribPointer(
                    (unsigned int) j, element->count, LIBMATTI_B3D_VertexFormatElement_GlType(element),
                    1, vertexSize, (const void *) (size_t) LIBMATTI_B3D_VertexFormat_GetOffset(format, j));
                break;
        }
    }
}

// Java: Separate - glVertexAttribFormat + glVertexAttribBinding per element.
static void setup_separate_attributes(const LIBMATTI_B3D_VertexFormat *format)
{
    int vertexSize = LIBMATTI_B3D_VertexFormat_GetVertexSize(format);
    int count = LIBMATTI_B3D_VertexFormat_GetElementCount(format);
    for (int j = 0; j < count; j++)
    {
        const LIBMATTI_B3D_VertexFormatElement *element = LIBMATTI_B3D_VertexFormat_GetElement(format, j);
        LIBMATTI_B3D_GlStateManager_EnableVertexAttribArray((unsigned int) j);
        switch (element->usage)
        {
            case LIBMATTI_B3D_VFE_POSITION:
            case LIBMATTI_B3D_VFE_GENERIC:
            case LIBMATTI_B3D_VFE_UV:
                if (element->type == LIBMATTI_B3D_VFE_FLOAT)
                {
                    LIBMATTI_GL_glVertexAttribFormat((unsigned int) j, element->count,
                                                     LIBMATTI_B3D_VertexFormatElement_GlType(element), 0,
                                                     (unsigned int) LIBMATTI_B3D_VertexFormat_GetOffset(format, j));
                }                else
                {
                    LIBMATTI_GL_glVertexAttribIFormat((unsigned int) j, element->count,
                                                      LIBMATTI_B3D_VertexFormatElement_GlType(element),
                                                      (unsigned int) LIBMATTI_B3D_VertexFormat_GetOffset(format, j));
                }
                break;
            case LIBMATTI_B3D_VFE_NORMAL:
            case LIBMATTI_B3D_VFE_COLOR:
                LIBMATTI_GL_glVertexAttribFormat((unsigned int) j, element->count,
                                                 LIBMATTI_B3D_VertexFormatElement_GlType(element), 1,
                                                 (unsigned int) LIBMATTI_B3D_VertexFormat_GetOffset(format, j));
                break;
        }
        LIBMATTI_GL_glVertexAttribBinding((unsigned int) j, 0);
    }
    (void) vertexSize;
}

void LIBMATTI_B3D_VertexArrayCache_BindVertexArray(LIBMATTI_B3D_VertexArrayCache *cache,
                                                   const LIBMATTI_B3D_VertexFormat *format,
                                                   const LIBMATTI_B3D_GpuBuffer *vertexBuffer)
{
    VertexArray *entry = cache_find(cache, format);
    if (cache->separate)
    {
        if (entry != NULL)
        {
            LIBMATTI_B3D_GlStateManager_BindVertexArray(entry->id);
            if (vertexBuffer != NULL && entry->lastVertexBuffer != vertexBuffer)
            {
                // Java: the Mesa workaround rebinds slot 0 to nothing first.
                if (cache->needsMesaWorkaround && entry->lastVertexBuffer != NULL &&
                    entry->lastVertexBuffer->handle == vertexBuffer->handle)
                {
                    LIBMATTI_GL_glBindVertexBuffer(0, 0, 0, 0);
                }
                LIBMATTI_GL_glBindVertexBuffer(0, vertexBuffer->handle, 0,
                                               (unsigned int) LIBMATTI_B3D_VertexFormat_GetVertexSize(format));
                entry->lastVertexBuffer = vertexBuffer;
            }
        }
        else
        {
            unsigned int id = LIBMATTI_B3D_GlStateManager_GenVertexArrays();
            LIBMATTI_B3D_GlStateManager_BindVertexArray(id);
            if (vertexBuffer != NULL)
            {
                setup_separate_attributes(format);
                LIBMATTI_GL_glBindVertexBuffer(0, vertexBuffer->handle, 0,
                                               (unsigned int) LIBMATTI_B3D_VertexFormat_GetVertexSize(format));
            }
            cache_put(cache, id, format, vertexBuffer);
        }
        return;
    }

    // Java: Emulated.bindVertexArray.
    if (entry != NULL)
    {
        LIBMATTI_B3D_GlStateManager_BindVertexArray(entry->id);
        if (vertexBuffer != NULL && entry->lastVertexBuffer != vertexBuffer)
        {
            LIBMATTI_B3D_GlStateManager_BindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, vertexBuffer->handle);
            entry->lastVertexBuffer = vertexBuffer;
            setup_combined_attributes(format, 0);
        }
    }
    else
    {
        unsigned int id = LIBMATTI_B3D_GlStateManager_GenVertexArrays();
        LIBMATTI_B3D_GlStateManager_BindVertexArray(id);
        if (vertexBuffer != NULL)
        {
            LIBMATTI_B3D_GlStateManager_BindBuffer(LIBMATTI_GL_GL_ARRAY_BUFFER, vertexBuffer->handle);
            setup_combined_attributes(format, 1);
        }
        cache_put(cache, id, format, vertexBuffer);
    }
}
