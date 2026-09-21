#include "libmatti/java/lang/ClassLoader.h"
#include "libmatti/net/neoforged/fml/loading/ClassLoadingGuardian.h"

#include "libmatti/cpw/modlauncher/LogManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final Logger LOGGER = LoggerFactory.getLogger(ClassLoadingGuardian.class);
static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

struct LIBMATTI_FML_ClassLoadingGuardian
{
    LIBMATTI_JLI_Instrumentation *instrumentation;
    char **protectedPackages;
    size_t protectedPackageCount;
    void *guardianTransformer;
    // Java: private volatile boolean uninstalled
    int uninstalled;
    // Java: private volatile ClassLoader allowedClassLoader
    void *allowedClassLoader;
};

// Java: private static Set<String> getPackages(List<ModFile> gameContent)
static char **getPackages(const char *const *packages, size_t packageCount)
{
    char **protectedPackages = malloc(sizeof(char *) * (packageCount > 0 ? packageCount : 1));
    for (size_t i = 0; i < packageCount; i++)
    {
        // Java: pkgName.replace('.', '/')
        protectedPackages[i] = strdup(packages[i]);
        for (char *c = protectedPackages[i]; *c != '\0'; c++)
            if (*c == '.')
                *c = '/';
    }
    return protectedPackages;
}

// Java: public ClassLoadingGuardian(Instrumentation instrumentation, List<ModFile> gameContent)
LIBMATTI_FML_ClassLoadingGuardian *LIBMATTI_FML_ClassLoadingGuardian_New(
    LIBMATTI_JLI_Instrumentation *instrumentation, const char *const *packages, size_t packageCount)
{
    LIBMATTI_FML_ClassLoadingGuardian *guardian = calloc(1, sizeof(LIBMATTI_FML_ClassLoadingGuardian));
    guardian->instrumentation = instrumentation;
    guardian->protectedPackages = getPackages(packages, packageCount);
    guardian->protectedPackageCount = packageCount;
    // Java: org.objectweb.asm generates a self-destructing class; the port runs the same
    //       check directly through LIBMATTI_FML_ClassLoadingGuardian_Transform.
    guardian->guardianTransformer = guardian;
    LIBMATTI_JLI_Instrumentation_AddTransformer(instrumentation, guardian->guardianTransformer, 0);
    return guardian;
}

// Java: public static void fail()
void LIBMATTI_FML_ClassLoadingGuardian_Fail(void)
{
    // Java: throw new IllegalStateException();
    LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "IllegalStateException");
}

// Java: ClassFileTransformer.transform(Module module, ClassLoader loader, String className, ...)
unsigned char *LIBMATTI_FML_ClassLoadingGuardian_Transform(LIBMATTI_FML_ClassLoadingGuardian *guardian,
                                                           void *loader, const char *className, size_t *byteCount)
{
    *byteCount = 0;

    // Java: if (uninstalled) return null; // This can happen due to multi-threaded class-loading
    if (guardian->uninstalled)
        return NULL;

    // Java: if (loader == allowedClassLoader) return null;
    if (loader == guardian->allowedClassLoader)
        return NULL;

    // Java: var packageName = getPackageName(className);
    char *packageName = NULL;
    const char *lastPkgSep = strrchr(className, '/');
    if (lastPkgSep != NULL)
    {
        size_t length = (size_t)(lastPkgSep - className);
        packageName = malloc(length + 1);
        memcpy(packageName, className, length);
        packageName[length] = '\0';
    }

    if (packageName != NULL)
    {
        int protected = 0;
        for (size_t i = 0; i < guardian->protectedPackageCount; i++)
        {
            if (strcmp(guardian->protectedPackages[i], packageName) == 0)
            {
                protected = 1;
                break;
            }
        }
        if (protected)
        {
            // Java: LOGGER.error("Illegal load of protected class {} into class-loader {}", className, loader, new Throwable())
            char loaderName[32];
            snprintf(loaderName, sizeof(loaderName), "%p", loader);
            LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Illegal load of protected class {} into class-loader {}",
                                     className, loaderName);
            // Java: transformers are actually not allowed to throw, so a self-destructing
            //       class is generated that calls ClassLoadingGuardian.fail() from <clinit>.
            //       The port has no bytecode to poison, so the fail path runs directly.
            LIBMATTI_FML_ClassLoadingGuardian_Fail();
        }
    }

    free(packageName);
    return NULL;
}

