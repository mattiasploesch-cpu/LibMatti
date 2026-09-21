// Port of net.neoforged.fml.common.asm.enumextension.EnumPrototype.

#include "libmatti/net/neoforged/fml/common/asm/enumextension/EnumPrototype.h"

#include "libmatti/com/google/gson/Gson.h"
#include "libmatti/javax/lang/model/SourceVersion.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"
#include "libmatti/net/neoforged/neoforgespi/locating/IModFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final Gson GSON = new Gson()
static LIBMATTI_GSON_Gson *GSON(void)
{
    static LIBMATTI_GSON_Gson *gson;
    if (gson == NULL) gson = LIBMATTI_GSON_Gson_New();
    return gson;
}

// Java: private static void error(String message, IModInfo mod, Object... params)
// Java passes the resource instance; Object.toString() there renders the class name and identity
// hash, which carries no information, so the port passes a stable label.
#define RESOURCE_LABEL "JarResource"

static void error(const char *message, const LIBMATTI_NEOFORGESPI_IModInfo *mod, const char **params, size_t paramCount)
{
    LIBMATTI_FML_ModLoadingIssue *issue = LIBMATTI_FML_ModLoadingIssue_Error(message, params, paramCount);

    // Java: .withAffectedMod(mod) -> affectedMod.getOwningFile().getFile() and its getFilePath()
    LIBMATTI_NEOFORGESPI_IModFileInfo *owningFile = LIBMATTI_NEOFORGESPI_IModInfo_GetOwningFile(mod);
    LIBMATTI_NEOFORGESPI_IModFile *file = LIBMATTI_NEOFORGESPI_IModFileInfo_GetFile(owningFile);

    LIBMATTI_FML_ModLoadingIssue *withMod = LIBMATTI_FML_ModLoadingIssue_WithAffectedMod(
        issue, mod, file, LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(file));
    // Java's record withers are garbage collected; the port frees the issue it replaced.
    LIBMATTI_FML_ModLoadingIssue_Free(issue);

    LIBMATTI_FML_ModLoader_AddLoadingIssue(withMod);
}

// Java: private static boolean isValidClassDescriptor(String desc)
static int isValidClassDescriptor(const char *desc)
{
    if (desc == NULL) return 0;

    size_t length = strlen(desc);
    size_t start = 0;
    for (size_t i = 0; i <= length; i++)
    {
        if (i != length && desc[i] != '/') continue;

        size_t partLength = i - start;
        if (partLength == 0) return 0;

        char *part = malloc(partLength + 1);
        memcpy(part, desc + start, partLength);
        part[partLength] = '\0';
        int valid = LIBMATTI_SV_SourceVersion_IsIdentifier(part);
        free(part);
        if (!valid) return 0;

        start = i + 1;
    }

    return 1;
}

// Java: private static boolean isValidConstructorDescriptor(String desc)
static int isValidConstructorDescriptor(const char *desc)
{
    if (desc == NULL) return 0;

    size_t length = strlen(desc);
    if (length < 3 || desc[0] != '(') return 0;
    if (desc[length - 2] != ')' || desc[length - 1] != 'V') return 0;

    int pendingArray = 0;
    for (size_t i = 1; i + 2 <= length - 1; i++)
    {
        char c = desc[i];

        if (strchr("ZCBSIFJD", c) != NULL)
        {
            pendingArray = 0;
            continue;
        }

        if (c == '[')
        {
            pendingArray = 1;
            continue;
        }

        if (c != 'L') return 0;

        const char *semicolon = strchr(desc + i, ';');
        if (semicolon == NULL || semicolon <= desc + i) return 0;

        size_t segmentLength = (size_t) (semicolon - (desc + i + 1));
        char *segment = malloc(segmentLength + 1);
        memcpy(segment, desc + i + 1, segmentLength);
        segment[segmentLength] = '\0';
        int valid = isValidClassDescriptor(segment);
        free(segment);
        if (!valid) return 0;

        i = (size_t) (semicolon - desc);
        pendingArray = 0;
    }

    return !pendingArray;
}

