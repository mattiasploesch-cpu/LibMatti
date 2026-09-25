// Port of net.minecraft.client.KeyMapping (the key-bind registry). Java keeps
// two statics - ALL (name -> mapping) and MAP (key -> mappings) - the port
// mirrors them with flat arrays; the key is the InputConstants.Key value the
// GLFW port hands over (KEYSYM codes, MOUSE_BUTTON codes, UNKNOWN = -1).
// ToggleKeyMapping (sneak/sprint/use/attack) rides the isToggle flag + the
// toggle-provider pointer the Options port fills.

#ifndef MATTICRAFT_NET_MINECRAFT_CLIENT_KEYMAPPING_H
#define MATTICRAFT_NET_MINECRAFT_CLIENT_KEYMAPPING_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Java: InputConstants.Key - the port keeps the numeric value (the GLFW code)
// plus the type discriminator the matches()/save path needs
typedef enum LIBMATTI_MC_InputConstants_Type
{
    LIBMATTI_MC_InputConstants_KEYSYM,
    LIBMATTI_MC_InputConstants_SCANCODE,
    LIBMATTI_MC_InputConstants_MOUSE,
} LIBMATTI_MC_InputConstants_Type;

// Java: InputConstants.UNKNOWN - the unbound key
#define LIBMATTI_MC_InputConstants_UNKNOWN (-1)

// Java: KeyMapping.Category - the registered category instances (SORT_ORDER)
typedef enum LIBMATTI_MC_KeyMappingCategory
{
    LIBMATTI_MC_KeyMappingCategory_MOVEMENT = 0,
    LIBMATTI_MC_KeyMappingCategory_MISC,
    LIBMATTI_MC_KeyMappingCategory_MULTIPLAYER,
    LIBMATTI_MC_KeyMappingCategory_GAMEPLAY,
    LIBMATTI_MC_KeyMappingCategory_INVENTORY,
    LIBMATTI_MC_KeyMappingCategory_CREATIVE,
    LIBMATTI_MC_KeyMappingCategory_SPECTATOR,
    LIBMATTI_MC_KeyMappingCategory_DEBUG,
    LIBMATTI_MC_KeyMappingCategory_COUNT,
} LIBMATTI_MC_KeyMappingCategory;

// Java: public class KeyMapping implements Comparable<KeyMapping>
typedef struct LIBMATTI_MC_KeyMapping
{
    // Java: private final String name (the translation key, "key.forward")
    char *name;
    // Java: private final InputConstants.Key defaultKey / protected Key key
    int defaultKey;
    int key;
    LIBMATTI_MC_InputConstants_Type keyType;
    // Java: private final KeyMapping.Category category
    LIBMATTI_MC_KeyMappingCategory category;
    // Java: private final int order
    int order;
    // Java: private boolean isDown / private int clickCount
    bool isDown;
    int clickCount;
    // Java: ToggleKeyMapping - the flag + the needsToggle provider (the toggle
    // option, default off; NULL = plain mapping). Java: setDown flips the state
    // only when needsToggle() is true, isDown() stays the plain field.
    bool isToggle;
    bool (*needsToggle)(void);
} LIBMATTI_MC_KeyMapping;

// Java: public KeyMapping(String, int, Category) - KEYSYM default
LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_New(const char *name, int key, LIBMATTI_MC_KeyMappingCategory category);
// Java: public KeyMapping(String, InputConstants.Type, int, Category)
LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_NewTyped(LIBMATTI_MC_InputConstants_Type type, const char *name, int key,
                                                        LIBMATTI_MC_KeyMappingCategory category);
// Java: public KeyMapping(String, Type, int, Category, int order)
LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_NewOrdered(LIBMATTI_MC_InputConstants_Type type, const char *name, int key,
                                                          LIBMATTI_MC_KeyMappingCategory category, int order);
// Java: ToggleKeyMapping(name, key, category, toggleGetter, needsToggle) - the
// port folds the needsToggle flag away (the provider answers isDown)
LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_NewToggle(const char *name, int key, LIBMATTI_MC_KeyMappingCategory category,
                                                         bool (*needsToggle)(void));
