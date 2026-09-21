// Port of net.minecraft.core.registries.BuiltInRegistries.
// Java's static initialisers become lazy constructors; internalRegister stores the bootstrap
// loader in the LOADERS map and registers the registry into WRITABLE_REGISTRY, bootStrap()
// runs the loaders (the per-registry contents are the game port's part), freezes everything
// and validates. The registration order and the NeoForge events are driven through GameData,
// exactly like the Java call sites.

#include "libmatti/net/minecraft/core/registries/BuiltInRegistries.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/minecraft/Bootstrap.h"
#include "libmatti/net/minecraft/core/Registry.h"
#include "libmatti/net/minecraft/core/registries/Registries.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBootstrap.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/neoforge/registries/GameData.h"
#include "libmatti/net/neoforged/neoforge/registries/RegistryManager.h"

#include <stdlib.h>
#include <string.h>

// Java: private static final Map<Identifier, Supplier<?>> LOADERS = Maps.newLinkedHashMap()
typedef struct RegistryLoader
{
    LIBMATTI_MC_Identifier *identifier;
    // Java: RegistryBootstrap<T>.run(registry) - NULL in the port until the game port fills
    // the registry (the contents are Minecraft's part)
    int ran;
    struct RegistryLoader *next;
} RegistryLoader;

static RegistryLoader *loaders = NULL;
static RegistryLoader *loadersTail = NULL;

// Java: private static final WritableRegistry<WritableRegistry<?>> WRITABLE_REGISTRY =
//       new MappedRegistry<>(ResourceKey.createRegistryKey(Registries.ROOT_REGISTRY_NAME),
//       Lifecycle.stable())
static LIBMATTI_MC_MappedRegistry *writableRegistry = NULL;

static LIBMATTI_MC_MappedRegistry *root_registry(void)
{
    if (writableRegistry == NULL)
    {
        LIBMATTI_MC_Identifier *root = LIBMATTI_MC_MC_Registries_RootRegistryName();
        writableRegistry = LIBMATTI_MC_MappedRegistry_New(LIBMATTI_MC_ResourceKey_CreateRegistryKey(root),
                                                          LIBMATTI_MJS_Lifecycle_Stable(), 0);
    }
    return writableRegistry;
}

// Java: private static <T, R extends WritableRegistry<T>> R internalRegister(
//       ResourceKey<? extends Registry<T>> key, R registry, RegistryBootstrap<T> bootstrap)
static LIBMATTI_MC_MappedRegistry *internal_register(LIBMATTI_MC_ResourceKey *key,
                                                     LIBMATTI_MC_MappedRegistry *registry)
{
    char *name = LIBMATTI_MC_Identifier_ToString(key->identifier);
    LIBMATTI_MC_Bootstrap_CheckBootstrapCalled(name);

    RegistryLoader *loader = calloc(1, sizeof(RegistryLoader));
    loader->identifier = LIBMATTI_MC_Identifier_New(LIBMATTI_MC_Identifier_GetNamespace(key->identifier),
                                                    LIBMATTI_MC_Identifier_GetPath(key->identifier));
    if (loadersTail == NULL)
        loaders = loader;
    else
        loadersTail->next = loader;
    loadersTail = loader;

    // Java: WRITABLE_REGISTRY.register((ResourceKey) key, registry, RegistrationInfo.BUILT_IN)
    LIBMATTI_MC_Registry_Register(root_registry(), key, registry);

    // The vanilla registry keys snapshot of RegistryManager and the GameData registration
    // order are built from the root registry in Java; the port feeds them here
    LIBMATTI_NEOFORGE_RegistryManager_AddVanillaRegistryKey(name);
    LIBMATTI_NEOFORGE_GameData_AddRootRegistry(name);
    free(name);
    return registry;
}

// Java: private static <T> Registry<T> registerSimple(ResourceKey, RegistryBootstrap<T>)
static LIBMATTI_MC_MappedRegistry *register_simple(LIBMATTI_MC_ResourceKey *key)
{
    return internal_register(key,
                             LIBMATTI_MC_MappedRegistry_New(key, LIBMATTI_MJS_Lifecycle_Stable(), 0));
}

