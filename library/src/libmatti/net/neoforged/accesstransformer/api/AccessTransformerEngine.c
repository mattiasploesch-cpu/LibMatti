// Port of net.neoforged.accesstransformer.AccessTransformerEngineImpl
// (+ AccessTransformerList and the Target.apply logic).

#include "libmatti/net/neoforged/accesstransformer/api/AccessTransformerEngine.h"

#include "libmatti/java/nio/file/Files.h"
#include "libmatti/org/objectweb/asm/Opcodes.h"
#include "libmatti/org/objectweb/asm/tree/FieldNode.h"
#include "libmatti/org/objectweb/asm/tree/MethodNode.h"

#include <stdlib.h>
#include <string.h>

// Java: private final AccessTransformerList masterList = new AccessTransformerList();
// The port keeps one growable entry list.
typedef struct
{
    LIBMATTI_AT_Target *target;
    LIBMATTI_AT_Transformation *transformation;
} ListEntry;

struct LIBMATTI_AT_AccessTransformerEngine
{
    ListEntry *entries;
    size_t entryCount;
    size_t entryCapacity;
};

static void add_entry(LIBMATTI_AT_AccessTransformerEngine *engine, LIBMATTI_AT_Target *target,
                      LIBMATTI_AT_Transformation *transformation)
{
    if (engine->entryCount == engine->entryCapacity)
    {
        engine->entryCapacity = engine->entryCapacity == 0 ? 16 : engine->entryCapacity * 2;
        engine->entries = realloc(engine->entries, sizeof(ListEntry) * engine->entryCapacity);
    }
    engine->entries[engine->entryCount].target = target;
    engine->entries[engine->entryCount].transformation = transformation;
    engine->entryCount++;
}

LIBMATTI_AT_AccessTransformerEngine *LIBMATTI_AT_AccessTransformerEngine_New(void)
{
    return calloc(1, sizeof(LIBMATTI_AT_AccessTransformerEngine));
}

void LIBMATTI_AT_AccessTransformerEngine_Free(LIBMATTI_AT_AccessTransformerEngine *engine)
{
    if (engine == NULL)
        return;
    for (size_t i = 0; i < engine->entryCount; i++)
    {
        LIBMATTI_AT_Target_Free(engine->entries[i].target);
        LIBMATTI_AT_Transformation_Free(engine->entries[i].transformation);
    }
    free(engine->entries);
    free(engine);
}

int LIBMATTI_AT_AccessTransformerEngine_LoadAT(LIBMATTI_AT_AccessTransformerEngine *engine,
                                               const char *text, const char *originName, int *errorLine)
{
    size_t count = 0;
    LIBMATTI_AT_AtEntry **entries = LIBMATTI_AT_AtParser_Parse(text, originName, &count, errorLine);
    if (entries == NULL)
        return 0;

    // Java: the masterList merges by target; the last transformation wins
    for (size_t i = 0; i < count; i++)
        add_entry(engine, entries[i]->target, entries[i]->transformation);
    free(entries);
    return 1;
}

int LIBMATTI_AT_AccessTransformerEngine_LoadATFromPath(LIBMATTI_AT_AccessTransformerEngine *engine,
                                                       const char *path, int *errorLine)
{
    size_t length = 0;
    unsigned char *bytes = LIBMATTI_JNF_Files_ReadAllBytes(path, &length);
    if (bytes == NULL)
    {
        *errorLine = -1;
        return 0;
    }
    bytes[length] = '\0';
    int loaded = LIBMATTI_AT_AccessTransformerEngine_LoadAT(engine, (const char *) bytes, path, errorLine);
    free(bytes);
    return loaded;
}

int LIBMATTI_AT_AccessTransformerEngine_LoadATFromResource(LIBMATTI_AT_AccessTransformerEngine *engine,
                                                           const char *resourceName, int *errorLine)
{
    return LIBMATTI_AT_AccessTransformerEngine_LoadATFromPath(engine, resourceName, errorLine);
}

// Java: Set<Type> getTargets()
char **LIBMATTI_AT_AccessTransformerEngine_GetTargets(const LIBMATTI_AT_AccessTransformerEngine *engine,
                                                      size_t *count)
{
    char **targets = malloc(sizeof(char *) * (engine->entryCount > 0 ? engine->entryCount : 1));
    size_t targetCount = 0;
    for (size_t i = 0; i < engine->entryCount; i++)
    {
        const char *className = engine->entries[i].target->className;
        int seen = 0;
        for (size_t j = 0; j < targetCount; j++)
        {
            if (strcmp(targets[j], className) == 0)
            {
                seen = 1;
                break;
            }
        }
        if (!seen)
            targets[targetCount++] = (char *) className;
    }
    *count = targetCount;
    return targets;
}

// Java: boolean containsClassTarget(Type type)
int LIBMATTI_AT_AccessTransformerEngine_ContainsClassTarget(const LIBMATTI_AT_AccessTransformerEngine *engine,
                                                            const char *internalName)
{
    for (size_t i = 0; i < engine->entryCount; i++)
        if (strcmp(engine->entries[i].target->className, internalName) == 0)
            return 1;
    return 0;
}

