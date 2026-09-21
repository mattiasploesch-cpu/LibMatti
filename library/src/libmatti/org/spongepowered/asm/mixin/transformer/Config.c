// Port of org.spongepowered.asm.mixin.transformer.Config.
// The inner MixinConfig parse (minVersion/refmap/mixins/…) belongs to the bytecode transformer
// the port replaces with the native hook table; the marshalling handle keeps the name, the
// environment, the parent chain and the decorations 1:1.

#include "libmatti/org/spongepowered/asm/mixin/transformer/Config.h"

#include "libmatti/com/google/gson/JsonReader.h"
#include "libmatti/org/spongepowered/asm/service/IMixinService.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static LIBMATTI_SP_ILogger *LOGGER(void)
{
    LIBMATTI_SP_IMixinService *service = LIBMATTI_SP_MixinService_GetService();
    return service != NULL ? service->getLogger(service->self, "mixin") : NULL;
}

// Java: private static final Map<String, Config> allConfigs = new HashMap<String, Config>();
typedef struct
{
    LIBMATTI_SP_Config **configs;
    size_t count;
} ConfigMap;

static ConfigMap allConfigs = {NULL, 0};

static LIBMATTI_SP_Config *all_configs_get(const char *name)
{
    for (size_t i = 0; i < allConfigs.count; i++)
        if (strcmp(allConfigs.configs[i]->name, name) == 0) return allConfigs.configs[i];

    return NULL;
}

static void all_configs_put(LIBMATTI_SP_Config *config)
{
    allConfigs.configs = realloc(allConfigs.configs, sizeof(LIBMATTI_SP_Config *) * (allConfigs.count + 1));
    allConfigs.configs[allConfigs.count++] = config;
}

// Java: the mixin list parser - entries are class-name strings or
// {"class":..., "required":...} objects (Java: MixinInfo)
static void parse_mixin_list(LIBMATTI_GSON_JsonReader *reader, LIBMATTI_SP_MixinInfo **outMixins,
                             size_t *outCount)
{
    if (LIBMATTI_GSON_JsonReader_Peek(reader) != LIBMATTI_GSON_JSON_TOKEN_BEGIN_ARRAY)
    {
        LIBMATTI_GSON_JsonReader_SkipValue(reader);
        return;
    }

    LIBMATTI_SP_MixinInfo *mixins = NULL;
    size_t count = 0;
    LIBMATTI_GSON_JsonReader_BeginArray(reader);
    while (LIBMATTI_GSON_JsonReader_HasNext(reader))
    {
        LIBMATTI_SP_MixinInfo info = {0};
        if (LIBMATTI_GSON_JsonReader_Peek(reader) == LIBMATTI_GSON_JSON_TOKEN_STRING)
        {
            // Java: a plain class-name entry
            info.className = LIBMATTI_GSON_JsonReader_NextString(reader);
            info.required = 1; // Java: a plain entry in a required config is required
        }
        else if (LIBMATTI_GSON_JsonReader_Peek(reader) == LIBMATTI_GSON_JSON_TOKEN_BEGIN_OBJECT)
        {
            // Java: MixinInfo {"class":..., "required":...}
            LIBMATTI_GSON_JsonReader_BeginObject(reader);
            while (LIBMATTI_GSON_JsonReader_HasNext(reader))
            {
                char *key = LIBMATTI_GSON_JsonReader_NextName(reader);
                if (strcmp(key, "class") == 0)
                    info.className = LIBMATTI_GSON_JsonReader_NextString(reader);
                else if (strcmp(key, "required") == 0)
                    info.required = LIBMATTI_GSON_JsonReader_NextBoolean(reader);
                else
                    LIBMATTI_GSON_JsonReader_SkipValue(reader);
                free(key);
            }
            LIBMATTI_GSON_JsonReader_EndObject(reader);
        }
        else
        {
            LIBMATTI_GSON_JsonReader_SkipValue(reader);
            continue;
        }

        if (info.className == NULL)
            continue;

        mixins = realloc(mixins, sizeof(LIBMATTI_SP_MixinInfo) * (count + 1));
        mixins[count++] = info;
    }
    LIBMATTI_GSON_JsonReader_EndArray(reader);

    *outMixins = mixins;
    *outCount = count;
}