// Java: private static <T> Registry<T> registerSimpleWithIntrusiveHolders(ResourceKey, ...)
static LIBMATTI_MC_MappedRegistry *register_simple_intrusive(LIBMATTI_MC_ResourceKey *key)
{
    return internal_register(key,
                             LIBMATTI_MC_MappedRegistry_New(key, LIBMATTI_MJS_Lifecycle_Stable(), 1));
}

// Java: private static <T> DefaultedRegistry<T> registerDefaulted(ResourceKey, String, ...)
static LIBMATTI_MC_MappedRegistry *register_defaulted(LIBMATTI_MC_ResourceKey *key, const char *defaultKey)
{
    return internal_register(key,
                             LIBMATTI_MC_DefaultedMappedRegistry_New(defaultKey, key,
                                                                     LIBMATTI_MJS_Lifecycle_Stable(), 0));
}

// Java: private static <T> DefaultedRegistry<T> registerDefaultedWithIntrusiveHolders(...)
static LIBMATTI_MC_MappedRegistry *register_defaulted_intrusive(LIBMATTI_MC_ResourceKey *key, const char *defaultKey)
{
    return internal_register(key,
                             LIBMATTI_MC_DefaultedMappedRegistry_New(defaultKey, key,
                                                                     LIBMATTI_MJS_Lifecycle_Stable(), 1));
}

// One lazy constructor per Java static, in declaration order (the vanilla registration order)

// Java: public static final DefaultedRegistry<GameEvent> GAME_EVENT = registerDefaulted(Registries.GAME_EVENT, "step", ...)
LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_GAME_EVENT(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_defaulted(LIBMATTI_MC_Registries_GAME_EVENT(), "step");
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_SOUND_EVENT(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_SOUND_EVENT());
    return registry;
}

// Java: FLUID = registerDefaultedWithIntrusiveHolders(Registries.FLUID, "empty", ...)
LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_FLUID(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_defaulted_intrusive(LIBMATTI_MC_Registries_FLUID(), "empty");
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_MOB_EFFECT(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_MOB_EFFECT());
    return registry;
}

// Java: BLOCK = registerDefaultedWithIntrusiveHolders(Registries.BLOCK, "air", ...)
LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_BLOCK(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_defaulted_intrusive(LIBMATTI_MC_Registries_BLOCK(), "air");
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_DEBUG_SUBSCRIPTION(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_DEBUG_SUBSCRIPTION());
    return registry;
}

// Java: ENTITY_TYPE = registerDefaultedWithIntrusiveHolders(Registries.ENTITY_TYPE, "pig", ...)
LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_ENTITY_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_defaulted_intrusive(LIBMATTI_MC_Registries_ENTITY_TYPE(), "pig");
    return registry;
}

// Java: ITEM = registerDefaultedWithIntrusiveHolders(Registries.ITEM, "air", ...)
LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_ITEM(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_defaulted_intrusive(LIBMATTI_MC_Registries_ITEM(), "air");
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_POTION(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_POTION());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_PARTICLE_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_PARTICLE_TYPE());
    return registry;
}

// Java: BLOCK_ENTITY_TYPE = registerSimpleWithIntrusiveHolders(Registries.BLOCK_ENTITY_TYPE, ...)
LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_BLOCK_ENTITY_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple_intrusive(LIBMATTI_MC_Registries_BLOCK_ENTITY_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_CUSTOM_STAT(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_CUSTOM_STAT());
    return registry;
}

// Java: CHUNK_STATUS = registerDefaulted(Registries.CHUNK_STATUS, "empty", ...)
LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_CHUNK_STATUS(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_defaulted(LIBMATTI_MC_Registries_CHUNK_STATUS(), "empty");
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_RULE_TEST(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_RULE_TEST());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_RULE_BLOCK_ENTITY_MODIFIER(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_RULE_BLOCK_ENTITY_MODIFIER());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_POS_RULE_TEST(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_POS_RULE_TEST());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_MENU(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_MENU());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_RECIPE_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_RECIPE_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_RECIPE_SERIALIZER(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_RECIPE_SERIALIZER());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_ATTRIBUTE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_ATTRIBUTE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_POSITION_SOURCE_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_POSITION_SOURCE_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_COMMAND_ARGUMENT_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_COMMAND_ARGUMENT_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_STAT_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_STAT_TYPE());
    return registry;
}

