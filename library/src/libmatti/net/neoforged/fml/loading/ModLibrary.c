// Port-only companion of net.neoforged.fml.javafmlmod.FMLModContainer. See ModLibrary.h.

#include "libmatti/net/neoforged/fml/loading/ModLibrary.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/Logging.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct LIBMATTI_FML_ModLibrary
{
    void *handle;
    char *path;
    char *entrypointName;
    LIBMATTI_FML_ModLibrary_Entrypoint entrypoint;
};

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: the entrypoint class name; the port derives the symbol name from the mod id.
static char *entrypoint_name(const char *modId)
{
    char *name = malloc(strlen(modId) + sizeof("_init"));

    size_t written = 0;
    for (const char *c = modId; *c != '\0'; c++)
    {
        int isNameChar = (*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z') || (*c >= '0' && *c <= '9');
        name[written++] = isNameChar ? *c : '_';
    }
    strcpy(name + written, "_init");
    return name;
}

// Java refuses to define a class from a file that is not bytecode; the port refuses to dlopen a file
// that is not an ELF shared object.
static int isSharedObject(const char *path)
{
    unsigned char magic[4];
    FILE *file = fopen(path, "rb");
    if (file == NULL) return 0;

    size_t read = fread(magic, 1, sizeof(magic), file);
    fclose(file);

    return read == sizeof(magic) && magic[0] == 0x7F && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F';
}

// Java: Class.forName(layer, entrypoint)
LIBMATTI_FML_ModLibrary *LIBMATTI_FML_ModLibrary_Load(const char *path, const char *modId)
{
    if (path == NULL || modId == NULL)
        return NULL;

    if (!isSharedObject(path))
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_Logging_LOADING, "{} is not a shared object", path);
        return NULL;
    }

    // Java: the module layer's class loader defines the classes and puts them in the mod's module;
    // RTLD_GLOBAL does the same for the mod's symbols, so mods can call each other.
    void *handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
    if (handle == NULL)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_Logging_LOADING, "Failed to load {}: {}", path, dlerror());
        return NULL;
    }

    LIBMATTI_FML_ModLibrary *library = calloc(1, sizeof(*library));
    library->handle = handle;
    library->path = strdup(path);
    library->entrypointName = entrypoint_name(modId);

    void *symbol = dlsym(handle, library->entrypointName);
    if (symbol == NULL)
    {
        LIBMATTI_ML_Logger_Debug(LOGGER(), &LIBMATTI_FML_Logging_LOADING, "{} has no entry point {}", path,
                                 library->entrypointName);
        return library;
    }

    memcpy(&library->entrypoint, &symbol, sizeof(symbol));
    return library;
}

// Java: constructor.newInstance(constructorArgs)
void LIBMATTI_FML_ModLibrary_Construct(const LIBMATTI_FML_ModLibrary *library,
                                       const LIBMATTI_FML_FMLModContainer_ConstructorArgs *args)
{
    if (library == NULL || library->entrypoint == NULL) return;

    library->entrypoint(args);
}

void LIBMATTI_FML_ModLibrary_Free(LIBMATTI_FML_ModLibrary *library)
{
    if (library == NULL) return;

    // The shared object stays mapped: Java's classes stay defined for the lifetime of the layer too,
    // and the mod's functions are still reachable through the event handlers it registered.
    free(library->path);
    free(library->entrypointName);
    free(library);
}

const char *LIBMATTI_FML_ModLibrary_Path(const LIBMATTI_FML_ModLibrary *library)
{
    return library != NULL ? library->path : NULL;
}

const char *LIBMATTI_FML_ModLibrary_EntrypointName(const LIBMATTI_FML_ModLibrary *library)
{
    return library != NULL ? library->entrypointName : NULL;
}

int LIBMATTI_FML_ModLibrary_HasEntrypoint(const LIBMATTI_FML_ModLibrary *library)
{
    return library != NULL && library->entrypoint != NULL;
}