// Java: the injector block parser - Map<String, InjectorInfo>
static void parse_injectors(LIBMATTI_GSON_JsonReader *reader, char ***outKeys,
                            LIBMATTI_SP_InjectorInfo **outValues, size_t *outCount)
{
    if (LIBMATTI_GSON_JsonReader_Peek(reader) != LIBMATTI_GSON_JSON_TOKEN_BEGIN_OBJECT)
    {
        LIBMATTI_GSON_JsonReader_SkipValue(reader);
        return;
    }

    char **keys = NULL;
    LIBMATTI_SP_InjectorInfo *values = NULL;
    size_t count = 0;

    LIBMATTI_GSON_JsonReader_BeginObject(reader);
    while (LIBMATTI_GSON_JsonReader_HasNext(reader))
    {
        char *mixinClass = LIBMATTI_GSON_JsonReader_NextName(reader);
        LIBMATTI_SP_InjectorInfo info = {0};
        info.ordinal = -1;

        // Java: InjectorInfo {"method":..., "ordinal":..., "value":...}
        if (LIBMATTI_GSON_JsonReader_Peek(reader) == LIBMATTI_GSON_JSON_TOKEN_BEGIN_OBJECT)
        {
            LIBMATTI_GSON_JsonReader_BeginObject(reader);
            while (LIBMATTI_GSON_JsonReader_HasNext(reader))
            {
                char *key = LIBMATTI_GSON_JsonReader_NextName(reader);
                if (strcmp(key, "method") == 0)
                    info.method = LIBMATTI_GSON_JsonReader_NextString(reader);
                else if (strcmp(key, "ordinal") == 0)
                    info.ordinal = atoi(LIBMATTI_GSON_JsonReader_NextString(reader));
                else if (strcmp(key, "value") == 0)
                    info.value = atoi(LIBMATTI_GSON_JsonReader_NextString(reader));
                else
                    LIBMATTI_GSON_JsonReader_SkipValue(reader);
                free(key);
            }
            LIBMATTI_GSON_JsonReader_EndObject(reader);
        }
        else
        {
            LIBMATTI_GSON_JsonReader_SkipValue(reader);
            free(mixinClass);
            continue;
        }

        keys = realloc(keys, sizeof(char *) * (count + 1));
        values = realloc(values, sizeof(LIBMATTI_SP_InjectorInfo) * (count + 1));
        keys[count] = mixinClass;
        values[count] = info;
        count++;
    }
    LIBMATTI_GSON_JsonReader_EndObject(reader);

    *outKeys = keys;
    *outValues = values;
    *outCount = count;
}