// Java: private static EnumParameters loadConstantParameters(IModInfo mod, String enumName, String fieldName, String ctorDesc, JsonArray obj)
static LIBMATTI_FML_EnumParameters *loadConstantParameters(const LIBMATTI_NEOFORGESPI_IModInfo *mod,
                                                           const char *enumName, const char *fieldName,
                                                           const char *ctorDesc,
                                                           LIBMATTI_GSON_JsonElement *obj)
{
    // Java: Type[] argTypes = Type.getArgumentTypes(ctorDesc);
    LIBMATTI_ASM_Type *methodType = LIBMATTI_ASM_Type_GetMethodType(ctorDesc);
    size_t argumentCount = 0;
    LIBMATTI_ASM_Type **argumentTypes = LIBMATTI_ASM_Type_GetArgumentTypes(methodType, &argumentCount);
    LIBMATTI_ASM_Type_Free(methodType);

    // Java: if (argTypes.length != obj.size()) { error("fml.modloadingissue.enumextender.argument.constant.count_mismatch", mod, obj.size(), argTypes.length, ctorDesc, fieldName, enumName); return null; }
    size_t elementCount = LIBMATTI_GSON_JsonElement_ElementCount(obj);
    if (argumentCount != elementCount)
    {
        char elementText[32];
        char argumentText[32];
        snprintf(elementText, sizeof(elementText), "%zu", elementCount);
        snprintf(argumentText, sizeof(argumentText), "%zu", argumentCount);
        const char *args[] = {elementText, argumentText, ctorDesc, fieldName, enumName};
        error("fml.modloadingissue.enumextender.argument.constant.count_mismatch", mod, args, 5);

        for (size_t i = 0; i < argumentCount; i++) LIBMATTI_ASM_Type_Free(argumentTypes[i]);
        free(argumentTypes);
        return NULL;
    }

    LIBMATTI_FML_EnumParameters_Constant *params = calloc(argumentCount > 0 ? argumentCount : 1,
                                                          sizeof(*params));

    for (size_t idx = 0; idx < argumentCount; idx++)
    {
        const char *descriptor = LIBMATTI_ASM_Type_GetDescriptor(argumentTypes[idx]);
        LIBMATTI_GSON_JsonElement *element = LIBMATTI_GSON_JsonElement_ElementAt(obj, idx);

        // Java: switch (argType.getDescriptor())
        if (strcmp(descriptor, "Z") == 0)
        {
            params[idx].type = LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_BOOLEAN;
            params[idx].intValue = LIBMATTI_GSON_JsonElement_GetAsBoolean(element);
        }
        else if (strcmp(descriptor, "C") == 0)
        {
            char *param = LIBMATTI_GSON_JsonElement_GetAsString(element);
            // Java: if (param.length() != 1) { error("...constant.invalid_char", mod, param, idx, fieldName, enumName); return null; }
            if (param == NULL || strlen(param) != 1)
            {
                char indexText[32];
                snprintf(indexText, sizeof(indexText), "%zu", idx);
                const char *args[] = {param != NULL ? param : "", indexText, fieldName, enumName};
                error("fml.modloadingissue.enumextender.argument.constant.invalid_char", mod, args, 4);
                free(param);

                for (size_t i = 0; i < argumentCount; i++) LIBMATTI_ASM_Type_Free(argumentTypes[i]);
                free(argumentTypes);
                LIBMATTI_FML_EnumParameters_Free(
                    LIBMATTI_FML_EnumParameters_NewConstant(params, argumentCount));
                return NULL;
            }

            params[idx].type = LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_CHARACTER;
            params[idx].intValue = (unsigned char) param[0];
            free(param);
        }
        else if (strcmp(descriptor, "B") == 0)
        {
            params[idx].type = LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_BYTE;
            params[idx].intValue = LIBMATTI_GSON_JsonElement_GetAsByte(element);
        }
        else if (strcmp(descriptor, "S") == 0)
        {
            params[idx].type = LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_SHORT;
            params[idx].intValue = LIBMATTI_GSON_JsonElement_GetAsShort(element);
        }
        else if (strcmp(descriptor, "I") == 0)
        {
            params[idx].type = LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_INTEGER;
            params[idx].intValue = LIBMATTI_GSON_JsonElement_GetAsInt(element);
        }
        else if (strcmp(descriptor, "F") == 0)
        {
            params[idx].type = LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_FLOAT;
            params[idx].floatValue = LIBMATTI_GSON_JsonElement_GetAsFloat(element);
        }
        else if (strcmp(descriptor, "J") == 0)
        {
            params[idx].type = LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_LONG;
            params[idx].longValue = LIBMATTI_GSON_JsonElement_GetAsLong(element);
        }
        else if (strcmp(descriptor, "D") == 0)
        {
            params[idx].type = LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_DOUBLE;
            params[idx].doubleValue = LIBMATTI_GSON_JsonElement_GetAsDouble(element);
        }
        else if (strcmp(descriptor, "Ljava/lang/String;") == 0)
        {
            if (LIBMATTI_GSON_JsonElement_IsJsonNull(element))
            {
                params[idx].type = LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_NULL;
            }
            else
            {
                params[idx].type = LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_STRING;
                params[idx].stringValue = LIBMATTI_GSON_JsonElement_GetAsString(element);
            }
        }
        else if (!LIBMATTI_GSON_JsonElement_IsJsonNull(element))
        {
            // Java: error("fml.modloadingissue.enumextender.argument.constant.unsupported_type", mod, argType, idx, fieldName, enumName); return null;
            char indexText[32];
            snprintf(indexText, sizeof(indexText), "%zu", idx);
            const char *args[] = {descriptor, indexText, fieldName, enumName};
            error("fml.modloadingissue.enumextender.argument.constant.unsupported_type", mod, args, 4);

            for (size_t i = 0; i < argumentCount; i++) LIBMATTI_ASM_Type_Free(argumentTypes[i]);
            free(argumentTypes);
            LIBMATTI_FML_EnumParameters_Free(LIBMATTI_FML_EnumParameters_NewConstant(params, argumentCount));
            return NULL;
        }
        else
        {
            params[idx].type = LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_NULL;
        }
    }

    for (size_t i = 0; i < argumentCount; i++) LIBMATTI_ASM_Type_Free(argumentTypes[i]);
    free(argumentTypes);

    return LIBMATTI_FML_EnumParameters_NewConstant(params, argumentCount);
}

