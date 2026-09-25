// Port of net.minecraft.client.player.{ClientInput,KeyboardInput,LocalPlayer}
// (implementation). The KeyboardInput.tick() body reads the Options KeyMappings
// into the Input record and normalises the move vector - 1:1 over the port's
// KeyMapping statics.

#include "libmatti/net/minecraft/client/player/LocalPlayer.h"

#include "libmatti/net/minecraft/world/entity/VanillaEntities.h"

#include <math.h>
#include <stdlib.h>

// Java: ClientInput public Input keyPresses = Input.EMPTY - the port resets the
// record through the EMPTY constructor
static void input_reset(LIBMATTI_MC_Input *input)
{
    *input = LIBMATTI_MC_Input_Empty();
}

// Java: KeyboardInput private static float calculateImpulse(boolean, boolean)
static float calculate_impulse(bool positive, bool negative)
{
    if (positive == negative)
        return 0.0f;
    return positive ? 1.0f : -1.0f;
}

// Java: public LocalPlayer(Minecraft, Level, ClientPacketListener, StatsCounter,
// ClientRecipeBook) - the session uuid rides the Player base
LIBMATTI_MC_LocalPlayer *LIBMATTI_MC_LocalPlayer_New(struct LIBMATTI_MC_Level *level, const char *name,
                                                     const LIBMATTI_JU_UUID *uuid)
{
    LIBMATTI_MC_LocalPlayer *localPlayer = calloc(1, sizeof(LIBMATTI_MC_LocalPlayer));
    if (localPlayer == NULL)
        return NULL;
    // Java: super(client, level, connection, stats, recipeBook) - the Player
    // ctor tail runs over the embedded struct (abilities/food defaults, the
    // base entity init)
    LIBMATTI_MC_Player_Init(&localPlayer->player, level, name);
    if (uuid != NULL)
        LIBMATTI_MC_Entity_SetUUID(&localPlayer->player.base.base, *uuid);
    // Java: this.keyPresses = Input.EMPTY (the ClientInput field initializer)
    input_reset(&localPlayer->keyPresses);
    localPlayer->moveVector = LIBMATTI_MC_Vec2_Zero();
    return localPlayer;
}

// Java: the destroy path - the Player free owns the base (the local player's
// struct is a value member, the *player copy above took the heap state over)
void LIBMATTI_MC_LocalPlayer_Free(LIBMATTI_MC_LocalPlayer *localPlayer)
{
    if (localPlayer == NULL)
        return;
    LIBMATTI_MC_Player_Free(&localPlayer->player);
}

// Java: KeyboardInput public void tick() - 1:1
void LIBMATTI_MC_LocalPlayer_TickInput(LIBMATTI_MC_LocalPlayer *localPlayer)
{
    if (localPlayer == NULL)
        return;
    // Java: this.keyPresses = new Input(options.keyUp.isDown(), keyDown, keyLeft,
    //       keyRight, keyJump, keyShift, keySprint)
    localPlayer->keyPresses.forward = LIBMATTI_MC_KeyMapping_IsDown(LIBMATTI_MC_KeyMapping_Forward());
    localPlayer->keyPresses.backward = LIBMATTI_MC_KeyMapping_IsDown(LIBMATTI_MC_KeyMapping_Back());
    localPlayer->keyPresses.left = LIBMATTI_MC_KeyMapping_IsDown(LIBMATTI_MC_KeyMapping_Left());
    localPlayer->keyPresses.right = LIBMATTI_MC_KeyMapping_IsDown(LIBMATTI_MC_KeyMapping_Right());
    localPlayer->keyPresses.jump = LIBMATTI_MC_KeyMapping_IsDown(LIBMATTI_MC_KeyMapping_Jump());
    localPlayer->keyPresses.shift = LIBMATTI_MC_KeyMapping_IsDown(LIBMATTI_MC_KeyMapping_Shift());
    localPlayer->keyPresses.sprint = LIBMATTI_MC_KeyMapping_IsDown(LIBMATTI_MC_KeyMapping_Sprint());

    // Java: float f = calculateImpulse(keyPresses.forward(), keyPresses.backward())
    float f = calculate_impulse(localPlayer->keyPresses.forward, localPlayer->keyPresses.backward);
    // Java: float f1 = calculateImpulse(keyPresses.left(), keyPresses.right())
    float f1 = calculate_impulse(localPlayer->keyPresses.left, localPlayer->keyPresses.right);
    // Java: this.moveVector = new Vec2(f1, f).normalized()
    localPlayer->moveVector = LIBMATTI_MC_Vec2_Normalized(LIBMATTI_MC_Vec2_Of(f1, f));
}

// Java: ClientInput public Vec2 getMoveVector()
LIBMATTI_MC_Vec2 LIBMATTI_MC_LocalPlayer_GetMoveVector(const LIBMATTI_MC_LocalPlayer *localPlayer)
{
    return localPlayer != NULL ? localPlayer->moveVector : LIBMATTI_MC_Vec2_Zero();
}

// Java: ClientInput public Input keyPresses (the field read)
const LIBMATTI_MC_Input *LIBMATTI_MC_LocalPlayer_GetKeyPresses(const LIBMATTI_MC_LocalPlayer *localPlayer)
{
    return localPlayer != NULL ? &localPlayer->keyPresses : NULL;
}

// Java: ClientInput public boolean hasForwardImpulse() - moveVector.y > 1.0E-5F
bool LIBMATTI_MC_LocalPlayer_HasForwardImpulse(const LIBMATTI_MC_LocalPlayer *localPlayer)
{
    return localPlayer != NULL && localPlayer->moveVector.y > 1.0e-5f;
}

// Java: LocalPlayer.tick() - the movement-agnostic tail the skeleton drives:
// the entity base tick (the rotation anchors ride it)
void LIBMATTI_MC_LocalPlayer_Tick(LIBMATTI_MC_LocalPlayer *localPlayer)
{
    if (localPlayer == NULL)
        return;
    LIBMATTI_MC_Entity_Tick(&localPlayer->player.base.base);
}