// Java: reads the config's JSON fields the loader layer cares about
// (MixinConfig.onLoad's parse: parent, package, refmap, required, minVersion, compatibilityLevel)
static void parse_config(LIBMATTI_SP_Config *config, const unsigned char *content, size_t length)
{
    LIBMATTI_GSON_JsonReader *reader = LIBMATTI_GSON_JsonReader_New((const char *) content, length);
    if (reader == NULL) return;

    if (LIBMATTI_GSON_JsonReader_Peek(reader) != LIBMATTI_GSON_JSON_TOKEN_BEGIN_OBJECT)
    {
        LIBMATTI_GSON_JsonReader_Free(reader);
        return;
    }
    LIBMATTI_GSON_JsonReader_BeginObject(reader);

    while (LIBMATTI_GSON_JsonReader_HasNext(reader))
    {
        char *name = LIBMATTI_GSON_JsonReader_NextName(reader);
        if (name == NULL) break;

        if (strcmp(name, "parent") == 0)
        {
            // Java: @SerializedName("parent") private String parentName;
            char *parent = LIBMATTI_GSON_JsonReader_NextString(reader);
            if (parent != NULL)
            {
                free(config->parentName);
                config->parentName = parent;
            }
        }
        else if (strcmp(name, "package") == 0)
        {
            // Java: @SerializedName("package") private String pack;
            char *value = LIBMATTI_GSON_JsonReader_NextString(reader);
            if (value != NULL)
                config->packageName = value;
        }
        else if (strcmp(name, "refmap") == 0)
        {
            // Java: @SerializedName("refmap") private String refMapName;
            char *value = LIBMATTI_GSON_JsonReader_NextString(reader);
            if (value != NULL)
                config->refMapName = value;
        }
        else if (strcmp(name, "required") == 0)
        {
            // Java: @SerializedName("required") private boolean required;
            config->required = LIBMATTI_GSON_JsonReader_NextBoolean(reader);
        }
        else if (strcmp(name, "minVersion") == 0)
        {
            // Java: @SerializedName("minVersion") private float minVersion;
            config->minVersion = (float) atof(LIBMATTI_GSON_JsonReader_NextString(reader));
        }
        else if (strcmp(name, "compatibilityLevel") == 0)
        {
            // Java: @SerializedName("compatibilityLevel") private CompatibilityLevel compatibilityLevel;
            char *value = LIBMATTI_GSON_JsonReader_NextString(reader);
            if (value != NULL)
                config->compatibilityLevel = value;
        }
        else if (strcmp(name, "plugin") == 0)
        {
            // Java: @SerializedName("plugin") private String pluginClass;
            char *value = LIBMATTI_GSON_JsonReader_NextString(reader);
            if (value != NULL)
                config->pluginClass = value;
        }
        else if (strcmp(name, "mixins") == 0)
        {
            // Java: @SerializedName("mixins") private List<MixinInfo> mixins;
            parse_mixin_list(reader, &config->mixins, &config->mixinCount);
        }
        else if (strcmp(name, "client") == 0)
        {
            // Java: @SerializedName("client") private List<MixinInfo> clientMixins;
            parse_mixin_list(reader, &config->clientMixins, &config->clientMixinCount);
        }
        else if (strcmp(name, "server") == 0)
        {
            // Java: @SerializedName("server") private List<MixinInfo> serverMixins;
            parse_mixin_list(reader, &config->serverMixins, &config->serverMixinCount);
        }
        else if (strcmp(name, "injectors") == 0)
        {
            // Java: @SerializedName("injectors") private Map<String,InjectorInfo> injectors;
            parse_injectors(reader, &config->injectorKeys, &config->injectorValues, &config->injectorCount);
        }
        else
        {
            // Java: the remaining transformer fields (overwrites, verbose, showMessageTypes);
            // the native backend keeps them in the config content.
            LIBMATTI_GSON_JsonReader_SkipValue(reader);
        }
        free(name);
    }

    LIBMATTI_GSON_JsonReader_EndObject(reader);
    LIBMATTI_GSON_JsonReader_Free(reader);
}

// Java: public Config(MixinConfig config)
LIBMATTI_SP_Config *LIBMATTI_SP_Config_New(const char *name, const unsigned char *content, size_t length)
{
    LIBMATTI_SP_Config *config = calloc(1, sizeof(*config));
    config->name = strdup(name);
    if (content != NULL && length > 0)
    {
        config->configContent = malloc(length);
        memcpy(config->configContent, content, length);
        config->configContentLength = length;
        parse_config(config, content, length);
    }
    return config;
}

// Java: Set.add(config) - deduplicates by name like Config.equals
void LIBMATTI_SP_ConfigSet_Add(LIBMATTI_SP_ConfigSet *set, LIBMATTI_SP_Config *config)
{
    for (size_t i = 0; i < set->count; i++)
        if (strcmp(set->items[i]->name, config->name) == 0) return;

    if (set->count == set->capacity)
    {
        set->capacity = set->capacity > 0 ? set->capacity * 2 : 4;
        set->items = realloc(set->items, sizeof(LIBMATTI_SP_Config *) * set->capacity);
    }
    set->items[set->count++] = config;
}

// Java: public String getName()
const char *LIBMATTI_SP_Config_GetName(const LIBMATTI_SP_Config *config)
{
    return config->name;
}

// Java: public boolean isVisited()
int LIBMATTI_SP_Config_IsVisited(const LIBMATTI_SP_Config *config)
{
    return config->visited;
}