// Java: VILLAGER_TYPE = registerDefaulted(Registries.VILLAGER_TYPE, "plains", ...)
LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_VILLAGER_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_defaulted(LIBMATTI_MC_Registries_VILLAGER_TYPE(), "plains");
    return registry;
}

// Java: VILLAGER_PROFESSION = registerDefaulted(Registries.VILLAGER_PROFESSION, "none", ...)
LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_VILLAGER_PROFESSION(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_defaulted(LIBMATTI_MC_Registries_VILLAGER_PROFESSION(), "none");
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_POINT_OF_INTEREST_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_POINT_OF_INTEREST_TYPE());
    return registry;
}

// Java: MEMORY_MODULE_TYPE = registerDefaulted(Registries.MEMORY_MODULE_TYPE, "dummy", ...)
LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_MEMORY_MODULE_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_defaulted(LIBMATTI_MC_Registries_MEMORY_MODULE_TYPE(), "dummy");
    return registry;
}

// Java: SENSOR_TYPE = registerDefaulted(Registries.SENSOR_TYPE, "dummy", ...)
LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_SENSOR_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_defaulted(LIBMATTI_MC_Registries_SENSOR_TYPE(), "dummy");
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_ACTIVITY(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_ACTIVITY());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_LOOT_POOL_ENTRY_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_LOOT_POOL_ENTRY_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_LOOT_FUNCTION_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_LOOT_FUNCTION_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_LOOT_CONDITION_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_LOOT_CONDITION_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_LOOT_NUMBER_PROVIDER_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_LOOT_NUMBER_PROVIDER_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_LOOT_NBT_PROVIDER_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_LOOT_NBT_PROVIDER_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_LOOT_SCORE_PROVIDER_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_LOOT_SCORE_PROVIDER_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_FLOAT_PROVIDER_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_FLOAT_PROVIDER_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_INT_PROVIDER_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_INT_PROVIDER_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_HEIGHT_PROVIDER_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_HEIGHT_PROVIDER_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_BLOCK_PREDICATE_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_BLOCK_PREDICATE_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_CARVER(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_CARVER());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_FEATURE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_FEATURE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_STRUCTURE_PLACEMENT(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_STRUCTURE_PLACEMENT());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_STRUCTURE_PIECE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_STRUCTURE_PIECE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_STRUCTURE_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_STRUCTURE_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_PLACEMENT_MODIFIER_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_PLACEMENT_MODIFIER_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_BLOCKSTATE_PROVIDER_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_BLOCK_STATE_PROVIDER_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_FOLIAGE_PLACER_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_FOLIAGE_PLACER_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_TRUNK_PLACER_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_TRUNK_PLACER_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_ROOT_PLACER_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_ROOT_PLACER_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_TREE_DECORATOR_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_TREE_DECORATOR_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_FEATURE_SIZE_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_FEATURE_SIZE_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_BIOME_SOURCE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_BIOME_SOURCE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_CHUNK_GENERATOR(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_CHUNK_GENERATOR());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_MATERIAL_CONDITION(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_MATERIAL_CONDITION());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_MATERIAL_RULE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_MATERIAL_RULE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_DENSITY_FUNCTION_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_DENSITY_FUNCTION_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_BLOCK_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_BLOCK_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_STRUCTURE_PROCESSOR(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_STRUCTURE_PROCESSOR());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_STRUCTURE_POOL_ELEMENT(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_STRUCTURE_POOL_ELEMENT());
    return registry;
}

// Java: POOL_ALIAS_BINDING_TYPE = registerSimple(Registries.POOL_ALIAS_BINDING, ...)
LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_POOL_ALIAS_BINDING_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_POOL_ALIAS_BINDING());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_DECORATED_POT_PATTERN(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_DECORATED_POT_PATTERN());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_CREATIVE_MODE_TAB(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_CREATIVE_MODE_TAB());
    return registry;
}

