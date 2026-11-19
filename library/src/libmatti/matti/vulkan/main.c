#include "libmatti/matti/vulkan/main.h"

#include "libmatti/rs/matti_rs.h"

// Compatibility entry point for the old LIBMATTI_VK=TRUE switch. The window
// after the early window and the menus live in Rust, so this entry hands over
// exactly like the game main does.
int LIBMATTI_VK_MAIN(int argc, char **argv)
{
    return matti_rs_init(argc, argv);
}
