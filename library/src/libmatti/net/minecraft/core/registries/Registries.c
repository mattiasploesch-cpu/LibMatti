// Port of net.minecraft.core.registries.Registries.

#include "libmatti/net/minecraft/core/registries/Registries.h"

// Java: private static <T> ResourceKey<Registry<T>> createRegistryKey(String name) -
// ResourceKey.createRegistryKey(Identifier.withDefaultNamespace(name)); the identifier
// assertion runs on every declaration in Java, the port validates on first use too
static LIBMATTI_MC_ResourceKey *key(const char *name)
{
    LIBMATTI_MC_Identifier *identifier = LIBMATTI_MC_Identifier_WithDefaultNamespace(name);
    LIBMATTI_MC_ResourceKey *resourceKey = LIBMATTI_MC_ResourceKey_CreateRegistryKey(identifier);
    LIBMATTI_MC_Identifier_Free(identifier);
    return resourceKey;
}

// Java: public static final Identifier ROOT_REGISTRY_NAME
LIBMATTI_MC_Identifier *LIBMATTI_MC_MC_Registries_RootRegistryName(void)
{
    static LIBMATTI_MC_Identifier *root = NULL;
    if (root == NULL)
        root = LIBMATTI_MC_Identifier_WithDefaultNamespace("root");
    return root;
}