// Java: TRIGGER_TYPES = registerSimple(Registries.TRIGGER_TYPE, ...)
LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_TRIGGER_TYPES(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_TRIGGER_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_NUMBER_FORMAT_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_NUMBER_FORMAT_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_DATA_COMPONENT_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_DATA_COMPONENT_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_GAME_RULE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_GAME_RULE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_ENTITY_SUB_PREDICATE_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_ENTITY_SUB_PREDICATE_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_DATA_COMPONENT_PREDICATE_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_DATA_COMPONENT_PREDICATE_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_MAP_DECORATION_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_MAP_DECORATION_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_ENCHANTMENT_EFFECT_COMPONENT_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_ENCHANTMENT_EFFECT_COMPONENT_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_ENCHANTMENT_LEVEL_BASED_VALUE_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_ENCHANTMENT_LEVEL_BASED_VALUE_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_ENCHANTMENT_ENTITY_EFFECT_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_ENCHANTMENT_ENTITY_EFFECT_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_ENCHANTMENT_LOCATION_BASED_EFFECT_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_ENCHANTMENT_LOCATION_BASED_EFFECT_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_ENCHANTMENT_VALUE_EFFECT_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_ENCHANTMENT_VALUE_EFFECT_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_ENCHANTMENT_PROVIDER_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_ENCHANTMENT_PROVIDER_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_CONSUME_EFFECT_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_CONSUME_EFFECT_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_RECIPE_DISPLAY(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_RECIPE_DISPLAY());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_SLOT_DISPLAY(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_SLOT_DISPLAY());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_RECIPE_BOOK_CATEGORY(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_RECIPE_BOOK_CATEGORY());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_TICKET_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_TICKET_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_INCOMING_RPC_METHOD(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_INCOMING_RPC_METHOD());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_OUTGOING_RPC_METHOD(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_OUTGOING_RPC_METHOD());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_TEST_ENVIRONMENT_DEFINITION_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_TEST_ENVIRONMENT_DEFINITION_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_TEST_INSTANCE_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_TEST_INSTANCE_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_SPAWN_CONDITION_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_SPAWN_CONDITION_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_DIALOG_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_DIALOG_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_DIALOG_ACTION_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_DIALOG_ACTION_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_INPUT_CONTROL_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_INPUT_CONTROL_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_DIALOG_BODY_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_DIALOG_BODY_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_PERMISSION_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_PERMISSION_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_PERMISSION_CHECK_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_PERMISSION_CHECK_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_ENVIRONMENT_ATTRIBUTE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_ENVIRONMENT_ATTRIBUTE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_ATTRIBUTE_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_ATTRIBUTE_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_SLOT_SOURCE_TYPE(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_SLOT_SOURCE_TYPE());
    return registry;
}

LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_TEST_FUNCTION(void)
{
    static LIBMATTI_MC_MappedRegistry *registry = NULL;
    if (registry == NULL)
        registry = register_simple(LIBMATTI_MC_Registries_TEST_FUNCTION());
    return registry;
}

// Java: public static final Registry<? extends Registry<?>> REGISTRY = WRITABLE_REGISTRY
LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_Registry(void)
{
    return root_registry();
}

// Java: public static final WritableRegistry<WritableRegistry<?>> WRITABLE_REGISTRY
LIBMATTI_MC_MappedRegistry *LIBMATTI_MC_BuiltInRegistries_WritableRegistry(void)
{
    return root_registry();
}

// Java: static List<Identifier> getVanillaRegistrationOrder() - the vanilla root registries in
// vanilla order, without the GameData exceptions (attribute/data components/particle types are
// ordered ahead by GameData itself)
LIBMATTI_MC_Identifier **LIBMATTI_MC_BuiltInRegistries_GetVanillaRegistrationOrder(size_t *count)
{
    LIBMATTI_MC_MappedRegistry *registry = root_registry();
    size_t rootCount = 0;
    LIBMATTI_MC_Identifier **rootKeys = LIBMATTI_MC_MappedRegistry_KeySet(registry, &rootCount);

    LIBMATTI_MC_Identifier **ordered = NULL;
    size_t orderedCount = 0;
    if (rootCount > 0)
    {
        ordered = malloc(sizeof(LIBMATTI_MC_Identifier *) * rootCount);
        // Java: the WRITABLE_REGISTRY registration order is the vanilla bootstrap order; the
        // root key set is insertion-ordered in the port, so the order is carried over directly
        for (size_t i = 0; i < rootCount; i++)
            ordered[orderedCount++] = rootKeys[i];
    }
    *count = orderedCount;
    return ordered;
}

