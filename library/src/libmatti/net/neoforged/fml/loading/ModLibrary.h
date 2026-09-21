// Port-only companion of net.neoforged.fml.javafmlmod.FMLModContainer.
//
// Java's mod code is a class the JVM defines (Class.forName + constructor.newInstance); a Matticraft
// mod is an ELF shared object, so "loading the mod's code" is dlopen and "running the mod" is calling
// the entry point symbol of the mod file. The symbol is the mod id with every character that is not a
// letter, a digit or '_' replaced by '_', followed by "_init":
//
//     mod id "examplemod"   -> symbol "examplemod_init"
//     mod id "com.demo.app" -> symbol "com_demo_app_init"
//
// The entry point receives the same arguments Java injects into a mod's constructor.
//
// TODO: net.neoforged.fml.loading.mixin - the hook table is resolved against these loaded objects.

#ifndef MATTICRAFT_FML_LOADING_MODLIBRARY_H
#define MATTICRAFT_FML_LOADING_MODLIBRARY_H

#include "libmatti/net/neoforged/fml/javafmlmod/FMLModContainer.h"

#include <stddef.h>

typedef void (*LIBMATTI_FML_ModLibrary_Entrypoint)(const LIBMATTI_FML_FMLModContainer_ConstructorArgs *args);

typedef struct LIBMATTI_FML_ModLibrary LIBMATTI_FML_ModLibrary;

// Java: Class.forName(layer, entrypoint) - NULL when the file carries no loadable code or cannot be
// loaded at all; a mod file without an entry point symbol still loads (it may only ship data).
LIBMATTI_FML_ModLibrary *LIBMATTI_FML_ModLibrary_Load(const char *path, const char *modId);

// Java: constructor.newInstance(constructorArgs) - does nothing when the mod declares no entry point.
void LIBMATTI_FML_ModLibrary_Construct(const LIBMATTI_FML_ModLibrary *library,
                                       const LIBMATTI_FML_FMLModContainer_ConstructorArgs *args);

// Java: the class stays defined for the lifetime of the layer, so the port keeps the shared object
// mapped; only the bookkeeping is released.
void LIBMATTI_FML_ModLibrary_Free(LIBMATTI_FML_ModLibrary *library);

const char *LIBMATTI_FML_ModLibrary_Path(const LIBMATTI_FML_ModLibrary *library);
const char *LIBMATTI_FML_ModLibrary_EntrypointName(const LIBMATTI_FML_ModLibrary *library);
int LIBMATTI_FML_ModLibrary_HasEntrypoint(const LIBMATTI_FML_ModLibrary *library);

#endif //MATTICRAFT_FML_LOADING_MODLIBRARY_H
