// Port of net.minecraft.world.level.block.entity.BlockEntity.

#include "libmatti/net/minecraft/world/level/block/entity/BlockEntity.h"

#include "libmatti/net/minecraft/core/registries/BuiltInRegistries.h"
#include "libmatti/net/minecraft/resources/Identifier.h"
#include "libmatti/net/minecraft/resources/ResourceKey.h"
#include "libmatti/net/minecraft/world/level/block/entity/BlockEntityType.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_MC_BlockEntity *LIBMATTI_MC_BlockEntity_New(const LIBMATTI_MC_BlockEntityType *type, const LIBMATTI_MC_BlockPos *pos, LIBMATTI_MC_BlockState *state)
{
    LIBMATTI_MC_BlockEntity *blockEntity = calloc(1, sizeof(LIBMATTI_MC_BlockEntity));
    blockEntity->type = type;
    blockEntity->worldPosition = *pos;
    blockEntity->blockState = state;
    return blockEntity;
}

const LIBMATTI_MC_BlockPos *LIBMATTI_MC_BlockEntity_GetBlockPos(const LIBMATTI_MC_BlockEntity *blockEntity)
{
    return &blockEntity->worldPosition;
}

LIBMATTI_MC_BlockState *LIBMATTI_MC_BlockEntity_GetBlockState(const LIBMATTI_MC_BlockEntity *blockEntity)
{
    return blockEntity->blockState;
}

const LIBMATTI_MC_BlockEntityType *LIBMATTI_MC_BlockEntity_GetType(const LIBMATTI_MC_BlockEntity *blockEntity)
{
    return blockEntity->type;
}

bool LIBMATTI_MC_BlockEntity_IsValidBlockState(const LIBMATTI_MC_BlockEntity *blockEntity, LIBMATTI_MC_BlockState *state)
{
    return LIBMATTI_MC_BlockEntityType_IsValid(blockEntity->type, state);
}

struct LIBMATTI_MC_Level *LIBMATTI_MC_BlockEntity_GetLevel(const LIBMATTI_MC_BlockEntity *blockEntity)
{
    return blockEntity->level;
}

void LIBMATTI_MC_BlockEntity_SetLevel(LIBMATTI_MC_BlockEntity *blockEntity, struct LIBMATTI_MC_Level *level)
{
    blockEntity->level = level;
}

bool LIBMATTI_MC_BlockEntity_HasLevel(const LIBMATTI_MC_BlockEntity *blockEntity)
{
    return blockEntity->level != NULL;
}

bool LIBMATTI_MC_BlockEntity_IsRemoved(const LIBMATTI_MC_BlockEntity *blockEntity)
{
    return blockEntity->remove;
}

void LIBMATTI_MC_BlockEntity_SetRemoved(LIBMATTI_MC_BlockEntity *blockEntity)
{
    blockEntity->remove = true;
}

void LIBMATTI_MC_BlockEntity_ClearRemoved(LIBMATTI_MC_BlockEntity *blockEntity)
{
    blockEntity->remove = false;
}

void LIBMATTI_MC_BlockEntity_SetBlockState(LIBMATTI_MC_BlockEntity *blockEntity, LIBMATTI_MC_BlockState *state)
{
    blockEntity->blockState = state;
}

// ---------------------------------------------------------------------------
// Java: the save/load surface
// ---------------------------------------------------------------------------

// The virtual saveAdditional/loadAdditional hooks: the base implementation is
// a no-op (Java's abstract methods become overridable no-ops until the game
// content subclasses arrive); the BlockEntityType can install the hooks.
__attribute__((weak)) void LIBMATTI_MC_BlockEntity_SaveAdditional(LIBMATTI_MC_BlockEntity *blockEntity, LIBMATTI_MC_Nbt_CompoundTag *tag)
{
    (void) blockEntity;
    (void) tag;
}

__attribute__((weak)) void LIBMATTI_MC_BlockEntity_LoadAdditional(LIBMATTI_MC_BlockEntity *blockEntity, LIBMATTI_MC_Nbt_CompoundTag *tag)
{
    (void) blockEntity;
    (void) tag;
}

// Java: private void saveMetadata(CompoundTag tag) - the type id and the
// keepPacked flag ride every non-stripped save.
void LIBMATTI_MC_BlockEntity_SaveMetadata(LIBMATTI_MC_BlockEntity *blockEntity, LIBMATTI_MC_Nbt_CompoundTag *tag)
{
    const char *id = LIBMATTI_MC_BlockEntityType_GetId(blockEntity->type);
    if (id != NULL)
        LIBMATTI_MC_Nbt_CompoundTag_PutString(tag, "id", id);
}

