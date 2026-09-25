// Port of net.minecraft.client.player.{ClientInput,KeyboardInput,LocalPlayer}.
// Java's three classes collapse into one C module: the ClientInput state
// (keyPresses + moveVector), the KeyboardInput tick (the Options mappings into
// the Input record) and the LocalPlayer struct (the Player entity + the client
// input + the session data the skeleton carries). The persistence/packet tails
// of LocalPlayer (chat, advancement sounds, close-containers) ride P6.

#ifndef MATTICRAFT_NET_MINECRAFT_CLIENT_PLAYER_LOCALPLAYER_H
#define MATTICRAFT_NET_MINECRAFT_CLIENT_PLAYER_LOCALPLAYER_H

#include "libmatti/net/minecraft/client/KeyMapping.h"
#include "libmatti/net/minecraft/world/entity/VanillaEntities.h"
#include "libmatti/net/minecraft/world/entity/player/Input.h"
#include "libmatti/net/minecraft/world/phys/Vec2.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class LocalPlayer extends AbstractClientPlayer -> Player
typedef struct LIBMATTI_MC_LocalPlayer
{
    // the base is the first member (the struct doubles as the Player pointer)
    LIBMATTI_MC_Player player;

    // Java: ClientInput public Input keyPresses = Input.EMPTY
    LIBMATTI_MC_Input keyPresses;
    // Java: ClientInput protected Vec2 moveVector = Vec2.ZERO
    LIBMATTI_MC_Vec2 moveVector;
} LIBMATTI_MC_LocalPlayer;

// Java: public LocalPlayer(Minecraft, Level, ClientPacketListener, StatsCounter,
// ClientRecipeBook) - the port takes the level + the session name/uuid
LIBMATTI_MC_LocalPlayer *LIBMATTI_MC_LocalPlayer_New(struct LIBMATTI_MC_Level *level, const char *name,
                                                     const LIBMATTI_JU_UUID *uuid);
// the typed free (through the Player free)
void LIBMATTI_MC_LocalPlayer_Free(LIBMATTI_MC_LocalPlayer *localPlayer);

// Java: ClientInput.tick() -> KeyboardInput.tick() - the KeyMapping reads into
// keyPresses, the moveVector normalises (left/right, forward/back impulses)
void LIBMATTI_MC_LocalPlayer_TickInput(LIBMATTI_MC_LocalPlayer *localPlayer);

// Java: public Vec2 getMoveVector() / public Input keyPresses
LIBMATTI_MC_Vec2 LIBMATTI_MC_LocalPlayer_GetMoveVector(const LIBMATTI_MC_LocalPlayer *localPlayer);
const LIBMATTI_MC_Input *LIBMATTI_MC_LocalPlayer_GetKeyPresses(const LIBMATTI_MC_LocalPlayer *localPlayer);
// Java: public boolean hasForwardImpulse()
bool LIBMATTI_MC_LocalPlayer_HasForwardImpulse(const LIBMATTI_MC_LocalPlayer *localPlayer);

// Java: LocalPlayer.tick() - the client-side per-tick tail over the entity base
// (the position/rotation anchors; the physics lands with P5.3)
void LIBMATTI_MC_LocalPlayer_Tick(LIBMATTI_MC_LocalPlayer *localPlayer);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_NET_MINECRAFT_CLIENT_PLAYER_LOCALPLAYER_H
