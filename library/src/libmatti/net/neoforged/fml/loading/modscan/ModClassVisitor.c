// Port of net.neoforged.fml.loading.modscan.ModClassVisitor.

#include "libmatti/net/neoforged/fml/loading/modscan/ModClassVisitor.h"

#include <stdlib.h>

// Java: public ModClassVisitor()
LIBMATTI_FML_ModClassVisitor *LIBMATTI_FML_ModClassVisitor_New(void)
{
    return calloc(1, sizeof(LIBMATTI_FML_ModClassVisitor));
}

void LIBMATTI_FML_ModClassVisitor_Free(LIBMATTI_FML_ModClassVisitor *visitor)
{
    if (visitor == NULL) return;

    LIBMATTI_ASM_Type_Free(visitor->asmType);
    LIBMATTI_ASM_Type_Free(visitor->asmSuperType);
    for (size_t i = 0; i < visitor->interfaceCount; i++) LIBMATTI_ASM_Type_Free(visitor->interfaces[i]);
    free(visitor->interfaces);
    LIBMATTI_FML_ModAnnotationList_Free(&visitor->annotations);
    free(visitor);
}

// Java: public void visit(int version, int access, String name, String signature, String superName, String[] interfaces)
void LIBMATTI_FML_ModClassVisitor_Visit(LIBMATTI_FML_ModClassVisitor *visitor, int version, int access,
                                        const char *name, const char *signature, const char *superName,
                                        char **interfaces, size_t interfaceCount)
{
    (void) version;
    (void) access;
    (void) signature;

    visitor->asmType = LIBMATTI_ASM_Type_GetObjectType(name);
    visitor->asmSuperType = superName != NULL && superName[0] != '\0'
                                ? LIBMATTI_ASM_Type_GetObjectType(superName)
                                : NULL;

    visitor->interfaceCount = interfaceCount;
    visitor->interfaces = calloc(interfaceCount > 0 ? interfaceCount : 1, sizeof(void *));
    for (size_t i = 0; i < interfaceCount; i++)
        visitor->interfaces[i] = LIBMATTI_ASM_Type_GetObjectType(interfaces[i]);
}

// Java: public AnnotationVisitor visitAnnotation(String annotationName, boolean runtimeVisible)
LIBMATTI_FML_ModAnnotationVisitor *LIBMATTI_FML_ModClassVisitor_VisitAnnotation(
    LIBMATTI_FML_ModClassVisitor *visitor, const char *annotationName, int runtimeVisible)
{
    (void) runtimeVisible;

    // Java: this.asmType.getClassName() is a temporary String
    char *member = LIBMATTI_ASM_Type_GetClassName(visitor->asmType);

    LIBMATTI_FML_ModAnnotation *annotation = LIBMATTI_FML_ModAnnotation_New(
        LIBMATTI_NEOFORGESPI_ElementType_TYPE, LIBMATTI_ASM_Type_GetType(annotationName), member);
    LIBMATTI_FML_ModAnnotationList_AddFirst(&visitor->annotations, annotation);
    free(member);

    return LIBMATTI_FML_ModAnnotationVisitor_New(&visitor->annotations, annotation);
}

// Java: public FieldVisitor visitField(int access, String name, String desc, String signature, Object value)
LIBMATTI_FML_ModFieldVisitor *LIBMATTI_FML_ModClassVisitor_VisitField(
    LIBMATTI_FML_ModClassVisitor *visitor, int access, const char *name, const char *desc, const char *signature)
{
    (void) access;
    (void) desc;
    (void) signature;

    return LIBMATTI_FML_ModFieldVisitor_New(name, &visitor->annotations);
}

// Java: public MethodVisitor visitMethod(int access, String name, String desc, String signature, String[] exceptions)
LIBMATTI_FML_ModMethodVisitor *LIBMATTI_FML_ModClassVisitor_VisitMethod(
    LIBMATTI_FML_ModClassVisitor *visitor, int access, const char *name, const char *desc, const char *signature)
{
    (void) access;
    (void) signature;

    return LIBMATTI_FML_ModMethodVisitor_New(name, desc, &visitor->annotations);
}

