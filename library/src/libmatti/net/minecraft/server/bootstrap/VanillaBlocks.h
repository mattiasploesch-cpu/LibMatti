






#ifndef MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLABLOCKS_H
#define MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLABLOCKS_H

#include <stddef.h>

#include "libmatti/net/minecraft/world/level/block/Block.h"

// Java: the number of Block statics in Blocks
#define LIBMATTI_MC_VanillaBlocks_COUNT 1142

// Java: the Blocks statics in declaration order (the vanilla registration order)
LIBMATTI_MC_Block **LIBMATTI_MC_VanillaBlocks_All(void);


LIBMATTI_MC_Block *LIBMATTI_MC_VanillaBlocks_GetByName(const char *name);


const char *LIBMATTI_MC_VanillaBlocks_IdOf(LIBMATTI_MC_Block *block);
// Java: Blocks.AIR - the block every DefaultedMappedRegistry falls back to
LIBMATTI_MC_Block *LIBMATTI_MC_VanillaBlocks_AIR(void);



void LIBMATTI_MC_VanillaBlocks_Initialize(void);

// Java: the static block - register every entry into BuiltInRegistries.BLOCK
void LIBMATTI_MC_VanillaBlocks_RegisterAll(void);

#endif
