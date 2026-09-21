#ifndef MATTICRAFT_JAVA_LANG_CLASS_H
#define MATTICRAFT_JAVA_LANG_CLASS_H

#include "libmatti/java/net/URL.h"

#include <stddef.h>

typedef struct LIBMATTI_JL_Class LIBMATTI_JL_Class;

// Java: the VM's method table entry for a static method
typedef struct
{
    char *name;
    void (*function)(int argc, char *argv[]);
} LIBMATTI_JL_Class_StaticMethod;

// Java: public final class Class<T>
struct LIBMATTI_JL_Class
{
    char *name;
    // Java: the defining ClassLoader
    void *classLoader;
    // Java: the package directory used by Class.getResource
    char *resourceDirectory;
    // Java: Class.getSuperclass() (NULL for Object and interfaces)
    LIBMATTI_JL_Class *superclass;
    // Java: Class.getModifiers()
    int modifiers;
    // Java: Class.getInterfaces()
    LIBMATTI_JL_Class **interfaces;
    size_t interfaceCount;
    LIBMATTI_JL_Class_StaticMethod *staticMethods;
    size_t staticMethodCount;
};

// Java: the class registry (the classes the VM has loaded); the host registers them
LIBMATTI_JL_Class *LIBMATTI_JL_Class_Register(const char *name, void *classLoader);

// Java: the VM's class definition entry point - Register plus the superclass and the
// class modifiers (the C port passes what Class.getSuperclass()/getModifiers() return)
LIBMATTI_JL_Class *LIBMATTI_JL_Class_RegisterClass(const char *name, void *classLoader,
                                                    LIBMATTI_JL_Class *superclass, int modifiers);

// Java: Object.class
LIBMATTI_JL_Class *LIBMATTI_JL_Class_ObjectType(void);

// Java: the VM's method table - a static method is registered on its class
void LIBMATTI_JL_Class_RegisterStaticMethod(LIBMATTI_JL_Class *clazz, const char *name,
                                            void (*function)(int argc, char *argv[]));

// Java: public static Class<?> forName(String name, boolean initialize, ClassLoader loader)
//       throws ClassNotFoundException - NULL in the C port when the class is not registered
LIBMATTI_JL_Class *LIBMATTI_JL_Class_ForName(const char *name, int initialize, void *classLoader);

// Java: public String getName()
const char *LIBMATTI_JL_Class_GetName(const LIBMATTI_JL_Class *clazz);
// Java: public ClassLoader getClassLoader()
void *LIBMATTI_JL_Class_GetClassLoader(const LIBMATTI_JL_Class *clazz);
// Java: public Class<?> getSuperclass()
LIBMATTI_JL_Class *LIBMATTI_JL_Class_GetSuperclass(const LIBMATTI_JL_Class *clazz);
// Java: public int getModifiers()
int LIBMATTI_JL_Class_GetModifiers(const LIBMATTI_JL_Class *clazz);
// Java: public boolean isInterface()
int LIBMATTI_JL_Class_IsInterface(const LIBMATTI_JL_Class *clazz);
// Java: public boolean isAssignableFrom(Class<?> cls)
int LIBMATTI_JL_Class_IsAssignableFrom(const LIBMATTI_JL_Class *clazz, const LIBMATTI_JL_Class *other);
// Java: public String getSimpleName()
char *LIBMATTI_JL_Class_GetSimpleName(const LIBMATTI_JL_Class *clazz);
// Java: public Class<?>[] getInterfaces() - internal array, do not free
LIBMATTI_JL_Class **LIBMATTI_JL_Class_GetInterfaces(const LIBMATTI_JL_Class *clazz, size_t *count);
// Java: the VM registers the interfaces a class implements (what getInterfaces() returns)
void LIBMATTI_JL_Class_AddInterface(LIBMATTI_JL_Class *clazz, LIBMATTI_JL_Class *interface);
// Java: the VM method lookup behind MethodHandles.Lookup.findStatic
LIBMATTI_JL_Class_StaticMethod *LIBMATTI_JL_Class_FindStaticMethod(LIBMATTI_JL_Class *clazz, const char *name);
// Java: public URL getResource(String name)
LIBMATTI_JN_URL *LIBMATTI_JL_Class_GetResource(const LIBMATTI_JL_Class *clazz, const char *name);

// Java: void.class - the primitive void type
LIBMATTI_JL_Class *LIBMATTI_JL_Class_VoidType(void);
// Java: String[].class
LIBMATTI_JL_Class *LIBMATTI_JL_Class_StringArrayType(void);

#endif //MATTICRAFT_JAVA_LANG_CLASS_H