// Java: public void buildData(Set<ClassData> classes, Set<AnnotationData> annotations)
void LIBMATTI_FML_ModClassVisitor_BuildData(LIBMATTI_FML_ModClassVisitor *visitor,
                                            LIBMATTI_NEOFORGESPI_ModFileScanData *result)
{
    // Java: new ClassData(this.asmType, this.asmSuperType, interfaces) - the Type objects move in
    LIBMATTI_NEOFORGESPI_ClassData classData = {0};
    classData.clazz = LIBMATTI_ASM_Type_GetObjectType(
        LIBMATTI_ASM_Type_GetInternalName(visitor->asmType));
    classData.parent = visitor->asmSuperType != NULL
                           ? LIBMATTI_ASM_Type_GetObjectType(LIBMATTI_ASM_Type_GetInternalName(visitor->asmSuperType))
                           : NULL;
    classData.interfaceCount = visitor->interfaceCount;
    classData.interfaces = calloc(visitor->interfaceCount > 0 ? visitor->interfaceCount : 1, sizeof(LIBMATTI_ASM_Type *));
    for (size_t i = 0; i < visitor->interfaceCount; i++)
        classData.interfaces[i] = LIBMATTI_ASM_Type_GetObjectType(LIBMATTI_ASM_Type_GetInternalName(visitor->interfaces[i]));

    LIBMATTI_NEOFORGESPI_ModFileScanData_AddClass(result, &classData);

    // Java: fromModAnnotation(clazz, annotation) hands the Type over
    char *clazzName = LIBMATTI_ASM_Type_GetClassName(classData.clazz);
    for (size_t i = 0; i < visitor->annotations.count; i++)
    {
        LIBMATTI_NEOFORGESPI_AnnotationData annotation = LIBMATTI_FML_ModAnnotation_FromModAnnotation(
            clazzName, visitor->annotations.items[i]);
        LIBMATTI_NEOFORGESPI_ModFileScanData_AddAnnotation(result, &annotation);
    }
    free(clazzName);
}

// Java: the Object AnnotationNode.accept passes to AnnotationVisitor.visit
static LIBMATTI_FML_ModAnnotationValue *accept_value(const LIBMATTI_ASMT_AnnotationValue *value)
{
    switch (value->tag)
    {
    case 'B':
    case 'C':
    case 'I':
    case 'S':
        return LIBMATTI_FML_ModAnnotationValue_OfInt(value->intValue);
    case 'Z':
        return LIBMATTI_FML_ModAnnotationValue_OfBoolean(value->intValue);
    case 'J':
        return LIBMATTI_FML_ModAnnotationValue_OfLong(value->longValue);
    case 'D':
        return LIBMATTI_FML_ModAnnotationValue_OfDouble(value->doubleValue);
    case 'F':
        return LIBMATTI_FML_ModAnnotationValue_OfFloat((float) value->doubleValue);
    case 's':
        return LIBMATTI_FML_ModAnnotationValue_OfString(value->stringValue);
    case 'c':
        return LIBMATTI_FML_ModAnnotationValue_OfType(value->classDescriptor);
    default:
        return LIBMATTI_FML_ModAnnotationValue_OfString(value->stringValue);
    }
}

// Java: public void accept(AnnotationVisitor av) - the caller hands the visitor over
static void accept_annotation(const LIBMATTI_ASMT_AnnotationNode *node, LIBMATTI_FML_ModAnnotationVisitor *visitor)
{
    if (visitor == NULL) return;

    for (size_t i = 0; i < node->valueCount; i++)
    {
        const char *name = LIBMATTI_ASMT_AnnotationNode_ValueNameAt(node, i);
        LIBMATTI_ASMT_AnnotationValue *value = LIBMATTI_ASMT_AnnotationNode_ValueAt(node, i);

        if (value->tag == 'e')
        {
            LIBMATTI_FML_ModAnnotationVisitor_VisitEnum(visitor, name,
                                                        LIBMATTI_ASMT_AnnotationValue_EnumType(value),
                                                        LIBMATTI_ASMT_AnnotationValue_EnumConst(value));
        }
        else if (value->tag == '@')
        {
            LIBMATTI_ASMT_AnnotationNode *child = LIBMATTI_ASMT_AnnotationValue_Annotation(value);
            accept_annotation(child, LIBMATTI_FML_ModAnnotationVisitor_VisitAnnotation(visitor, name, child->desc));
        }
        else if (value->tag == '[')
        {
            LIBMATTI_FML_ModAnnotationVisitor *array =
                LIBMATTI_FML_ModAnnotationVisitor_VisitArray(visitor, name);
            for (size_t k = 0; k < LIBMATTI_ASMT_AnnotationValue_ArrayCount(value); k++)
            {
                LIBMATTI_ASMT_AnnotationValue *element = LIBMATTI_ASMT_AnnotationValue_ArrayAt(value, k);
                if (element->tag == 'e')
                    LIBMATTI_FML_ModAnnotationVisitor_VisitEnum(
                        array, NULL, LIBMATTI_ASMT_AnnotationValue_EnumType(element),
                        LIBMATTI_ASMT_AnnotationValue_EnumConst(element));
                else if (element->tag == '@')
                    accept_annotation(element->annotation,
                                      LIBMATTI_FML_ModAnnotationVisitor_VisitAnnotation(
                                          array, NULL, element->annotation->desc));
                else
                    LIBMATTI_FML_ModAnnotationVisitor_Visit(array, NULL, accept_value(element));
            }
            LIBMATTI_FML_ModAnnotationVisitor_VisitEnd(array);
            LIBMATTI_FML_ModAnnotationVisitor_Free(array);
        }
        else
        {
            LIBMATTI_FML_ModAnnotationVisitor_Visit(visitor, name, accept_value(value));
        }
    }

    LIBMATTI_FML_ModAnnotationVisitor_VisitEnd(visitor);
    LIBMATTI_FML_ModAnnotationVisitor_Free(visitor);
}

