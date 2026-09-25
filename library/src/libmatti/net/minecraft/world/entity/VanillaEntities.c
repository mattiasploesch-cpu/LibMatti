// Port of the vanilla EntityType constants + the concrete entity subclasses.

#include "libmatti/net/minecraft/world/entity/VanillaEntities.h"

#include "libmatti/net/minecraft/util/Mth.h"
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

// ---------------------------------------------------------------------------
// Java: public class Player extends LivingEntity - the surface the game port
// drives before the inventory/XP/hunger-tick ports land (P6)
// ---------------------------------------------------------------------------

// Java: public Player(Level p_36114_, GameProfile p_36115_) - the port takes
// the profile name; the field initializers ride. The LocalPlayer embeds the
// struct and runs the same tail (C has no constructor chaining).
void LIBMATTI_MC_Player_Init(LIBMATTI_MC_Player *player, struct LIBMATTI_MC_Level *level, const char *name)
{
    if (player == NULL)
        return;
    // Java: protected LivingEntity(EntityType, Level) - the defaults: maxHealth
    // 20.0f, health = maxHealth
    LIBMATTI_MC_Entity_Init(&player->base.base, LIBMATTI_MC_EntityType_PLAYER(), level);
    player->base.maxHealth = 20.0f;
    player->base.health = 20.0f;
    player->name = name != NULL ? strdup(name) : NULL;
    // Java: public final Abilities abilities = new Abilities()
    LIBMATTI_MC_Abilities_Init(&player->abilities);
    // Java: public final FoodData foodData = new FoodData()
    LIBMATTI_MC_FoodData_Init(&player->foodData);
}

LIBMATTI_MC_Player *LIBMATTI_MC_Player_Create(struct LIBMATTI_MC_Level *level, const char *name)
{
    LIBMATTI_MC_Player *player = calloc(1, sizeof(LIBMATTI_MC_Player));
    if (player == NULL)
        return NULL;
    LIBMATTI_MC_Player_Init(player, level, name);
    return player;
}

void LIBMATTI_MC_Player_Free(LIBMATTI_MC_Player *player)
{
    if (player == NULL)
        return;
    LIBMATTI_MC_Entity_Free(&player->base.base);
    free(player->name);
    // the embedded struct is freed by the owner (Create allocates, the
    // LocalPlayer's calloc'd struct owns the memory)
    free(player);
}

const char *LIBMATTI_MC_Player_GetName(const LIBMATTI_MC_Player *player)
{
    return player != NULL ? player->name : NULL;
}

float LIBMATTI_MC_Player_GetHealth(const LIBMATTI_MC_Player *player)
{
    return player != NULL ? player->base.health : 0.0f;
}

// Java: public void setHealth(float) - clamped to [0, maxHealth]
void LIBMATTI_MC_Player_SetHealth(LIBMATTI_MC_Player *player, float health)
{
    if (player == NULL)
        return;
    player->base.health = LIBMATTI_MC_Mth_Clamp(health, 0.0f, player->base.maxHealth);
}

LIBMATTI_MC_Abilities *LIBMATTI_MC_Player_GetAbilities(LIBMATTI_MC_Player *player)
{
    return player != NULL ? &player->abilities : NULL;
}

LIBMATTI_MC_FoodData *LIBMATTI_MC_Player_GetFoodData(LIBMATTI_MC_Player *player)
{
    return player != NULL ? &player->foodData : NULL;
}

int LIBMATTI_MC_Player_GetScore(const LIBMATTI_MC_Player *player)
{
    return player != NULL ? player->score : 0;
}

void LIBMATTI_MC_Player_SetScore(LIBMATTI_MC_Player *player, int score)
{
    if (player != NULL)
        player->score = score;
}