// Java: MixinConfig get() - the inner config view
const unsigned char *LIBMATTI_SP_Config_GetContent(const LIBMATTI_SP_Config *config, size_t *length)
{
    *length = config->configContentLength;
    return config->configContent;
}

// Java: public MixinEnvironment getEnvironment()
void *LIBMATTI_SP_Config_GetEnvironment(const LIBMATTI_SP_Config *config)
{
    return config->environment;
}

// Java: public Config getParent()
LIBMATTI_SP_Config *LIBMATTI_SP_Config_GetParent(const LIBMATTI_SP_Config *config)
{
    return config->parent;
}

// Java: boolean assignParent(Config parentConfig)
int LIBMATTI_SP_Config_AssignParent(LIBMATTI_SP_Config *config, LIBMATTI_SP_Config *parentConfig)
{
    if (config->parent != NULL) return 0;
    config->parent = parentConfig;
    return 1;
}

// Java: String getParentName()
const char *LIBMATTI_SP_Config_GetParentName(const LIBMATTI_SP_Config *config)
{
    return config->parentName;
}

// Java: config.decorate(FabricUtil.KEY_MOD_ID, modFile.getId())
void LIBMATTI_SP_Config_DecorateModId(LIBMATTI_SP_Config *config, const char *modId)
{
    free(config->modId);
    config->modId = strdup(modId);
}

// Java: config.decorate(FabricUtil.KEY_COMPATIBILITY, annotationInfo.behaviorVersion())
void LIBMATTI_SP_Config_DecorateBehaviorVersion(LIBMATTI_SP_Config *config, int behaviorVersion)
{
    config->behaviorVersion = behaviorVersion;
    config->hasBehaviorVersion = 1;
}

// Java: MixinInfo.getRefName() - the package-qualified mixin class name
char *LIBMATTI_SP_Config_MixinRefName(const LIBMATTI_SP_Config *config, const LIBMATTI_SP_MixinInfo *mixin)
{
    // Java: pack != null ? pack + '.' + name : name
    if (config->packageName != NULL)
    {
        size_t length = strlen(config->packageName) + strlen(mixin->className) + 2;
        char *refName = malloc(length);
        snprintf(refName, length, "%s.%s", config->packageName, mixin->className);
        return refName;
    }
    return strdup(mixin->className);
}

const char *LIBMATTI_SP_Config_GetPackage(const LIBMATTI_SP_Config *config)
{
    return config->packageName;
}

const char *LIBMATTI_SP_Config_GetRefMapName(const LIBMATTI_SP_Config *config)
{
    return config->refMapName;
}

int LIBMATTI_SP_Config_IsRequired(const LIBMATTI_SP_Config *config)
{
    return config->required;
}

float LIBMATTI_SP_Config_GetMinVersion(const LIBMATTI_SP_Config *config)
{
    return config->minVersion;
}

const char *LIBMATTI_SP_Config_GetCompatibilityLevel(const LIBMATTI_SP_Config *config)
{
    return config->compatibilityLevel;
}

const LIBMATTI_SP_MixinInfo *LIBMATTI_SP_Config_GetMixins(const LIBMATTI_SP_Config *config, size_t *count)
{
    *count = config->mixinCount;
    return config->mixins;
}

const LIBMATTI_SP_MixinInfo *LIBMATTI_SP_Config_GetClientMixins(const LIBMATTI_SP_Config *config, size_t *count)
{
    *count = config->clientMixinCount;
    return config->clientMixins;
}

const LIBMATTI_SP_MixinInfo *LIBMATTI_SP_Config_GetServerMixins(const LIBMATTI_SP_Config *config, size_t *count)
{
    *count = config->serverMixinCount;
    return config->serverMixins;
}

const char *LIBMATTI_SP_Config_GetPluginClass(const LIBMATTI_SP_Config *config)
{
    return config->pluginClass;
}

