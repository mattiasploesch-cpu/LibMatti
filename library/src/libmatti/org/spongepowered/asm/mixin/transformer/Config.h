// Port of org.spongepowered.asm.mixin.transformer.Config (sponge-mixin 0.17.3+mixin.0.8.7).
// "Handle for marshalling mixin configs outside of the transformer package". The bytecode
// transformer's MixinConfig (the parsed JSON) stays behind the port's native hook table; the
// marshalling handle keeps the config name, its environment, its parent and the decorations.

#ifndef MATTICRAFT_SP_ASM_MIXIN_TRANSFORMER_CONFIG_H
#define MATTICRAFT_SP_ASM_MIXIN_TRANSFORMER_CONFIG_H

#include "libmatti/org/spongepowered/asm/launch/platform/container/IContainerHandle.h"
#include "libmatti/org/spongepowered/asm/mixin/MixinEnvironment.h"

#include <stddef.h>

// Java: public class Config
typedef struct LIBMATTI_SP_Config LIBMATTI_SP_Config;

// Java: the Set<Config> the blackboard carries (a LinkedHashSet; the port keeps insertion order
// and deduplicates by name)
typedef struct
{
    LIBMATTI_SP_Config **items;
    size_t count;
    size_t capacity;
} LIBMATTI_SP_ConfigSet;

// Java: Set.add(config) - deduplicates by name like Config.equals
void LIBMATTI_SP_ConfigSet_Add(LIBMATTI_SP_ConfigSet *set, LIBMATTI_SP_Config *config);

// Java: record MixinInfo - the "mixins" array entries as parsed
// (the class names the config declares; the package prefix resolves them)
typedef struct
{
    char *className;   // the entry as written in the JSON
    int required;      // Java: @SerializedName("required") on MixinInfo
} LIBMATTI_SP_MixinInfo;

// Java: record InjectorInfo - the per-mixin injector block
// ({"method":"target()V","ordinal":0,"shift":0,...})
typedef struct
{
    char *method;  // the target method spec, NULL = default
    int ordinal;   // -1 = unset
    int value;     // Java: the "value" field (groupId/expect)
} LIBMATTI_SP_InjectorInfo;

struct LIBMATTI_SP_Config
{
    // Java: private final String name
    char *name;
    // Java: private final MixinConfig config - the parsed config; the port keeps the JSON text
    unsigned char *configContent;
    size_t configContentLength;
    // Java: MixinEnvironment getEnvironment() (the config's env)
    void *environment;
    // Java: the decorations (FabricUtil.KEY_MOD_ID / KEY_COMPATIBILITY write these)
    char *modId;
    int behaviorVersion;
    int hasBehaviorVersion;
    // Java: private transient boolean visited
    int visited;
    // Java: String getParentName() of the inner config
    char *parentName;
    // Java: private final Config parent (assigned by assignParent)
    LIBMATTI_SP_Config *parent;

    // ---- the parsed MixinConfig fields (Java: MixinConfig.onLoad / readObject) ----
    // Java: @SerializedName("package") private String pack
    char *packageName;
    // Java: @SerializedName("refmap") private String refMapName
    char *refMapName;
    // Java: @SerializedName("required") private boolean required
    int required;
    // Java: @SerializedName("minVersion") private float minVersion
    float minVersion;
    // Java: @SerializedName("compatibilityLevel") private CompatibilityLevel compatibilityLevel
    char *compatibilityLevel;
    // Java: @SerializedName("mixins") private List<MixinInfo> mixins
    LIBMATTI_SP_MixinInfo *mixins;
    size_t mixinCount;
    // Java: @SerializedName("client") private List<MixinInfo> clientMixins
    LIBMATTI_SP_MixinInfo *clientMixins;
    size_t clientMixinCount;
    // Java: @SerializedName("server") private List<MixinInfo> serverMixins
    LIBMATTI_SP_MixinInfo *serverMixins;
    size_t serverMixinCount;
    // Java: private String pluginClass (@SerializedName("plugin"))
    char *pluginClass;
    // Java: @SerializedName("injectors") private Map<String,InjectorInfo> injectors (per mixin class)
    char **injectorKeys;
    LIBMATTI_SP_InjectorInfo *injectorValues;
    size_t injectorCount;
};