// Java: the class initialisation order - every static field constructor in declaration order
void LIBMATTI_MC_BuiltInRegistries_InitializeAll(void)
{
    LIBMATTI_MC_BuiltInRegistries_GAME_EVENT();
    LIBMATTI_MC_BuiltInRegistries_SOUND_EVENT();
    LIBMATTI_MC_BuiltInRegistries_FLUID();
    LIBMATTI_MC_BuiltInRegistries_MOB_EFFECT();
    LIBMATTI_MC_BuiltInRegistries_BLOCK();
    LIBMATTI_MC_BuiltInRegistries_DEBUG_SUBSCRIPTION();
    LIBMATTI_MC_BuiltInRegistries_ENTITY_TYPE();
    LIBMATTI_MC_BuiltInRegistries_ITEM();
    LIBMATTI_MC_BuiltInRegistries_POTION();
    LIBMATTI_MC_BuiltInRegistries_PARTICLE_TYPE();
    LIBMATTI_MC_BuiltInRegistries_BLOCK_ENTITY_TYPE();
    LIBMATTI_MC_BuiltInRegistries_CUSTOM_STAT();
    LIBMATTI_MC_BuiltInRegistries_CHUNK_STATUS();
    LIBMATTI_MC_BuiltInRegistries_RULE_TEST();
    LIBMATTI_MC_BuiltInRegistries_RULE_BLOCK_ENTITY_MODIFIER();
    LIBMATTI_MC_BuiltInRegistries_POS_RULE_TEST();
    LIBMATTI_MC_BuiltInRegistries_MENU();
    LIBMATTI_MC_BuiltInRegistries_RECIPE_TYPE();
    LIBMATTI_MC_BuiltInRegistries_RECIPE_SERIALIZER();
    LIBMATTI_MC_BuiltInRegistries_ATTRIBUTE();
    LIBMATTI_MC_BuiltInRegistries_POSITION_SOURCE_TYPE();
    LIBMATTI_MC_BuiltInRegistries_COMMAND_ARGUMENT_TYPE();
    LIBMATTI_MC_BuiltInRegistries_STAT_TYPE();
    LIBMATTI_MC_BuiltInRegistries_VILLAGER_TYPE();
    LIBMATTI_MC_BuiltInRegistries_VILLAGER_PROFESSION();
    LIBMATTI_MC_BuiltInRegistries_POINT_OF_INTEREST_TYPE();
    LIBMATTI_MC_BuiltInRegistries_MEMORY_MODULE_TYPE();
    LIBMATTI_MC_BuiltInRegistries_SENSOR_TYPE();
    LIBMATTI_MC_BuiltInRegistries_ACTIVITY();
    LIBMATTI_MC_BuiltInRegistries_LOOT_POOL_ENTRY_TYPE();
    LIBMATTI_MC_BuiltInRegistries_LOOT_FUNCTION_TYPE();
    LIBMATTI_MC_BuiltInRegistries_LOOT_CONDITION_TYPE();
    LIBMATTI_MC_BuiltInRegistries_LOOT_NUMBER_PROVIDER_TYPE();
    LIBMATTI_MC_BuiltInRegistries_LOOT_NBT_PROVIDER_TYPE();
    LIBMATTI_MC_BuiltInRegistries_LOOT_SCORE_PROVIDER_TYPE();
    LIBMATTI_MC_BuiltInRegistries_FLOAT_PROVIDER_TYPE();
    LIBMATTI_MC_BuiltInRegistries_INT_PROVIDER_TYPE();
    LIBMATTI_MC_BuiltInRegistries_HEIGHT_PROVIDER_TYPE();
    LIBMATTI_MC_BuiltInRegistries_BLOCK_PREDICATE_TYPE();
    LIBMATTI_MC_BuiltInRegistries_CARVER();
    LIBMATTI_MC_BuiltInRegistries_FEATURE();
    LIBMATTI_MC_BuiltInRegistries_STRUCTURE_PLACEMENT();
    LIBMATTI_MC_BuiltInRegistries_STRUCTURE_PIECE();
    LIBMATTI_MC_BuiltInRegistries_STRUCTURE_TYPE();
    LIBMATTI_MC_BuiltInRegistries_PLACEMENT_MODIFIER_TYPE();
    LIBMATTI_MC_BuiltInRegistries_BLOCKSTATE_PROVIDER_TYPE();
    LIBMATTI_MC_BuiltInRegistries_FOLIAGE_PLACER_TYPE();
    LIBMATTI_MC_BuiltInRegistries_TRUNK_PLACER_TYPE();
    LIBMATTI_MC_BuiltInRegistries_ROOT_PLACER_TYPE();
    LIBMATTI_MC_BuiltInRegistries_TREE_DECORATOR_TYPE();
    LIBMATTI_MC_BuiltInRegistries_FEATURE_SIZE_TYPE();
    LIBMATTI_MC_BuiltInRegistries_BIOME_SOURCE();
    LIBMATTI_MC_BuiltInRegistries_CHUNK_GENERATOR();
    LIBMATTI_MC_BuiltInRegistries_MATERIAL_CONDITION();
    LIBMATTI_MC_BuiltInRegistries_MATERIAL_RULE();
    LIBMATTI_MC_BuiltInRegistries_DENSITY_FUNCTION_TYPE();
    LIBMATTI_MC_BuiltInRegistries_BLOCK_TYPE();
    LIBMATTI_MC_BuiltInRegistries_STRUCTURE_PROCESSOR();
    LIBMATTI_MC_BuiltInRegistries_STRUCTURE_POOL_ELEMENT();
    LIBMATTI_MC_BuiltInRegistries_POOL_ALIAS_BINDING_TYPE();
    LIBMATTI_MC_BuiltInRegistries_DECORATED_POT_PATTERN();
    LIBMATTI_MC_BuiltInRegistries_CREATIVE_MODE_TAB();
    LIBMATTI_MC_BuiltInRegistries_TRIGGER_TYPES();
    LIBMATTI_MC_BuiltInRegistries_NUMBER_FORMAT_TYPE();
    LIBMATTI_MC_BuiltInRegistries_DATA_COMPONENT_TYPE();
    LIBMATTI_MC_BuiltInRegistries_GAME_RULE();
    LIBMATTI_MC_BuiltInRegistries_ENTITY_SUB_PREDICATE_TYPE();
    LIBMATTI_MC_BuiltInRegistries_DATA_COMPONENT_PREDICATE_TYPE();
    LIBMATTI_MC_BuiltInRegistries_MAP_DECORATION_TYPE();
    LIBMATTI_MC_BuiltInRegistries_ENCHANTMENT_EFFECT_COMPONENT_TYPE();
    LIBMATTI_MC_BuiltInRegistries_ENCHANTMENT_LEVEL_BASED_VALUE_TYPE();
    LIBMATTI_MC_BuiltInRegistries_ENCHANTMENT_ENTITY_EFFECT_TYPE();
    LIBMATTI_MC_BuiltInRegistries_ENCHANTMENT_LOCATION_BASED_EFFECT_TYPE();
    LIBMATTI_MC_BuiltInRegistries_ENCHANTMENT_VALUE_EFFECT_TYPE();
    LIBMATTI_MC_BuiltInRegistries_ENCHANTMENT_PROVIDER_TYPE();
    LIBMATTI_MC_BuiltInRegistries_CONSUME_EFFECT_TYPE();
    LIBMATTI_MC_BuiltInRegistries_RECIPE_DISPLAY();
    LIBMATTI_MC_BuiltInRegistries_SLOT_DISPLAY();
    LIBMATTI_MC_BuiltInRegistries_RECIPE_BOOK_CATEGORY();
    LIBMATTI_MC_BuiltInRegistries_TICKET_TYPE();
    LIBMATTI_MC_BuiltInRegistries_INCOMING_RPC_METHOD();
    LIBMATTI_MC_BuiltInRegistries_OUTGOING_RPC_METHOD();
    LIBMATTI_MC_BuiltInRegistries_TEST_ENVIRONMENT_DEFINITION_TYPE();
    LIBMATTI_MC_BuiltInRegistries_TEST_INSTANCE_TYPE();
    LIBMATTI_MC_BuiltInRegistries_SPAWN_CONDITION_TYPE();
    LIBMATTI_MC_BuiltInRegistries_DIALOG_TYPE();
    LIBMATTI_MC_BuiltInRegistries_DIALOG_ACTION_TYPE();
    LIBMATTI_MC_BuiltInRegistries_INPUT_CONTROL_TYPE();
    LIBMATTI_MC_BuiltInRegistries_DIALOG_BODY_TYPE();
    LIBMATTI_MC_BuiltInRegistries_PERMISSION_TYPE();
    LIBMATTI_MC_BuiltInRegistries_PERMISSION_CHECK_TYPE();
    LIBMATTI_MC_BuiltInRegistries_ENVIRONMENT_ATTRIBUTE();
    LIBMATTI_MC_BuiltInRegistries_ATTRIBUTE_TYPE();
    LIBMATTI_MC_BuiltInRegistries_SLOT_SOURCE_TYPE();
    LIBMATTI_MC_BuiltInRegistries_TEST_FUNCTION();
}

