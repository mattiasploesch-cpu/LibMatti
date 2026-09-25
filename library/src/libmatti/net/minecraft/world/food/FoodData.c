// Port of net.minecraft.world.food.FoodData (implementation). The tick() body
// needs the ServerPlayer/difficulty/gamerule surface - it rides the P6 health
// work; the port keeps the state machine the eat/exhaustion paths share.

#include "libmatti/net/minecraft/world/food/FoodData.h"

#include "libmatti/net/minecraft/util/Mth.h"

#include <math.h>

// Java: private void add(int p_335153_, float p_332105_)
void LIBMATTI_MC_FoodData_Add(LIBMATTI_MC_FoodData *foodData, int food, float saturation)
{
    if (foodData == NULL)
        return;
    // Java: this.foodLevel = Mth.clamp(p_335153_ + this.foodLevel, 0, 20)
    foodData->foodLevel = LIBMATTI_MC_Mth_ClampI(food + foodData->foodLevel, 0, LIBMATTI_MC_FoodConstants_MAX_FOOD);
    // Java: this.saturationLevel = Mth.clamp(p_332105_ + this.saturationLevel, 0.0F, this.foodLevel)
    foodData->saturationLevel = LIBMATTI_MC_Mth_Clamp(saturation + foodData->saturationLevel, 0.0f, (float) foodData->foodLevel);
}

// Java: FoodConstants.saturationByModifier(int p_328800_, float p_333065_)
float LIBMATTI_MC_FoodData_SaturationByModifier(int nutrition, float modifier)
{
    return (float) nutrition * modifier * 2.0f;
}

// Java: public void eat(int p_38708_, float p_38709_)
void LIBMATTI_MC_FoodData_Eat(LIBMATTI_MC_FoodData *foodData, int nutrition, float saturationModifier)
{
    LIBMATTI_MC_FoodData_Add(foodData, nutrition,
                             LIBMATTI_MC_FoodData_SaturationByModifier(nutrition, saturationModifier));
}

// Java: public void addExhaustion(float p_38704_) - Math.min(exhaustion + v, 40)
void LIBMATTI_MC_FoodData_AddExhaustion(LIBMATTI_MC_FoodData *foodData, float exhaustion)
{
    if (foodData == NULL)
        return;
    // Java: this.exhaustionLevel = Math.min(this.exhaustionLevel + p_38704_, 40.0F)
    float next = foodData->exhaustionLevel + exhaustion;
    foodData->exhaustionLevel = next > 40.0f ? 40.0f : next;
}

// The tick() exhaustion head: Java drains exhaustion in 4-steps, saturation
// first, then the food level (the peaceful check rides the difficulty port).
void LIBMATTI_MC_FoodData_DrainExhaustion(LIBMATTI_MC_FoodData *foodData)
{
    if (foodData == NULL)
        return;
    // Java: if (this.exhaustionLevel > 4.0F) { this.exhaustionLevel -= 4.0F; ... }
    if (foodData->exhaustionLevel > LIBMATTI_MC_FoodConstants_EXHAUSTION_DROP)
    {
        foodData->exhaustionLevel -= LIBMATTI_MC_FoodConstants_EXHAUSTION_DROP;
        // Java: if (this.saturationLevel > 0) saturation = max(saturation - 1, 0)
        //       else foodLevel = max(foodLevel - 1, 0) (non-peaceful only)
        if (foodData->saturationLevel > 0.0f)
            foodData->saturationLevel = foodData->saturationLevel - 1.0f > 0.0f ? foodData->saturationLevel - 1.0f : 0.0f;
        else
            foodData->foodLevel = foodData->foodLevel - 1 > 0 ? foodData->foodLevel - 1 : 0;
    }
}

int LIBMATTI_MC_FoodData_GetFoodLevel(const LIBMATTI_MC_FoodData *foodData)
{
    return foodData != NULL ? foodData->foodLevel : 0;
}

float LIBMATTI_MC_FoodData_GetSaturationLevel(const LIBMATTI_MC_FoodData *foodData)
{
    return foodData != NULL ? foodData->saturationLevel : 0.0f;
}

// Java: public boolean hasEnoughFood() - foodLevel > 6 (sprinting needs it)
bool LIBMATTI_MC_FoodData_HasEnoughFood(const LIBMATTI_MC_FoodData *foodData)
{
    return foodData != NULL && (float) foodData->foodLevel > 6.0f;
}

// Java: public boolean needsFood()
bool LIBMATTI_MC_FoodData_NeedsFood(const LIBMATTI_MC_FoodData *foodData)
{
    return foodData != NULL && foodData->foodLevel < LIBMATTI_MC_FoodConstants_MAX_FOOD;
}

void LIBMATTI_MC_FoodData_SetFoodLevel(LIBMATTI_MC_FoodData *foodData, int foodLevel)
{
    if (foodData != NULL)
        foodData->foodLevel = foodLevel;
}

void LIBMATTI_MC_FoodData_SetSaturation(LIBMATTI_MC_FoodData *foodData, float saturationLevel)
{
    if (foodData != NULL)
        foodData->saturationLevel = saturationLevel;
}

// Java: the field initializers (foodLevel 20, saturation 5)
void LIBMATTI_MC_FoodData_Init(LIBMATTI_MC_FoodData *foodData)
{
    if (foodData == NULL)
        return;
    foodData->foodLevel = 20;
    foodData->saturationLevel = 5.0f;
    foodData->exhaustionLevel = 0.0f;
    foodData->tickTimer = 0;
}

// Java: public void addAdditionalSaveData(ValueOutput p_406888_)
void LIBMATTI_MC_FoodData_AddAdditionalSaveData(LIBMATTI_MC_FoodData *foodData, LIBMATTI_MC_Nbt_CompoundTag *tag)
{
    if (foodData == NULL || tag == NULL)
        return;
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(tag, "foodLevel", foodData->foodLevel);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(tag, "foodTickTimer", foodData->tickTimer);
    LIBMATTI_MC_Nbt_CompoundTag_PutFloat(tag, "foodSaturationLevel", foodData->saturationLevel);
    LIBMATTI_MC_Nbt_CompoundTag_PutFloat(tag, "foodExhaustionLevel", foodData->exhaustionLevel);
}

// Java: public void readAdditionalSaveData(ValueInput p_409025_)
void LIBMATTI_MC_FoodData_ReadAdditionalSaveData(LIBMATTI_MC_FoodData *foodData, LIBMATTI_MC_Nbt_CompoundTag *tag)
{
    if (foodData == NULL || tag == NULL)
        return;
    foodData->foodLevel = LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(tag, "foodLevel", 20);
    foodData->tickTimer = LIBMATTI_MC_Nbt_CompoundTag_GetIntOr(tag, "foodTickTimer", 0);
    foodData->saturationLevel = LIBMATTI_MC_Nbt_CompoundTag_GetFloatOr(tag, "foodSaturationLevel", 5.0f);
    foodData->exhaustionLevel = LIBMATTI_MC_Nbt_CompoundTag_GetFloatOr(tag, "foodExhaustionLevel", 0.0f);
}
