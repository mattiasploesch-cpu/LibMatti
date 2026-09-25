// Port of the vanilla EntityType constants + the concrete entity subclasses.

#include "libmatti/net/minecraft/world/entity/VanillaEntities.h"

#include "libmatti/net/minecraft/world/entity/EntityType.h"

#include <stdlib.h>
#include <string.h>

// Java: protected Entity(EntityType, Level) through each subclass constructor -
// the port allocates the subclass struct and runs the Entity base init over it.
#define DEFINE_SIMPLE_CTOR(ctorName, structName)                                                              \
    LIBMATTI_MC_Entity *ctorName(const LIBMATTI_MC_EntityType *type, struct LIBMATTI_MC_Level *level)         \
    {                                                                                                         \
        structName *entity = calloc(1, sizeof(structName));                                                    \
        if (entity == NULL)                                                                                    \
            return NULL;                                                                                       \
        /* the base is the first member (the subclass embeds it Java-style), */                                \
        /* so the struct pointer doubles as the Entity pointer */                                              \
        LIBMATTI_MC_Entity_Init((LIBMATTI_MC_Entity *) entity, type, level);                                   \
        return (LIBMATTI_MC_Entity *) entity;                                                                  \
    }

DEFINE_SIMPLE_CTOR(LIBMATTI_MC_Player_New, LIBMATTI_MC_Player)
DEFINE_SIMPLE_CTOR(LIBMATTI_MC_ItemEntity_New, LIBMATTI_MC_ItemEntity)
DEFINE_SIMPLE_CTOR(LIBMATTI_MC_FallingBlockEntity_New, LIBMATTI_MC_FallingBlockEntity)
DEFINE_SIMPLE_CTOR(LIBMATTI_MC_PrimedTnt_New, LIBMATTI_MC_PrimedTnt)
DEFINE_SIMPLE_CTOR(LIBMATTI_MC_ExperienceOrb_New, LIBMATTI_MC_ExperienceOrb)
DEFINE_SIMPLE_CTOR(LIBMATTI_MC_ArmorStand_New, LIBMATTI_MC_LivingEntity)
DEFINE_SIMPLE_CTOR(LIBMATTI_MC_Arrow_New, LIBMATTI_MC_Arrow)
DEFINE_SIMPLE_CTOR(LIBMATTI_MC_Creeper_New, LIBMATTI_MC_Mob)
DEFINE_SIMPLE_CTOR(LIBMATTI_MC_Zombie_New, LIBMATTI_MC_Mob)
DEFINE_SIMPLE_CTOR(LIBMATTI_MC_Pig_New, LIBMATTI_MC_Mob)
DEFINE_SIMPLE_CTOR(LIBMATTI_MC_Cow_New, LIBMATTI_MC_Mob)
DEFINE_SIMPLE_CTOR(LIBMATTI_MC_Sheep_New, LIBMATTI_MC_Mob)
DEFINE_SIMPLE_CTOR(LIBMATTI_MC_Chicken_New, LIBMATTI_MC_Mob)

