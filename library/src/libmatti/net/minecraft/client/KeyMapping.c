// Port of net.minecraft.client.KeyMapping (implementation). The statics ALL
// (name -> mapping) and MAP (key value -> mappings) become flat arrays over a
// fixed registry: the game creates ~30 mappings, the port caps the table at a
// bound the vanilla set stays well under.

#include "libmatti/net/minecraft/client/KeyMapping.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final Map<String, KeyMapping> ALL - the port keeps one
// growable array (the MAP rebuild rides on it)
#define KEYMAPPING_MAX 64

static LIBMATTI_MC_KeyMapping *allMappings[KEYMAPPING_MAX];
static int allMappingCount = 0;

// Java: the toggle providers the Options port fills (Java: the toggleCrouch/
// toggleSprint/toggleUse/toggleAttack OptionInstances; the skeleton defaults
// them to false)
static LIBMATTI_MC_KeyMapping *mapping_new(const char *name, LIBMATTI_MC_InputConstants_Type type, int key,
                                           LIBMATTI_MC_KeyMappingCategory category, int order);

static bool vanilla_toggle_false(void)
{
    return false;
}

// Java: ToggleKeyMapping(String, InputConstants.Type, int, Category, provider) -
// the typed toggle constructor the mouse-bound use/attack ride
static LIBMATTI_MC_KeyMapping *mapping_new_toggle_typed(LIBMATTI_MC_InputConstants_Type type, const char *name, int key,
                                                        LIBMATTI_MC_KeyMappingCategory category,
                                                        bool (*shouldIsDown)(void))
{
    LIBMATTI_MC_KeyMapping *mapping = mapping_new(name, type, key, category, 0);
    if (mapping == NULL)
        return NULL;
    mapping->isToggle = true;
    mapping->shouldIsDown = shouldIsDown != NULL ? shouldIsDown : vanilla_toggle_false;
    return mapping;
}

// Java: InputConstants.Key - the port keys the MAP by (type << 24 | value);
// the UNKNOWN key (-1) hashes to its own bucket through the same mix
static int map_slot(LIBMATTI_MC_InputConstants_Type type, int key)
{
    int mixed = ((int) type << 24) | (key & 0x00FFFFFF);
    return mixed % KEYMAPPING_MAX;
}

// Java: private static final Map<Key, List<KeyMapping>> MAP - one bucket per
// mixed key, each holding the mappings bound to it
typedef struct KeyMappingBucket
{
    LIBMATTI_MC_KeyMapping *mappings[8];
    int count;
} KeyMappingBucket;

static KeyMappingBucket mapBuckets[KEYMAPPING_MAX];

// Java: private void registerMapping(Key) - append to the bucket
static void register_mapping(LIBMATTI_MC_KeyMapping *mapping)
{
    KeyMappingBucket *bucket = &mapBuckets[map_slot(mapping->keyType, mapping->key)];
    if (bucket->count < (int) (sizeof(bucket->mappings) / sizeof(bucket->mappings[0])))
        bucket->mappings[bucket->count++] = mapping;
}

// Java: public static void resetMapping() - clear the MAP, re-register all
void LIBMATTI_MC_KeyMapping_ResetMapping(void)
{
    memset(mapBuckets, 0, sizeof(mapBuckets));
    for (int i = 0; i < allMappingCount; i++)
        register_mapping(allMappings[i]);
}

static LIBMATTI_MC_KeyMapping *mapping_new(const char *name, LIBMATTI_MC_InputConstants_Type type, int key,
                                           LIBMATTI_MC_KeyMappingCategory category, int order)
{
    if (name == NULL || allMappingCount >= KEYMAPPING_MAX)
        return NULL;
    LIBMATTI_MC_KeyMapping *mapping = calloc(1, sizeof(LIBMATTI_MC_KeyMapping));
    if (mapping == NULL)
        return NULL;
    mapping->name = strdup(name);
    if (mapping->name == NULL)
    {
        free(mapping);
        return NULL;
    }
    // Java: this.key = p_460964_.getOrCreate(p_457112_); this.defaultKey = this.key
    mapping->key = key;
    mapping->defaultKey = key;
    mapping->keyType = type;
    mapping->category = category;
    mapping->order = order;
    // Java: ALL.put(p_455154_, this); this.registerMapping(this.key)
    allMappings[allMappingCount++] = mapping;
    register_mapping(mapping);
    return mapping;
}

LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_New(const char *name, int key, LIBMATTI_MC_KeyMappingCategory category)
{
    return mapping_new(name, LIBMATTI_MC_InputConstants_KEYSYM, key, category, 0);
}

LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_NewTyped(LIBMATTI_MC_InputConstants_Type type, const char *name, int key,
                                                        LIBMATTI_MC_KeyMappingCategory category)
{
    return mapping_new(name, type, key, category, 0);
}

LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_NewOrdered(LIBMATTI_MC_InputConstants_Type type, const char *name, int key,
                                                          LIBMATTI_MC_KeyMappingCategory category, int order)
{
    return mapping_new(name, type, key, category, order);
}

LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_NewToggle(const char *name, int key, LIBMATTI_MC_KeyMappingCategory category,
                                                         bool (*shouldIsDown)(void))
{
    LIBMATTI_MC_KeyMapping *mapping = mapping_new(name, LIBMATTI_MC_InputConstants_KEYSYM, key, category, 0);
    if (mapping == NULL)
        return NULL;
    // Java: ToggleKeyMapping stores the provider; needsToggle defaults to true
    mapping->isToggle = true;
    mapping->shouldIsDown = shouldIsDown != NULL ? shouldIsDown : vanilla_toggle_false;
    return mapping;
}

void LIBMATTI_MC_KeyMapping_Free(LIBMATTI_MC_KeyMapping *mapping)
{
    if (mapping == NULL)
        return;
    // drop the statics' references (the port frees only through explicit calls)
    for (int i = 0; i < allMappingCount; i++)
    {
        if (allMappings[i] == mapping)
        {
            allMappings[i] = NULL;
            allMappingCount--;
            for (int j = i; j < allMappingCount; j++)
                allMappings[j] = allMappings[j + 1];
            break;
        }
    }
    LIBMATTI_MC_KeyMapping_ResetMapping();
    free(mapping->name);
    free(mapping);
}

// Java: private static void forAllKeyMappings(Key, Consumer) - the bucket walk
static void for_all_key_mappings(LIBMATTI_MC_InputConstants_Type type, int key,
                                 void (*consumer)(LIBMATTI_MC_KeyMapping *, void *), void *userData)
{
    KeyMappingBucket *bucket = &mapBuckets[map_slot(type, key)];
    for (int i = 0; i < bucket->count; i++)
        consumer(bucket->mappings[i], userData);
}

static void click_consumer(LIBMATTI_MC_KeyMapping *mapping, void *userData)
{
    (void) userData;
    // Java: p_420622_ -> p_420622_.clickCount++
    mapping->clickCount++;
}

static void set_consumer(LIBMATTI_MC_KeyMapping *mapping, void *userData)
{
    // Java: p_420621_ -> p_420621_.setDown(p_90839_)
    LIBMATTI_MC_KeyMapping_SetDown(mapping, userData != NULL);
}

// Java: public static void click(InputConstants.Key)
void LIBMATTI_MC_KeyMapping_Click(int keyType, int keyValue)
{
    for_all_key_mappings((LIBMATTI_MC_InputConstants_Type) keyType, keyValue, click_consumer, NULL);
}

// Java: public static void set(InputConstants.Key, boolean)
void LIBMATTI_MC_KeyMapping_Set(int keyType, int keyValue, bool down)
{
    for_all_key_mappings((LIBMATTI_MC_InputConstants_Type) keyType, keyValue, set_consumer, down ? (void *) 1 : NULL);
}

// Java: public static void releaseAll()
void LIBMATTI_MC_KeyMapping_ReleaseAll(void)
{
    for (int i = 0; i < allMappingCount; i++)
        LIBMATTI_MC_KeyMapping_Release(allMappings[i]);
}

// Java: public boolean isDown() - ToggleKeyMapping overrides it with the provider
bool LIBMATTI_MC_KeyMapping_IsDown(const LIBMATTI_MC_KeyMapping *mapping)
{
    if (mapping == NULL)
        return false;
    if (mapping->isToggle && mapping->shouldIsDown != NULL)
        return mapping->shouldIsDown() && mapping->isDown;
    return mapping->isDown;
}

// Java: public boolean consumeClick()
bool LIBMATTI_MC_KeyMapping_ConsumeClick(LIBMATTI_MC_KeyMapping *mapping)
{
    if (mapping == NULL || mapping->clickCount == 0)
        return false;
    mapping->clickCount--;
    return true;
}

// Java: protected void release()
void LIBMATTI_MC_KeyMapping_Release(LIBMATTI_MC_KeyMapping *mapping)
{
    if (mapping == NULL)
        return;
    mapping->clickCount = 0;
    LIBMATTI_MC_KeyMapping_SetDown(mapping, false);
}

// Java: public void setDown(boolean p_90846_)
void LIBMATTI_MC_KeyMapping_SetDown(LIBMATTI_MC_KeyMapping *mapping, bool down)
{
    if (mapping != NULL)
        mapping->isDown = down;
}

const char *LIBMATTI_MC_KeyMapping_GetName(const LIBMATTI_MC_KeyMapping *mapping)
{
    return mapping != NULL ? mapping->name : NULL;
}