void LIBMATTI_MC_KeyMapping_Free(LIBMATTI_MC_KeyMapping *mapping);

// Java: private void registerMapping(Key) / public static void resetMapping() -
// the port rebuilds the MAP table from the ALL list
void LIBMATTI_MC_KeyMapping_ResetMapping(void);

// Java: public static void click(Key) - every mapping on that key clicks
void LIBMATTI_MC_KeyMapping_Click(int keyType, int keyValue);
// Java: public static void set(Key, boolean) - every mapping on that key sets
void LIBMATTI_MC_KeyMapping_Set(int keyType, int keyValue, bool down);
// Java: public static void releaseAll() - clickCount 0 + isDown false
void LIBMATTI_MC_KeyMapping_ReleaseAll(void);

// Java: public boolean isDown() - the toggle provider answers first
bool LIBMATTI_MC_KeyMapping_IsDown(const LIBMATTI_MC_KeyMapping *mapping);
// Java: public boolean consumeClick() - the clickCount take
bool LIBMATTI_MC_KeyMapping_ConsumeClick(LIBMATTI_MC_KeyMapping *mapping);
// Java: protected void release()
void LIBMATTI_MC_KeyMapping_Release(LIBMATTI_MC_KeyMapping *mapping);
// Java: public void setDown(boolean)
void LIBMATTI_MC_KeyMapping_SetDown(LIBMATTI_MC_KeyMapping *mapping, bool down);
// Java: public String getName() / public Key getDefaultKey()
const char *LIBMATTI_MC_KeyMapping_GetName(const LIBMATTI_MC_KeyMapping *mapping);
int LIBMATTI_MC_KeyMapping_GetDefaultKey(const LIBMATTI_MC_KeyMapping *mapping);
LIBMATTI_MC_KeyMappingCategory LIBMATTI_MC_KeyMapping_GetCategory(const LIBMATTI_MC_KeyMapping *mapping);
// Java: public void setKey(Key) - re-registers the mapping table
void LIBMATTI_MC_KeyMapping_SetKey(LIBMATTI_MC_KeyMapping *mapping, LIBMATTI_MC_InputConstants_Type type, int key);
// Java: public boolean isUnbound() / isDefault() / same(KeyMapping)
bool LIBMATTI_MC_KeyMapping_IsUnbound(const LIBMATTI_MC_KeyMapping *mapping);
bool LIBMATTI_MC_KeyMapping_IsDefault(const LIBMATTI_MC_KeyMapping *mapping);
bool LIBMATTI_MC_KeyMapping_Same(const LIBMATTI_MC_KeyMapping *a, const LIBMATTI_MC_KeyMapping *b);
// Java: public boolean matches(KeyEvent) / matchesMouse(MouseButtonEvent) - the
// port takes the (type, value) pair the GLFW callbacks produce
bool LIBMATTI_MC_KeyMapping_Matches(const LIBMATTI_MC_KeyMapping *mapping, LIBMATTI_MC_InputConstants_Type type, int value);
// Java: public String saveString() - the key name ("key.keyboard.w"/"mouse.left")
const char *LIBMATTI_MC_KeyMapping_SaveString(const LIBMATTI_MC_KeyMapping *mapping, char *out, size_t outSize);

// Java: public static KeyMapping get(String) - the ALL lookup
LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Get(const char *name);

// The port's Options surface: the vanilla movement/inventory mappings the
// KeyboardInput reads (Java: the Options field initializers). NULL mappings
// mean the statics were not created yet.
void LIBMATTI_MC_KeyMapping_CreateVanillaMappings(void);
LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Forward(void);
LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Back(void);
LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Left(void);
LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Right(void);
LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Jump(void);
LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Shift(void);
LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Sprint(void);
LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Inventory(void);
LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Drop(void);
LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Attack(void);
LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Use(void);
LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_PickItem(void);
LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Chat(void);
LIBMATTI_MC_KeyMapping *LIBMATTI_MC_KeyMapping_Hotbar(int slot);

#ifdef __cplusplus
}
#endif

#endif //MATTICRAFT_NET_MINECRAFT_CLIENT_KEYMAPPING_H