// Java: private static void createContents() - runs every loader, logs when one returned null
static void create_contents(void)
{
    LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
    for (RegistryLoader *loader = loaders; loader != NULL; loader = loader->next)
    {
        // Java: RegistryBootstrap.run(registry) - the vanilla content loaders are the
        // Blocks/Items class initialisations; every other registry stays empty until the
        // game port bootstraps it (Java: the supplier returning null logs the error)
        if (loader->identifier != NULL &&
            strcmp(LIBMATTI_MC_Identifier_GetPath(loader->identifier), "block") == 0)
        {
            LIBMATTI_MC_VanillaBootstrap_RegisterVanillaContent();
        }
        loader->ran = 1;
        (void) logger;
    }
}

// Java: private static void freeze() - REGISTRY.freeze(), then every registry
static void freeze_all(void)
{
    LIBMATTI_MC_MappedRegistry *registry = root_registry();
    LIBMATTI_MC_MappedRegistry_Freeze(registry);

    size_t count = 0;
    LIBMATTI_MC_ResourceKey **keys = LIBMATTI_MC_MappedRegistry_RegistryKeySet(registry, &count);
    for (size_t i = 0; i < count; i++)
    {
        LIBMATTI_MC_MappedRegistry *entry = LIBMATTI_MC_MappedRegistry_GetValueByKey(registry, keys[i]);
        if (entry != NULL)
            LIBMATTI_MC_MappedRegistry_Freeze(entry);
    }
}