int LIBMATTI_MC_KeyMapping_GetDefaultKey(const LIBMATTI_MC_KeyMapping *mapping)
{
    return mapping != NULL ? mapping->defaultKey : LIBMATTI_MC_InputConstants_UNKNOWN;
}

LIBMATTI_MC_KeyMappingCategory LIBMATTI_MC_KeyMapping_GetCategory(const LIBMATTI_MC_KeyMapping *mapping)
{
    return mapping != NULL ? mapping->category : LIBMATTI_MC_KeyMappingCategory_MISC;
}

// Java: public void setKey(Key p_90849_) - the port re-registers the table
void LIBMATTI_MC_KeyMapping_SetKey(LIBMATTI_MC_KeyMapping *mapping, LIBMATTI_MC_InputConstants_Type type, int key)
{
    if (mapping == NULL)
        return;
    mapping->key = key;
    mapping->keyType = type;
    LIBMATTI_MC_KeyMapping_ResetMapping();
}

// Java: public boolean isUnbound()
bool LIBMATTI_MC_KeyMapping_IsUnbound(const LIBMATTI_MC_KeyMapping *mapping)
{
    return mapping != NULL && mapping->key == LIBMATTI_MC_InputConstants_UNKNOWN;
}

// Java: public boolean isDefault()
bool LIBMATTI_MC_KeyMapping_IsDefault(const LIBMATTI_MC_KeyMapping *mapping)
{
    return mapping != NULL && mapping->key == mapping->defaultKey && mapping->keyType == mapping->keyType;
}

// Java: public boolean same(KeyMapping) - the key equality
bool LIBMATTI_MC_KeyMapping_Same(const LIBMATTI_MC_KeyMapping *a, const LIBMATTI_MC_KeyMapping *b)
{
    return a != NULL && b != NULL && a->key == b->key && a->keyType == b->keyType;
}

// Java: public boolean matches(KeyEvent) / matchesMouse(MouseButtonEvent) - the
// port folds both into one (type, value) pair (an UNKNOWN value checks the
// scancode type like Java's matches())
bool LIBMATTI_MC_KeyMapping_Matches(const LIBMATTI_MC_KeyMapping *mapping, LIBMATTI_MC_InputConstants_Type type, int value)
{
    if (mapping == NULL)
        return false;
    if (value == LIBMATTI_MC_InputConstants_UNKNOWN)
        return mapping->keyType == LIBMATTI_MC_InputConstants_SCANCODE && mapping->key == LIBMATTI_MC_InputConstants_UNKNOWN;
    return mapping->keyType == type && mapping->key == value;
}

// Java: public String saveString() - key.getName() ("key.keyboard.w"/"mouse.left")
const char *LIBMATTI_MC_KeyMapping_SaveString(const LIBMATTI_MC_KeyMapping *mapping, char *out, size_t outSize)
{
    if (out == NULL || outSize == 0)
        return NULL;
    out[0] = '\0';
    if (mapping == NULL)
        return out;
    if (mapping->key == LIBMATTI_MC_InputConstants_UNKNOWN)
    {
        // Java: InputConstants.UNKNOWN -> "key.keyboard.unknown" / the port's
        // shared name
        snprintf(out, outSize, "key.keyboard.unknown");
        return out;
    }
    if (mapping->keyType == LIBMATTI_MC_InputConstants_MOUSE)
        snprintf(out, outSize, "mouse.%d", mapping->key);
    else
        snprintf(out, outSize, "key.%d", mapping->key);
    return out;
}

// Java: public static KeyMapping get(String) - the ALL lookup
LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Get(const char *name)
{
    if (name == NULL)
        return NULL;
    for (int i = 0; i < allMappingCount; i++)
    {
        if (allMappings[i] != NULL && strcmp(allMappings[i]->name, name) == 0)
            return allMappings[i];
    }
    return NULL;
}

// ---------------------------------------------------------------------------
// Java: the Options field initializers - the vanilla set the KeyboardInput and
// the game windows read (the port keeps the subset the skeleton drives)
// ---------------------------------------------------------------------------

static LIBMATTI_MC_KeyMapping *vanillaForward;
static LIBMATTI_MC_KeyMapping *vanillaBack;
static LIBMATTI_MC_KeyMapping *vanillaLeft;
static LIBMATTI_MC_KeyMapping *vanillaRight;
static LIBMATTI_MC_KeyMapping *vanillaJump;
static LIBMATTI_MC_KeyMapping *vanillaShift;
static LIBMATTI_MC_KeyMapping *vanillaSprint;
static LIBMATTI_MC_KeyMapping *vanillaInventory;
static LIBMATTI_MC_KeyMapping *vanillaDrop;
static LIBMATTI_MC_KeyMapping *vanillaAttack;
static LIBMATTI_MC_KeyMapping *vanillaUse;
static LIBMATTI_MC_KeyMapping *vanillaPickItem;
static LIBMATTI_MC_KeyMapping *vanillaChat;
static LIBMATTI_MC_KeyMapping *vanillaHotbar[9];