// Java: Set<String> getSourcesForTarget(String className, TargetType type, String targetName)
char **LIBMATTI_AT_AccessTransformerEngine_GetSourcesForTarget(const LIBMATTI_AT_AccessTransformerEngine *engine,
                                                               const char *internalName, LIBMATTI_AT_TargetType type,
                                                               const char *targetName, size_t *count)
{
    char **sources = malloc(sizeof(char *) * (engine->entryCount > 0 ? engine->entryCount : 1));
    size_t sourceCount = 0;
    LIBMATTI_AT_TargetKind kind;
    switch (type)
    {
        case LIBMATTI_AT_TargetType_CLASS:
            kind = LIBMATTI_AT_TargetKind_CLASS;
            break;
        case LIBMATTI_AT_TargetType_METHOD:
            kind = LIBMATTI_AT_TargetKind_METHOD;
            break;
        default:
            kind = LIBMATTI_AT_TargetKind_FIELD;
            break;
    }

    for (size_t i = 0; i < engine->entryCount; i++)
    {
        const LIBMATTI_AT_Target *target = engine->entries[i].target;
        if (target->kind != kind || strcmp(target->className, internalName) != 0)
            continue;
        if (targetName != NULL &&
            (target->memberName == NULL || strcmp(target->memberName, targetName) != 0))
            continue;
        if (engine->entries[i].transformation->origin == NULL)
            continue;

        int seen = 0;
        for (size_t j = 0; j < sourceCount; j++)
        {
            if (strcmp(sources[j], engine->entries[i].transformation->origin) == 0)
            {
                seen = 1;
                break;
            }
        }
        if (!seen)
            sources[sourceCount++] = engine->entries[i].transformation->origin;
    }
    *count = sourceCount;
    return sources;
}

// ---------------------------------------------------------------------------
// Java: the Transformation.apply chain (Target -> the node's access update)
// ---------------------------------------------------------------------------

// Java: Modifier merges: the new visibility + optional final flip
static int apply_access(int access, const LIBMATTI_AT_Transformation *transformation)
{
    // Java: the modifier replaces the visibility bits
    access &= ~(LIBMATTI_ASM_ACC_PUBLIC | LIBMATTI_ASM_ACC_PRIVATE | LIBMATTI_ASM_ACC_PROTECTED);
    switch (transformation->modifier)
    {
        case LIBMATTI_AT_Modifier_PUBLIC:
            access |= LIBMATTI_ASM_ACC_PUBLIC;
            break;
        case LIBMATTI_AT_Modifier_PRIVATE:
            access |= LIBMATTI_ASM_ACC_PRIVATE;
            break;
        case LIBMATTI_AT_Modifier_PROTECTED:
            access |= LIBMATTI_ASM_ACC_PROTECTED;
            break;
        case LIBMATTI_AT_Modifier_DEFAULT:
            break;
    }

    // Java: the final state
    if (transformation->finalState == LIBMATTI_AT_FinalState_MAKEFINAL)
        access |= LIBMATTI_ASM_ACC_FINAL;
    else if (transformation->finalState == LIBMATTI_AT_FinalState_REMOVEFINAL)
        access &= ~LIBMATTI_ASM_ACC_FINAL;
    return access;
}

int LIBMATTI_AT_AccessTransformerEngine_Transform(LIBMATTI_AT_AccessTransformerEngine *engine,
                                                  LIBMATTI_ASMT_ClassNode *classNode, const char *internalName)
{
    // Java: if (!masterList.containsClassTarget(classType)) return false;
    if (!LIBMATTI_AT_AccessTransformerEngine_ContainsClassTarget(engine, internalName))
        return 0;

    int transformed = 0;

    // Java: the class target and the wildcards
    for (size_t i = 0; i < engine->entryCount; i++)
    {
        const LIBMATTI_AT_Target *target = engine->entries[i].target;
        const LIBMATTI_AT_Transformation *transformation = engine->entries[i].transformation;
        if (strcmp(target->className, internalName) != 0)
            continue;

        if (target->kind == LIBMATTI_AT_TargetKind_CLASS)
        {
            classNode->access = apply_access(classNode->access, transformation);
            transformed = 1;
        }
        else if (target->kind == LIBMATTI_AT_TargetKind_WILDCARD_FIELD)
        {
            for (size_t f = 0; f < classNode->fieldCount; f++)
            {
                classNode->fields[f]->access = apply_access(classNode->fields[f]->access, transformation);
                transformed = 1;
            }
        }
        else if (target->kind == LIBMATTI_AT_TargetKind_WILDCARD_METHOD)
        {
            for (size_t m = 0; m < classNode->methodCount; m++)
            {
                classNode->methods[m]->access = apply_access(classNode->methods[m]->access, transformation);
                transformed = 1;
            }
        }
        else if (target->kind == LIBMATTI_AT_TargetKind_FIELD)
        {
            for (size_t f = 0; f < classNode->fieldCount; f++)
            {
                if (strcmp(classNode->fields[f]->name, target->memberName) == 0)
                {
                    classNode->fields[f]->access = apply_access(classNode->fields[f]->access, transformation);
                    transformed = 1;
                }
            }
        }
        else if (target->kind == LIBMATTI_AT_TargetKind_METHOD)
        {
            for (size_t m = 0; m < classNode->methodCount; m++)
            {
                LIBMATTI_ASMT_MethodNode *method = classNode->methods[m];
                // Java: the map key is name + desc
                if (strcmp(method->name, target->memberName) == 0 &&
                    strcmp(method->desc, target->descriptor) == 0)
                {
                    method->access = apply_access(method->access, transformation);
                    transformed = 1;
                }
            }
        }
    }
    return transformed;
}
