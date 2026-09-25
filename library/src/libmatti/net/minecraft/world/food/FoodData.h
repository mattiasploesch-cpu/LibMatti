// Port of net.minecraft.world.food.FoodData + FoodConstants (the hunger bar
// state machine: foodLevel/saturationLevel/exhaustionLevel/tickTimer). The
// tick() body rides the P6 health work - the port carries the state, the eat/
// exhaustion paths and the NBT surface the Player save writes.

#ifndef MATTICRAFT_MC_WORLD_FOOD_FOODDATA_H
#define MATTICRAFT_MC_WORLD_FOOD_FOODDATA_H

#include "libmatti/net/minecraft/nbt/CompoundTag.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: FoodConstants - the constants the tick/eat paths share
#define LIBMATTI_MC_FoodConstants_MAX_FOOD 20
#define LIBMATTI_MC_FoodConstants_MAX_SATURATION 20.0f
#define LIBMATTI_MC_FoodConstants_START_SATURATION 5.0f
#define LIBMATTI_MC_FoodConstants_SATURATION_FLOOR 2.5f
#define LIBMATTI_MC_FoodConstants_EXHAUSTION_DROP 4.0f
#define LIBMATTI_MC_FoodConstants_HEALTH_TICK_COUNT 80
#define LIBMATTI_MC_FoodConstants_HEALTH_TICK_COUNT_SATURATED 10
#define LIBMATTI_MC_FoodConstants_HEAL_LEVEL 18
#define LIBMATTI_MC_FoodConstants_SPRINT_LEVEL 6
#define LIBMATTI_MC_FoodConstants_STARVE_LEVEL 0
#define LIBMATTI_MC_FoodConstants_EXHAUSTION_HEAL 6.0f
#define LIBMATTI_MC_FoodConstants_EXHAUSTION_JUMP 0.05f
#define LIBMATTI_MC_FoodConstants_EXHAUSTION_SPRINT_JUMP 0.2f
#define LIBMATTI_MC_FoodConstants_EXHAUSTION_MINE 0.005f
#define LIBMATTI_MC_FoodConstants_EXHAUSTION_ATTACK 0.1f
#define LIBMATTI_MC_FoodConstants_EXHAUSTION_SPRINT 0.1f
#define LIBMATTI_MC_FoodConstants_EXHAUSTION_SWIM 0.01f

// Java: public class FoodData
typedef struct LIBMATTI_MC_FoodData
{
    // Java: private int foodLevel = 20
    int foodLevel;
    // Java: private float saturationLevel = 5.0F
    float saturationLevel;
    // Java: private float exhaustionLevel
    float exhaustionLevel;
    // Java: private int tickTimer
    int tickTimer;
} LIBMATTI_MC_FoodData;

// Java: the field initializers (foodLevel 20, saturation 5, exhaustion 0)
void LIBMATTI_MC_FoodData_Init(LIBMATTI_MC_FoodData *foodData);

// Java: private void add(int, float) - clamp food to 0..20, saturation to
// 0..foodLevel
void LIBMATTI_MC_FoodData_Add(LIBMATTI_MC_FoodData *foodData, int food, float saturation);
// Java: public void eat(int nutrition, float saturationModifier) - the
// saturation rides saturationByModifier(nutrition * modifier * 2)
void LIBMATTI_MC_FoodData_Eat(LIBMATTI_MC_FoodData *foodData, int nutrition, float saturationModifier);
// Java: FoodConstants.saturationByModifier(int, float)
float LIBMATTI_MC_FoodData_SaturationByModifier(int nutrition, float modifier);

// Java: public void addExhaustion(float) - capped at 40
void LIBMATTI_MC_FoodData_AddExhaustion(LIBMATTI_MC_FoodData *foodData, float exhaustion);
// Java: the exhaustion drain (exhaustion > 4 -> saturation first, then food) -
// the ServerPlayer/difficulty tails land with the tick port
void LIBMATTI_MC_FoodData_DrainExhaustion(LIBMATTI_MC_FoodData *foodData);

// Java: public int getFoodLevel() / public float getSaturationLevel()
int LIBMATTI_MC_FoodData_GetFoodLevel(const LIBMATTI_MC_FoodData *foodData);
float LIBMATTI_MC_FoodData_GetSaturationLevel(const LIBMATTI_MC_FoodData *foodData);
// Java: public boolean hasEnoughFood() / needsFood()
bool LIBMATTI_MC_FoodData_HasEnoughFood(const LIBMATTI_MC_FoodData *foodData);
bool LIBMATTI_MC_FoodData_NeedsFood(const LIBMATTI_MC_FoodData *foodData);
// Java: public void setFoodLevel(int) / setSaturation(float)
void LIBMATTI_MC_FoodData_SetFoodLevel(LIBMATTI_MC_FoodData *foodData, int foodLevel);
void LIBMATTI_MC_FoodData_SetSaturation(LIBMATTI_MC_FoodData *foodData, float saturationLevel);

// Java: addAdditionalSaveData - the keys "foodLevel"/"foodTickTimer"/
// "foodSaturationLevel"/"foodExhaustionLevel"
void LIBMATTI_MC_FoodData_AddAdditionalSaveData(LIBMATTI_MC_FoodData *foodData, LIBMATTI_MC_Nbt_CompoundTag *tag);
// Java: readAdditionalSaveData - defaults 20/0/5/0
void LIBMATTI_MC_FoodData_ReadAdditionalSaveData(LIBMATTI_MC_FoodData *foodData, LIBMATTI_MC_Nbt_CompoundTag *tag);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_MC_WORLD_FOOD_FOODDATA_H