// Java: public final CompoundTag saveWithFullMetadata() - the anvil chunk
// format: metadata + the position long + the custom data.
LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_BlockEntity_SaveWithFullMetadata(LIBMATTI_MC_BlockEntity *blockEntity)
{
    LIBMATTI_MC_Nbt_CompoundTag *tag = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_BlockEntity_SaveMetadata(blockEntity, tag);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(tag, "x", blockEntity->worldPosition.base.x);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(tag, "y", blockEntity->worldPosition.base.y);
    LIBMATTI_MC_Nbt_CompoundTag_PutInt(tag, "z", blockEntity->worldPosition.base.z);
    LIBMATTI_MC_BlockEntity_SaveAdditional(blockEntity, tag);
    return tag;
}

// Java: public final CompoundTag saveWithId() - metadata + custom data, no
// position (the item-stack block-entity form).
LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_BlockEntity_SaveWithId(LIBMATTI_MC_BlockEntity *blockEntity)
{
    LIBMATTI_MC_Nbt_CompoundTag *tag = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_BlockEntity_SaveMetadata(blockEntity, tag);
    LIBMATTI_MC_BlockEntity_SaveAdditional(blockEntity, tag);
    return tag;
}

// Java: public CompoundTag saveCustomOnly() - only the custom data.
LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_BlockEntity_SaveCustomOnly(LIBMATTI_MC_BlockEntity *blockEntity)
{
    LIBMATTI_MC_Nbt_CompoundTag *tag = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_BlockEntity_SaveAdditional(blockEntity, tag);
    return tag;
}

// Java: public CompoundTag saveWithoutMetadata() - custom data without id.
LIBMATTI_MC_Nbt_CompoundTag *LIBMATTI_MC_BlockEntity_SaveWithoutMetadata(LIBMATTI_MC_BlockEntity *blockEntity)
{
    return LIBMATTI_MC_BlockEntity_SaveCustomOnly(blockEntity);
}

// Java: public final void loadWithComponents(CompoundTag tag) - the load entry
// (the components overlay is data-component content; the port loads directly).
void LIBMATTI_MC_BlockEntity_LoadWithComponents(LIBMATTI_MC_BlockEntity *blockEntity, LIBMATTI_MC_Nbt_CompoundTag *tag)
{
    LIBMATTI_MC_BlockEntity_LoadAdditional(blockEntity, tag);
}

// Java: public static BlockEntity loadStatic(Level, BlockPos, BlockState, CompoundTag)
// - the id names the type, the factory builds the instance, the data loads in.
LIBMATTI_MC_BlockEntity *LIBMATTI_MC_BlockEntity_LoadStatic(struct LIBMATTI_MC_Level *level,
                                                             const LIBMATTI_MC_BlockPos *pos,
                                                             LIBMATTI_MC_BlockState *state,
                                                             LIBMATTI_MC_Nbt_CompoundTag *tag)
{
    const char *id = NULL;
    if (!LIBMATTI_MC_Nbt_CompoundTag_GetString(tag, "id", &id) || id == NULL)
        return NULL; // Java: LOGGER.warn("Skipping BlockEntity with invalid id: {}", string)
    // resolve the type through the registered types (the by-id scan over the
    // vanilla constants; the registry becomes the lookup with the P5 tick port)
    const LIBMATTI_MC_BlockEntityType *type = NULL;
    for (int i = 0; i < 49; i++)
    {
        const LIBMATTI_MC_BlockEntityType *candidate = LIBMATTI_MC_BlockEntityType_AccessIndex(i);
        if (candidate != NULL && LIBMATTI_MC_BlockEntityType_GetId(candidate) != NULL
            && strcmp(LIBMATTI_MC_BlockEntityType_GetId(candidate), id) == 0)
        {
            type = candidate;
            break;
        }
    }
    if (type == NULL)
        return NULL;
    LIBMATTI_MC_BlockEntity *blockEntity = LIBMATTI_MC_BlockEntityType_Create(type, pos, state);
    if (blockEntity == NULL)
        return NULL;
    blockEntity->level = level;
    LIBMATTI_MC_BlockEntity_LoadWithComponents(blockEntity, tag);
    return blockEntity;
}
