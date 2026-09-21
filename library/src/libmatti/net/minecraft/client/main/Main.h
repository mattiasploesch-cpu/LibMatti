// Port of net.minecraft.client.main.Main (the game class FML hands over to).
//
// Java's net.neoforged.fml.startup.Client does
//
//     var main = createMainMethodCallable(startupResult, "net.minecraft.client.main.Main");
//     main.invokeExact(startupResult.loader().getProgramArgs().getArguments());
//
// so the JVM loads the class through the game content classloader and runs its
// static main(String[] args) with the full FML program arguments. The port
// registers the class on the TransformingClassLoader (the loader FML set as
// current class loader) and exposes main with the process entry point signature
// int main(int argc, char *argv[]) - the full parsed argument vector Minecraft
// would parse into its GameConfig.
//
// Main.main: parses the option set into a GameConfig, bootstraps and constructs
// new Minecraft(gameconfig); the run loop lives in Minecraft.run. The port
// parses the same options joptsimple accepts (width/height/fullscreen*), skips
// the unknown network/user ones (the game port), and runs the skeleton.

#ifndef MATTICRAFT_NET_MINECRAFT_CLIENT_MAIN_MAIN_H
#define MATTICRAFT_NET_MINECRAFT_CLIENT_MAIN_MAIN_H

// Java: the JVM defines net.minecraft.client.main.Main lazily; the port
// registers the class and its static main(String[]) on the game content
// classloader so Class.forName + findStatic resolve.
void LIBMATTI_MC_Client_Main_Register(void *classLoader);

// Java: public static void main(String[] args) - exposed separately so the
// harness can drive the game skeleton like the JVM drives main.
int LIBMATTI_MC_Client_Main_Main(int argc, char *argv[]);

#endif //MATTICRAFT_NET_MINECRAFT_CLIENT_MAIN_MAIN_H
