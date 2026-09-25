// Entity harness: the P5.1 core - the EntityType registry (register/byString,
// dimensions), the Entity base (position/rotation/bounding box, tags, NBT
// round trip), the vanilla entity constants and the Level entity surface
// (add/remove/query through the AABB overlap).

#include "libmatti/net/minecraft/Bootstrap.h"
#include "libmatti/net/minecraft/core/BlockPos.h"
#include "libmatti/net/minecraft/nbt/CompoundTag.h"
#include "libmatti/net/minecraft/world/entity/Entity.h"
#include "libmatti/net/minecraft/world/entity/EntityAttachments.h"
#include "libmatti/net/minecraft/world/entity/EntityDimensions.h"
#include "libmatti/net/minecraft/world/entity/EntityType.h"
#include "libmatti/net/minecraft/world/entity/VanillaEntities.h"
#include "libmatti/net/minecraft/world/level/Level.h"

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
    // the registry bootstrap fills BLOCK/ITEM/ENTITY_TYPE and raises the guard
    LIBMATTI_MC_Bootstrap_BootStrap();

    // ---- the EntityType registry -------------------------------------------------
    const LIBMATTI_MC_EntityType *zombie = LIBMATTI_MC_EntityType_ZOMBIE();
    check(zombie != NULL, "zombie type registered");
    check(zombie != NULL && strcmp(LIBMATTI_MC_EntityType_GetId(zombie), "minecraft:zombie") == 0, "zombie id round trip");
    check(zombie != NULL && LIBMATTI_MC_EntityType_ByString("minecraft:zombie") == zombie, "byString resolves the same object");
    check(LIBMATTI_MC_EntityType_ByString("minecraft:not_a_thing") == NULL, "byString misses unknown id");
    check(zombie != NULL && LIBMATTI_MC_EntityType_GetCategory(zombie) == LIBMATTI_MC_MobCategory_MONSTER, "zombie is a monster");
    check(zombie != NULL && fabs(LIBMATTI_MC_EntityType_GetWidth(zombie) - 0.6f) < 1e-5, "zombie width 0.6");
    check(zombie != NULL && fabs(LIBMATTI_MC_EntityType_GetHeight(zombie) - 1.95f) < 1e-5, "zombie height 1.95");
    check(zombie != NULL && LIBMATTI_MC_EntityType_GetDescriptionId(zombie) != NULL
              && strcmp(LIBMATTI_MC_EntityType_GetDescriptionId(zombie), "entity.minecraft.zombie") == 0, "zombie description id");

    const LIBMATTI_MC_EntityType *player = LIBMATTI_MC_EntityType_PLAYER();
    check(player != NULL && !LIBMATTI_MC_EntityType_CanSerialize(player), "player is noSave");
    check(player != NULL && !LIBMATTI_MC_EntityType_CanSummon(player), "player is noSummon");
    const LIBMATTI_MC_EntityType *tnt = LIBMATTI_MC_EntityType_TNT();
    check(tnt != NULL && LIBMATTI_MC_EntityType_FireImmune(tnt), "tnt is fire immune");
    const LIBMATTI_MC_EntityType *item = LIBMATTI_MC_EntityType_ITEM();
    check(item != NULL && LIBMATTI_MC_EntityType_TrackDeltas(item), "item tracks deltas");
    check(player != NULL && !LIBMATTI_MC_EntityType_TrackDeltas(player), "player does not track deltas");

    // ---- MobCategory --------------------------------------------------------------
    check(LIBMATTI_MC_MobCategory_GetMax(LIBMATTI_MC_MobCategory_MONSTER) == 70, "monster cap 70");
    check(LIBMATTI_MC_MobCategory_GetMaxInstancesPerChunk(LIBMATTI_MC_MobCategory_MISC) == 0, "misc has no cap");
    check(LIBMATTI_MC_MobCategory_ByName("creature") == LIBMATTI_MC_MobCategory_CREATURE, "MobCategory byName");

    // ---- EntityDimensions ----------------------------------------------------------
    LIBMATTI_MC_EntityDimensions *dims = LIBMATTI_MC_EntityDimensions_Scalable(0.6f, 1.8f);
    check(dims != NULL && fabs(dims->eyeHeight - 1.8f * 0.85f) < 1e-5, "default eye height = height * 0.85");
    LIBMATTI_MC_Vec3 *pos = LIBMATTI_MC_Vec3_New(10.0, 64.0, -3.0);
    LIBMATTI_MC_AABB *bb = LIBMATTI_MC_EntityDimensions_MakeBoundingBox(dims, pos);
    check(bb != NULL && fabs(bb->minX - 9.7) < 1e-5 && fabs(bb->maxX - 10.3) < 1e-5, "bbox x spans width/2");
    check(bb != NULL && fabs(bb->minY - 64.0) < 1e-5 && fabs(bb->maxY - 65.8) < 1e-5, "bbox y spans height");
    LIBMATTI_MC_AABB *moved = LIBMATTI_MC_EntityDimensions_MakeBoundingBox(dims, pos);
    LIBMATTI_MC_EntityDimensions *scaled = LIBMATTI_MC_EntityDimensions_Scale(dims, 2.0f);
    check(scaled != NULL && fabs(scaled->width - 1.2f) < 1e-5, "scale doubles the width");
    LIBMATTI_MC_EntityDimensions *fixed = LIBMATTI_MC_EntityDimensions_Fixed(1.0f, 1.0f);
    LIBMATTI_MC_EntityDimensions *fixedScaled = LIBMATTI_MC_EntityDimensions_Scale(fixed, 2.0f);
    check(fixedScaled != NULL && fabs(fixedScaled->width - 1.0f) < 1e-5, "fixed dimensions ignore scale");

    // ---- EntityAttachments -----------------------------------------------------------
    LIBMATTI_MC_EntityAttachmentsBuilder *builder = LIBMATTI_MC_EntityAttachments_Builder();
    LIBMATTI_MC_EntityAttachmentsBuilder_Attach(builder, LIBMATTI_MC_EntityAttachment_PASSENGER, 0.0f, 2.0f, 0.5f);
    LIBMATTI_MC_EntityAttachments *attachments = LIBMATTI_MC_EntityAttachmentsBuilder_Build(builder, 0.6f, 1.8f);
    LIBMATTI_MC_Vec3 *p0 = LIBMATTI_MC_EntityAttachments_Get(attachments, LIBMATTI_MC_EntityAttachment_PASSENGER, 0, 0.0f);
    check(p0 != NULL && fabs(p0->y - 2.0) < 1e-9 && fabs(p0->z - 0.5) < 1e-9, "explicit passenger attachment");
    LIBMATTI_MC_Vec3 *fallback = LIBMATTI_MC_EntityAttachments_Get(attachments, LIBMATTI_MC_EntityAttachment_NAME_TAG, 0, 0.0f);
    check(fallback != NULL && fabs(fallback->y - 1.8) < 1e-5, "name tag falls back to height");
    LIBMATTI_MC_Vec3 *center = LIBMATTI_MC_EntityAttachments_Get(attachments, LIBMATTI_MC_EntityAttachment_WARDEN_CHEST, 0, 0.0f);
    check(center != NULL && fabs(center->y - 0.9) < 1e-5, "warden chest falls back to center");
    LIBMATTI_MC_Vec3 *rotated = LIBMATTI_MC_EntityAttachments_Get(attachments, LIBMATTI_MC_EntityAttachment_PASSENGER, 0, 90.0f);
    // yRot(-90deg) turns (0, 2, 0.5): x = z*sin(-90)?? the transform: x = x*c + z*s, z = z*c - x*s with s=sin(90)
    check(rotated != NULL && fabs(rotated->x + 0.5) < 1e-6 && fabs(rotated->z) < 1e-6, "attachment rotates with yRot");

    // ---- the Entity base over a vanilla subclass -------------------------------------
    LIBMATTI_MC_Level *level = LIBMATTI_MC_Level_New(-64, 384, LIBMATTI_MC_Level_OVERWORLD, true);
    LIBMATTI_MC_Entity *entity = LIBMATTI_MC_EntityType_CreateAt(zombie, level, LIBMATTI_MC_BlockPos_New(3, 65, 4));
    check(entity != NULL, "zombie spawned through the factory");
    check(entity != NULL && LIBMATTI_MC_Entity_GetType(entity) == zombie, "entity carries the type");
    check(entity != NULL && LIBMATTI_MC_Entity_GetId(entity) > 0, "entity id from the counter");
    check(entity != NULL && fabs(LIBMATTI_MC_Entity_GetX(entity) - 3.5) < 1e-9, "spawn centres x");
    check(entity != NULL && fabs(LIBMATTI_MC_Entity_GetY(entity) - 65.0) < 1e-9, "spawn keeps y");
    LIBMATTI_MC_Entity *second = LIBMATTI_MC_EntityType_CreateAt(zombie, level, LIBMATTI_MC_BlockPos_New(0, 65, 0));
    check(entity != NULL && second != NULL && LIBMATTI_MC_Entity_GetId(second) != LIBMATTI_MC_Entity_GetId(entity), "ids increment");
    LIBMATTI_JU_UUID uuid1 = LIBMATTI_MC_Entity_GetUUID(entity);
    LIBMATTI_JU_UUID uuid2 = LIBMATTI_MC_Entity_GetUUID(second);
    check(!LIBMATTI_JU_UUID_Equals(&uuid1, &uuid2), "uuids differ per entity");

    // rotation: the % 360 normalisation + the clamp of turn()
    LIBMATTI_MC_Entity_SetRot(entity, 540.0f, 10.0f);
    check(fabs(LIBMATTI_MC_Entity_GetYRot(entity) - 180.0f) < 1e-5, "setRot normalises yaw (540 -> 180)");
    LIBMATTI_MC_Entity_Turn(entity, 10.0, 0.0);
    check(fabs(LIBMATTI_MC_Entity_GetYRot(entity) - 181.5f) < 1e-5, "turn adds yaw * 0.15");

    // the bounding box follows the position
    LIBMATTI_MC_Entity_SetPos(entity, 100.0, 70.0, 200.0);
    const LIBMATTI_MC_AABB *entityBox = LIBMATTI_MC_Entity_GetBoundingBox(entity);
    check(entityBox != NULL && fabs(entityBox->minX - 99.7) < 1e-5 && fabs(entityBox->maxX - 100.3) < 1e-5, "bbox refreshes on setPos");
    check(entityBox != NULL && fabs(entityBox->maxY - 71.95) < 1e-5, "bbox height 1.95");
    check(fabs(LIBMATTI_MC_Entity_GetEyeHeight(entity) - 1.74f) < 1e-5, "zombie eye height 1.74");

    // tags: sorted set semantics
    check(LIBMATTI_MC_Entity_AddTag(entity, "persistent"), "addTag");
    check(!LIBMATTI_MC_Entity_AddTag(entity, "persistent"), "addTag twice fails");
    check(LIBMATTI_MC_Entity_AddTag(entity, "boss") && LIBMATTI_MC_Entity_AddTag(entity, "cull"), "more tags");
    int tagCount = 0;
    const char *const *tags = LIBMATTI_MC_Entity_GetTags(entity, &tagCount);
    check(tagCount == 3, "three tags");
    check(tags != NULL && strcmp(tags[0], "boss") == 0 && strcmp(tags[1], "cull") == 0 && strcmp(tags[2], "persistent") == 0, "tags sorted");
    check(LIBMATTI_MC_Entity_RemoveTag(entity, "cull"), "removeTag");
    LIBMATTI_MC_Entity_GetTags(entity, &tagCount);
    check(tagCount == 2, "two tags after remove");

    // the removal lifecycle
    check(!LIBMATTI_MC_Entity_IsRemoved(entity), "alive after spawn");
    LIBMATTI_MC_Entity_Discard(entity);
    check(LIBMATTI_MC_Entity_IsRemoved(entity), "discard removes");

    // ---- the NBT round trip -----------------------------------------------------------
    LIBMATTI_MC_Entity *saved = LIBMATTI_MC_EntityType_CreateAt(zombie, level, LIBMATTI_MC_BlockPos_New(1, 70, 2));
    LIBMATTI_MC_Entity_SetPos(saved, 1.5, 70.0, 2.5);
    LIBMATTI_MC_Entity_SetRot(saved, 45.0f, -20.0f);
    LIBMATTI_MC_Vec3 motion = {0.1, -0.2, 0.3};
    LIBMATTI_MC_Entity_SetDeltaMovement(saved, &motion);
    LIBMATTI_MC_Entity_SetOnGround(saved, true);
    LIBMATTI_MC_Entity_SetInvulnerable(saved, true);
    LIBMATTI_MC_Entity_AddTag(saved, "harness_tag");
    LIBMATTI_MC_Nbt_CompoundTag *tag = LIBMATTI_MC_Nbt_CompoundTag_New();
    LIBMATTI_MC_Entity_SaveWithoutId(saved, tag);
    // the vanilla keys exist
    check(LIBMATTI_MC_Nbt_CompoundTag_Contains(tag, "Pos"), "save writes Pos");
    check(LIBMATTI_MC_Nbt_CompoundTag_Contains(tag, "Motion"), "save writes Motion");
    check(LIBMATTI_MC_Nbt_CompoundTag_Contains(tag, "Rotation"), "save writes Rotation");
    check(LIBMATTI_MC_Nbt_CompoundTag_Contains(tag, "UUID"), "save writes UUID");
    check(LIBMATTI_MC_Nbt_CompoundTag_GetBooleanOr(tag, "OnGround", false), "save writes OnGround");
    // load into a fresh entity and compare
    LIBMATTI_MC_Entity *loaded = LIBMATTI_MC_EntityType_Create(zombie, level);
    LIBMATTI_MC_Entity_Load(loaded, tag);
    check(fabs(LIBMATTI_MC_Entity_GetX(loaded) - 1.5) < 1e-9 && fabs(LIBMATTI_MC_Entity_GetY(loaded) - 70.0) < 1e-9, "load restores Pos");
    check(fabs(LIBMATTI_MC_Entity_GetYRot(loaded) - 45.0f) < 1e-5, "load restores Rotation");
    LIBMATTI_MC_Vec3 loadedMotion;
    LIBMATTI_MC_Entity_GetDeltaMovement(loaded, &loadedMotion);
    check(fabs(loadedMotion.y + 0.2) < 1e-9, "load restores Motion");
    check(LIBMATTI_MC_Entity_OnGround(loaded), "load restores OnGround");
    check(LIBMATTI_MC_Entity_IsInvulnerable(loaded), "load restores Invulnerable");
    LIBMATTI_JU_UUID savedUuid = LIBMATTI_MC_Entity_GetUUID(saved);
    LIBMATTI_JU_UUID loadedUuid = LIBMATTI_MC_Entity_GetUUID(loaded);
    check(LIBMATTI_JU_UUID_Equals(&savedUuid, &loadedUuid), "load restores UUID");
    int loadedTags = 0;
    LIBMATTI_MC_Entity_GetTags(loaded, &loadedTags);
    check(loadedTags == 1, "load restores Tags");

    // ---- the Level entity surface -------------------------------------------------------
    check(LIBMATTI_MC_Level_GetEntityCount(level) == 0, "level starts empty");
    LIBMATTI_MC_Entity *a = LIBMATTI_MC_EntityType_CreateAt(zombie, level, LIBMATTI_MC_BlockPos_New(0, 65, 0));
    LIBMATTI_MC_Entity *b = LIBMATTI_MC_EntityType_CreateAt(zombie, level, LIBMATTI_MC_BlockPos_New(100, 65, 100));
    check(LIBMATTI_MC_Level_AddEntity(level, a), "addEntity a");
    check(LIBMATTI_MC_Level_AddEntity(level, b), "addEntity b");
    check(LIBMATTI_MC_Level_GetEntityCount(level) == 2, "two entities in the level");
    check(LIBMATTI_MC_Entity_Level(a) == level, "addEntity wires the level");
    struct LIBMATTI_MC_Entity *found[8];
    int foundCount = LIBMATTI_MC_Level_GetEntitiesInBox(level, -2, 60, -2, 16, 80, 16, found, 8);
    check(foundCount == 1, "box query finds the close entity");
    check(foundCount == 1 && found[0] == a, "box query returns a");
    foundCount = LIBMATTI_MC_Level_GetEntitiesInBox(level, -1000, -64, -1000, 1000, 320, 1000, found, 8);
    check(foundCount == 2, "the world box finds both");
    check(LIBMATTI_MC_Level_RemoveEntity(level, a), "removeEntity a");
    check(LIBMATTI_MC_Level_GetEntityCount(level) == 1, "one entity after removal");
    check(!LIBMATTI_MC_Level_RemoveEntity(level, a), "removing twice fails");

    // the fallback spawn path: item + orb + tnt factories
    LIBMATTI_MC_Entity *orb = LIBMATTI_MC_EntityType_Create(LIBMATTI_MC_EntityType_EXPERIENCE_ORB(), level);
    check(orb != NULL && LIBMATTI_MC_Entity_GetType(orb) == LIBMATTI_MC_EntityType_EXPERIENCE_ORB(), "orb factory");
    LIBMATTI_MC_Entity *itemEntity = LIBMATTI_MC_EntityType_Create(LIBMATTI_MC_EntityType_ITEM(), level);
    check(itemEntity != NULL && fabs(LIBMATTI_MC_Entity_GetEyeHeight(itemEntity) - 0.25f * 0.85f) < 1e-5, "item entity default eye height");

    printf("entity: %d checks passed (%s)\n", checks, failures ? "FAILURES" : "ok");
    return failures != 0;
}