// Java: public Config(MixinConfig config) / the port's factory from the resource content
LIBMATTI_SP_Config *LIBMATTI_SP_Config_New(const char *name, const unsigned char *content, size_t length);
void LIBMATTI_SP_Config_Free(LIBMATTI_SP_Config *config);

// Java: public String getName()
const char *LIBMATTI_SP_Config_GetName(const LIBMATTI_SP_Config *config);
// Java: public boolean isVisited()
int LIBMATTI_SP_Config_IsVisited(const LIBMATTI_SP_Config *config);
// Java: MixinConfig get() - the inner config view (the port's content)
const unsigned char *LIBMATTI_SP_Config_GetContent(const LIBMATTI_SP_Config *config, size_t *length);
// Java: public MixinEnvironment getEnvironment()
void *LIBMATTI_SP_Config_GetEnvironment(const LIBMATTI_SP_Config *config);
// Java: public Config getParent()
LIBMATTI_SP_Config *LIBMATTI_SP_Config_GetParent(const LIBMATTI_SP_Config *config);
// Java: boolean assignParent(Config parentConfig) (package-private)
int LIBMATTI_SP_Config_AssignParent(LIBMATTI_SP_Config *config, LIBMATTI_SP_Config *parentConfig);
// Java: String getParentName() (package-private)
const char *LIBMATTI_SP_Config_GetParentName(const LIBMATTI_SP_Config *config);

// Java: the config's decorations (FabricUtil KEY_MOD_ID / KEY_COMPATIBILITY)
void LIBMATTI_SP_Config_DecorateModId(LIBMATTI_SP_Config *config, const char *modId);
void LIBMATTI_SP_Config_DecorateBehaviorVersion(LIBMATTI_SP_Config *config, int behaviorVersion);

// Java: the parsed MixinConfig accessors
const char *LIBMATTI_SP_Config_GetPackage(const LIBMATTI_SP_Config *config);
const char *LIBMATTI_SP_Config_GetRefMapName(const LIBMATTI_SP_Config *config);
int LIBMATTI_SP_Config_IsRequired(const LIBMATTI_SP_Config *config);
float LIBMATTI_SP_Config_GetMinVersion(const LIBMATTI_SP_Config *config);
const char *LIBMATTI_SP_Config_GetCompatibilityLevel(const LIBMATTI_SP_Config *config);
// Java: MixinInfo list accessors - the caller frees nothing (config-owned)
const LIBMATTI_SP_MixinInfo *LIBMATTI_SP_Config_GetMixins(const LIBMATTI_SP_Config *config, size_t *count);
const LIBMATTI_SP_MixinInfo *LIBMATTI_SP_Config_GetClientMixins(const LIBMATTI_SP_Config *config, size_t *count);
const LIBMATTI_SP_MixinInfo *LIBMATTI_SP_Config_GetServerMixins(const LIBMATTI_SP_Config *config, size_t *count);
// Java: the plugin (@SerializedName("plugin")) - NULL when the config has none
const char *LIBMATTI_SP_Config_GetPluginClass(const LIBMATTI_SP_Config *config);
// Java: the injector block for a mixin class name, NULL when none
const LIBMATTI_SP_InjectorInfo *LIBMATTI_SP_Config_GetInjector(const LIBMATTI_SP_Config *config,
                                                               const char *mixinClassName);
// Java: MixinInfo.getRefName() - the package-qualified mixin class name; caller frees
char *LIBMATTI_SP_Config_MixinRefName(const LIBMATTI_SP_Config *config, const LIBMATTI_SP_MixinInfo *mixin);

// Java: @Deprecated public static Config create(String configFile, MixinEnvironment outer,
//                                               IMixinConfigSource source)
LIBMATTI_SP_Config *LIBMATTI_SP_Config_Create(const char *configFile, void *outer,
                                              LIBMATTI_SP_ContainerHandle *source);

#endif //MATTICRAFT_SP_ASM_MIXIN_TRANSFORMER_CONFIG_H
