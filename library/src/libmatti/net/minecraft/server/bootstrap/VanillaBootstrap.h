// Port of the vanilla content bootstrap (net.minecraft.data.worldgen / the static blocks
// that fill BuiltInRegistries in the real game). Java class-initialises Blocks and Items
// while createContents() runs the per-registry RegistryBootstrap; the C port registers the
// generated vanilla tables through the same loaders so BuiltInRegistries.bootStrap() fills
// BLOCK and ITEM before the freeze - mods then see the vanilla ids through the RegisterEvent
// path (GameData.postRegisterEvents) exactly like a mod registration.

#ifndef MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLABOOTSTRAP_H
#define MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLABOOTSTRAP_H

// The createContents hook Java's BuiltInRegistries loaders run: initialises the vanilla
// Blocks/Items tables. Called through the BuiltInRegistries loader list.
void LIBMATTI_MC_VanillaBootstrap_RegisterVanillaContent(void);

#endif //MATTICRAFT_MC_SERVER_BOOTSTRAP_VANILLABOOTSTRAP_H