// Java: private static <T extends Registry<?>> void validate(Registry<T> registry)
static void validate(void)
{
    LIBMATTI_MC_MappedRegistry *registry = root_registry();
    LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();

    size_t count = 0;
    LIBMATTI_MC_ResourceKey **keys = LIBMATTI_MC_MappedRegistry_RegistryKeySet(registry, &count);
    for (size_t i = 0; i < count; i++)
    {
        LIBMATTI_MC_MappedRegistry *entry = LIBMATTI_MC_MappedRegistry_GetValueByKey(registry, keys[i]);
        if (entry == NULL)
            continue;

        // Java: if (registry.keySet().isEmpty()) Util.logAndPauseIfInIde("Registry '...' was empty
        //       after loading") - until the game port bootstraps the vanilla contents every
        //       registry is empty, so the port logs at debug (the ERROR+pause is the IDE path
        //       once real contents exist)
        if (LIBMATTI_MC_MappedRegistry_IsEmpty(entry))
        {
            char *name = LIBMATTI_MC_Identifier_ToString(keys[i]->identifier);
            LIBMATTI_ML_Logger_Debug(logger, NULL, "Registry '{}' was empty after loading", name);
            free(name);
        }

        // Java: if (registry instanceof DefaultedRegistry) Objects.requireNonNull(getValue(defaultKey))
        // - reported like the empty check until the game port fills the registries; the game
        // bootstrap makes every default resolvable
        const char *defaultKey = LIBMATTI_MC_MappedRegistry_GetDefaultKey(entry);
        if (defaultKey != NULL)
        {
            LIBMATTI_MC_Identifier *location = LIBMATTI_MC_Identifier_Parse(defaultKey);
            void *defaultValue = LIBMATTI_MC_MappedRegistry_GetValue(entry, location);
            LIBMATTI_MC_Identifier_Free(location);
            if (defaultValue == NULL)
                LIBMATTI_ML_Logger_Debug(logger, NULL,
                                         "Missing default of DefaultedMappedRegistry: {}", defaultKey);
        }
    }
}

// Java: public static void bootStrap() - createContents(), freeze(), validate(REGISTRY)
void LIBMATTI_MC_BuiltInRegistries_BootStrap(void)
{
    create_contents();
    freeze_all();
    validate();
    LIBMATTI_MC_Bootstrap_MarkBootstrapped();
}
