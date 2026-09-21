// Throwaway harness for MixinFacade: builds a LoadingModList from two real .so mod files and
// runs finishInitialization, so every branch of addMixins is exercised.

#include "libmatti/java/lang/Thread.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/fml/loading/LoadingModList.h"
#include "libmatti/net/neoforged/fml/loading/mixin/MixinFacade.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/readers/JarModsDotTomlModFileReader.h"
#include "libmatti/net/neoforged/neoforgespi/locating/ModFileDiscoveryAttributes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void check(int condition, const char *what)
{
    printf("%s: %s\n", condition ? "ok  " : "FAIL", what);
    if (!condition) failures++;
}

static LIBMATTI_FML_ModFile *loadModFile(const char *path)
{
    LIBMATTI_FML_JarContents *contents = LIBMATTI_FML_JarContents_OfPath(path);
    if (contents == NULL)
    {
        printf("FAIL: cannot open %s\n", path);
        failures++;
        return NULL;
    }

    LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes attributes = {0};
    LIBMATTI_NEOFORGESPI_IModFile *file =
        LIBMATTI_FML_JarModsDotTomlModFileReader_CreateModFile(contents, &attributes);
    if (file == NULL)
    {
        printf("FAIL: %s is not a mod file\n", path);
        failures++;
        return NULL;
    }

    return LIBMATTI_FML_ModFile_AsConcrete(file);
}

static int issue_count(const char *key)
{
    size_t count = 0;
    LIBMATTI_FML_ModLoadingIssue **issues = LIBMATTI_FML_ModLoader_GetLoadingIssues(&count);
    int found = 0;
    for (size_t i = 0; i < count; i++)
        if (strcmp(issues[i]->translationKey, key) == 0) found++;
    free(issues);
    return found;
}

int main(int argc, char **argv)
{
    if (argc < 3) return 2;

    LIBMATTI_FML_ModFile *modA = loadModFile(argv[1]);
    LIBMATTI_FML_ModFile *modB = loadModFile(argv[2]);
    if (modA == NULL || modB == NULL) return 1;

    LIBMATTI_FML_ModFile *modFiles[2] = {modA, modB};
    LIBMATTI_FML_LoadingModList *loadingModList = LIBMATTI_FML_LoadingModList_Of(
        NULL, 0, NULL, 0, modFiles, 2, NULL, 0, NULL, 0, NULL);

    // The facade checks that the context class loader is the one it was handed; with no JVM the
    // port's default is NULL, so a stand-in pointer stands in for the transforming class loader.
    LIBMATTI_JL_Thread_SetContextClassLoader(LIBMATTI_JL_Thread_CurrentThread(), (LIBMATTI_JL_ClassLoader *) 0x1);

    LIBMATTI_FML_MixinFacade *facade = LIBMATTI_FML_MixinFacade_New();
    LIBMATTI_FML_MixinFacade_FinishInitialization(facade, loadingModList,
                                                  (LIBMATTI_FML_TransformingClassLoader *) 0x1);

    LIBMATTI_FML_MixinFacade_MixinConfig *registered = LIBMATTI_FML_MixinFacade_GetConfig(
        facade, "mixina.mixins.json");
    check(registered != NULL, "the config that exists in the mod file is registered");
    if (registered != NULL)
    {
        printf("dump: config=%s contentLength=%zu modId=%s behaviorVersion=%d\n", registered->name,
               registered->contentLength, registered->modId, registered->behaviorVersion);
        check(strcmp(registered->modId, LIBMATTI_NEOFORGESPI_IModFile_GetId(
                                         LIBMATTI_FML_ModFile_AsModFile(modA))) == 0,
              "the config is decorated with the mod id of its mod file");
        check(registered->behaviorVersion == 17001, "the behaviour version defaults to 17001");
    }

    check(LIBMATTI_FML_MixinFacade_GetConfig(facade, "missing.mixins.json") == NULL,
          "a config whose content is missing is not registered");
    check(LIBMATTI_FML_MixinFacade_GetConfig(facade, "needsmod.mixins.json") == NULL,
          "a config with a missing required mod is not registered");
    check(LIBMATTI_FML_MixinFacade_GetConfig(facade, "toonew.mixins.json") == NULL,
          "a config asking for a newer behaviour than mixin offers is not registered");
    check(LIBMATTI_FML_MixinFacade_GetConfig(facade, "tooold.mixins.json") == NULL,
          "a config asking for an older behaviour than mixin offers is not registered");
    check(facade->configCount == 1, "only the one valid config was registered");

    check(issue_count("fml.modloadingissue.mixin.missing_config") == 1, "one missing_config error");
    check(issue_count("fml.modloadingissue.mixin.duplicate_config") == 1, "one duplicate_config error");
    check(issue_count("fml.modloadingissue.mixin.requested_behavior_too_new") == 1, "one too_new error");
    check(issue_count("fml.modloadingissue.mixin.requested_behavior_too_old") == 1, "one too_old error");

    size_t issueCount = 0;
    LIBMATTI_FML_ModLoadingIssue **issues = LIBMATTI_FML_ModLoader_GetLoadingIssues(&issueCount);
    for (size_t i = 0; i < issueCount; i++)
    {
        char *text = LIBMATTI_FML_ModLoadingIssue_ToString(issues[i]);
        printf("dump: issue %s\n", text);
        free(text);
    }
    free(issues);

    check(LIBMATTI_FML_MixinFacade_CalculateBehaviorVersion(NULL) == 17001,
          "calculateBehaviorVersion(NULL) is the default behaviour version");
    printf("dump: highest=%s lowest=%s\n",
           LIBMATTI_NEOFORGESPI_MavenVersionAdapter_VersionToString(LIBMATTI_FML_MixinFacade_HIGHEST_MIXIN_VERSION),
           LIBMATTI_NEOFORGESPI_MavenVersionAdapter_VersionToString(LIBMATTI_FML_MixinFacade_LOWEST_MIXIN_VERSION));

    LIBMATTI_FML_MixinFacade_Free(facade);
    LIBMATTI_FML_LoadingModList_Free(loadingModList);
    LIBMATTI_FML_ModFile_Free(modA);
    LIBMATTI_FML_ModFile_Free(modB);

    printf("%s\n", failures == 0 ? "ALL OK" : "FAILURES");
    return failures != 0;
}
