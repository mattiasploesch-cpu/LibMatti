#include "libmatti/java/lang/Class.h"

#include "libmatti/java/lang/StringBuilder.h"
#include "libmatti/java/lang/instrument/Instrumentation.h"
#include "libmatti/java/lang/reflect/Modifier.h"

#include <stdlib.h>
#include <string.h>

// Java: the classes the VM has loaded
static LIBMATTI_JL_Class **classes = NULL;
static size_t classCount = 0;

// Java: the resource lookup of a class is relative to its package directory
static char *package_directory(const char *name)
{
    size_t lastDot = strlen(name);
    while (lastDot > 0 && name[lastDot - 1] != '.') lastDot--;

    char *directory = strdup(name);
    if (lastDot == 0)
    {
        directory[0] = '\0';
        return directory;
    }

    directory[lastDot - 1] = '\0';
    for (size_t i = 0; directory[i] != '\0'; i++)
        if (directory[i] == '.') directory[i] = '/';
    return directory;
}

LIBMATTI_JL_Class *LIBMATTI_JL_Class_Register(const char *name, void *classLoader)
{
    for (size_t i = 0; i < classCount; i++)
        if (strcmp(classes[i]->name, name) == 0) return classes[i];

    LIBMATTI_JL_Class *clazz = calloc(1, sizeof(LIBMATTI_JL_Class));
    clazz->name = strdup(name);
    clazz->classLoader = classLoader;
    clazz->resourceDirectory = package_directory(name);
    // Java: every class except Object has Object as its superclass
    if (strcmp(name, "java.lang.Object") != 0) clazz->superclass = LIBMATTI_JL_Class_ObjectType();

    classes = realloc(classes, sizeof(*classes) * (classCount + 1));
    classes[classCount++] = clazz;
    // Java: the JVM runs the registered java.lang.instrument transformers at define-time
    LIBMATTI_JLI_Instrumentation_OnClassDefined(clazz->name, classLoader, clazz->resourceDirectory);
    return clazz;
}

LIBMATTI_JL_Class *LIBMATTI_JL_Class_RegisterClass(const char *name, void *classLoader,
                                                    LIBMATTI_JL_Class *superclass, int modifiers)
{
    LIBMATTI_JL_Class *clazz = LIBMATTI_JL_Class_Register(name, classLoader);
    clazz->superclass = superclass;
    clazz->modifiers = modifiers;
    return clazz;
}

// Java: Object.class
LIBMATTI_JL_Class *LIBMATTI_JL_Class_ObjectType(void)
{
    return LIBMATTI_JL_Class_Register("java.lang.Object", NULL);
}

void LIBMATTI_JL_Class_RegisterStaticMethod(LIBMATTI_JL_Class *clazz, const char *name,
                                            void (*function)(int argc, char *argv[]))
{
    clazz->staticMethods = realloc(clazz->staticMethods,
                                   sizeof(*clazz->staticMethods) * (clazz->staticMethodCount + 1));
    clazz->staticMethods[clazz->staticMethodCount].name = strdup(name);
    clazz->staticMethods[clazz->staticMethodCount].function = function;
    clazz->staticMethodCount++;
}

LIBMATTI_JL_Class *LIBMATTI_JL_Class_ForName(const char *name, int initialize, void *classLoader)
{
    (void)initialize;
    (void)classLoader;

    for (size_t i = 0; i < classCount; i++)
        if (strcmp(classes[i]->name, name) == 0) return classes[i];
    return NULL;
}

const char *LIBMATTI_JL_Class_GetName(const LIBMATTI_JL_Class *clazz)
{
    return clazz->name;
}

void *LIBMATTI_JL_Class_GetClassLoader(const LIBMATTI_JL_Class *clazz)
{
    return clazz->classLoader;
}

LIBMATTI_JL_Class *LIBMATTI_JL_Class_GetSuperclass(const LIBMATTI_JL_Class *clazz)
{
    return clazz->superclass;
}

int LIBMATTI_JL_Class_GetModifiers(const LIBMATTI_JL_Class *clazz)
{
    return clazz->modifiers;
}

// Java: Modifier.isInterface(getModifiers())
int LIBMATTI_JL_Class_IsInterface(const LIBMATTI_JL_Class *clazz)
{
    return LIBMATTI_JL_Modifier_IsInterface(clazz->modifiers);
}

// Java: clazz.isAssignableFrom(other) - other is clazz or a subtype of clazz
int LIBMATTI_JL_Class_IsAssignableFrom(const LIBMATTI_JL_Class *clazz, const LIBMATTI_JL_Class *other)
{
    if (clazz == other) return 1;
    if (other == NULL) return 0;
    if (LIBMATTI_JL_Class_IsAssignableFrom(clazz, other->superclass)) return 1;

    for (size_t i = 0; i < other->interfaceCount; i++)
        if (LIBMATTI_JL_Class_IsAssignableFrom(clazz, other->interfaces[i])) return 1;
    return 0;
}

// Java: the simple name is the part after the last '.' (top level) or '$' (nested class)
char *LIBMATTI_JL_Class_GetSimpleName(const LIBMATTI_JL_Class *clazz)
{
    const char *start = clazz->name;
    for (const char *p = clazz->name; *p != '\0'; p++)
        if (*p == '.' || *p == '$') start = p + 1;
    return strdup(start);
}

// Java: public Class<?>[] getInterfaces()
LIBMATTI_JL_Class **LIBMATTI_JL_Class_GetInterfaces(const LIBMATTI_JL_Class *clazz, size_t *count)
{
    *count = clazz->interfaceCount;
    return clazz->interfaces;
}

void LIBMATTI_JL_Class_AddInterface(LIBMATTI_JL_Class *clazz, LIBMATTI_JL_Class *interface)
{
    clazz->interfaces = realloc(clazz->interfaces, sizeof(*clazz->interfaces) * (clazz->interfaceCount + 1));
    clazz->interfaces[clazz->interfaceCount++] = interface;
}

LIBMATTI_JL_Class_StaticMethod *LIBMATTI_JL_Class_FindStaticMethod(LIBMATTI_JL_Class *clazz, const char *name)
{
    for (size_t i = 0; i < clazz->staticMethodCount; i++)
        if (strcmp(clazz->staticMethods[i].name, name) == 0) return &clazz->staticMethods[i];
    return NULL;
}

LIBMATTI_JN_URL *LIBMATTI_JL_Class_GetResource(const LIBMATTI_JL_Class *clazz, const char *name)
{
    LIBMATTI_JL_StringBuilder *builder = LIBMATTI_JL_StringBuilder_NewFromString(clazz->resourceDirectory);
    if (name[0] != '/') LIBMATTI_JL_StringBuilder_AppendChar(builder, '/');
    LIBMATTI_JL_StringBuilder_Append(builder, name);

    LIBMATTI_JN_URL *url = LIBMATTI_JN_URL_New(LIBMATTI_JL_StringBuilder_ToString(builder));
    LIBMATTI_JL_StringBuilder_Free(builder);
    return url;
}

LIBMATTI_JL_Class *LIBMATTI_JL_Class_VoidType(void)
{
    return LIBMATTI_JL_Class_Register("void", NULL);
}

LIBMATTI_JL_Class *LIBMATTI_JL_Class_StringArrayType(void)
{
    return LIBMATTI_JL_Class_Register("[Ljava.lang.String;", NULL);
}