// Java: private boolean isReachableFrom(ClassLoader classLoader, ClassLoader origin)
static int isReachableFrom(void *classLoader, void *origin)
{
    if (classLoader == origin)
        return 1;
    // Java: return classLoader.getParent() != null && isReachableFrom(classLoader.getParent(), origin);
    LIBMATTI_JL_ClassLoader *parent = LIBMATTI_JL_ClassLoader_GetParent(classLoader);
    return parent != NULL && isReachableFrom(parent, origin);
}

// Java: public void setAllowedClassLoader(ClassLoader allowedClassLoader)
void LIBMATTI_FML_ClassLoadingGuardian_SetAllowedClassLoader(LIBMATTI_FML_ClassLoadingGuardian *guardian,
                                                             void *allowedClassLoader)
{
    guardian->allowedClassLoader = allowedClassLoader;

    // Java: final check for class-loading bugs
    size_t loadedClassCount = 0;
    const LIBMATTI_JLI_LoadedClassInfo *loadedClasses =
        LIBMATTI_JLI_Instrumentation_GetAllLoadedClasses(guardian->instrumentation, &loadedClassCount);

    char **foundIssues = NULL;
    size_t foundIssueCount = 0;
    for (size_t i = 0; i < loadedClassCount; i++)
    {
        // Java: if (loadedClass.getClassLoader() == null) continue; // JDK built-in
        if (loadedClasses[i].classLoader == NULL)
            continue;

        const char *physicalPackage = loadedClasses[i].packageName;
        if (physicalPackage == NULL)
            continue;

        int protected = 0;
        for (size_t j = 0; j < guardian->protectedPackageCount; j++)
        {
            if (strcmp(guardian->protectedPackages[j], physicalPackage) == 0)
            {
                protected = 1;
                break;
            }
        }

        // Java: it's ok if the class is not reachable from the now current class-loader
        if (protected && isReachableFrom(loadedClasses[i].classLoader, allowedClassLoader))
        {
            foundIssues = realloc(foundIssues, sizeof(char *) * (foundIssueCount + 1));
            foundIssues[foundIssueCount] = strdup(loadedClasses[i].name);
            foundIssueCount++;
        }
    }

    if (foundIssueCount > 0)
    {
        // Java: throw new IllegalArgumentException(message.toString())
        char *message = malloc(1);
        message[0] = '\0';
        size_t messageLength = 1;
        const char *header = "Classes were loaded on the wrong class-loader:\n";
        message = realloc(message, messageLength + strlen(header));
        strcpy(message, header);
        messageLength += strlen(header);
        for (size_t i = 0; i < foundIssueCount; i++)
        {
            size_t entryLength = 1 + strlen(foundIssues[i]) + 6 + 1;
            message = realloc(message, messageLength + entryLength);
            snprintf(message + messageLength - 1, entryLength + 1, " %s from \n", foundIssues[i]);
            messageLength += entryLength;
        }
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "{}", message);
        free(message);
    }

    for (size_t i = 0; i < foundIssueCount; i++)
        free(foundIssues[i]);
    free(foundIssues);
}

// Java: public void close()
void LIBMATTI_FML_ClassLoadingGuardian_Close(LIBMATTI_FML_ClassLoadingGuardian *guardian)
{
    if (!guardian->uninstalled)
    {
        guardian->uninstalled = 1;
        LIBMATTI_JLI_Instrumentation_RemoveTransformer(guardian->instrumentation, guardian->guardianTransformer);
        guardian->allowedClassLoader = NULL;
    }
}

// Java: no explicit free; the port releases the struct after close
void LIBMATTI_FML_ClassLoadingGuardian_Free(LIBMATTI_FML_ClassLoadingGuardian *guardian)
{
    if (guardian == NULL) return;
    LIBMATTI_FML_ClassLoadingGuardian_Close(guardian);
    free(guardian);
}
