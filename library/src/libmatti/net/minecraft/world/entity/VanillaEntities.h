// Port of the vanilla EntityType constants (Java: the EntityType static block)
// plus the concrete base entity structs the P5 phase needs. Java's subclasses
// (Player, ItemEntity, ...) each embed the Entity base; the port mirrors that
// with a struct per subclass and the shared factory shapes.

#ifndef MATTICRAFT_MC_WORLD_ENTITY_VANILLAENTITIES_H
#define MATTICRAFT_MC_WORLD_ENTITY_VANILLAENTITIES_H

#include "libmatti/net/minecraft/world/entity/Entity.h"

#ifdef __cplusplus
extern "C" {
#endif

// Java: public class LivingEntity extends Entity - the health/damage core the
// mobs and the player share (the full surface arrives with the P5 tick port)
typedef struct LIBMATTI_MC_LivingEntity
{
    LIBMATTI_MC_Entity base;
    // Java: private float absorptionAmount / protected int hurtTime ...
    float health;
    float maxHealth;
    int hurtTime;
    int deathTime;
    bool dead;
} LIBMATTI_MC_LivingEntity;

// Java: public class Player extends LivingEntity (the Avatar side)
typedef struct LIBMATTI_MC_Player
{
    LIBMATTI_MC_LivingEntity base;
    // Java: private float oAttackPower / the inventory lands with P6
    float attackPower;
    // Java: the game profile name (the display name)
    char *name;
} LIBMATTI_MC_Player;

// Java: public class ItemEntity extends Entity
typedef struct LIBMATTI_MC_ItemEntity
{
    LIBMATTI_MC_Entity base;
    // Java: private static final ItemStack ITEM - the dropped stack (the
    // ItemStack port carries the data components)
    void *item;
    int pickupDelay;
    short age;
} LIBMATTI_MC_ItemEntity;

// Java: public class FallingBlockEntity extends Entity
typedef struct LIBMATTI_MC_FallingBlockEntity
{
    LIBMATTI_MC_Entity base;
    LIBMATTI_MC_BlockState *blockState;
    int time;
} LIBMATTI_MC_FallingBlockEntity;

// Java: public class PrimedTnt extends Entity
typedef struct LIBMATTI_MC_PrimedTnt
{
    LIBMATTI_MC_Entity base;
    int fuse;
} LIBMATTI_MC_PrimedTnt;

// Java: public class ExperienceOrb extends Entity
typedef struct LIBMATTI_MC_ExperienceOrb
{
    LIBMATTI_MC_Entity base;
    int value;
} LIBMATTI_MC_ExperienceOrb;

// Java: public class Mob extends LivingEntity - the AI/behaviour core (the
// goals land later; the struct exists so the monster/creature types share it)
typedef struct LIBMATTI_MC_Mob
{
    LIBMATTI_MC_LivingEntity base;
    // Java: private boolean persistenceRequired
    bool persistenceRequired;
    // Java: private final BlockPos pickupCandidate - the leash/angry bookkeeping
    int angerTime;
} LIBMATTI_MC_Mob;

// Java: public class Arrow extends Entity (the projectile core)
typedef struct LIBMATTI_MC_Arrow
{
    LIBMATTI_MC_Entity base;
    bool inGround;
    int life;
} LIBMATTI_MC_Arrow;

// ---------------------------------------------------------------------------
// Java: the EntityType<T> static constants - one accessor per constant
// ---------------------------------------------------------------------------

const LIBMATTI_MC_EntityType *LIBMATTI_MC_EntityType_PLAYER(void);
const LIBMATTI_MC_EntityType *LIBMATTI_MC_EntityType_ITEM(void);
const LIBMATTI_MC_EntityType *LIBMATTI_MC_EntityType_FALLING_BLOCK(void);
const LIBMATTI_MC_EntityType *LIBMATTI_MC_EntityType_TNT(void);
const LIBMATTI_MC_EntityType *LIBMATTI_MC_EntityType_EXPERIENCE_ORB(void);
const LIBMATTI_MC_EntityType *LIBMATTI_MC_EntityType_ARMOR_STAND(void);
const LIBMATTI_MC_EntityType *LIBMATTI_MC_EntityType_ARROW(void);
const LIBMATTI_MC_EntityType *LIBMATTI_MC_EntityType_CREEPER(void);
const LIBMATTI_MC_EntityType *LIBMATTI_MC_EntityType_ZOMBIE(void);
const LIBMATTI_MC_EntityType *LIBMATTI_MC_EntityType_PIG(void);
const LIBMATTI_MC_EntityType *LIBMATTI_MC_EntityType_COW(void);
const LIBMATTI_MC_EntityType *LIBMATTI_MC_EntityType_SHEEP(void);
const LIBMATTI_MC_EntityType *LIBMATTI_MC_EntityType_CHICKEN(void);

// Java: the static block registration - VanillaBootstrap calls this after the
// blocks/items so the ENTITY_TYPE registry fills
void LIBMATTI_MC_VanillaEntities_RegisterAll(void);

// The factory hooks the concrete constructors ride (Java: the ::new references)
LIBMATTI_MC_Entity *LIBMATTI_MC_Player_New(const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level);
LIBMATTI_MC_Entity *LIBMATTI_MC_ItemEntity_New(const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level);
LIBMATTI_MC_Entity *LIBMATTI_MC_FallingBlockEntity_New(const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level);
LIBMATTI_MC_Entity *LIBMATTI_MC_PrimedTnt_New(const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level);
LIBMATTI_MC_Entity *LIBMATTI_MC_ExperienceOrb_New(const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level);
LIBMATTI_MC_Entity *LIBMATTI_MC_ArmorStand_New(const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level);
LIBMATTI_MC_Entity *LIBMATTI_MC_Arrow_New(const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level);
LIBMATTI_MC_Entity *LIBMATTI_MC_Creeper_New(const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level);
LIBMATTI_MC_Entity *LIBMATTI_MC_Zombie_New(const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level);
LIBMATTI_MC_Entity *LIBMATTI_MC_Pig_New(const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level);
LIBMATTI_MC_Entity *LIBMATTI_MC_Cow_New(const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level);
LIBMATTI_MC_Entity *LIBMATTI_MC_Sheep_New(const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level);
LIBMATTI_MC_Entity *LIBMATTI_MC_Chicken_New(const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_ENTITY_VANILLAENTITIES_H