const LIBMATTI_SP_InjectorInfo *LIBMATTI_SP_Config_GetInjector(const LIBMATTI_SP_Config *config,
                                                               const char *mixinClassName)
{
    // Java: injectors.get(mixinClassName) - the key matches with and without the package prefix
    for (size_t i = 0; i < config->injectorCount; i++)
    {
        if (strcmp(config->injectorKeys[i], mixinClassName) == 0)
            return &config->injectorValues[i];
    }
    for (size_t i = 0; i < config->injectorCount; i++)
    {
        // Java: the key without the package (the JSON often writes the short name)
        const char *lastDot = strrchr(config->injectorKeys[i], '.');
        const char *keyShort = lastDot != NULL ? lastDot + 1 : config->injectorKeys[i];
        const char *classShort = strrchr(mixinClassName, '.');
        classShort = classShort != NULL ? classShort + 1 : mixinClassName;
        if (strcmp(keyShort, classShort) == 0)
            return &config->injectorValues[i];
    }
    return NULL;
}

// Java: public void free / the C port's destructor
void LIBMATTI_SP_Config_Free(LIBMATTI_SP_Config *config)
{
    if (config == NULL) return;
    free(config->name);
    free(config->configContent);
    free(config->modId);
    free(config->parentName);
    free(config->packageName);
    free(config->refMapName);
    free(config->compatibilityLevel);
    free(config->pluginClass);
    for (size_t i = 0; i < config->mixinCount; i++)
        free(config->mixins[i].className);
    free(config->mixins);
    for (size_t i = 0; i < config->clientMixinCount; i++)
        free(config->clientMixins[i].className);
    free(config->clientMixins);
    for (size_t i = 0; i < config->serverMixinCount; i++)
        free(config->serverMixins[i].className);
    free(config->serverMixins);
    for (size_t i = 0; i < config->injectorCount; i++)
    {
        free(config->injectorKeys[i]);
        free(config->injectorValues[i].method);
    }
    free(config->injectorKeys);
    free(config->injectorValues);
    free(config);
}

// Java: @Deprecated public static Config create(String configFile, MixinEnvironment outer,
//                                               IMixinConfigSource source)
LIBMATTI_SP_Config *LIBMATTI_SP_Config_Create(const char *configFile, void *outer,
                                              LIBMATTI_SP_ContainerHandle *source)
{
    (void) source;

    // Java: Config config = Config.allConfigs.get(configFile); if (config != null) return config;
    LIBMATTI_SP_Config *existing = all_configs_get(configFile);
    if (existing != NULL) return existing;

    // Java: MixinConfig.create reads the config resource through
    //       MixinService.getService().getResourceAsStream(configFile) - the service carries what
    //       MixinFacade handed over via addMixinConfigContent; an absent resource yields no config.
    LIBMATTI_SP_IMixinService *service = LIBMATTI_SP_MixinService_GetService();
    size_t contentLength = 0;
    unsigned char *content = NULL;
    if (service != NULL && service->getResourceAsStream != NULL)
        content = service->getResourceAsStream(service->self, configFile, &contentLength);
    if (content == NULL) return NULL;

    LIBMATTI_SP_Config *config = LIBMATTI_SP_Config_New(configFile, content, contentLength);
    config->environment = outer;

    all_configs_put(config);

    // Java: String parent = config.get().getParentName();
    //       if (!Strings.isNullOrEmpty(parent)) { parentConfig = Config.create(parent, outer, source); ... }
    const char *parent = config->parentName;
    if (parent != NULL && parent[0] != '\0')
    {
        LIBMATTI_SP_Config *parentConfig = LIBMATTI_SP_Config_Create(parent, outer, source);
        if (parentConfig != NULL)
        {
            // Java: if (!config.get().assignParent(parentConfig)) config = null;
            if (!LIBMATTI_SP_Config_AssignParent(config, parentConfig)) config = NULL;
        }
        else
        {
            // Java: Config.logger.error("Error encountered initialising mixin config {0}: ...");
            char *msg = malloc(strlen("Error encountered initialising mixin config : The parent  could not be read.")
                               + strlen(configFile) + strlen(parent) + 1);
            sprintf(msg, "Error encountered initialising mixin config %s: The parent %s could not be read.",
                    configFile, parent);
            LIBMATTI_SP_ILogger_Error(LOGGER(), msg);
            free(msg);
        }
    }

    return config;
}
