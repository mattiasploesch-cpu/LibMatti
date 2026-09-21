#include "libmatti/java/lang/instrument/Instrumentation.h"

#include <stdlib.h>
#include <string.h>

// Java: the JVM keeps a registry of loaded classes for getAllLoadedClasses()
static LIBMATTI_JLI_LoadedClassInfo *loadedClasses = NULL;
static size_t loadedClassCount = 0;
// Java: every created Instrumentation (the JVM has exactly one); OnClassDefined runs
// the transformer chain of the first one, like the JVM's single instrument instance.
static LIBMATTI_JLI_Instrumentation **instrumentations = NULL;
static size_t instrumentationCount = 0;

// Java: the JVM creates the object
LIBMATTI_JLI_Instrumentation *LIBMATTI_JLI_Instrumentation_New(void)
{
    LIBMATTI_JLI_Instrumentation *instrumentation = calloc(1, sizeof(LIBMATTI_JLI_Instrumentation));
    instrumentation->attached = 1;
    instrumentations = realloc(instrumentations, sizeof(*instrumentations) * (instrumentationCount + 1));
    instrumentations[instrumentationCount++] = instrumentation;
    return instrumentation;
}

void LIBMATTI_JLI_Instrumentation_Free(LIBMATTI_JLI_Instrumentation *instrumentation)
{
    free(instrumentation);
}

// Java: boolean isRetransformClassesSupported()
int LIBMATTI_JLI_Instrumentation_IsRetransformClassesSupported(const LIBMATTI_JLI_Instrumentation *instrumentation)
{
    return instrumentation->attached;
}

// Java: void addTransformer(ClassFileTransformer transformer, boolean canRetransform)
void LIBMATTI_JLI_Instrumentation_AddTransformer(LIBMATTI_JLI_Instrumentation *instrumentation,
                                                 void *transformer, int canRetransform)
{
    (void) canRetransform;
    if (instrumentation->transformerCount == instrumentation->transformerCapacity)
    {
        instrumentation->transformerCapacity = instrumentation->transformerCapacity == 0
                                                  ? 4
                                                  : instrumentation->transformerCapacity * 2;
        instrumentation->transformers = realloc(instrumentation->transformers,
                                                sizeof(void *) * instrumentation->transformerCapacity);
    }
    instrumentation->transformers[instrumentation->transformerCount++] = transformer;
}

// Java: the JVM runs every registered transformer at define-time; the first
// non-null result wins (Java: SecurityManager-free default behaviour)
unsigned char *LIBMATTI_JLI_Instrumentation_Transform(LIBMATTI_JLI_Instrumentation *instrumentation,
                                                      void *loader, const char *className,
                                                      unsigned char *classfileBuffer, size_t length,
                                                      size_t *outLength)
{
    *outLength = 0;
    for (size_t i = 0; i < instrumentation->transformerCount; i++)
    {
        LIBMATTI_JLI_ClassFileTransformer *transformer = instrumentation->transformers[i];
        size_t transformedLength = 0;
        unsigned char *transformed = transformer->transform(transformer->self, loader, className,
                                                            classfileBuffer, length, &transformedLength);
        if (transformed != NULL)
        {
            *outLength = transformedLength;
            return transformed;
        }
    }
    return NULL;
}

// Java: void removeTransformer(ClassFileTransformer transformer)
void LIBMATTI_JLI_Instrumentation_RemoveTransformer(LIBMATTI_JLI_Instrumentation *instrumentation, void *transformer)
{
    for (size_t i = 0; i < instrumentation->transformerCount; i++)
    {
        if (instrumentation->transformers[i] == transformer)
        {
            memmove(instrumentation->transformers + i, instrumentation->transformers + i + 1,
                    sizeof(void *) * (instrumentation->transformerCount - i - 1));
            instrumentation->transformerCount--;
            return;
        }
    }
}

// Java: Class<?>[] getAllLoadedClasses()
const LIBMATTI_JLI_LoadedClassInfo *LIBMATTI_JLI_Instrumentation_GetAllLoadedClasses(
    const LIBMATTI_JLI_Instrumentation *instrumentation, size_t *count)
{
    (void) instrumentation;
    *count = loadedClassCount;
    return loadedClasses;
}

// Java: the JVM registers loaded classes with the agent
void LIBMATTI_JLI_Instrumentation_RegisterLoadedClass(const char *name, void *classLoader, const char *packageName)
{
    loadedClasses = realloc(loadedClasses, sizeof(*loadedClasses) * (loadedClassCount + 1));
    loadedClasses[loadedClassCount].name = strdup(name);
    loadedClasses[loadedClassCount].classLoader = classLoader;
    loadedClasses[loadedClassCount].packageName = packageName != NULL ? strdup(packageName) : NULL;
    loadedClassCount++;
}

// Java: the JVM runs every registered ClassFileTransformer inside defineClass before the
// class is committed. The port's class definition is Class_Register; this hook runs the
// transformer chain there (a guard that fails the load reports like Java's poisoned class).
void LIBMATTI_JLI_Instrumentation_OnClassDefined(const char *name, void *classLoader, const char *packageName)
{
    if (instrumentations == NULL) return;

    LIBMATTI_JLI_Instrumentation_RegisterLoadedClass(name, classLoader, packageName);

    for (size_t i = 0; i < instrumentations[0]->transformerCount; i++)
    {
        LIBMATTI_JLI_ClassFileTransformer *transformer = instrumentations[0]->transformers[i];
        size_t outLength = 0;
        unsigned char *bytes = transformer->transform(transformer->self, classLoader, name, NULL, 0, &outLength);
        free(bytes);
    }
}

// Java: void retransformClasses(Class<?>... classes) - the JVM replays the stored class
// bytes through every canRetransform transformer. The port keeps no bytes (the classes
// are native code), so the replay runs the transformer chain per registered class.
void LIBMATTI_JLI_Instrumentation_RetransformClasses(LIBMATTI_JLI_Instrumentation *instrumentation, size_t count)
{
    (void) count;
    if (instrumentation == NULL) return;

    for (size_t i = 0; i < instrumentation->transformerCount; i++)
    {
        LIBMATTI_JLI_ClassFileTransformer *transformer = instrumentation->transformers[i];
        for (size_t c = 0; c < loadedClassCount; c++)
        {
            size_t outLength = 0;
            unsigned char *bytes = transformer->transform(transformer->self, loadedClasses[c].classLoader,
                                                          loadedClasses[c].name, NULL, 0, &outLength);
            free(bytes);
        }
    }
}
