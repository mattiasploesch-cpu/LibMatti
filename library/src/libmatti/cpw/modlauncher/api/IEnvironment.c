//
// Port of the static members of cpw.mods.modlauncher.api.IEnvironment
// (Keys + the ModList list type).
//
// Java builds the keys in the IEnvironment.Keys static initializer; the C port
// creates the KeyBuilders lazily on first use, which has the same effect
// (KeyBuilder.get() self-heals into the owner map).
//

#include "IEnvironment.h"

#include <stdlib.h>
#include <string.h>

// Java: TypesafeMap.KeyBuilder<>(name, clazz, IEnvironment.class)
static const char *IENVIRONMENT_CLASS = "cpw.mods.modlauncher.api.IEnvironment";

static LIBMATTI_MLA_KeyBuilder *versionBuilder;
static LIBMATTI_MLA_KeyBuilder *gameDirBuilder;
static LIBMATTI_MLA_KeyBuilder *assetsDirBuilder;
static LIBMATTI_MLA_KeyBuilder *uuidBuilder;
static LIBMATTI_MLA_KeyBuilder *launchTargetBuilder;
static LIBMATTI_MLA_KeyBuilder *auditTrailBuilder;
static LIBMATTI_MLA_KeyBuilder *modListBuilder;
static LIBMATTI_MLA_KeyBuilder *mlSpecVersionBuilder;
static LIBMATTI_MLA_KeyBuilder *mlImplVersionBuilder;

// Java: static final Supplier<Key<String>> VERSION = buildKey("version", String.class)
LIBMATTI_MLA_Key *LIBMATTI_MLA_IEnvironment_Keys_Version(void)
{
    if (versionBuilder == NULL)
        versionBuilder = LIBMATTI_MLA_KeyBuilder_New("version", "java.lang.String", IENVIRONMENT_CLASS);
    return LIBMATTI_MLA_KeyBuilder_Get(versionBuilder);
}

// Java: static final Supplier<Key<Path>> GAMEDIR = buildKey("gamedir", Path.class)
LIBMATTI_MLA_Key *LIBMATTI_MLA_IEnvironment_Keys_GameDir(void)
{
    if (gameDirBuilder == NULL)
        gameDirBuilder = LIBMATTI_MLA_KeyBuilder_New("gamedir", "java.nio.file.Path", IENVIRONMENT_CLASS);
    return LIBMATTI_MLA_KeyBuilder_Get(gameDirBuilder);
}

// Java: static final Supplier<Key<Path>> ASSETSDIR = buildKey("assetsdir", Path.class)
LIBMATTI_MLA_Key *LIBMATTI_MLA_IEnvironment_Keys_AssetsDir(void)
{
    if (assetsDirBuilder == NULL)
        assetsDirBuilder = LIBMATTI_MLA_KeyBuilder_New("assetsdir", "java.nio.file.Path", IENVIRONMENT_CLASS);
    return LIBMATTI_MLA_KeyBuilder_Get(assetsDirBuilder);
}

// Java: static final Supplier<Key<String>> UUID = buildKey("uuid", String.class)
LIBMATTI_MLA_Key *LIBMATTI_MLA_IEnvironment_Keys_Uuid(void)
{
    if (uuidBuilder == NULL)
        uuidBuilder = LIBMATTI_MLA_KeyBuilder_New("uuid", "java.lang.String", IENVIRONMENT_CLASS);
    return LIBMATTI_MLA_KeyBuilder_Get(uuidBuilder);
}

// Java: static final Supplier<Key<String>> LAUNCHTARGET = buildKey("launchtarget", String.class)
LIBMATTI_MLA_Key *LIBMATTI_MLA_IEnvironment_Keys_LaunchTarget(void)
{
    if (launchTargetBuilder == NULL)
        launchTargetBuilder = LIBMATTI_MLA_KeyBuilder_New("launchtarget", "java.lang.String", IENVIRONMENT_CLASS);
    return LIBMATTI_MLA_KeyBuilder_Get(launchTargetBuilder);
}

// Java: static final Supplier<Key<ITransformerAuditTrail>> AUDITTRAIL = buildKey("audittrail", ITransformerAuditTrail.class)
LIBMATTI_MLA_Key *LIBMATTI_MLA_IEnvironment_Keys_AuditTrail(void)
{
    if (auditTrailBuilder == NULL)
        auditTrailBuilder = LIBMATTI_MLA_KeyBuilder_New("audittrail",
                                                        "cpw.mods.modlauncher.api.ITransformerAuditTrail",
                                                        IENVIRONMENT_CLASS);
    return LIBMATTI_MLA_KeyBuilder_Get(auditTrailBuilder);
}

// Java: static final Supplier<Key<List<Map<String,String>>>> MODLIST = buildKey("modlist", List.class)
LIBMATTI_MLA_Key *LIBMATTI_MLA_IEnvironment_Keys_ModList(void)
{
    if (modListBuilder == NULL)
        modListBuilder = LIBMATTI_MLA_KeyBuilder_New("modlist", "java.util.List", IENVIRONMENT_CLASS);
    return LIBMATTI_MLA_KeyBuilder_Get(modListBuilder);
}

// Java: static final Supplier<Key<String>> MLSPEC_VERSION = buildKey("mlspecVersion", String.class)
LIBMATTI_MLA_Key *LIBMATTI_MLA_IEnvironment_Keys_MlSpecVersion(void)
{
    if (mlSpecVersionBuilder == NULL)
        mlSpecVersionBuilder = LIBMATTI_MLA_KeyBuilder_New("mlspecVersion", "java.lang.String", IENVIRONMENT_CLASS);
    return LIBMATTI_MLA_KeyBuilder_Get(mlSpecVersionBuilder);
}

// Java: static final Supplier<Key<String>> MLIMPL_VERSION = buildKey("mlimplVersion", String.class)
LIBMATTI_MLA_Key *LIBMATTI_MLA_IEnvironment_Keys_MlImplVersion(void)
{
    if (mlImplVersionBuilder == NULL)
        mlImplVersionBuilder = LIBMATTI_MLA_KeyBuilder_New("mlimplVersion", "java.lang.String", IENVIRONMENT_CLASS);
    return LIBMATTI_MLA_KeyBuilder_Get(mlImplVersionBuilder);
}

// ---------------------------------------------------------------------------
// ModList - the C model of List<Map<String,String>>
// ---------------------------------------------------------------------------

typedef struct
{
    char *name;
    char *type;
    char *file;
} ModListEntry;

struct LIBMATTI_MLA_ModList
{
    ModListEntry *entries;
    size_t count;
};

// Java: new ArrayList<>()
LIBMATTI_MLA_ModList *LIBMATTI_MLA_ModList_New(void)
{
    return calloc(1, sizeof(LIBMATTI_MLA_ModList));
}

// Java: list.add(Map.of("name", n, "type", t, "file", f))
void LIBMATTI_MLA_ModList_Add(LIBMATTI_MLA_ModList *list, const char *name, const char *type, const char *file)
{
    if (list == NULL) return;

    list->entries = realloc(list->entries, sizeof(ModListEntry) * (list->count + 1));
    list->entries[list->count].name = strdup(name);
    list->entries[list->count].type = strdup(type);
    list->entries[list->count].file = file != NULL ? strdup(file) : NULL;
    list->count++;
}

size_t LIBMATTI_MLA_ModList_Count(const LIBMATTI_MLA_ModList *list)
{
    return list != NULL ? list->count : 0;
}
