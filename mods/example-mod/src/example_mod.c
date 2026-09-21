// Example mod "examplemod".
//
// A Matticraft mod is a plain ELF shared object. libmatti loads the .so at
// runtime, so the mod is not linked against the library: it includes the
// libmatti headers and uses the library the running game process provides.
//
// The manifest (".matti_manifest") and every entry below resources/ are embedded
// by matti_add_mod() - see ../CMakeLists.txt.

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/matti_mixin.h"
#include "libmatti/net/minecraft/core/registries/BuiltInRegistries.h"
#include "libmatti/net/minecraft/core/registries/Registries.h"
#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaItems.h"
#include "libmatti/net/minecraft/world/item/Item.h"
#include "libmatti/net/minecraft/world/level/block/Block.h"
#include "libmatti/net/neoforged/bus/api/IEventBus.h"
#include "libmatti/net/neoforged/fml/config/ModConfigSpec.h"
#include "libmatti/net/neoforged/fml/javafmlmod/FMLModContainer.h"
#include "libmatti/net/neoforged/fml/config/ModConfig.h"
#include "libmatti/net/neoforged/neoforge/registries/RegisterEvent.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

const char *examplemod_name(void)
{
    return "examplemod";
}

int examplemod_version(void)
{
    return 100;
}

// What a mod can do today: use the loader's own services, for example log
// through the launcher's LogManager.
void examplemod_log(const char *message)
{
    LIBMATTI_ML_Logger_Info(LIBMATTI_ML_LogManager_GetLogger(), NULL, "examplemod: {}", message);
}

// A mixin: this handler runs in the game target's hook chain. The game binary
// declares its mixable functions with MATTI_MIXIN_TARGET (see client/src/main.c)
// and dispatches them through LIBMATTI_MIXIN_Invoke; the handler below runs
// before the target body and can cancel it.
static void examplemod_mixin_demo(MattiMixinCallbackInfo *info, void *userdata)
{
    (void) userdata;
    // The port's logger substitutes {} with strings only.
    char detail[96];
    snprintf(detail, sizeof(detail), "%s (%zu argument(s))", info->target, info->argCount);
    LIBMATTI_ML_Logger_Info(LIBMATTI_ML_LogManager_GetLogger(), NULL, "examplemod: mixin into {}", detail);
}
static void examplemod_mixin_run(MattiMixinCallbackInfo *info, void *userdata)
{
    (void) userdata;    
    // The port's logger substitutes {} with strings only.
    char detail[96];
    snprintf(detail, sizeof(detail), "%s (%zu argument(s))", info->target, info->argCount);
    LIBMATTI_ML_Logger_Info(LIBMATTI_ML_LogManager_GetLogger(), NULL, "examplemod: mixin into {}", detail);
}

MATTI_MIXIN("examplemod", "matticraft::demo::tick", examplemod_mixin_demo, NULL, MATTI_MIXIN_AT_HEAD,
            MATTI_MIXIN_PRIORITY_DEFAULT)
MATTI_MIXIN("examplemod", "matticraft::demo::run", examplemod_mixin_run, NULL, MATTI_MIXIN_AT_HEAD,
            MATTI_MIXIN_PRIORITY_DEFAULT)


// Java: container.registerConfig(ModConfig.Type.CLIENT, SPEC) - the mod registers its client
// config during construction (NeoForgeMod does the same in its constructor).
static LIBMATTI_FML_ModConfigSpec *examplemod_client_spec = NULL;

// ---------------------------------------------------------------------------
// Java: @SubscribeEvent in the mod constructor's RegisterEvent listener -
// the mod registers its own Block "examplemod:example_block" and the BlockItem
// "examplemod:example_item" next to the vanilla data.
// ---------------------------------------------------------------------------

static LIBMATTI_MC_Block *examplemod_example_block = NULL;
static LIBMATTI_MC_Item *examplemod_example_item = NULL;