// Java: static List<EnumPrototype> load(IModInfo mod, JarResource resource)
LIBMATTI_FML_EnumPrototype *LIBMATTI_FML_EnumPrototype_Load(const LIBMATTI_NEOFORGESPI_IModInfo *mod,
                                                           const LIBMATTI_FML_JarResource *resource,
                                                           size_t *count)
{
    *count = 0;

    // Java: try (Reader reader = resource.bufferedReader()) { ... } catch (Throwable e) { ... }
    size_t length = 0;
    unsigned char *bytes = LIBMATTI_FML_JarResource_ReadAllBytes(resource, &length);
    LIBMATTI_GSON_JsonElement *json = LIBMATTI_GSON_Gson_FromJson(GSON(), (const char *) bytes, length);
    free(bytes);

    if (json == NULL)
    {
        // Java: catch (Throwable e) { ModLoader.addLoadingIssue(ModLoadingIssue.error("fml.modloadingissue.enumextender.loading_error", resource).withAffectedMod(mod).withCause(e)); return List.of(); }
        const char *args[] = {RESOURCE_LABEL};
        LIBMATTI_FML_ModLoadingIssue *issue = LIBMATTI_FML_ModLoadingIssue_Error(
            "fml.modloadingissue.enumextender.loading_error", args, 1);

        LIBMATTI_NEOFORGESPI_IModFileInfo *owningFile = LIBMATTI_NEOFORGESPI_IModInfo_GetOwningFile(mod);
        LIBMATTI_NEOFORGESPI_IModFile *file = LIBMATTI_NEOFORGESPI_IModFileInfo_GetFile(owningFile);
        LIBMATTI_FML_ModLoader_AddLoadingIssue(LIBMATTI_FML_ModLoadingIssue_WithAffectedMod(
            issue, mod, file, LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(file)));
        LIBMATTI_FML_ModLoadingIssue_Free(issue);
        return NULL;
    }

    // Java: JsonArray entries = json.getAsJsonArray("entries");
    LIBMATTI_GSON_JsonElement *entries =
        LIBMATTI_GSON_JsonElement_GetAsJsonArray(LIBMATTI_GSON_JsonElement_GetMember(json, "entries"));
    if (entries == NULL)
    {
        LIBMATTI_GSON_JsonElement_Free(json);
        return NULL;
    }

    size_t entryCount = LIBMATTI_GSON_JsonElement_ElementCount(entries);
    LIBMATTI_FML_EnumPrototype *prototypes = calloc(entryCount > 0 ? entryCount : 1, sizeof(*prototypes));

    const char *modId = LIBMATTI_NEOFORGESPI_IModInfo_GetModId(mod);

    for (size_t i = 0; i < entryCount; i++)
    {
        LIBMATTI_GSON_JsonElement *entry =
            LIBMATTI_GSON_JsonElement_GetAsJsonObject(LIBMATTI_GSON_JsonElement_ElementAt(entries, i));
        if (entry == NULL) continue;

        char *enumName = LIBMATTI_GSON_JsonElement_GetAsString(
            LIBMATTI_GSON_JsonElement_GetMember(entry, "enum"));
        if (!isValidClassDescriptor(enumName))
        {
            // Java: error("fml.modloadingissue.enumextender.invalid_enum_name", mod, enumName);
            const char *args[] = {enumName != NULL ? enumName : ""};
            error("fml.modloadingissue.enumextender.invalid_enum_name", mod, args, 1);
            free(enumName);
            continue;
        }

        char *fieldName = LIBMATTI_GSON_JsonElement_GetAsString(
            LIBMATTI_GSON_JsonElement_GetMember(entry, "name"));

        // Java: if (!fieldName.toLowerCase(Locale.ROOT).startsWith(mod.getModId())) { error(...field_name.missing_prefix...); continue; }
        char *lowerFieldName = fieldName != NULL ? strdup(fieldName) : strdup("");
        for (char *c = lowerFieldName; *c != '\0'; c++)
            if (*c >= 'A' && *c <= 'Z') *c = (char) (*c - 'A' + 'a');

        if (strncmp(lowerFieldName, modId, strlen(modId)) != 0)
        {
            const char *args[] = {fieldName != NULL ? fieldName : "", enumName};
            error("fml.modloadingissue.enumextender.field_name.missing_prefix", mod, args, 2);
            free(lowerFieldName);
            free(fieldName);
            free(enumName);
            continue;
        }
        free(lowerFieldName);

        // Java: if (!SourceVersion.isIdentifier(fieldName)) { error(...field_name.invalid...); continue; }
        if (!LIBMATTI_SV_SourceVersion_IsIdentifier(fieldName))
        {
            const char *args[] = {fieldName != NULL ? fieldName : "", enumName};
            error("fml.modloadingissue.enumextender.field_name.invalid", mod, args, 2);
            free(fieldName);
            free(enumName);
            continue;
        }

        char *ctorDesc = LIBMATTI_GSON_JsonElement_GetAsString(
            LIBMATTI_GSON_JsonElement_GetMember(entry, "constructor"));
        if (!isValidConstructorDescriptor(ctorDesc))
        {
            const char *args[] = {ctorDesc != NULL ? ctorDesc : "", enumName};
            error("fml.modloadingissue.enumextender.invalid_constructor", mod, args, 2);
            free(ctorDesc);
            free(fieldName);
            free(enumName);
            continue;
        }

        LIBMATTI_GSON_JsonElement *paramElem = LIBMATTI_GSON_JsonElement_GetMember(entry, "parameters");
        LIBMATTI_FML_EnumParameters *ctorParams = NULL;

        if (LIBMATTI_GSON_JsonElement_IsJsonArray(paramElem))
        {
            ctorParams = loadConstantParameters(mod, enumName, fieldName, ctorDesc, paramElem);
            if (ctorParams == NULL)
            {
                free(ctorDesc);
                free(fieldName);
                free(enumName);
                continue;
            }
        }
        else if (LIBMATTI_GSON_JsonElement_IsJsonObject(paramElem))
        {
            char *className = LIBMATTI_GSON_JsonElement_GetAsString(
                LIBMATTI_GSON_JsonElement_GetMember(paramElem, "class"));

            if (!isValidClassDescriptor(className))
            {
                // Java: error("...argument.reference.invalid_src_class", mod, className, fieldName, enumName); continue;
                const char *args[] = {className != NULL ? className : "", fieldName, enumName};
                error("fml.modloadingissue.enumextender.argument.reference.invalid_src_class", mod, args, 3);
                free(className);
                free(ctorDesc);
                free(fieldName);
                free(enumName);
                continue;
            }

            LIBMATTI_ASM_Type *owner = LIBMATTI_ASM_Type_GetObjectType(className);

            if (LIBMATTI_GSON_JsonElement_HasMember(paramElem, "method"))
            {
                char *methodName = LIBMATTI_GSON_JsonElement_GetAsString(
                    LIBMATTI_GSON_JsonElement_GetMember(paramElem, "method"));
                if (!LIBMATTI_SV_SourceVersion_IsIdentifier(methodName))
                {
                    const char *args[] = {methodName != NULL ? methodName : "", fieldName, enumName};
                    error("fml.modloadingissue.enumextender.argument.reference.invalid_src_method", mod, args, 3);
                    free(methodName);
                    free(className);
                    free(ctorDesc);
                    free(fieldName);
                    free(enumName);
                    continue;
                }
                ctorParams = LIBMATTI_FML_EnumParameters_NewMethodReference(owner, methodName);
                free(methodName);
            }
            else if (LIBMATTI_GSON_JsonElement_HasMember(paramElem, "field"))
            {
                char *srcFieldName = LIBMATTI_GSON_JsonElement_GetAsString(
                    LIBMATTI_GSON_JsonElement_GetMember(paramElem, "field"));
                if (!LIBMATTI_SV_SourceVersion_IsIdentifier(srcFieldName))
                {
                    const char *args[] = {srcFieldName != NULL ? srcFieldName : "", fieldName, enumName};
                    error("fml.modloadingissue.enumextender.argument.reference.invalid_src_field", mod, args, 3);
                    free(srcFieldName);
                    free(className);
                    free(ctorDesc);
                    free(fieldName);
                    free(enumName);
                    continue;
                }
                ctorParams = LIBMATTI_FML_EnumParameters_NewFieldReference(owner, srcFieldName);
                free(srcFieldName);
            }
            else
            {
                // Java: error("fml.modloadingissue.enumextender.argument.reference.unexpected_decl", mod, paramElem, fieldName, enumName); continue;
                char *decl = LIBMATTI_GSON_JsonElement_ToString(paramElem);
                const char *args[] = {decl != NULL ? decl : "", fieldName, enumName};
                error("fml.modloadingissue.enumextender.argument.reference.unexpected_decl", mod, args, 3);
                free(decl);
                free(className);
                free(ctorDesc);
                free(fieldName);
                free(enumName);
                continue;
            }

            LIBMATTI_ASM_Type_Free(owner);
            free(className);
        }
        else
        {
            // Java: error("fml.modloadingissue.enumextender.argument.unexpected_decl", mod, paramElem, fieldName, enumName); continue;
            char *decl = LIBMATTI_GSON_JsonElement_ToString(paramElem);
            const char *args[] = {decl != NULL ? decl : "", fieldName, enumName};
            error("fml.modloadingissue.enumextender.argument.unexpected_decl", mod, args, 3);
            free(decl);
            free(ctorDesc);
            free(fieldName);
            free(enumName);
            continue;
        }

        // Java: String fullCtorDesc = "(" + ENUM_CTOR_BASE_DESC + ctorDesc.substring(1);
        size_t fullLength = strlen(LIBMATTI_FML_ENUMPROTOTYPE_ENUM_CTOR_BASE_DESC) + strlen(ctorDesc) + 1;
        char *fullCtorDesc = malloc(fullLength);
        snprintf(fullCtorDesc, fullLength, "(%s%s", LIBMATTI_FML_ENUMPROTOTYPE_ENUM_CTOR_BASE_DESC,
                 ctorDesc + 1);

        LIBMATTI_FML_EnumPrototype *prototype = &prototypes[(*count)++];
        prototype->owningMod = strdup(modId);
        prototype->enumName = enumName;
        prototype->fieldName = fieldName;
        prototype->ctorDesc = ctorDesc;
        prototype->fullCtorDesc = fullCtorDesc;
        prototype->ctorParams = ctorParams;
    }

    LIBMATTI_GSON_JsonElement_Free(json);
    return prototypes;
}

// Java: @Override public int compareTo(EnumPrototype other)
int LIBMATTI_FML_EnumPrototype_Compare(const LIBMATTI_FML_EnumPrototype *prototype,
                                       const LIBMATTI_FML_EnumPrototype *other)
{
    int comparison = strcmp(prototype->owningMod, other->owningMod);
    if (comparison != 0) return comparison < 0 ? -1 : 1;

    comparison = strcmp(prototype->fieldName, other->fieldName);
    return comparison < 0 ? -1 : (comparison > 0 ? 1 : 0);
}

void LIBMATTI_FML_EnumPrototype_Free(LIBMATTI_FML_EnumPrototype *prototype)
{
    if (prototype == NULL) return;

    free(prototype->owningMod);
    free(prototype->enumName);
    free(prototype->fieldName);
    free(prototype->ctorDesc);
    free(prototype->fullCtorDesc);
    LIBMATTI_FML_EnumParameters_Free(prototype->ctorParams);
}