// Java: addAdditionalSaveData - the player keys over the Entity save (the
// inventory/enderchest lists ride the P6 item port)
void LIBMATTI_MC_Player_SaveWithoutId(LIBMATTI_MC_Player *player, LIBMATTI_MC_Nbt_CompoundTag *tag)
{
    if (player == NULL || tag == NULL)
        return;
    // Java: super.addAdditionalSaveData + NbtUtils.addCurrentDataVersion
    LIBMATTI_MC_Entity_SaveWithoutId(&player->base.base, tag);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(tag, "SelectedItemSlot", 0);
    LIBMATTI_MC_Nbt_CompoundTag_PutShort(tag, "SleepTimer", (int16_t) 0);
    LIBMATTI_MC_Nbt_CompoundTag_PutFloat(tag, "XpP", player->experienceProgress);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(tag, "XpLevel", player->experienceLevel);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(tag, "XpTotal", player->totalExperience);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(tag, "Score", player->score);
    // Java: this.foodData.addAdditionalSaveData(p_406026_)
    LIBMATTI_MC_FoodData_AddAdditionalSaveData(&player->foodData, tag);
    // Java: p_406026_.store("abilities", Abilities.Packed.CODEC, this.abilities.pack())
    {
        LIBMATTI_MC_Nbt_CompoundTag *abilities = LIBMATTI_MC_Nbt_CompoundTag_New();
        LIBMATTI_MC_Nbt_CompoundTag_PutBoolean(abilities, "invulnerable", player->abilities.invulnerable);
        LIBMATTI_MC_Nbt_CompoundTag_PutBoolean(abilities, "flying", player->abilities.flying);
        LIBMATTI_MC_Nbt_CompoundTag_PutBoolean(abilities, "mayfly", player->abilities.mayfly);
        LIBMATTI_MC_Nbt_CompoundTag_PutBoolean(abilities, "instabuild", player->abilities.instabuild);
        LIBMATTI_MC_Nbt_CompoundTag_PutBoolean(abilities, "mayBuild", player->abilities.mayBuild);
        LIBMATTI_MC_Nbt_CompoundTag_PutFloat(abilities, "flySpeed", player->abilities.flyingSpeed);
        LIBMATTI_MC_Nbt_CompoundTag_PutFloat(abilities, "walkSpeed", player->abilities.walkingSpeed);
        LIBMATTI_MC_Nbt_CompoundTag_Put(tag, "abilities", (LIBMATTI_MC_Nbt_Tag *) abilities);
    }
}

// Java: readAdditionalSaveData
void LIBMATTI_MC_Player_Load(LIBMATTI_MC_Player *player, LIBMATTI_MC_Nbt_CompoundTag *tag)
{
    if (player == NULL || tag == NULL)
        return;
    // Java: super.readAdditionalSaveData
    LIBMATTI_MC_Entity_Load(&player->base.base, tag);
    player->experienceProgress = LIBMATTI_MC_Nbt_CompoundTag_GetFloatOr(tag, "XpP", 0.0f);
    player->experienceLevel = LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(tag, "XpLevel", 0);
    player->totalExperience = LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(tag, "XpTotal", 0);
    player->score = LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(tag, "Score", 0);
    // Java: this.foodData.readAdditionalSaveData(p_410352_)
    LIBMATTI_MC_FoodData_ReadAdditionalSaveData(&player->foodData, tag);
    // Java: p_410352_.read("abilities", Abilities.Packed.CODEC).ifPresent(this.abilities::apply) -
    // the missing key keeps the defaults (getCompoundOrEmpty is fresh + empty)
    {
        LIBMATTI_MC_Nbt_CompoundTag *abilities = LIBMATTI_MC_Nbt_CompoundTag_GetCompoundOrEmpty(tag, "abilities");
        if (abilities != NULL && LIBMATTI_MC_Nbt_CompoundTag_Contains(abilities, "mayBuild"))
        {
            player->abilities.invulnerable = LIBMATTI_MC_Nbt_CompoundTag_GetBooleanOr(abilities, "invulnerable", false);
            player->abilities.flying = LIBMATTI_MC_Nbt_CompoundTag_GetBooleanOr(abilities, "flying", false);
            player->abilities.mayfly = LIBMATTI_MC_Nbt_CompoundTag_GetBooleanOr(abilities, "mayfly", false);
            player->abilities.instabuild = LIBMATTI_MC_Nbt_CompoundTag_GetBooleanOr(abilities, "instabuild", false);
            player->abilities.mayBuild = LIBMATTI_MC_Nbt_CompoundTag_GetBooleanOr(abilities, "mayBuild", true);
            player->abilities.flyingSpeed = LIBMATTI_MC_Nbt_CompoundTag_GetFloatOr(abilities, "flySpeed", 0.05f);
            player->abilities.walkingSpeed = LIBMATTI_MC_Nbt_CompoundTag_GetFloatOr(abilities, "walkSpeed", 0.1f);
        }
    }
    // Java: this.getAttribute(MOVEMENT_SPEED).setBaseValue(abilities.getWalkingSpeed())
    //       - the attribute map lands with the P5.3 physics port
}
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
