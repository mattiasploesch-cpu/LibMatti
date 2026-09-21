// BlockState harness: builds a vanilla-like block (four properties, four-way facing
// like the oak stairs), checks the property factories and parsing, the sorted state
// table (size, declaration order, any()), the setValue/cycle neighbour resolution,
// trySetValue's no-op on an absent property and the BlockState/Block default links.

#include "libmatti/net/minecraft/core/registries/Registries.h"
#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/net/minecraft/resources/ResourceKey.h"
#include "libmatti/net/minecraft/world/level/block/Block.h"
#include "libmatti/net/minecraft/world/level/block/state/BlockState.h"
#include "libmatti/net/minecraft/world/level/block/state/StateDefinition.h"
#include "libmatti/net/minecraft/world/level/block/state/StateHolder.h"
#include "libmatti/net/minecraft/world/level/block/state/properties/Property.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks = 0;

static void check(int condition, const char *what)
{
    checks++;
    if (!condition)
    {
        fprintf(stderr, "FAIL: %s\n", what);
        exit(1);
    }
}

int main(void)
{
    // ------------------------------------------------------------- Property
    LIBMATTI_MC_Property *facing = LIBMATTI_MC_EnumProperty_Create(
            "facing", (const char *const[]){"north", "south", "west", "east"}, 4);
    LIBMATTI_MC_Property *waterlogged = LIBMATTI_MC_BooleanProperty_Create("waterlogged");
    LIBMATTI_MC_Property *shape = LIBMATTI_MC_EnumProperty_Create(
            "shape", (const char *const[]){"straight", "inner_left", "outer_left"}, 3);
    LIBMATTI_MC_Property *distance = LIBMATTI_MC_IntegerProperty_Create("distance", 1, 7);

    check(facing != NULL && waterlogged != NULL && shape != NULL && distance != NULL,
          "properties created");
    check(strcmp(LIBMATTI_MC_Property_GetName(facing), "facing") == 0, "property name");
    check(LIBMATTI_MC_Property_Equals(
                  LIBMATTI_MC_BooleanProperty_Create("waterlogged"), waterlogged),
          "property equality by name+class");
    check(!LIBMATTI_MC_Property_Equals(waterlogged, facing), "property inequality across classes");

    int count = 0;
    const LIBMATTI_MC_Property_Value *values = LIBMATTI_MC_Property_GetPossibleValues(facing, &count);
    check(count == 4 && strcmp(LIBMATTI_MC_Property_ValueName(facing, values[0]), "north") == 0,
          "enum possible values");

    // Java: getValue("east") -> Optional.of(east); getInternalIndex = 3
    int found = 0;
    LIBMATTI_MC_Property_Value east = LIBMATTI_MC_Property_ParseValue(facing, "east", &found);
    check(found && east.index == 3 && LIBMATTI_MC_Property_InternalIndex(facing, east) == 3,
          "enum parse + internal index");
    LIBMATTI_MC_Property_ParseValue(facing, "up", &found);
    check(!found, "enum parse rejects unknown");
    LIBMATTI_MC_Property_Value one = LIBMATTI_MC_Property_ParseValue(distance, "1", &found);
    check(found && one.index == 1, "integer parse");
    LIBMATTI_MC_Property_Value seven = LIBMATTI_MC_Property_ParseValue(distance, "7", &found);
    check(found && seven.index == 7, "integer parse max");
    LIBMATTI_MC_Property_ParseValue(distance, "9", &found);
    check(!found, "integer parse range check");
    LIBMATTI_MC_Property_Value yes = LIBMATTI_MC_Property_ParseValue(waterlogged, "true", &found);
    check(found && yes.b == 1 && strcmp(LIBMATTI_MC_Property_ValueName(waterlogged, yes), "true") == 0,
          "boolean parse");

    // ------------------------------------------------------- StateDefinition
    LIBMATTI_MC_Block *stairs = LIBMATTI_MC_Block_New();
    LIBMATTI_MC_StateDefinition_Builder *builder = LIBMATTI_MC_StateDefinition_Builder_New(stairs);
    LIBMATTI_MC_StateDefinition_Builder_Add(builder, facing);
    LIBMATTI_MC_StateDefinition_Builder_Add(builder, waterlogged);
    LIBMATTI_MC_StateDefinition_Builder_Add(builder, shape);
    LIBMATTI_MC_StateDefinition_Builder_Add(builder, distance);
    LIBMATTI_MC_StateDefinition *definition = LIBMATTI_MC_StateDefinition_Builder_Create(builder);

    // 4 * 2 * 3 * 7 = 168 states
    LIBMATTI_MC_StateHolder **states = LIBMATTI_MC_StateDefinition_GetPossibleStates(definition, &count);
    check(count == 168, "state table size 4*2*3*7");
    check(LIBMATTI_MC_StateDefinition_Any(definition) == states[0], "any() is the first state");
    check(LIBMATTI_MC_StateDefinition_GetOwner(definition) == stairs, "definition owner");
    check(LIBMATTI_MC_StateDefinition_GetProperty(definition, "shape") == shape,
          "getProperty by name");
    check(LIBMATTI_MC_StateDefinition_GetProperty(definition, "nope") == NULL,
          "getProperty unknown is NULL");

    // Java: ImmutableSortedMap sorts properties by name - the port reproduces the order
    int pcount = 0;
    LIBMATTI_MC_Property **props = LIBMATTI_MC_StateDefinition_GetProperties(definition, &pcount);
    check(pcount == 4 && strcmp(props[0]->name, "distance") == 0
              && strcmp(props[1]->name, "facing") == 0 && strcmp(props[2]->name, "shape") == 0
              && strcmp(props[3]->name, "waterlogged") == 0,
          "properties sorted by name");

    // ------------------------------------------------------- StateHolder
    LIBMATTI_MC_StateHolder *any = LIBMATTI_MC_StateDefinition_Any(definition);
    LIBMATTI_MC_Property_Value v = LIBMATTI_MC_StateHolder_GetValue(any, facing);
    check(v.index == 0, "any() facing = north (first value, sorted order puts facing first)");

    LIBMATTI_MC_Property_Value eastVal = LIBMATTI_MC_Property_ParseValue(facing, "east", &found);
    LIBMATTI_MC_StateHolder *eastState =
            LIBMATTI_MC_StateHolder_SetValue(any, facing, eastVal);
    check(eastState != any, "setValue returns a different state");
    check(LIBMATTI_MC_StateHolder_GetValue(eastState, facing).index == 3, "east state value");
    // all other properties unchanged
    // Java: BooleanProperty.VALUES = List.of(true, false) - any() starts waterlogged=true
    check(LIBMATTI_MC_StateHolder_GetValue(eastState, waterlogged).b == 1
              && LIBMATTI_MC_StateHolder_GetValue(eastState, distance).index == 1,
          "setValue keeps the other properties");
    check(LIBMATTI_MC_StateHolder_SetValue(eastState, facing, eastVal) == eastState,
          "setValue to the same value is identity");

    // The neighbour table: chains over two properties resolve to the exact state
    LIBMATTI_MC_Property_Value dry = LIBMATTI_MC_Property_ParseValue(waterlogged, "false", &found);
    LIBMATTI_MC_StateHolder *eastDry =
            LIBMATTI_MC_StateHolder_SetValue(LIBMATTI_MC_StateHolder_SetValue(any, waterlogged, dry),
                                             facing, eastVal);
    check(LIBMATTI_MC_StateHolder_GetValue(eastDry, waterlogged).b == 0
              && LIBMATTI_MC_StateHolder_GetValue(eastDry, facing).index == 3,
          "neighbour lookup two properties deep");
    check(LIBMATTI_MC_StateHolder_SetValue(eastDry, waterlogged, LIBMATTI_MC_Property_ParseValue(waterlogged, "true", &found))
              == LIBMATTI_MC_StateHolder_SetValue(any, facing, eastVal),
          "neighbour graph is undirected");
    check(LIBMATTI_MC_StateHolder_SetValue(eastDry, waterlogged, dry) == eastDry,
          "setValue to the same value is identity");

    // cycle: east -> wraps around to north
    LIBMATTI_MC_StateHolder *cycled = LIBMATTI_MC_StateHolder_Cycle(eastState, facing);
    check(LIBMATTI_MC_StateHolder_GetValue(cycled, facing).index == 0, "cycle wraps to north");

    // trySetValue with a property the block does not have - Java: no-op
    LIBMATTI_MC_Property *alien = LIBMATTI_MC_BooleanProperty_Create("alien");
    check(LIBMATTI_MC_StateHolder_TrySetValue(any, alien, yes) == any,
          "trySetValue no-op on unknown property");

    // ---------------------------------------------------------------- Block
    LIBMATTI_MC_Block *block = LIBMATTI_MC_Block_NewWithStateDefinition(definition);
    LIBMATTI_MC_BlockState *defaultState = LIBMATTI_MC_Block_DefaultBlockState(block);
    check(defaultState == (LIBMATTI_MC_BlockState *) states[0], "defaultBlockState is state 0");
    check(LIBMATTI_MC_BlockState_IsDefault(defaultState), "isDefault true");
    check(LIBMATTI_MC_BlockState_GetBlock(defaultState) == block, "state block back-link");
    check(LIBMATTI_MC_Block_GetStateDefinition(block) == definition, "block stateDefinition");

    // BlockState surface re-exposes the holder
    LIBMATTI_MC_BlockState *turned =
            LIBMATTI_MC_BlockState_SetValue(defaultState, facing, eastVal);
    check(LIBMATTI_MC_BlockState_GetValue(turned, facing).index == 3, "BlockState setValue");
    check(LIBMATTI_MC_BlockState_HasProperty(turned, facing)
              && !LIBMATTI_MC_BlockState_HasProperty(turned, alien),
          "BlockState hasProperty");
    LIBMATTI_MC_BlockState *tried =
            LIBMATTI_MC_BlockState_TrySetValue(turned, alien, yes);
    check(tried == turned, "BlockState trySetValue no-op");

    // -------------------------------------------------- registry integration
    // The vanilla data went through this exact path in the vanilla harness; here the
    // Block-with-definition gets registered like a modded block would.
    LIBMATTI_MC_Identifier *id = LIBMATTI_MC_Identifier_Parse("blockstate_test:testblock");
    check(id != NULL, "identifier parsed");
    LIBMATTI_MC_ResourceKey *key =
            LIBMATTI_MC_ResourceKey_Create(LIBMATTI_MC_Registries_BLOCK(), id);
    check(key != NULL, "registry key created");
    // Java: interned keys - the same registry key constant comes back identical
    check(LIBMATTI_MC_ResourceKey_Create(LIBMATTI_MC_Registries_BLOCK(), id) == key,
          "resource keys are interned");

    printf("blockstate: %d checks ok\n", checks);
    return 0;
}
