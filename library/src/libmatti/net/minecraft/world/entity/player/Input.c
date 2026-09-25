// Port of net.minecraft.world.entity.player.Input (implementation).

#include "libmatti/net/minecraft/world/entity/player/Input.h"

// Java: public static Input EMPTY = new Input(false, false, false, false, false, false, false)
LIBMATTI_MC_Input LIBMATTI_MC_Input_Empty(void)
{
    LIBMATTI_MC_Input input = {false, false, false, false, false, false, false};
    return input;
}
