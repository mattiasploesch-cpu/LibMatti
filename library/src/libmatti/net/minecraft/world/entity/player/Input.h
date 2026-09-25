// Port of net.minecraft.world.entity.player.Input (the movement key-press
// record the KeyboardInput ticks into the player; Java's byte flags become the
// plain bools the C struct carries).

#ifndef MATTICRAFT_MC_WORLD_ENTITY_PLAYER_INPUT_H
#define MATTICRAFT_MC_WORLD_ENTITY_PLAYER_INPUT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public record Input(boolean forward, boolean backward, boolean left,
//                           boolean right, boolean jump, boolean shift, boolean sprint)
typedef struct LIBMATTI_MC_Input
{
    bool forward;
    bool backward;
    bool left;
    bool right;
    bool jump;
    bool shift;
    bool sprint;
} LIBMATTI_MC_Input;

// Java: public static Input EMPTY
LIBMATTI_MC_Input LIBMATTI_MC_Input_Empty(void);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_ENTITY_PLAYER_INPUT_H