void LIBMATTI_MC_KeyMapping_CreateVanillaMappings(void)
{
    if (vanillaForward != NULL)
        return; // Java: the fields are final - created once
    // Java: Options.java lines 563..567 (the GLFW codes ride the port constants)
    vanillaForward = LIBMATTI_MC_KeyMapping_New("key.forward", 87, LIBMATTI_MC_KeyMappingCategory_MOVEMENT);
    vanillaLeft = LIBMATTI_MC_KeyMapping_New("key.left", 65, LIBMATTI_MC_KeyMappingCategory_MOVEMENT);
    vanillaBack = LIBMATTI_MC_KeyMapping_New("key.back", 83, LIBMATTI_MC_KeyMappingCategory_MOVEMENT);
    vanillaRight = LIBMATTI_MC_KeyMapping_New("key.right", 68, LIBMATTI_MC_KeyMappingCategory_MOVEMENT);
    vanillaJump = LIBMATTI_MC_KeyMapping_New("key.jump", 32, LIBMATTI_MC_KeyMappingCategory_MOVEMENT);
    // Java: ToggleKeyMapping("key.sneak", 340, MOVEMENT, toggleCrouch, true)
    vanillaShift = mapping_new_toggle_typed(LIBMATTI_MC_InputConstants_KEYSYM, "key.sneak", 340,
                                            LIBMATTI_MC_KeyMappingCategory_MOVEMENT, NULL);
    // Java: ToggleKeyMapping("key.sprint", 341, MOVEMENT, toggleSprint, true)
    vanillaSprint = mapping_new_toggle_typed(LIBMATTI_MC_InputConstants_KEYSYM, "key.sprint", 341,
                                             LIBMATTI_MC_KeyMappingCategory_MOVEMENT, NULL);
    vanillaInventory = LIBMATTI_MC_KeyMapping_New("key.inventory", 69, LIBMATTI_MC_KeyMappingCategory_INVENTORY);
    vanillaDrop = LIBMATTI_MC_KeyMapping_New("key.drop", 81, LIBMATTI_MC_KeyMappingCategory_INVENTORY);
    // Java: keyAttack = ToggleKeyMapping("key.attack", MOUSE 0, GAMEPLAY, ...)
    vanillaAttack = mapping_new_toggle_typed(LIBMATTI_MC_InputConstants_MOUSE, "key.attack", 0,
                                             LIBMATTI_MC_KeyMappingCategory_GAMEPLAY, NULL);
    // Java: keyUse = ToggleKeyMapping("key.use", MOUSE 1, GAMEPLAY, ...)
    vanillaUse = mapping_new_toggle_typed(LIBMATTI_MC_InputConstants_MOUSE, "key.use", 1,
                                          LIBMATTI_MC_KeyMappingCategory_GAMEPLAY, NULL);
    vanillaPickItem = LIBMATTI_MC_KeyMapping_NewTyped(LIBMATTI_MC_InputConstants_MOUSE, "key.pickItem", 2,
                                                      LIBMATTI_MC_KeyMappingCategory_GAMEPLAY);
    vanillaChat = LIBMATTI_MC_KeyMapping_New("key.chat", 84, LIBMATTI_MC_KeyMappingCategory_MULTIPLAYER);
    // Java: keyHotbarSlots - "key.hotbar.1".."key.hotbar.9" on the digits 49..57
    for (int slot = 0; slot < 9; slot++)
    {
        char name[16];
        snprintf(name, sizeof(name), "key.hotbar.%d", slot + 1);
        vanillaHotbar[slot] = LIBMATTI_MC_KeyMapping_New(name, 49 + slot, LIBMATTI_MC_KeyMappingCategory_INVENTORY);
    }
}

LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Forward(void)
{
    return vanillaForward;
}

LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Back(void)
{
    return vanillaBack;
}

LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Left(void)
{
    return vanillaLeft;
}

LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Right(void)
{
    return vanillaRight;
}

LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Jump(void)
{
    return vanillaJump;
}

LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Shift(void)
{
    return vanillaShift;
}

LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Sprint(void)
{
    return vanillaSprint;
}

LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Inventory(void)
{
    return vanillaInventory;
}

LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Drop(void)
{
    return vanillaDrop;
}

LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Attack(void)
{
    return vanillaAttack;
}

LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Use(void)
{
    return vanillaUse;
}

LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_PickItem(void)
{
    return vanillaPickItem;
}

LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Chat(void)
{
    return vanillaChat;
}

LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Hotbar(int slot)
{
    if (slot < 0 || slot >= 9)
        return NULL;
    return vanillaHotbar[slot];
}
