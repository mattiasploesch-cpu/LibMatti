// Port of net.minecraft.client.main.Main (the game class FML hands over to).
//
// C keeps the API: the bootstrap launcher chain, the early window, the render
// backends, the block/item registries and the texture/font resources. The
// window after the early window and every screen are Rust
// (library/src/libmatti/rs/src/ui); this entry point only forwards to it.
//
// Java: Main.main parses the program arguments with joptsimple and runs the
// Minecraft skeleton. The Rust UI parses the same arguments (--layer, --api,
// --width, --height, --fullscreen, ...) and opens the game window itself.

#include "libmatti/net/minecraft/client/main/Main.h"

#include "libmatti/java/lang/Class.h"
#include "libmatti/rs/matti_rs.h"

int LIBMATTI_MC_Client_Main_Main(int argc, char *argv[]) {
  return matti_rs_init(argc, argv);
}

void LIBMATTI_MC_Client_Main_Register(void *classLoader) {
  LIBMATTI_JL_Class *mainClass = LIBMATTI_JL_Class_RegisterClass(
      "net.minecraft.client.main.Main", classLoader, NULL, 1);
  LIBMATTI_JL_Class_RegisterStaticMethod(
      mainClass, "main", (void (*)(int, char *[]))LIBMATTI_MC_Client_Main_Main);
}