// Java: public static final ResourceKey<Registry<Activity>> ACTIVITY = createRegistryKey("activity")
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_ACTIVITY(void) { return key("activity"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_ATTRIBUTE(void) { return key("attribute"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_BIOME_SOURCE(void) { return key("worldgen/biome_source"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_BLOCK_ENTITY_TYPE(void) { return key("block_entity_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_BLOCK_PREDICATE_TYPE(void) { return key("block_predicate_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_BLOCK_STATE_PROVIDER_TYPE(void) { return key("worldgen/block_state_provider_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_BLOCK_TYPE(void) { return key("block_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_BLOCK(void) { return key("block"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_CARVER(void) { return key("worldgen/carver"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_CHUNK_GENERATOR(void) { return key("worldgen/chunk_generator"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_CHUNK_STATUS(void) { return key("chunk_status"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_COMMAND_ARGUMENT_TYPE(void) { return key("command_argument_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_CONSUME_EFFECT_TYPE(void) { return key("consume_effect_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_CREATIVE_MODE_TAB(void) { return key("creative_mode_tab"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_CUSTOM_STAT(void) { return key("custom_stat"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_DATA_COMPONENT_PREDICATE_TYPE(void) { return key("data_component_predicate_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_DATA_COMPONENT_TYPE(void) { return key("data_component_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_GAME_RULE(void) { return key("game_rule"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_DEBUG_SUBSCRIPTION(void) { return key("debug_subscription"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_DECORATED_POT_PATTERN(void) { return key("decorated_pot_pattern"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_DENSITY_FUNCTION_TYPE(void) { return key("worldgen/density_function_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_DIALOG_BODY_TYPE(void) { return key("dialog_body_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_DIALOG_TYPE(void) { return key("dialog_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_ENCHANTMENT_EFFECT_COMPONENT_TYPE(void) { return key("enchantment_effect_component_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_ENCHANTMENT_ENTITY_EFFECT_TYPE(void) { return key("enchantment_entity_effect_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_ENCHANTMENT_LEVEL_BASED_VALUE_TYPE(void) { return key("enchantment_level_based_value_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_ENCHANTMENT_LOCATION_BASED_EFFECT_TYPE(void) { return key("enchantment_location_based_effect_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_ENCHANTMENT_PROVIDER_TYPE(void) { return key("enchantment_provider_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_ENCHANTMENT_VALUE_EFFECT_TYPE(void) { return key("enchantment_value_effect_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_ENTITY_SUB_PREDICATE_TYPE(void) { return key("entity_sub_predicate_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_ENTITY_TYPE(void) { return key("entity_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_ENVIRONMENT_ATTRIBUTE(void) { return key("environment_attribute"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_ATTRIBUTE_TYPE(void) { return key("attribute_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_FEATURE_SIZE_TYPE(void) { return key("worldgen/feature_size_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_FEATURE(void) { return key("worldgen/feature"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_FLOAT_PROVIDER_TYPE(void) { return key("float_provider_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_FLUID(void) { return key("fluid"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_FOLIAGE_PLACER_TYPE(void) { return key("worldgen/foliage_placer_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_GAME_EVENT(void) { return key("game_event"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_HEIGHT_PROVIDER_TYPE(void) { return key("height_provider_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_INPUT_CONTROL_TYPE(void) { return key("input_control_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_INT_PROVIDER_TYPE(void) { return key("int_provider_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_ITEM(void) { return key("item"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_SLOT_SOURCE_TYPE(void) { return key("slot_source_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_LOOT_CONDITION_TYPE(void) { return key("loot_condition_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_LOOT_FUNCTION_TYPE(void) { return key("loot_function_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_LOOT_NBT_PROVIDER_TYPE(void) { return key("loot_nbt_provider_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_LOOT_NUMBER_PROVIDER_TYPE(void) { return key("loot_number_provider_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_LOOT_POOL_ENTRY_TYPE(void) { return key("loot_pool_entry_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_LOOT_SCORE_PROVIDER_TYPE(void) { return key("loot_score_provider_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_MAP_DECORATION_TYPE(void) { return key("map_decoration_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_MATERIAL_CONDITION(void) { return key("worldgen/material_condition"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_MATERIAL_RULE(void) { return key("worldgen/material_rule"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_MEMORY_MODULE_TYPE(void) { return key("memory_module_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_MENU(void) { return key("menu"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_MOB_EFFECT(void) { return key("mob_effect"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_NUMBER_FORMAT_TYPE(void) { return key("number_format_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_PARTICLE_TYPE(void) { return key("particle_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_PLACEMENT_MODIFIER_TYPE(void) { return key("worldgen/placement_modifier_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_POINT_OF_INTEREST_TYPE(void) { return key("point_of_interest_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_POOL_ALIAS_BINDING(void) { return key("worldgen/pool_alias_binding"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_POSITION_SOURCE_TYPE(void) { return key("position_source_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_POS_RULE_TEST(void) { return key("pos_rule_test"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_POTION(void) { return key("potion"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_RECIPE_BOOK_CATEGORY(void) { return key("recipe_book_category"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_RECIPE_DISPLAY(void) { return key("recipe_display"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_RECIPE_SERIALIZER(void) { return key("recipe_serializer"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_RECIPE_TYPE(void) { return key("recipe_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_ROOT_PLACER_TYPE(void) { return key("worldgen/root_placer_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_RULE_BLOCK_ENTITY_MODIFIER(void) { return key("rule_block_entity_modifier"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_RULE_TEST(void) { return key("rule_test"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_SENSOR_TYPE(void) { return key("sensor_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_SLOT_DISPLAY(void) { return key("slot_display"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_SOUND_EVENT(void) { return key("sound_event"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_SPAWN_CONDITION_TYPE(void) { return key("spawn_condition_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_STAT_TYPE(void) { return key("stat_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_STRUCTURE_PIECE(void) { return key("worldgen/structure_piece"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_STRUCTURE_PLACEMENT(void) { return key("worldgen/structure_placement"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_STRUCTURE_POOL_ELEMENT(void) { return key("worldgen/structure_pool_element"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_STRUCTURE_PROCESSOR(void) { return key("worldgen/structure_processor"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_STRUCTURE_TYPE(void) { return key("worldgen/structure_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_DIALOG_ACTION_TYPE(void) { return key("dialog_action_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_TEST_ENVIRONMENT_DEFINITION_TYPE(void) { return key("test_environment_definition_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_TEST_FUNCTION(void) { return key("test_function"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_TEST_INSTANCE_TYPE(void) { return key("test_instance_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_TICKET_TYPE(void) { return key("ticket_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_TREE_DECORATOR_TYPE(void) { return key("worldgen/tree_decorator_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_TRUNK_PLACER_TYPE(void) { return key("worldgen/trunk_placer_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_VILLAGER_PROFESSION(void) { return key("villager_profession"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_VILLAGER_TYPE(void) { return key("villager_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_INCOMING_RPC_METHOD(void) { return key("incoming_rpc_methods"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_OUTGOING_RPC_METHOD(void) { return key("outgoing_rpc_methods"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_PERMISSION_TYPE(void) { return key("permission_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_PERMISSION_CHECK_TYPE(void) { return key("permission_check_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_BANNER_PATTERN(void) { return key("banner_pattern"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_BIOME(void) { return key("worldgen/biome"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_CAT_VARIANT(void) { return key("cat_variant"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_CHAT_TYPE(void) { return key("chat_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_CHICKEN_VARIANT(void) { return key("chicken_variant"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_ZOMBIE_NAUTILUS_VARIANT(void) { return key("zombie_nautilus_variant"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_CONFIGURED_CARVER(void) { return key("worldgen/configured_carver"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_CONFIGURED_FEATURE(void) { return key("worldgen/configured_feature"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_COW_VARIANT(void) { return key("cow_variant"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_DAMAGE_TYPE(void) { return key("damage_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_DENSITY_FUNCTION(void) { return key("worldgen/density_function"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_DIALOG(void) { return key("dialog"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_DIMENSION_TYPE(void) { return key("dimension_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_ENCHANTMENT_PROVIDER(void) { return key("enchantment_provider"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_ENCHANTMENT(void) { return key("enchantment"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_FLAT_LEVEL_GENERATOR_PRESET(void) { return key("worldgen/flat_level_generator_preset"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_FROG_VARIANT(void) { return key("frog_variant"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_INSTRUMENT(void) { return key("instrument"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_JUKEBOX_SONG(void) { return key("jukebox_song"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_MULTI_NOISE_BIOME_SOURCE_PARAMETER_LIST(void) { return key("worldgen/multi_noise_biome_source_parameter_list"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_NOISE_SETTINGS(void) { return key("worldgen/noise_settings"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_NOISE(void) { return key("worldgen/noise"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_PAINTING_VARIANT(void) { return key("painting_variant"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_PIG_VARIANT(void) { return key("pig_variant"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_PLACED_FEATURE(void) { return key("worldgen/placed_feature"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_PROCESSOR_LIST(void) { return key("worldgen/processor_list"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_STRUCTURE_SET(void) { return key("worldgen/structure_set"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_STRUCTURE(void) { return key("worldgen/structure"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_TEMPLATE_POOL(void) { return key("worldgen/template_pool"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_TEST_ENVIRONMENT(void) { return key("test_environment"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_TEST_INSTANCE(void) { return key("test_instance"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_TIMELINE(void) { return key("timeline"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_TRIAL_SPAWNER(void) { return key("trial_spawner"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_TRIGGER_TYPE(void) { return key("trigger_type"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_TRIM_MATERIAL(void) { return key("trim_material"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_TRIM_PATTERN(void) { return key("trim_pattern"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_WOLF_VARIANT(void) { return key("wolf_variant"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_WOLF_SOUND_VARIANT(void) { return key("wolf_sound_variant"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_WORLD_PRESET(void) { return key("worldgen/world_preset"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_DIMENSION(void) { return key("dimension"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_LOOT_TABLE(void) { return key("loot_table"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_ITEM_MODIFIER(void) { return key("item_modifier"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_PREDICATE(void) { return key("predicate"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_ADVANCEMENT(void) { return key("advancement"); }
LIBMATTI_MC_ResourceKey *LIBMATTI_MC_Registries_RECIPE(void) { return key("recipe"); }
