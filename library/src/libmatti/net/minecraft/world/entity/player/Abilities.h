// Port of net.minecraft.world.entity.player.Abilities (the player capability
// record: invulnerable/flying/mayfly/instabuild/mayBuild + the walk/fly speeds
// the movement uses). Java's codec becomes the NBT read/write on the Player
// save path (the keys "invulnerable"/"mayfly"/"instabuild"/"mayBuild"/
// "flySpeed"/"walkSpeed" of the "abilities" sub-tag).

#ifndef MATTICRAFT_MC_WORLD_ENTITY_PLAYER_ABILITIES_H
#define MATTICRAFT_MC_WORLD_ENTITY_PLAYER_ABILITIES_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: the field defaults - mayBuild true, flyingSpeed 0.05, walkingSpeed 0.1
typedef struct LIBMATTI_MC_Abilities
{
    bool invulnerable;
    bool flying;
    bool mayfly;
    bool instabuild;
    bool mayBuild;
    float flyingSpeed;
    float walkingSpeed;
} LIBMATTI_MC_Abilities;

// Java: the constructor defaults (mayBuild = true, speeds 0.05 / 0.1)
void LIBMATTI_MC_Abilities_Init(LIBMATTI_MC_Abilities *abilities);

// Java: public float getFlyingSpeed() / setFlyingSpeed(float)
float LIBMATTI_MC_Abilities_GetFlyingSpeed(const LIBMATTI_MC_Abilities *abilities);
void LIBMATTI_MC_Abilities_SetFlyingSpeed(LIBMATTI_MC_Abilities *abilities, float flyingSpeed);
// Java: public float getWalkingSpeed() / setWalkingSpeed(float)
float LIBMATTI_MC_Abilities_GetWalkingSpeed(const LIBMATTI_MC_Abilities *abilities);
void LIBMATTI_MC_Abilities_SetWalkingSpeed(LIBMATTI_MC_Abilities *abilities, float walkingSpeed);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_ENTITY_PLAYER_ABILITIES_H
