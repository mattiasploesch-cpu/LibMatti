// Port of cpw.mods.modlauncher.TransformTargetLabel.

#include "libmatti/cpw/modlauncher/TransformTargetLabel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *internal_name(const char *className)
{
    char *result = strdup(className);
    for (char *c = result; *c != '\0'; c++) if (*c == '.') *c = '/';
    return result;
}

static LIBMATTI_ML_TransformTargetLabel *label_new(const char *className, const char *elementName,
                                                   const char *elementDescriptor,
                                                   const LIBMATTI_MLA_TargetType *labelType)
{
    LIBMATTI_ML_TransformTargetLabel *label = calloc(1, sizeof(LIBMATTI_ML_TransformTargetLabel));
    char *name = internal_name(className);
    label->className = LIBMATTI_ASM_Type_GetObjectType(name);
    free(name);
    label->elementName = strdup(elementName != NULL ? elementName : "");
    label->elementDescriptor = elementDescriptor != NULL && elementDescriptor[0] != '\0'
                                   ? LIBMATTI_ASM_Type_GetMethodType(elementDescriptor)
                                   : LIBMATTI_ASM_Type_GetType(LIBMATTI_ASM_Type_GetDescriptor(LIBMATTI_ASM_Type_VOID()));
    label->labelType = labelType;
    return label;
}

LIBMATTI_ML_TransformTargetLabel *LIBMATTI_ML_TransformTargetLabel_NewFromTarget(
    const LIBMATTI_MLA_ITransformer_Target *target)
{
    return label_new(target->className, target->elementName, target->elementDescriptor, target->targetType);
}

LIBMATTI_ML_TransformTargetLabel *LIBMATTI_ML_TransformTargetLabel_NewField(const char *className, const char *fieldName)
{
    return label_new(className, fieldName, "", LIBMATTI_MLA_TargetType_Field());
}

LIBMATTI_ML_TransformTargetLabel *LIBMATTI_ML_TransformTargetLabel_NewMethod(const char *className, const char *methodName,
                                                                             const char *methodDesc)
{
    return label_new(className, methodName, methodDesc, LIBMATTI_MLA_TargetType_Method());
}

LIBMATTI_ML_TransformTargetLabel *LIBMATTI_ML_TransformTargetLabel_NewClass(const char *className)
{
    return label_new(className, "", "", LIBMATTI_MLA_TargetType_Class());
}

LIBMATTI_ML_TransformTargetLabel *LIBMATTI_ML_TransformTargetLabel_NewClassWithType(
    const char *className, const LIBMATTI_MLA_TargetType *type)
{
    return label_new(className, "", "", type);
}

LIBMATTI_ASM_Type *LIBMATTI_ML_TransformTargetLabel_GetClassName(const LIBMATTI_ML_TransformTargetLabel *label)
{
    return label->className;
}

const char *LIBMATTI_ML_TransformTargetLabel_GetElementName(const LIBMATTI_ML_TransformTargetLabel *label)
{
    return label->elementName;
}

LIBMATTI_ASM_Type *LIBMATTI_ML_TransformTargetLabel_GetElementDescriptor(const LIBMATTI_ML_TransformTargetLabel *label)
{
    return label->elementDescriptor;
}

const LIBMATTI_MLA_TargetType *LIBMATTI_ML_TransformTargetLabel_GetTargetType(
    const LIBMATTI_ML_TransformTargetLabel *label)
{
    return label->labelType;
}

int LIBMATTI_ML_TransformTargetLabel_HashCode(const LIBMATTI_ML_TransformTargetLabel *label)
{
    // Java: Objects.hash(className, elementName, elementDescriptor)
    int hash = 1;
    hash = hash * 31 + (int)strlen(LIBMATTI_ASM_Type_GetDescriptor(label->className));
    for (const char *c = label->elementName; *c != '\0'; c++) hash = hash * 31 + *c;
    hash = hash * 31 + (int)strlen(LIBMATTI_ASM_Type_GetDescriptor(label->elementDescriptor));
    return hash;
}

int LIBMATTI_ML_TransformTargetLabel_Equals(const LIBMATTI_ML_TransformTargetLabel *label,
                                            const LIBMATTI_ML_TransformTargetLabel *other)
{
    if (other == NULL) return 0;

    return strcmp(LIBMATTI_ASM_Type_GetDescriptor(label->className),
                  LIBMATTI_ASM_Type_GetDescriptor(other->className)) == 0 &&
           strcmp(label->elementName, other->elementName) == 0 &&
           strcmp(LIBMATTI_ASM_Type_GetDescriptor(label->elementDescriptor),
                  LIBMATTI_ASM_Type_GetDescriptor(other->elementDescriptor)) == 0;
}

char *LIBMATTI_ML_TransformTargetLabel_ToString(const LIBMATTI_ML_TransformTargetLabel *label)
{
    const char *nodeType = label->labelType != NULL ? label->labelType->name : "null";
    const char *className = LIBMATTI_ASM_Type_GetDescriptor(label->className);
    const char *descriptor = LIBMATTI_ASM_Type_GetDescriptor(label->elementDescriptor);

    size_t length = strlen(nodeType) + strlen(className) + strlen(label->elementName) + strlen(descriptor) + 32;
    char *result = malloc(length);
    snprintf(result, length, "Target : %s {%s} {%s} {%s}", nodeType, className, label->elementName, descriptor);
    return result;
}

void LIBMATTI_ML_TransformTargetLabel_Free(LIBMATTI_ML_TransformTargetLabel *label)
{
    if (label == NULL) return;
    LIBMATTI_ASM_Type_Free(label->className);
    free(label->elementName);
    LIBMATTI_ASM_Type_Free(label->elementDescriptor);
    free(label);
}