static void examplemod_register_accept(void *self, void *value)
{
    (void) self;
    LIBMATTI_NEOFORGE_Registries_RegisterEvent *event = value;
    if (strcmp(event->registryName, "minecraft:block") == 0)
    {
        // Java: new Block(BlockBehaviour.Properties.of().setId(key)) - the registry write
        // itself happens through event.register below, like NeoForge's RegisterEvent
        LIBMATTI_MC_Identifier *identifier = LIBMATTI_MC_Identifier_Parse("examplemod:example_block");
        LIBMATTI_MC_ResourceKey *key = LIBMATTI_MC_ResourceKey_Create(LIBMATTI_MC_Registries_BLOCK(), identifier);
        LIBMATTI_MC_BlockBehaviour_Properties *properties = LIBMATTI_MC_BlockBehaviour_Properties_Of();
        LIBMATTI_MC_BlockBehaviour_Properties_SetId(properties, key);
        examplemod_example_block = LIBMATTI_MC_Block_NewWithProperties(properties);
        LIBMATTI_MC_Identifier_Free(identifier);
        LIBMATTI_NEOFORGE_Registries_RegisterEvent_Register(event, "examplemod:example_block",
                                                            examplemod_example_block);
    }
    else if (strcmp(event->registryName, "minecraft:item") == 0)
    {
        // Java: new BlockItem(block, new Item.Properties()) with the registry id set
        // before construction (the register() helper) - the description id resolves
        // through the key like any vanilla item
        LIBMATTI_MC_Identifier *identifier = LIBMATTI_MC_Identifier_Parse("examplemod:example_item");
        LIBMATTI_MC_ResourceKey *key = LIBMATTI_MC_ResourceKey_Create(LIBMATTI_MC_Registries_ITEM(), identifier);
        LIBMATTI_MC_ItemProperties *properties = LIBMATTI_MC_ItemProperties_New();
        LIBMATTI_MC_ItemProperties_SetId(properties, key);
        examplemod_example_item = LIBMATTI_MC_BlockItem_New(examplemod_example_block, properties);
        LIBMATTI_MC_Identifier_Free(identifier);
        LIBMATTI_NEOFORGE_Registries_RegisterEvent_Register(event, "examplemod:example_item",
                                                            examplemod_example_item);
    }
}

static const LIBMATTI_JU_Consumer examplemod_register_listener = {
    NULL, examplemod_register_accept};

// The mod's entry point. The loader loads the mod file and calls "<modid>_init"
// once during mod construction, with the same arguments Java injects into a
// mod's constructor.
void examplemod_init(const LIBMATTI_FML_FMLModContainer_ConstructorArgs *args)
{
    LIBMATTI_ML_Logger_Info(LIBMATTI_ML_LogManager_GetLogger(), NULL, "examplemod: {} constructed for {}",
                            examplemod_name(),
                            args->dist == LIBMATTI_DIST_CLIENT ? "the client" : "the dedicated server");
    examplemod_log("Hallo Papa das ist ein test!");
    examplemod_log("thats another test");

    // Java: the vanilla content is bootstrapped before the mods construct - read it like any
    // registry access: BuiltInRegistries.BLOCK.getValue(Identifier.parse("minecraft:stone"))
    LIBMATTI_MC_Block *stone = LIBMATTI_MC_VanillaBlocks_GetByName("STONE");
    const char *stoneId = LIBMATTI_MC_VanillaBlocks_IdOf(stone);
    LIBMATTI_MC_Item *diamondSword = LIBMATTI_MC_VanillaItems_GetByName("DIAMOND_SWORD");
    LIBMATTI_ML_Logger_Info(LIBMATTI_ML_LogManager_GetLogger(), NULL,
                            "examplemod: vanilla {} blocks and {} items bootstrapped (stone = {}, diamond_sword = {})",
                            "1142", "1481",
                            stoneId != NULL ? stoneId : "?",
                            LIBMATTI_MC_VanillaItems_IdOf(diamondSword) != NULL ? "ok" : "?");

    // Java: modEventBus.addListener(this::register) - the mod listens on its mod event bus for
    // the RegisterEvent and puts its own entries next to the vanilla ones
    LIBMATTI_BUS_IEventBus_AddListenerDefault(args->eventBus,
                                              LIBMATTI_NEOFORGE_Registries_RegisterEvent_Class(),
                                              &examplemod_register_listener);

    // Java: public ClientNeoForgeMod(IEventBus modEventBus, ModContainer container) {
    //           container.registerConfig(ModConfig.Type.CLIENT, NeoForgeClientConfig.SPEC); }
    examplemod_client_spec = LIBMATTI_FML_ModConfigSpec_New();
    LIBMATTI_FML_ModContainer_RegisterConfig(args->modContainer, LIBMATTI_FML_ModConfig_TYPE_CLIENT,
                                             &examplemod_client_spec->spec);
}
