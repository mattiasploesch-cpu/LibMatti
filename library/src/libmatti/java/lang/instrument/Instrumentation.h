// Port of java.lang.instrument.Instrumentation.
// The interface is JVM machinery; the C port keeps the handle the agent was
// attached with, so the loader can tell attached from not attached.

#ifndef MATTICRAFT_JAVA_LANG_INSTRUMENT_INSTRUMENTATION_H
#define MATTICRAFT_JAVA_LANG_INSTRUMENT_INSTRUMENTATION_H

#include <stddef.h>

typedef struct LIBMATTI_JLI_Instrumentation LIBMATTI_JLI_Instrumentation;

struct LIBMATTI_JLI_Instrumentation
{// Java: the JVM hands this to premain/agentmain
    int attached;

    // Java: private final List<ClassFileTransformer> transformers
    void **transformers;
    size_t transformerCount;
    size_t transformerCapacity;};

// Java: the JVM creates the object; the port's host hands it to the agent
LIBMATTI_JLI_Instrumentation *LIBMATTI_JLI_Instrumentation_New(void);
void LIBMATTI_JLI_Instrumentation_Free(LIBMATTI_JLI_Instrumentation *instrumentation);

// Java: interface java.lang.instrument.ClassFileTransformer - one transform
// hook; NULL bytes means "no change"
typedef struct
{
    void *self;
    unsigned char *(*transform)(void *self, void *loader, const char *className,
                                unsigned char *classfileBuffer, size_t length, size_t *outLength);
} LIBMATTI_JLI_ClassFileTransformer;

// Java: boolean isRetransformClassesSupported()
int LIBMATTI_JLI_Instrumentation_IsRetransformClassesSupported(const LIBMATTI_JLI_Instrumentation *instrumentation);
// Java: void addTransformer(ClassFileTransformer transformer, boolean canRetransform)
void LIBMATTI_JLI_Instrumentation_AddTransformer(LIBMATTI_JLI_Instrumentation *instrumentation,
                                                 void *transformer, int canRetransform);
// Java: the JVM runs every registered transformer at define-time
unsigned char *LIBMATTI_JLI_Instrumentation_Transform(LIBMATTI_JLI_Instrumentation *instrumentation,
                                                      void *loader, const char *className,
                                                      unsigned char *classfileBuffer, size_t length,
                                                      size_t *outLength);
// Java: void retransformClasses(Class<?>... classes)
void LIBMATTI_JLI_Instrumentation_RetransformClasses(LIBMATTI_JLI_Instrumentation *instrumentation, size_t count);
// Java: void removeTransformer(ClassFileTransformer transformer)
void LIBMATTI_JLI_Instrumentation_RemoveTransformer(LIBMATTI_JLI_Instrumentation *instrumentation, void *transformer);
// Java: Class<?>[] getAllLoadedClasses()
// The C port exposes the loaded classes the host registered as (name, loader, packageName) tuples.
typedef struct
{
    const char *name;
    void *classLoader;
    const char *packageName;
} LIBMATTI_JLI_LoadedClassInfo;
const LIBMATTI_JLI_LoadedClassInfo *LIBMATTI_JLI_Instrumentation_GetAllLoadedClasses(
    const LIBMATTI_JLI_Instrumentation *instrumentation, size_t *count);
// Java: the JVM registers loaded classes with the agent
void LIBMATTI_JLI_Instrumentation_RegisterLoadedClass(const char *name, void *classLoader, const char *packageName);

// Java: the JVM-side defineClass hook - runs the transformer chain when a class is
// defined (the port's Class_Register calls this) and records the loaded class.
void LIBMATTI_JLI_Instrumentation_OnClassDefined(const char *name, void *classLoader, const char *packageName);

#endif //MATTICRAFT_JAVA_LANG_INSTRUMENT_INSTRUMENTATION_H
