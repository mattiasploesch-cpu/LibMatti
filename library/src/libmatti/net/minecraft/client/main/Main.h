



















#ifndef MATTICRAFT_NET_MINECRAFT_CLIENT_MAIN_MAIN_H
#define MATTICRAFT_NET_MINECRAFT_CLIENT_MAIN_MAIN_H

// Java: the JVM defines net.minecraft.client.main.Main lazily; the port
// registers the class and its static main(String[]) on the game content
// classloader so Class.forName + findStatic resolve.
void LIBMATTI_MC_Client_Main_Register(void *classLoader);

// Java: public static void main(String[] args) - exposed separately so the
// harness can drive the game skeleton like the JVM drives main.
int LIBMATTI_MC_Client_Main_Main(int argc, char *argv[]);

#endif
