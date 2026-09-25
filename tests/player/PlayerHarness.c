// Player harness: the P5.2 core - the KeyMapping registry (bind/isDown/click,
// the vanilla set), the Input record + move vector through the KeyboardInput
// tick, the Player entity surface (abilities/food/health/NBT) and the
// LocalPlayer wiring (input tick into the entity).

#include "libmatti/net/minecraft/Bootstrap.h"
#include "libmatti/net/minecraft/client/KeyMapping.h"
#include "libmatti/net/minecraft/client/player/LocalPlayer.h"
#include "libmatti/net/minecraft/world/entity/VanillaEntities.h"
#include "libmatti/net/minecraft/world/food/FoodData.h"
#include "libmatti/net/minecraft/world/level/Level.h"
#include "libmatti/net/minecraft/world/phys/Vec2.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static int failures;
static int checks;

static void check(int condition, const char *what)
{
    checks++;
    if (!condition)
    {
        failures++;
        printf("FAIL: %s\n", what);
    }
}

int main(void)
{
    // the registry bootstrap raises the guard + fills BLOCK/ITEM/ENTITY_TYPE
    LIBMATTI_MC_Bootstrap_BootStrap();
    // Java: the Options field initializers (the KeyMapping constructor statics)
    LIBMATTI_MC_KeyMapping_CreateVanillaMappings();

    // ---- the KeyMapping registry -------------------------------------------------
    LIBMATTI_MC_KeyMapping *forward = LIBMATTI_MC_KeyMapping_Forward();
    LIBMATTI_MC_KeyMapping *left = LIBMATTI_MC_KeyMapping_Left();
    check(forward != NULL && strcmp(LIBMATTI_MC_KeyMapping_GetName(forward), "key.forward") == 0, "forward binding name");
    check(LIBMATTI_MC_KeyMapping_Get(forward != NULL ? LIBMATTI_MC_KeyMapping_GetName(forward) : "") == forward, "get() through ALL");
    check(LIBMATTI_MC_KeyMapping_GetDefaultKey(forward) == 87, "forward default W (87)");
    check(LIBMATTI_MC_KeyMapping_GetCategory(forward) == LIBMATTI_MC_KeyMappingCategory_MOVEMENT, "forward category movement");
    LIBMATTI_MC_KeyMapping *attack = LIBMATTI_MC_KeyMapping_Attack();
    check(attack != NULL && LIBMATTI_MC_KeyMapping_Matches(attack, LIBMATTI_MC_InputConstants_MOUSE, 0), "attack binds mouse 0");
    LIBMATTI_MC_KeyMapping *hotbar9 = LIBMATTI_MC_KeyMapping_Hotbar(8);
    check(hotbar9 != NULL && LIBMATTI_MC_KeyMapping_GetDefaultKey(hotbar9) == 57, "hotbar 9 on digit 57");

    // set/click/isDown through the static dispatch (Java: KeyMapping.set/click)
    check(!LIBMATTI_MC_KeyMapping_IsDown(forward), "forward starts up");
    LIBMATTI_MC_KeyMapping_Set(LIBMATTI_MC_InputConstants_KEYSYM, 87, true);
    check(LIBMATTI_MC_KeyMapping_IsDown(forward), "set(key, true) marks down");
    LIBMATTI_MC_KeyMapping_Set(LIBMATTI_MC_InputConstants_KEYSYM, 87, false);
    check(!LIBMATTI_MC_KeyMapping_IsDown(forward), "set(key, false) marks up");

    LIBMATTI_MC_KeyMapping *use = LIBMATTI_MC_KeyMapping_Use();
    LIBMATTI_MC_KeyMapping_Click(LIBMATTI_MC_InputConstants_MOUSE, 1);
    LIBMATTI_MC_KeyMapping_Click(LIBMATTI_MC_InputConstants_MOUSE, 1);
    check(LIBMATTI_MC_KeyMapping_ConsumeClick(use), "first consumeClick takes");
    check(LIBMATTI_MC_KeyMapping_ConsumeClick(use), "second consumeClick takes");
    check(!LIBMATTI_MC_KeyMapping_ConsumeClick(use), "third consumeClick misses");
    // releaseAll clears the down states (Java: KeyMapping.releaseAll)
    LIBMATTI_MC_KeyMapping_Set(LIBMATTI_MC_InputConstants_KEYSYM, 87, true);
    LIBMATTI_MC_KeyMapping_ReleaseAll();
    check(!LIBMATTI_MC_KeyMapping_IsDown(forward), "releaseAll drops the down state");

    // the save string + the unbound/default checks
    char saved[32];
    LIBMATTI_MC_KeyMapping_SaveString(forward, saved, sizeof(saved));
    check(strcmp(saved, "key.87") == 0, "forward save string");
    LIBMATTI_MC_KeyMapping_SaveString(attack, saved, sizeof(saved));
    check(strcmp(saved, "mouse.0") == 0, "attack save string");
    check(LIBMATTI_MC_KeyMapping_IsDefault(forward), "forward on the default key");
    check(!LIBMATTI_MC_KeyMapping_IsUnbound(forward), "forward is bound");

    // ---- the Input record + Vec2 --------------------------------------------------
    LIBMATTI_MC_Input empty = LIBMATTI_MC_Input_Empty();
    check(!empty.forward && !empty.jump && !empty.sprint, "Input.EMPTY all false");
    LIBMATTI_MC_Vec2 zero = LIBMATTI_MC_Vec2_Zero();
    LIBMATTI_MC_Vec2 norm = LIBMATTI_MC_Vec2_Normalized(LIBMATTI_MC_Vec2_Of(3.0f, 4.0f));
    check(fabs(zero.x) < 1e-6f && fabs(norm.x - 0.6f) < 1e-5f && fabs(norm.y - 0.8f) < 1e-5f, "Vec2 normalize (3,4)");
    LIBMATTI_MC_Vec2 tiny = LIBMATTI_MC_Vec2_Normalized(LIBMATTI_MC_Vec2_Of(1.0e-5f, 0.0f));
    check(fabs(tiny.x) < 1e-6f && fabs(tiny.y) < 1e-6f, "Vec2 normalize floor returns ZERO");

    // ---- the Player entity ---------------------------------------------------------
    LIBMATTI_MC_Level *level = LIBMATTI_MC_Level_New(-64, 384, LIBMATTI_MC_Level_OVERWORLD, true);
    LIBMATTI_MC_Player *player = LIBMATTI_MC_Player_Create(level, "Steve");
    check(player != NULL, "player created");
    check(LIBMATTI_MC_Entity_GetType(&player->base.base) == LIBMATTI_MC_EntityType_PLAYER(), "player type is player");
    check(strcmp(LIBMATTI_MC_Player_GetName(player), "Steve") == 0, "player name");
    check(fabs(LIBMATTI_MC_Player_GetHealth(player) - 20.0f) < 1e-5f, "player health 20");
    LIBMATTI_MC_Player_SetHealth(player, 200.0f);
    check(fabs(LIBMATTI_MC_Player_GetHealth(player) - 20.0f) < 1e-5f, "setHealth clamps to maxHealth");
    // the vanilla player dimensions (0.6 x 1.8, eye 1.8 * 0.85)
    check(fabs(LIBMATTI_MC_Entity_GetEyeHeight(&player->base.base) - 1.53f) < 1e-5f, "player eye height 1.53");
    const LIBMATTI_MC_AABB *bb = LIBMATTI_MC_Entity_GetBoundingBox(&player->base.base);
    check(bb != NULL && fabs(bb->maxY - 1.8) < 1e-5f, "player bbox height 1.8");

    // the abilities defaults
    LIBMATTI_MC_Abilities *abilities = LIBMATTI_MC_Player_GetAbilities(player);
    check(abilities != NULL && !abilities->invulnerable && !abilities->flying && abilities->mayBuild, "ability defaults");
    check(fabs(LIBMATTI_MC_Abilities_GetWalkingSpeed(abilities) - 0.1f) < 1e-6f, "walk speed 0.1");
    check(fabs(LIBMATTI_MC_Abilities_GetFlyingSpeed(abilities) - 0.05f) < 1e-6f, "fly speed 0.05");

    // the food data: eat + exhaustion + the NBT keys
    LIBMATTI_MC_FoodData *food = LIBMATTI_MC_Player_GetFoodData(player);
    check(food != NULL && LIBMATTI_MC_FoodData_GetFoodLevel(food) == 20, "food starts 20");
    LIBMATTI_MC_FoodData_SetFoodLevel(food, 10);
    LIBMATTI_MC_FoodData_Eat(food, 4, 0.6f);
    check(LIBMATTI_MC_FoodData_GetFoodLevel(food) == 14, "eat adds nutrition");
    // 4.8 from the eat ride the 5.0 start saturation -> 9.8, clamped to foodLevel 14
    check(fabs(LIBMATTI_MC_FoodData_GetSaturationLevel(food) - 9.8f) < 1e-4f, "eat raises the start saturation (5 + 4.8)");
    LIBMATTI_MC_FoodData_AddExhaustion(food, 5.0f);
    LIBMATTI_MC_FoodData_DrainExhaustion(food);
    check(LIBMATTI_MC_FoodData_GetSaturationLevel(food) == 8.8f, "exhaustion drain takes saturation first");
    check(LIBMATTI_MC_FoodData_HasEnoughFood(food), "food > 6 is enough");
    LIBMATTI_MC_FoodData_SetFoodLevel(food, 2);
    check(!LIBMATTI_MC_FoodData_HasEnoughFood(food), "food <= 6 blocks sprint");
    check(LIBMATTI_MC_FoodData_NeedsFood(food), "needsFood below 20");

    // the NBT round trip over the player keys
    LIBMATTI_MC_Entity_SetPos(&player->base.base, 1.5, 70.0, 2.5);
    LIBMATTI_MC_Player_SetScore(player, 42);
    LIBMATTI_MC_FoodData_SetFoodLevel(food, 17);
    abilities->mayfly = true;
    LIBMATTI_MC_Nbt_CompoundTag *tag = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Player_SaveWithoutId(player, tag);
    check(LIBMATTI_MC_Nbt_CompoundTag_Contains(tag, "foodLevel"), "save writes foodLevel");
    check(LIBMATTI_MC_Nbt_CompoundTag_Contains(tag, "abilities"), "save writes abilities");
    check(LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(tag, "Score", -1) == 42, "save writes Score");
    LIBMATTI_MC_Player *loaded = LIBMATTI_MC_Player_Create(level, "Alex");
    LIBMATTI_MC_Player_Load(loaded, tag);
    check(LIBMATTI_MC_FoodData_GetFoodLevel(LIBMATTI_MC_Player_GetFoodData(loaded)) == 17, "load restores foodLevel");
    check(LIBMATTI_MC_Player_GetScore(loaded) == 42, "load restores Score");
    check(LIBMATTI_MC_Player_GetAbilities(loaded)->mayfly, "load restores mayfly");
    check(fabs(LIBMATTI_MC_Entity_GetX(&loaded->base.base) - 1.5) < 1e-9, "load restores the entity Pos");
    LIBMATTI_MC_Player_Free(loaded);
    LIBMATTI_MC_Nbt_Tag_Free((LIBMATTI_MC_Nbt_Tag *) tag);

    // ---- the LocalPlayer + the KeyboardInput tick ------------------------------------
    LIBMATTI_MC_LocalPlayer *local = LIBMATTI_MC_LocalPlayer_New(level, "Local", NULL);
    check(local != NULL, "local player created");
    check(LIBMATTI_MC_Entity_GetType(&local->player.base.base) == LIBMATTI_MC_EntityType_PLAYER(), "local player base type");
    check(LIBMATTI_MC_Level_GetEntityCount(level) == 0, "not added yet (the client owns the spawn)");
    check(LIBMATTI_MC_Level_AddEntity(level, &local->player.base.base), "local player enters the level");
    check(LIBMATTI_MC_Entity_Level(&local->player.base.base) == level, "level wired");

    // nothing pressed -> empty input, zero move vector
    LIBMATTI_MC_LocalPlayer_TickInput(local);
    const LIBMATTI_MC_Input *presses = LIBMATTI_MC_LocalPlayer_GetKeyPresses(local);
    check(presses != NULL && !presses->forward && !presses->left && !presses->jump, "no keys -> empty input");
    LIBMATTI_MC_Vec2 move = LIBMATTI_MC_LocalPlayer_GetMoveVector(local);
    check(fabs(move.x) < 1e-6f && fabs(move.y) < 1e-6f, "no keys -> zero move vector");

    // W + A: forward impulse 1, strafe +1 (Java: LEFT drives +1) -> the
    // diagonal normalizes to (0.707, 0.707)
    LIBMATTI_MC_KeyMapping_Set(LIBMATTI_MC_InputConstants_KEYSYM, 87, true);
    LIBMATTI_MC_KeyMapping_Set(LIBMATTI_MC_InputConstants_KEYSYM, 65, true);
    LIBMATTI_MC_LocalPlayer_TickInput(local);
    presses = LIBMATTI_MC_LocalPlayer_GetKeyPresses(local);
    check(presses->forward && presses->left && !presses->backward, "W+A read into the Input record");
    move = LIBMATTI_MC_LocalPlayer_GetMoveVector(local);
    check(fabs(move.x - 0.70710678f) < 1e-5f && fabs(move.y - 0.70710678f) < 1e-5f, "diagonal move vector normalized");
    check(LIBMATTI_MC_LocalPlayer_HasForwardImpulse(local), "W gives forward impulse");

    // W + S cancel -> impulse 0 (Java: calculateImpulse)
    LIBMATTI_MC_KeyMapping_Set(LIBMATTI_MC_InputConstants_KEYSYM, 83, true);
    LIBMATTI_MC_LocalPlayer_TickInput(local);
    presses = LIBMATTI_MC_LocalPlayer_GetKeyPresses(local);
    check(presses->forward && presses->backward, "W+S both read");
    move = LIBMATTI_MC_LocalPlayer_GetMoveVector(local);
    check(fabs(move.y) < 1e-6f, "W+S cancel the forward impulse");
    LIBMATTI_MC_KeyMapping_Set(LIBMATTI_MC_InputConstants_KEYSYM, 87, false);
    LIBMATTI_MC_KeyMapping_Set(LIBMATTI_MC_InputConstants_KEYSYM, 65, false);
    LIBMATTI_MC_KeyMapping_Set(LIBMATTI_MC_InputConstants_KEYSYM, 83, false);

    // the mouse-look rides Entity.turn (the MouseHandler port): dx 10 -> yaw +1.5
    LIBMATTI_MC_Entity_SetRot(&local->player.base.base, 0.0f, 0.0f);
    LIBMATTI_MC_Entity_Turn(&local->player.base.base, 10.0, 0.0);
    check(fabs(LIBMATTI_MC_Entity_GetYRot(&local->player.base.base) - 1.5f) < 1e-5f, "turn adds yaw * 0.15");
    LIBMATTI_MC_Entity_Turn(&local->player.base.base, 0.0, 800.0);
    check(fabs(LIBMATTI_MC_Entity_GetXRot(&local->player.base.base) - 90.0f) < 1e-5f, "turn clamps pitch to +90");

    // the per-tick tail runs the entity base tick
    LIBMATTI_MC_LocalPlayer_Tick(local);
    check(!LIBMATTI_MC_Entity_IsRemoved(&local->player.base.base), "local tick keeps the entity alive");

    // the leave: the entity drops out of the level list before the free
    // (Java: the ClientLevel removes the player on disconnect)
    LIBMATTI_MC_Level_RemoveEntity(level, &local->player.base.base);
    LIBMATTI_MC_LocalPlayer_Free(local);
    check(LIBMATTI_MC_Level_GetEntityCount(level) == 0, "removing the local player empties the level");
    LIBMATTI_MC_Player_Free(player);
    LIBMATTI_MC_Level_Free(level);

    printf("player: %d checks passed (%s)\n", checks, failures ? "FAILURES" : "ok");
    return failures != 0;
}
