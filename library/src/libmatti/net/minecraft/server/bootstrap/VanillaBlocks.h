// Port of net.minecraft.world.level.block.Blocks (the registry-data part).
// Generated from the vendor Java sources (vendor/MCP-Reborn .../world/level/block/Blocks.java):
// one row per public static final Block, in Java declaration order, with the real
// BlockBehaviour.Properties (map colour, strength, sound, instrument, flags) and the real
// StateDefinition (the createBlockStateDefinition properties of the block class).

#ifndef MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLABLOCKS_H
#define MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLABLOCKS_H

#include <stddef.h>

#include "libmatti/net/minecraft/world/level/block/Block.h"

// Java: the number of Block statics in Blocks
#define LIBMATTI_MC_VanillaBlocks_COUNT 1142

// Java: the Blocks statics in declaration order (the vanilla registration order)
LIBMATTI_MC_Block **LIBMATTI_MC_VanillaBlocks_All(void);
// The port's accessor by Java constant name ("STONE"); NULL when unknown
LIBMATTI_MC_Block *LIBMATTI_MC_VanillaBlocks_GetByName(const char *name);
// The registered id of the entry ("stone"); NULL when the block is not in the table
const char *LIBMATTI_MC_VanillaBlocks_IdOf(LIBMATTI_MC_Block *block);
// Java: Blocks.AIR - the block every DefaultedMappedRegistry falls back to
LIBMATTI_MC_Block *LIBMATTI_MC_VanillaBlocks_AIR(void);

// The class-initialisation Java's Blocks static block runs (fills the table)
void LIBMATTI_MC_VanillaBlocks_Initialize(void);

// Java: the static block - register every entry into BuiltInRegistries.BLOCK
void LIBMATTI_MC_VanillaBlocks_RegisterAll(void);

#endif //MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLABLOCKS_H
