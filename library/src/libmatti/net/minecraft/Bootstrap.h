// Port of net.minecraft.server.Bootstrap (the subset the registries need: the
// bootstrapped guard). The item/selector/dispense bootstraps are game-port parts.

#ifndef MATTICRAFT_NET_MINECRAFT_SERVER_BOOTSTRAP_H
#define MATTICRAFT_NET_MINECRAFT_SERVER_BOOTSTRAP_H

// Java: public static void checkBootstrapCalled(Supplier<String> location) -
// aborts with "Not bootstrapped (called from <location>)" when bootStrap() has not run
void LIBMATTI_MC_Bootstrap_CheckBootstrapCalled(const char *location);

// Java: public static boolean isBootstrapped()
int LIBMATTI_MC_Bootstrap_IsBootstrapped(void);

// Java: public static void bootStrap() - raises the guard first (isBootstrapped = true), then
// the BuiltInRegistries.REGISTRY.keySet() check class-initialises the registries and
// BuiltInRegistries.bootStrap() runs; the game bootstraps (FireBlock, ComposterBlock,
// EntitySelectorOptions, DispenseItemBehavior, CauldronInteraction) are the game port's part
void LIBMATTI_MC_Bootstrap_BootStrap(void);

// Java: the bootStrap() body ends with isBootstrapped = true - the registry bootstrap raises
// the guard through this setter
void LIBMATTI_MC_Bootstrap_MarkBootstrapped(void);

#endif //MATTICRAFT_NET_MINECRAFT_SERVER_BOOTSTRAP_H