// Java: private static <T> EntityType<T> register(String, Builder) - the vanilla
// dimensions/eye-heights ride the builder chains 1:1
void LIBMATTI_MC_VanillaEntities_RegisterAll(void)
{
    LIBMATTI_MC_EntityType_Register("minecraft:player",
        LIBMATTI_MC_EntityTypeBuilder_NoSave(
            LIBMATTI_MC_EntityTypeBuilder_NoSummon(
                LIBMATTI_MC_EntityTypeBuilder_Sized(LIBMATTI_MC_EntityTypeBuilder_CreateNothing(LIBMATTI_MC_MobCategory_MISC), 0.6f, 1.8f))));
    LIBMATTI_MC_EntityType_Register("minecraft:item",
        LIBMATTI_MC_EntityTypeBuilder_Sized(LIBMATTI_MC_EntityTypeBuilder_NoLootTable(
            LIBMATTI_MC_EntityTypeBuilder_Of(LIBMATTI_MC_ItemEntity_New, LIBMATTI_MC_MobCategory_MISC)), 0.25f, 0.25f));
    LIBMATTI_MC_EntityType_Register("minecraft:falling_block",
        LIBMATTI_MC_EntityTypeBuilder_Sized(LIBMATTI_MC_EntityTypeBuilder_NoLootTable(
            LIBMATTI_MC_EntityTypeBuilder_Of(LIBMATTI_MC_FallingBlockEntity_New, LIBMATTI_MC_MobCategory_MISC)), 0.98f, 0.98f));
    LIBMATTI_MC_EntityType_Register("minecraft:tnt",
        LIBMATTI_MC_EntityTypeBuilder_FireImmune(
            LIBMATTI_MC_EntityTypeBuilder_Sized(LIBMATTI_MC_EntityTypeBuilder_NoLootTable(
                LIBMATTI_MC_EntityTypeBuilder_Of(LIBMATTI_MC_PrimedTnt_New, LIBMATTI_MC_MobCategory_MISC)), 0.98f, 0.98f)));
    LIBMATTI_MC_EntityType_Register("minecraft:experience_orb",
        LIBMATTI_MC_EntityTypeBuilder_Sized(LIBMATTI_MC_EntityTypeBuilder_NoLootTable(
            LIBMATTI_MC_EntityTypeBuilder_Of(LIBMATTI_MC_ExperienceOrb_New, LIBMATTI_MC_MobCategory_MISC)), 0.5f, 0.5f));
    LIBMATTI_MC_EntityType_Register("minecraft:armor_stand",
        LIBMATTI_MC_EntityTypeBuilder_EyeHeight(
            LIBMATTI_MC_EntityTypeBuilder_Sized(LIBMATTI_MC_EntityTypeBuilder_Of(LIBMATTI_MC_ArmorStand_New, LIBMATTI_MC_MobCategory_MISC), 0.5f, 1.975f), 1.7775f));
    LIBMATTI_MC_EntityType_Register("minecraft:arrow",
        LIBMATTI_MC_EntityTypeBuilder_UpdateInterval(
            LIBMATTI_MC_EntityTypeBuilder_EyeHeight(
                LIBMATTI_MC_EntityTypeBuilder_Sized(LIBMATTI_MC_EntityTypeBuilder_NoLootTable(
                    LIBMATTI_MC_EntityTypeBuilder_Of(LIBMATTI_MC_Arrow_New, LIBMATTI_MC_MobCategory_MISC)), 0.5f, 0.5f), 0.13f), 20));
    LIBMATTI_MC_EntityType_Register("minecraft:creeper",
        LIBMATTI_MC_EntityTypeBuilder_Sized(LIBMATTI_MC_EntityTypeBuilder_Of(LIBMATTI_MC_Creeper_New, LIBMATTI_MC_MobCategory_MONSTER), 0.6f, 1.7f));
    LIBMATTI_MC_EntityType_Register("minecraft:zombie",
        LIBMATTI_MC_EntityTypeBuilder_EyeHeight(
            LIBMATTI_MC_EntityTypeBuilder_Sized(LIBMATTI_MC_EntityTypeBuilder_Of(LIBMATTI_MC_Zombie_New, LIBMATTI_MC_MobCategory_MONSTER), 0.6f, 1.95f), 1.74f));
    LIBMATTI_MC_EntityType_Register("minecraft:pig",
        LIBMATTI_MC_EntityTypeBuilder_Sized(LIBMATTI_MC_EntityTypeBuilder_Of(LIBMATTI_MC_Pig_New, LIBMATTI_MC_MobCategory_CREATURE), 0.9f, 0.9f));
    LIBMATTI_MC_EntityType_Register("minecraft:cow",
        LIBMATTI_MC_EntityTypeBuilder_EyeHeight(
            LIBMATTI_MC_EntityTypeBuilder_Sized(LIBMATTI_MC_EntityTypeBuilder_Of(LIBMATTI_MC_Cow_New, LIBMATTI_MC_MobCategory_CREATURE), 0.9f, 1.4f), 1.3f));
    LIBMATTI_MC_EntityType_Register("minecraft:sheep",
        LIBMATTI_MC_EntityTypeBuilder_EyeHeight(
            LIBMATTI_MC_EntityTypeBuilder_Sized(LIBMATTI_MC_EntityTypeBuilder_Of(LIBMATTI_MC_Sheep_New, LIBMATTI_MC_MobCategory_CREATURE), 0.9f, 1.3f), 1.235f));
    LIBMATTI_MC_EntityType_Register("minecraft:chicken",
        LIBMATTI_MC_EntityTypeBuilder_EyeHeight(
            LIBMATTI_MC_EntityTypeBuilder_Sized(LIBMATTI_MC_EntityTypeBuilder_Of(LIBMATTI_MC_Chicken_New, LIBMATTI_MC_MobCategory_CREATURE), 0.4f, 0.7f), 0.644f));
}

// Java: the static EntityType constants resolve through the registry (the port
// keeps the by-string lookup against the built-in registry)
#define DEFINE_TYPE_GETTER(getter, id)                                                                        \
    const LIBMATTI_MC_EntityType *getter(void)                                                                \
    {                                                                                                         \
        return LIBMATTI_MC_EntityType_ByString("minecraft:" id);                                              \
    }

DEFINE_TYPE_GETTER(LIBMATTI_MC_EntityType_PLAYER, "player")
DEFINE_TYPE_GETTER(LIBMATTI_MC_EntityType_ITEM, "item")
DEFINE_TYPE_GETTER(LIBMATTI_MC_EntityType_FALLING_BLOCK, "falling_block")
DEFINE_TYPE_GETTER(LIBMATTI_MC_EntityType_TNT, "tnt")
DEFINE_TYPE_GETTER(LIBMATTI_MC_EntityType_EXPERIENCE_ORB, "experience_orb")
DEFINE_TYPE_GETTER(LIBMATTI_MC_EntityType_ARMOR_STAND, "armor_stand")
DEFINE_TYPE_GETTER(LIBMATTI_MC_EntityType_ARROW, "arrow")
DEFINE_TYPE_GETTER(LIBMATTI_MC_EntityType_CREEPER, "creeper")
DEFINE_TYPE_GETTER(LIBMATTI_MC_EntityType_ZOMBIE, "zombie")
DEFINE_TYPE_GETTER(LIBMATTI_MC_EntityType_PIG, "pig")
DEFINE_TYPE_GETTER(LIBMATTI_MC_EntityType_COW, "cow")
DEFINE_TYPE_GETTER(LIBMATTI_MC_EntityType_SHEEP, "sheep")
DEFINE_TYPE_GETTER(LIBMATTI_MC_EntityType_CHICKEN, "chicken")
