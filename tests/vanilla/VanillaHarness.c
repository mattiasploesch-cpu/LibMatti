// Vanilla bootstrap harness: runs Bootstrap.bootStrap() and checks the generated vanilla
// content - the registry counts, the declaration order, the DefaultedMappedRegistry defaults
// and the BlockItem back-links. The RegisterEvent side (modded entries next to the vanilla
// ones) runs through the client with the example mod.

#include "libmatti/net/minecraft/Bootstrap.h"
#include "libmatti/net/minecraft/core/MappedRegistry.h"
#include "libmatti/net/minecraft/core/registries/BuiltInRegistries.h"
#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"
#include "libmatti/net/minecraft/server/bootstrap/VanillaItems.h"
#include "libmatti/net/minecraft/world/item/Item.h"
#include "libmatti/net/minecraft/world/level/block/Block.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks = 0;

static void check(int condition, const char *what)
{
    checks++;
    if (!condition)
    {
        fprintf(stderr, "FAIL: %s\n", what);
        exit(1);
    }
}

int main(void)
{
    // Java: Bootstrap.bootStrap() - createContents fills BLOCK/ITEM, freeze, validate
    LIBMATTI_MC_Bootstrap_BootStrap();
    check(LIBMATTI_MC_Bootstrap_IsBootstrapped(), "bootstrapped");

    LIBMATTI_MC_MappedRegistry *blockRegistry = LIBMATTI_MC_BuiltInRegistries_BLOCK();
    LIBMATTI_MC_MappedRegistry *itemRegistry = LIBMATTI_MC_BuiltInRegistries_ITEM();

    check(LIBMATTI_MC_MappedRegistry_Size(blockRegistry) == LIBMATTI_MC_VanillaBlocks_COUNT,
          "block registry holds every vanilla block");
    check(LIBMATTI_MC_MappedRegistry_Size(itemRegistry) == LIBMATTI_MC_VanillaItems_COUNT,
          "item registry holds every vanilla item");

    // Java: the declaration order - Blocks.AIR is id 0, Blocks.STONE id 1
    LIBMATTI_MC_Identifier *airLocation = LIBMATTI_MC_Identifier_Parse("minecraft:air");
    LIBMATTI_MC_Identifier *stoneLocation = LIBMATTI_MC_Identifier_Parse("minecraft:stone");
    LIBMATTI_MC_Block *air = LIBMATTI_MC_MappedRegistry_GetValue(blockRegistry, airLocation);
    LIBMATTI_MC_Block *stone = LIBMATTI_MC_MappedRegistry_GetValue(blockRegistry, stoneLocation);
    check(air != NULL && air == LIBMATTI_MC_VanillaBlocks_AIR(), "air registered");
    check(stone != NULL && stone == LIBMATTI_MC_VanillaBlocks_GetByName("STONE"),
          "stone registered");
    check(LIBMATTI_MC_MappedRegistry_GetId(blockRegistry, air) == 0, "air id is 0");
    check(LIBMATTI_MC_MappedRegistry_GetId(blockRegistry, stone) == 1, "stone id is 1");

    // Java: DefaultedMappedRegistry - BLOCK falls back to the air block
    check(LIBMATTI_MC_MappedRegistry_GetValueOrDefault(blockRegistry, NULL) == air,
          "block default is air");

    // Java: Items.AIR is the item id 0. registerBlock(Blocks.AIR, AirItem::new) constructs
    // the AirItem with the block, so the port keeps the back-link like every registerBlock
    // result (the Item-subclass split is the game port's part).
    LIBMATTI_MC_Item *itemAir = LIBMATTI_MC_VanillaItems_AIR();
    check(itemAir != NULL && LIBMATTI_MC_MappedRegistry_GetId(itemRegistry, itemAir) == 0,
          "item air id is 0");
    check(itemAir->block == air, "item air links to block air");
    check(LIBMATTI_MC_MappedRegistry_GetValueOrDefault(itemRegistry, NULL) == itemAir,
          "item default is air");

    // Java: Item.BY_BLOCK - the stone BlockItem places the stone block
    LIBMATTI_MC_Item *itemStone = LIBMATTI_MC_VanillaItems_GetByName("STONE");
    check(itemStone != NULL && itemStone->block == stone, "item stone links to block stone");

    // Java: the block-item id pairs match (registerBlock uses the block id)
    check(strcmp(LIBMATTI_MC_VanillaItems_IdOf(itemStone),
                 LIBMATTI_MC_VanillaBlocks_IdOf(stone)) == 0,
          "stone item id equals the block id");

    // Java: the registries are frozen after bootStrap (RegistrationInfo.BUILT_IN = stable)
    check(LIBMATTI_MC_MappedRegistry_RegistryLifecycle(blockRegistry).kind == LIBMATTI_MJS_Lifecycle_STABLE,
          "block lifecycle stable");

    LIBMATTI_MC_Identifier_Free(airLocation);
    LIBMATTI_MC_Identifier_Free(stoneLocation);

    printf("vanilla: %d checks passed (blocks %d, items %d)\n", checks,
           (int) LIBMATTI_MC_MappedRegistry_Size(blockRegistry),
           (int) LIBMATTI_MC_MappedRegistry_Size(itemRegistry));
    return 0;
}
