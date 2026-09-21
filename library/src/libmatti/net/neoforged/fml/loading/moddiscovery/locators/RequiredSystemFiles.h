// Port of net.neoforged.fml.loading.moddiscovery.locators.RequiredSystemFiles.
// Models the required files for opening a valid Minecraft and NeoForge jar from the classpath.

#ifndef MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_REQUIREDSYSTEMFILES_H
#define MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_REQUIREDSYSTEMFILES_H

#include "libmatti/java/lang/ClassLoader.h"
#include "libmatti/net/neoforged/fml/jarcontents/JarContents.h"
#include "libmatti/net/neoforged/neoforgespi/ILaunchContext.h"

#include <stddef.h>

// Java: static final String COMMON_CLASS = "net/minecraft/DetectedVersion.class"
#define LIBMATTI_FML_REQUIRED_COMMON_CLASS "net/minecraft/DetectedVersion.class"
// Java: static final String CLIENT_CLASS = "net/minecraft/client/Minecraft.class"
#define LIBMATTI_FML_REQUIRED_CLIENT_CLASS "net/minecraft/client/Minecraft.class"
// Java: static final String COMMON_RESOURCE_ROOT = "data/.mcassetsroot"
#define LIBMATTI_FML_REQUIRED_COMMON_RESOURCE_ROOT "data/.mcassetsroot"
// Java: static final String CLIENT_RESOURCE_ROOT = "assets/.mcassetsroot"
#define LIBMATTI_FML_REQUIRED_CLIENT_RESOURCE_ROOT "assets/.mcassetsroot"
// Java: static final String NEOFORGE_COMMON_CLASS = "net/neoforged/neoforge/common/NeoForgeMod.class"
#define LIBMATTI_FML_REQUIRED_NEOFORGE_COMMON_CLASS "net/neoforged/neoforge/common/NeoForgeMod.class"
// Java: static final String NEOFORGE_CLIENT_CLASS = "net/neoforged/neoforge/client/ClientNeoForgeMod.class"
#define LIBMATTI_FML_REQUIRED_NEOFORGE_CLIENT_CLASS "net/neoforged/neoforge/client/ClientNeoForgeMod.class"

// Java: final class RequiredSystemFiles implements AutoCloseable
typedef struct
{
    LIBMATTI_FML_JarContents *commonClasses;
    LIBMATTI_FML_JarContents *commonResources;
    LIBMATTI_FML_JarContents *clientClasses;
    LIBMATTI_FML_JarContents *clientResources;
    LIBMATTI_FML_JarContents *neoForgeCommonClasses;
    LIBMATTI_FML_JarContents *neoForgeClientClasses;
    LIBMATTI_FML_JarContents *neoForgeResources;
} LIBMATTI_FML_RequiredSystemFiles;

// Java: public static RequiredSystemFiles find(ILaunchContext context, ClassLoader loader) - NULL when the files could not be found
LIBMATTI_FML_RequiredSystemFiles *LIBMATTI_FML_RequiredSystemFiles_Find(LIBMATTI_NEOFORGESPI_ILaunchContext *context,
                                                                        LIBMATTI_JL_ClassLoader *loader);

// Java: public boolean areNeoForgeAndMinecraftSeparate()
int LIBMATTI_FML_RequiredSystemFiles_AreNeoForgeAndMinecraftSeparate(
    const LIBMATTI_FML_RequiredSystemFiles *files);
// Java: public void checkForMissingMinecraftFiles(boolean clientRequired) - 0 when files are missing (Java: throws ModLoadingException)
int LIBMATTI_FML_RequiredSystemFiles_CheckForMissingMinecraftFiles(const LIBMATTI_FML_RequiredSystemFiles *files,
                                                                  int clientRequired);
// Java: public List<JarContents> getAll()
LIBMATTI_FML_JarContents **LIBMATTI_FML_RequiredSystemFiles_GetAll(
    const LIBMATTI_FML_RequiredSystemFiles *files, size_t *count);
// Java: public List<JarContents> getClassesRoots()
LIBMATTI_FML_JarContents **LIBMATTI_FML_RequiredSystemFiles_GetClassesRoots(
    const LIBMATTI_FML_RequiredSystemFiles *files, size_t *count);
// Java: public boolean isEmpty()
int LIBMATTI_FML_RequiredSystemFiles_IsEmpty(const LIBMATTI_FML_RequiredSystemFiles *files);
// Java: public List<JarContents> getMinecraftJarComponents()
LIBMATTI_FML_JarContents **LIBMATTI_FML_RequiredSystemFiles_GetMinecraftJarComponents(
    const LIBMATTI_FML_RequiredSystemFiles *files, size_t *count);
// Java: public List<JarContents> getNeoForgeJarComponents()
LIBMATTI_FML_JarContents **LIBMATTI_FML_RequiredSystemFiles_GetNeoForgeJarComponents(
    const LIBMATTI_FML_RequiredSystemFiles *files, size_t *count);

// Java: public JarContents getCommonClasses() / getCommonResources() / getClientClasses() / getClientResources()
//         / getNeoForgeCommonClasses() / getNeoForgeClientClasses() / getNeoForgeResources()
LIBMATTI_FML_JarContents *LIBMATTI_FML_RequiredSystemFiles_GetCommonClasses(
    const LIBMATTI_FML_RequiredSystemFiles *files);
LIBMATTI_FML_JarContents *LIBMATTI_FML_RequiredSystemFiles_GetCommonResources(
    const LIBMATTI_FML_RequiredSystemFiles *files);
LIBMATTI_FML_JarContents *LIBMATTI_FML_RequiredSystemFiles_GetClientClasses(
    const LIBMATTI_FML_RequiredSystemFiles *files);
LIBMATTI_FML_JarContents *LIBMATTI_FML_RequiredSystemFiles_GetClientResources(
    const LIBMATTI_FML_RequiredSystemFiles *files);
LIBMATTI_FML_JarContents *LIBMATTI_FML_RequiredSystemFiles_GetNeoForgeCommonClasses(
    const LIBMATTI_FML_RequiredSystemFiles *files);
LIBMATTI_FML_JarContents *LIBMATTI_FML_RequiredSystemFiles_GetNeoForgeClientClasses(
    const LIBMATTI_FML_RequiredSystemFiles *files);
LIBMATTI_FML_JarContents *LIBMATTI_FML_RequiredSystemFiles_GetNeoForgeResources(
    const LIBMATTI_FML_RequiredSystemFiles *files);
// Java: public boolean isNeoForgeManifest(Manifest manifest)
int LIBMATTI_FML_RequiredSystemFiles_IsNeoForgeManifest(const LIBMATTI_JU_Manifest *manifest);
// Java: @Override public void close()
void LIBMATTI_FML_RequiredSystemFiles_Close(LIBMATTI_FML_RequiredSystemFiles *files);

#endif //MATTICRAFT_FML_LOADING_MODDISCOVERY_LOCATORS_REQUIREDSYSTEMFILES_H
