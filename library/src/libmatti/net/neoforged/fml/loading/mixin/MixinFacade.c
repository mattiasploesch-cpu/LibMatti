// Port of net.neoforged.fml.loading.mixin.MixinFacade.

#include "libmatti/net/neoforged/fml/loading/mixin/MixinFacade.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/lang/System.h"
#include "libmatti/java/lang/Thread.h"
#include "libmatti/matti/mixin/MixinHookTable.h"
#include "libmatti/net/neoforged/api/distmarker/Dist.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"
#include "libmatti/net/neoforged/fml/loading/FMLLoader.h"
#include "libmatti/net/neoforged/fml/loading/LoadingModList.h"
#include "libmatti/net/neoforged/fml/loading/mixin/FMLMixinClassProcessor.h"
#include "libmatti/net/neoforged/fml/loading/mixin/FMLMixinService.h"
#include "libmatti/net/neoforged/fml/loading/mixin/FMLModFileContainerHandle.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFileParser.h"
#include "libmatti/org/spongepowered/asm/launch/MixinBootstrap.h"
#include "libmatti/org/spongepowered/asm/launch/platform/MixinPlatformManager.h"
#include "libmatti/org/spongepowered/asm/launch/platform/container/IContainerHandle.h"
#include "libmatti/org/spongepowered/asm/mixin/MixinEnvironment.h"
#include "libmatti/org/spongepowered/asm/mixin/Mixins.h"
#include "libmatti/org/spongepowered/asm/mixin/transformer/Config.h"
#include "libmatti/org/spongepowered/asm/service/IMixinService.h"
#include "libmatti/org/spongepowered/asm/service/MixinService.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final Logger LOG = LoggerFactory.getLogger(MixinFacade.class);
static LIBMATTI_ML_Logger *LOG(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: public static final int DEFAULT_BEHAVIOUR_VERSION = FabricUtil.COMPATIBILITY_0_17_1
int LIBMATTI_FML_MixinFacade_DEFAULT_BEHAVIOUR_VERSION = 17001;
// Java: public static final ArtifactVersion HIGHEST_MIXIN_VERSION
LIBMATTI_NEOFORGESPI_ArtifactVersion *LIBMATTI_FML_MixinFacade_HIGHEST_MIXIN_VERSION = NULL;
// Java: public static final ArtifactVersion LOWEST_MIXIN_VERSION
LIBMATTI_NEOFORGESPI_ArtifactVersion *LIBMATTI_FML_MixinFacade_LOWEST_MIXIN_VERSION = NULL;

// Java: static { HIGHEST_MIXIN_VERSION = ...; LOWEST_MIXIN_VERSION = ...; }
static void initConstants(void)
{
    if (LIBMATTI_FML_MixinFacade_LOWEST_MIXIN_VERSION != NULL)
        return;

    LIBMATTI_FML_MixinFacade_HIGHEST_MIXIN_VERSION =
        LIBMATTI_NEOFORGESPI_MavenVersionAdapter_CreateFromVersion(LIBMATTI_FML_MixinFacade_MIXIN_VERSION);

    // Java: int defaultMixinVersion = DEFAULT_BEHAVIOUR_VERSION;
    //       int patch = defaultMixinVersion % 1000; defaultMixinVersion /= 1000;
    //       int minor = defaultMixinVersion % 1000; defaultMixinVersion /= 1000;
    //       int major = defaultMixinVersion;
    //       LOWEST_MIXIN_VERSION = new DefaultArtifactVersion(major + "." + minor + "." + patch);
    int defaultMixinVersion = LIBMATTI_FML_MixinFacade_DEFAULT_BEHAVIOUR_VERSION;
    int patch = defaultMixinVersion % 1000;
    defaultMixinVersion /= 1000;
    int minor = defaultMixinVersion % 1000;
    defaultMixinVersion /= 1000;
    int major = defaultMixinVersion;
    char version[64];
    snprintf(version, sizeof(version), "%d.%d.%d", major, minor, patch);
    LIBMATTI_FML_MixinFacade_LOWEST_MIXIN_VERSION = LIBMATTI_NEOFORGESPI_MavenVersionAdapter_CreateFromVersion(version);
}

// Java: public MixinFacade()
LIBMATTI_FML_MixinFacade *LIBMATTI_FML_MixinFacade_New(void)
{
    LIBMATTI_FML_MixinFacade *facade = calloc(1, sizeof(*facade));
    initConstants();

    // Java: if (FMLLoader.getCurrent().getDist() == null) throw new IllegalStateException(...)
    // The port's Dist is a non-nullable enum filled in by FMLLoader.detectDist.

    // Java: System.setProperty("mixin.service", FMLMixinService.class.getName());
    LIBMATTI_JL_System_SetProperty("mixin.service", "net.neoforged.fml.loading.mixin.FMLMixinService");
    // Java: System.setProperty("mixin.bootstrapService", FMLMixinServiceBootstrap.class.getName());
    LIBMATTI_JL_System_SetProperty("mixin.bootstrapService", "net.neoforged.fml.loading.mixin.FMLMixinServiceBootstrap");

    // Java: MixinBootstrap.init();
    LIBMATTI_SP_MixinBootstrap_Init();

    // Java: service = (FMLMixinService) MixinService.getService();
    facade->service = LIBMATTI_FML_FMLMixinService_Instance();

    // Java: this.classProcessor = new FMLMixinClassProcessor(service);
    facade->classProcessor = LIBMATTI_FML_FMLMixinClassProcessor_New();
    // Java: this.generatingClassProcessor = new FMLMixinGeneratingClassProcessor(service);
    facade->generatingClassProcessor = LIBMATTI_FML_FMLMixinGeneratingClassProcessor_New();
    // The native mixin backend: the facade collects into the process-wide table.
    facade->hookTable = LIBMATTI_MIXIN_HookTable_Default();
    return facade;
}

// Java: public FMLMixinClassProcessor getClassProcessor()
void *LIBMATTI_FML_MixinFacade_GetClassProcessor(const LIBMATTI_FML_MixinFacade *facade)
{
    return facade->classProcessor;
}

// The native mixin backend
struct LIBMATTI_MIXIN_HookTable *LIBMATTI_FML_MixinFacade_GetHookTable(const LIBMATTI_FML_MixinFacade *facade)
{
    return facade->hookTable;
}

// Java: public FMLMixinGeneratingClassProcessor getGeneratingClassProcessor()
void *LIBMATTI_FML_MixinFacade_GetGeneratingClassProcessor(const LIBMATTI_FML_MixinFacade *facade)
{
    return facade->generatingClassProcessor;
}

// Java: Mixins.getConfigs().stream().collect(toMap(Config::getName, Config::getConfig))
LIBMATTI_FML_MixinFacade_MixinConfig *LIBMATTI_FML_MixinFacade_GetConfig(const LIBMATTI_FML_MixinFacade *facade,
                                                                         const char *name)
{
    for (size_t i = 0; i < facade->configCount; i++)
        if (strcmp(facade->configs[i].name, name) == 0) return &facade->configs[i];

    return NULL;
}

// Java's record withers are garbage collected; the port frees the issue the with* call replaced.
static void add_issue_with_mod_file(LIBMATTI_FML_ModLoadingIssue *issue, LIBMATTI_NEOFORGESPI_IModFile *file)
{
    LIBMATTI_FML_ModLoader_AddLoadingIssue(LIBMATTI_FML_ModLoadingIssue_WithAffectedModFile(
        issue, file, LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(file)));
    LIBMATTI_FML_ModLoadingIssue_Free(issue);
}

// Java: private void gotoPhase(MixinEnvironment.Phase phase)
//       { var m = MixinEnvironment.class.getDeclaredMethod("gotoPhase", Phase.class);
//         m.setAccessible(true); m.invoke(null, phase); }
// The port's MixinEnvironment exposes gotoPhase directly (Java only goes through reflection
// because the method is package-private).
static void gotoPhase(LIBMATTI_SP_MixinEnvironment_Phase *phase)
{
    LIBMATTI_SP_MixinEnvironment_GotoPhase(phase);
}

// Java: private static boolean areRequiredModsPresent(ModFile modFile, ModFileParser.MixinConfig mixinConfig, LoadingModList modList)
static int areRequiredModsPresent(LIBMATTI_FML_ModFile *modFile,
                                  const LIBMATTI_FML_ModFileParser_MixinConfig *mixinConfig,
                                  LIBMATTI_FML_LoadingModList *modList)
{
    for (size_t i = 0; i < mixinConfig->requiredModCount; i++)
    {
        if (LIBMATTI_FML_LoadingModList_GetModFileById(modList, mixinConfig->requiredMods[i]) == NULL)
        {
            // Java logs the ModFile itself; the port's logger only substitutes strings.
            LIBMATTI_ML_Logger_Info(LOG(), NULL,
                                    "Mixin config {} from {} not applied as required mod '{}' is missing",
                                    mixinConfig->config,
                                    LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(
                                        LIBMATTI_FML_ModFile_AsModFile(modFile)),
                                    mixinConfig->requiredMods[i]);
            return 0;
        }
    }
    return 1;
}

// Java: private static boolean validateMixinBehavior(ModFile modFile, ModFileParser.MixinConfig mixinConfig)
static int validateMixinBehavior(LIBMATTI_FML_ModFile *modFile,
                                 const LIBMATTI_FML_ModFileParser_MixinConfig *mixinConfig)
{
    const LIBMATTI_NEOFORGESPI_ArtifactVersion *behaviorVersion = mixinConfig->behaviorVersion;
    // Java: if (behaviorVersion == null) return true;
    if (behaviorVersion == NULL)
        return 1;

    const char *behavior = LIBMATTI_NEOFORGESPI_MavenVersionAdapter_VersionToString(behaviorVersion);
    LIBMATTI_NEOFORGESPI_IModFile *file = LIBMATTI_FML_ModFile_AsModFile(modFile);

    if (LIBMATTI_NEOFORGESPI_MavenVersionAdapter_Compare(behaviorVersion,
                                                        LIBMATTI_FML_MixinFacade_HIGHEST_MIXIN_VERSION) > 0)
    {
        const char *args[] = {mixinConfig->config, behavior,
                              LIBMATTI_NEOFORGESPI_MavenVersionAdapter_VersionToString(
                                  LIBMATTI_FML_MixinFacade_HIGHEST_MIXIN_VERSION)};
        LIBMATTI_FML_ModLoadingIssue *issue = LIBMATTI_FML_ModLoadingIssue_Error(
            "fml.modloadingissue.mixin.requested_behavior_too_new", args, 3);
        add_issue_with_mod_file(issue, file);
        return 0;
    }
    if (LIBMATTI_NEOFORGESPI_MavenVersionAdapter_Compare(behaviorVersion,
                                                        LIBMATTI_FML_MixinFacade_LOWEST_MIXIN_VERSION) < 0)
    {
        const char *args[] = {mixinConfig->config, behavior,
                              LIBMATTI_NEOFORGESPI_MavenVersionAdapter_VersionToString(
                                  LIBMATTI_FML_MixinFacade_LOWEST_MIXIN_VERSION)};
        LIBMATTI_FML_ModLoadingIssue *issue = LIBMATTI_FML_ModLoadingIssue_Error(
            "fml.modloadingissue.mixin.requested_behavior_too_old", args, 3);
        add_issue_with_mod_file(issue, file);
        return 0;
    }
    return 1;
}

// Java: private static int calculateBehaviorVersion(@Nullable ArtifactVersion behaviorVersion)
int LIBMATTI_FML_MixinFacade_CalculateBehaviorVersion(const LIBMATTI_NEOFORGESPI_ArtifactVersion *behaviorVersion)
{
    // Java: if (behaviorVersion == null) return DEFAULT_BEHAVIOUR_VERSION;
    if (behaviorVersion == NULL)
        return LIBMATTI_FML_MixinFacade_DEFAULT_BEHAVIOUR_VERSION;

    // Java: return behaviorVersion.getMajorVersion() * (1000 * 1000) + behaviorVersion.getMinorVersion() * 1000
    //             + behaviorVersion.getIncrementalVersion();
    return LIBMATTI_MAVEN_ArtifactVersion_GetMajorVersion(behaviorVersion) * (1000 * 1000) +
           LIBMATTI_MAVEN_ArtifactVersion_GetMinorVersion(behaviorVersion) * 1000 +
           LIBMATTI_MAVEN_ArtifactVersion_GetIncrementalVersion(behaviorVersion);
}

// Java: record AnnotationInfo(IModFile modFile, int behaviorVersion)
typedef struct
{
    char *name;
    LIBMATTI_NEOFORGESPI_IModFile *modFile;
    int behaviorVersion;
} AnnotationInfo;

static AnnotationInfo *annotation_info_find(AnnotationInfo *infos, size_t count, const char *name)
{
    for (size_t i = 0; i < count; i++)
        if (strcmp(infos[i].name, name) == 0) return &infos[i];

    return NULL;
}

// Java: private void addMixins(LoadingModList modList)
static void addMixins(LIBMATTI_FML_MixinFacade *facade, LIBMATTI_FML_LoadingModList *modList)
{
    AnnotationInfo *configAnnotationInfo = NULL;
    size_t annotationInfoCount = 0;

    size_t modFileCount = 0;
    LIBMATTI_FML_ModFileInfo **modFiles = LIBMATTI_FML_LoadingModList_GetModFiles(modList, &modFileCount);

    for (size_t i = 0; i < modFileCount; i++)
    {
        LIBMATTI_FML_ModFile *modFile = LIBMATTI_FML_ModFileInfo_GetFile(modFiles[i]);
        if (modFile == NULL) continue;

        size_t mixinConfigCount = 0;
        LIBMATTI_FML_ModFileParser_MixinConfig *mixinConfigs =
            LIBMATTI_FML_ModFile_GetMixinConfigs(modFile, &mixinConfigCount);

        for (size_t k = 0; k < mixinConfigCount; k++)
        {
            LIBMATTI_FML_ModFileParser_MixinConfig *mixinConfig = &mixinConfigs[k];
            if (!areRequiredModsPresent(modFile, mixinConfig, modList)) continue;
            if (!validateMixinBehavior(modFile, mixinConfig)) continue;

            AnnotationInfo current = {mixinConfig->config, LIBMATTI_FML_ModFile_AsModFile(modFile),
                                      LIBMATTI_FML_MixinFacade_CalculateBehaviorVersion(mixinConfig->behaviorVersion)};
            AnnotationInfo *existingInfo = annotation_info_find(configAnnotationInfo, annotationInfoCount, mixinConfig->config);
            if (existingInfo != NULL)
            {
                if (existingInfo->modFile != current.modFile)
                {
                    const char *args[] = {mixinConfig->config,
                                          LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(existingInfo->modFile)};
                    LIBMATTI_FML_ModLoadingIssue *issue = LIBMATTI_FML_ModLoadingIssue_Error(
                        "fml.modloadingissue.mixin.duplicate_config", args, 2);
                    add_issue_with_mod_file(issue, current.modFile);
                    continue;
                }
            }
            else
            {
                // Java: configAnnotationInfo.putIfAbsent(mixinConfig.config(), currentInfo)
                configAnnotationInfo = realloc(configAnnotationInfo, sizeof(*configAnnotationInfo) * (annotationInfoCount + 1));
                configAnnotationInfo[annotationInfoCount].name = strdup(mixinConfig->config);
                configAnnotationInfo[annotationInfoCount].modFile = current.modFile;
                configAnnotationInfo[annotationInfoCount].behaviorVersion = current.behaviorVersion;
                annotationInfoCount++;
            }

            // Java: modFile.getContents().readFile(mixinConfig.config())
            size_t contentLength = 0;
            unsigned char *content =
                LIBMATTI_FML_JarContents_ReadFile(LIBMATTI_FML_ModFile_GetContents(modFile), mixinConfig->config,
                                                  &contentLength);
            if (content == NULL)
            {
                // Java: NOTE: Archloom does not put common modules into the same MOD_CLASSES/modFolders group ...
                size_t otherCount = 0;
                LIBMATTI_NEOFORGESPI_IModFile **otherFiles =
                    LIBMATTI_FML_LoadingModList_GetAllModFiles(modList, &otherCount);
                for (size_t j = 0; j < otherCount && content == NULL; j++)
                {
                    if (otherFiles[j] == current.modFile) continue;

                    content = LIBMATTI_FML_JarContents_ReadFile(
                        LIBMATTI_FML_ModFile_GetContents(LIBMATTI_FML_ModFile_AsConcrete(otherFiles[j])),
                        mixinConfig->config, &contentLength);
                    if (content != NULL)
                        LIBMATTI_ML_Logger_Warn(LOG(), NULL,
                                                "Mod file {} declares mixin config {}, but it actually comes from {}",
                                                LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(current.modFile),
                                                mixinConfig->config,
                                                LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(otherFiles[j]));
                }
            }

            if (content == NULL)
            {
                const char *args[] = {mixinConfig->config};
                LIBMATTI_FML_ModLoadingIssue *issue = LIBMATTI_FML_ModLoadingIssue_Error(
                    "fml.modloadingissue.mixin.missing_config", args, 1);
                add_issue_with_mod_file(issue, current.modFile);
                continue;
            }

            // Java: service.addMixinConfigContent(mixinConfig.config(), configContent);
            LIBMATTI_FML_FMLMixinService_AddMixinConfigContent(mixinConfig->config, content, contentLength);
            // Java: Mixins.addConfiguration(mixinConfig.config());
            LIBMATTI_SP_Mixins_AddConfiguration(mixinConfig->config);

            // Java: the MixinConfig parse carried the content into the transformer's config view;
            // the facade's map (Mixins.getConfigs() keyed by Config::getName) keeps the same
            // two pieces for the diagnostics the harness reads.
            LIBMATTI_FML_MixinFacade_MixinConfig *registered =
                LIBMATTI_FML_MixinFacade_GetConfig(facade, mixinConfig->config);
            if (registered == NULL)
            {
                facade->configs = realloc(facade->configs, sizeof(*facade->configs) * (facade->configCount + 1));
                registered = &facade->configs[facade->configCount++];
                registered->name = strdup(mixinConfig->config);
                registered->content = content;
                registered->contentLength = contentLength;
                registered->modId = NULL;
                registered->behaviorVersion = 0;
            }
            else
            {
                free(registered->content);
                registered->content = content;
                registered->contentLength = contentLength;
            }
        }
    }

    // Java: for (var plugin : modList.getPlugins()) service.addMixinContainer(new FMLModFileContainerHandle(plugin.getFile()));
    size_t containerCount = 0;
    LIBMATTI_NEOFORGESPI_IModFileInfo **plugins = LIBMATTI_FML_LoadingModList_GetPlugins(modList, &containerCount);
    for (size_t i = 0; i < containerCount; i++)
    {
        LIBMATTI_FML_FMLModFileContainerHandle *handle = LIBMATTI_FML_FMLModFileContainerHandle_New(
            LIBMATTI_NEOFORGESPI_IModFileInfo_GetFile(plugins[i]));
        LIBMATTI_FML_FMLMixinService_AddMixinContainer(&handle->base);
    }

    // Java: for (var gameLibrary : modList.getGameLibraries()) service.addMixinContainer(new FMLModFileContainerHandle(gameLibrary));
    size_t gameLibraryCount = 0;
    LIBMATTI_NEOFORGESPI_IModFile **gameLibraries = LIBMATTI_FML_LoadingModList_GetGameLibraries(modList, &gameLibraryCount);
    for (size_t i = 0; i < gameLibraryCount; i++)
    {
        LIBMATTI_FML_FMLModFileContainerHandle *handle = LIBMATTI_FML_FMLModFileContainerHandle_New(gameLibraries[i]);
        LIBMATTI_FML_FMLMixinService_AddMixinContainer(&handle->base);
    }
    (void) gameLibraries;

    // Java: var configMap = Mixins.getConfigs().stream().collect(toMap(Config::getName, Config::getConfig));
    //       for (var entry : configAnnotationInfo.entrySet()) { var config = configMap.get(name); ... }
    size_t configSetCount = 0;
    LIBMATTI_SP_Config **configSetItems = LIBMATTI_SP_Mixins_GetConfigs(&configSetCount);
    for (size_t i = 0; i < annotationInfoCount; i++)
    {
        LIBMATTI_SP_Config *config = NULL;
        for (size_t j = 0; j < configSetCount; j++)
        {
            if (strcmp(configSetItems[j]->name, configAnnotationInfo[i].name) == 0)
            {
                config = configSetItems[j];
                break;
            }
        }

        if (config == NULL)
        {
            LIBMATTI_ML_Logger_Error(LOG(), NULL, "Mixin config {} from {} was not registered!",
                                     configAnnotationInfo[i].name,
                                     LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(
                                         configAnnotationInfo[i].modFile));
            continue;
        }

        // Java: config.decorate(FabricUtil.KEY_MOD_ID, modFile.getId());
        LIBMATTI_SP_Config_DecorateModId(config, LIBMATTI_NEOFORGESPI_IModFile_GetId(configAnnotationInfo[i].modFile));
        // Java: config.decorate(FabricUtil.KEY_COMPATIBILITY, annotationInfo.behaviorVersion());
        LIBMATTI_SP_Config_DecorateBehaviorVersion(config, configAnnotationInfo[i].behaviorVersion);

        // The facade's view carries the same decorations (the harness reads them through
        // LIBMATTI_FML_MixinFacade_GetConfig).
        LIBMATTI_FML_MixinFacade_MixinConfig *view =
            LIBMATTI_FML_MixinFacade_GetConfig(facade, configAnnotationInfo[i].name);
        if (view != NULL)
        {
            free(view->modId);
            view->modId = strdup(LIBMATTI_NEOFORGESPI_IModFile_GetId(configAnnotationInfo[i].modFile));
            view->behaviorVersion = configAnnotationInfo[i].behaviorVersion;
        }
    }

    // Java's AnnotationInfo record is garbage collected; the port frees the copies it made.
    for (size_t i = 0; i < annotationInfoCount; i++) free(configAnnotationInfo[i].name);
    free(configAnnotationInfo);
}

// Java: public void finishInitialization(LoadingModList loadingModList, TransformingClassLoader classLoader)
void LIBMATTI_FML_MixinFacade_FinishInitialization(LIBMATTI_FML_MixinFacade *facade,
                                                   LIBMATTI_FML_LoadingModList *loadingModList,
                                                   LIBMATTI_FML_TransformingClassLoader *classLoader)
{
    // Java: if (Thread.currentThread().getContextClassLoader() != classLoader) throw new IllegalStateException(...)
    if (LIBMATTI_JL_Thread_GetContextClassLoader(LIBMATTI_JL_Thread_CurrentThread()) !=
        (LIBMATTI_JL_ClassLoader *) classLoader)
    {
        LIBMATTI_ML_Logger_Error(LOG(), NULL,
                                 "The class loader must be the context classloader in order to find the Mixin configurations.");
    }

    addMixins(facade, loadingModList);

    // The native backend (M4): collect the .matti_mixins descriptors of the game binary and of every
    // mod file that was loaded as a shared object. Java collects the same way - through the mixin
    // containers the service holds - just over class metadata instead of ELF sections.
    LIBMATTI_MIXIN_HookTable *hookTable = facade->hookTable;

    // Java: the game's own targets come from the game module; the port reads the host image
    // (dlopen(NULL) is the game binary itself, whether PIE or not).
    size_t gameTargets = LIBMATTI_MIXIN_HookTable_AddObject(hookTable, NULL, NULL);

    size_t hookCount = 0;
    size_t modFileCount = 0;
    LIBMATTI_FML_ModFileInfo **modFiles =
        loadingModList != NULL ? LIBMATTI_FML_LoadingModList_GetModFiles(loadingModList, &modFileCount) : NULL;
    for (size_t i = 0; i < modFileCount; i++)
    {
        LIBMATTI_FML_ModFile *modFile = LIBMATTI_FML_ModFileInfo_GetFile(modFiles[i]);
        if (modFile == NULL) continue;

        const char *path = LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(LIBMATTI_FML_ModFile_AsModFile(modFile));
        const char *modId = LIBMATTI_NEOFORGESPI_IModFile_GetId(LIBMATTI_FML_ModFile_AsModFile(modFile));
        hookCount += LIBMATTI_MIXIN_HookTable_AddObject(hookTable, path, modId);
    }

    // GetModFiles returns the list's own array (Java: an unmodifiable list); it must not be freed.

    // The port's logger substitutes {} with strings only.
    char counts[128];
    snprintf(counts, sizeof(counts), "%zu game target(s), %zu mod hook(s)", gameTargets, hookCount);
    LIBMATTI_ML_Logger_Info(LOG(), NULL, "Mixin hook table: {}", counts);

    // Java: We must transition to DEFAULT phase for normal Mixins to be applied at all
    gotoPhase(LIBMATTI_SP_MixinEnvironment_Phase_INIT());
    gotoPhase(LIBMATTI_SP_MixinEnvironment_Phase_DEFAULT());

    // Java: MixinBootstrap.init(); MixinBootstrap.getPlatform().inject();
    LIBMATTI_SP_MixinBootstrap_Init();
    LIBMATTI_SP_MixinPlatformManager_Inject(LIBMATTI_SP_MixinBootstrap_GetPlatform());
}

// Java: public void close()
void LIBMATTI_FML_MixinFacade_Close(LIBMATTI_FML_MixinFacade *facade)
{
    // Java: service.setBytecodeProvider(null) - the bytecode provider holds a static global strong
    //       reference to the entire class-loader chain which will keep JAR files opened.
    LIBMATTI_FML_FMLMixinService_SetBytecodeProvider(NULL);
}

void LIBMATTI_FML_MixinFacade_Free(LIBMATTI_FML_MixinFacade *facade)
{
    if (facade == NULL) return;

    for (size_t i = 0; i < facade->configCount; i++)
    {
        free(facade->configs[i].name);
        free(facade->configs[i].content);
        free(facade->configs[i].modId);
    }
    free(facade->configs);
    free(facade);
}