// Java: ClassNode.accept(ClassVisitor) - the port's ClassReader has no ClassVisitor,
// so the ClassNode is replayed into the ModClassVisitor here.
void LIBMATTI_FML_ModClassVisitor_VisitNode(LIBMATTI_FML_ModClassVisitor *visitor,
                                            const LIBMATTI_ASMT_ClassNode *node)
{
    LIBMATTI_FML_ModClassVisitor_Visit(visitor, node->version, node->access, node->name, node->signature,
                                       node->superName, node->interfaces, node->interfaceCount);

    for (size_t i = 0; i < node->visibleAnnotationCount; i++)
        accept_annotation(node->visibleAnnotations[i],
                          LIBMATTI_FML_ModClassVisitor_VisitAnnotation(visitor,
                                                                       node->visibleAnnotations[i]->desc, 1));
    for (size_t i = 0; i < node->invisibleAnnotationCount; i++)
        accept_annotation(node->invisibleAnnotations[i],
                          LIBMATTI_FML_ModClassVisitor_VisitAnnotation(visitor,
                                                                       node->invisibleAnnotations[i]->desc, 0));

    for (size_t i = 0; i < node->fieldCount; i++)
    {
        LIBMATTI_ASMT_FieldNode *field = node->fields[i];
        LIBMATTI_FML_ModFieldVisitor *fieldVisitor =
            LIBMATTI_FML_ModClassVisitor_VisitField(visitor, field->access, field->name, field->desc,
                                                    field->signature);

        for (size_t k = 0; k < field->visibleAnnotationCount; k++)
            accept_annotation(field->visibleAnnotations[k],
                              LIBMATTI_FML_ModFieldVisitor_VisitAnnotation(
                                  fieldVisitor, field->visibleAnnotations[k]->desc, 1));
        for (size_t k = 0; k < field->invisibleAnnotationCount; k++)
            accept_annotation(field->invisibleAnnotations[k],
                              LIBMATTI_FML_ModFieldVisitor_VisitAnnotation(
                                  fieldVisitor, field->invisibleAnnotations[k]->desc, 0));

        LIBMATTI_FML_ModFieldVisitor_Free(fieldVisitor);
    }

    for (size_t i = 0; i < node->methodCount; i++)
    {
        LIBMATTI_ASMT_MethodNode *method = node->methods[i];
        LIBMATTI_FML_ModMethodVisitor *methodVisitor =
            LIBMATTI_FML_ModClassVisitor_VisitMethod(visitor, method->access, method->name, method->desc,
                                                     method->signature);

        for (size_t k = 0; k < method->visibleAnnotationCount; k++)
            accept_annotation(method->visibleAnnotations[k],
                              LIBMATTI_FML_ModMethodVisitor_VisitAnnotation(
                                  methodVisitor, method->visibleAnnotations[k]->desc, 1));
        for (size_t k = 0; k < method->invisibleAnnotationCount; k++)
            accept_annotation(method->invisibleAnnotations[k],
                              LIBMATTI_FML_ModMethodVisitor_VisitAnnotation(
                                  methodVisitor, method->invisibleAnnotations[k]->desc, 0));

        LIBMATTI_FML_ModMethodVisitor_Free(methodVisitor);
    }
}
