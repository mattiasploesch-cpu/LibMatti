// Port of net.minecraft.world.entity.player.Abilities (implementation).

#include "libmatti/net/minecraft/world/entity/player/Abilities.h"

#include <stddef.h>

// Java: private static final boolean DEFAULT_INVULNERABLE = false ... the field
// initializers; the port folds them into one init (C has no field initializers)
void LIBMATTI_MC_Abilities_Init(LIBMATTI_MC_Abilities *abilities)
{
    if (abilities == NULL)
        return;
    abilities->invulnerable = false;
    abilities->flying = false;
    abilities->mayfly = false;
    abilities->instabuild = false;
    // Java: public boolean mayBuild = true
    abilities->mayBuild = true;
    // Java: private float flyingSpeed = 0.05F
    abilities->flyingSpeed = 0.05f;
    // Java: private float walkingSpeed = 0.1F
    abilities->walkingSpeed = 0.1f;
}

float LIBMATTI_MC_Abilities_GetFlyingSpeed(const LIBMATTI_MC_Abilities *abilities)
{
    return abilities != NULL ? abilities->flyingSpeed : 0.0f;
}

void LIBMATTI_MC_Abilities_SetFlyingSpeed(LIBMATTI_MC_Abilities *abilities, float flyingSpeed)
{
    if (abilities != NULL)
        abilities->flyingSpeed = flyingSpeed;
}

float LIBMATTI_MC_Abilities_GetWalkingSpeed(const LIBMATTI_MC_Abilities *abilities)
{
    return abilities != NULL ? abilities->walkingSpeed : 0.0f;
}

void LIBMATTI_MC_Abilities_SetWalkingSpeed(LIBMATTI_MC_Abilities *abilities, float walkingSpeed)
{
    if (abilities != NULL)
        abilities->walkingSpeed = walkingSpeed;
}
