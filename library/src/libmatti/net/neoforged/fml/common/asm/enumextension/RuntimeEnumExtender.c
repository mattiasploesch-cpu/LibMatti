// Port of net.neoforged.fml.common.asm.enumextension.RuntimeEnumExtender.

#include "libmatti/net/neoforged/fml/common/asm/enumextension/RuntimeEnumExtender.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/ModLoader.h"
#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"
#include "libmatti/net/neoforged/fml/common/asm/ListGeneratorAdapter.h"
#include "libmatti/net/neoforged/fml/common/asm/enumextension/EnumProxy.h"
#include "libmatti/net/neoforged/fml/common/asm/enumextension/ExtensionInfo.h"
#include "libmatti/net/neoforged/fml/common/asm/enumextension/IExtensibleEnum.h"
#include "libmatti/net/neoforged/fml/common/asm/enumextension/IndexedEnum.h"
#include "libmatti/net/neoforged/fml/common/asm/enumextension/NamedEnum.h"
#include "libmatti/net/neoforged/fml/common/asm/enumextension/NetworkedEnum.h"
#include "libmatti/net/neoforged/fml/common/asm/enumextension/ReservedConstructor.h"
#include "libmatti/net/neoforged/neoforgespi/transformation/ClassProcessorIds.h"
#include "libmatti/org/objectweb/asm/Opcodes.h"
#include "libmatti/org/objectweb/asm/commons/GeneratorAdapter.h"
#include "libmatti/org/objectweb/asm/commons/Method.h"
#include "libmatti/org/objectweb/asm/tree/InsnList.h"
#include "libmatti/org/objectweb/asm/tree/MethodNode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: private static final Type MARKER_IFACE = Type.getType(IExtensibleEnum.class)
static LIBMATTI_ASM_Type *MARKER_IFACE(void)
{
    static LIBMATTI_ASM_Type *type;
    if (type == NULL) type = LIBMATTI_ASM_Type_GetType(LIBMATTI_FML_IExtensibleEnum_DESCRIPTOR);
    return type;
}

// Java: private static final Type INDEXED_ANNOTATION = Type.getType(IndexedEnum.class)
static LIBMATTI_ASM_Type *INDEXED_ANNOTATION(void)
{
    static LIBMATTI_ASM_Type *type;
    if (type == NULL) type = LIBMATTI_ASM_Type_GetType(LIBMATTI_FML_INDEXEDENUM_DESCRIPTOR);
    return type;
}

// Java: private static final Type NAMED_ANNOTATION = Type.getType(NamedEnum.class)
static LIBMATTI_ASM_Type *NAMED_ANNOTATION(void)
{
    static LIBMATTI_ASM_Type *type;
    if (type == NULL) type = LIBMATTI_ASM_Type_GetType(LIBMATTI_FML_NAMEDENUM_DESCRIPTOR);
    return type;
}

// Java: private static final Type RESERVED_ANNOTATION = Type.getType(ReservedConstructor.class)
static LIBMATTI_ASM_Type *RESERVED_ANNOTATION(void)
{
    static LIBMATTI_ASM_Type *type;
    if (type == NULL) type = LIBMATTI_ASM_Type_GetType(LIBMATTI_FML_RESERVEDCONSTRUCTOR_DESCRIPTOR);
    return type;
}

// Java: private static final Type ENUM_PROXY = Type.getType(EnumProxy.class)
static LIBMATTI_ASM_Type *ENUM_PROXY(void)
{
    static LIBMATTI_ASM_Type *type;
    if (type == NULL) type = LIBMATTI_ASM_Type_GetType(LIBMATTI_FML_EnumProxy_DESCRIPTOR);
    return type;
}

// Java: private static final Type NET_CHECK = Type.getType(NetworkedEnum.NetworkCheck.class)
static LIBMATTI_ASM_Type *NET_CHECK(void)
{
    static LIBMATTI_ASM_Type *type;
    if (type == NULL) type = LIBMATTI_ASM_Type_GetType(LIBMATTI_FML_NETWORKEDENUM_NETWORKCHECK_DESCRIPTOR);
    return type;
}

// Java: private static final Type EXT_INFO = Type.getType(ExtensionInfo.class)
static LIBMATTI_ASM_Type *EXT_INFO(void)
{
    static LIBMATTI_ASM_Type *type;
    if (type == NULL) type = LIBMATTI_ASM_Type_GetType(LIBMATTI_FML_ExtensionInfo_DESCRIPTOR);
    return type;
}

// Java: private static final Type NETWORKED_ANNOTATION = Type.getType(NetworkedEnum.class)
static LIBMATTI_ASM_Type *NETWORKED_ANNOTATION(void)
{
    static LIBMATTI_ASM_Type *type;
    if (type == NULL) type = LIBMATTI_ASM_Type_GetType(LIBMATTI_FML_NETWORKEDENUM_DESCRIPTOR);
    return type;
}

// Java: private static final Type EXTENDER = Type.getType(RuntimeEnumExtender.class)
static LIBMATTI_ASM_Type *EXTENDER(void)
{
    static LIBMATTI_ASM_Type *type;
    if (type == NULL) type = LIBMATTI_ASM_Type_GetType("Lnet/neoforged/fml/common/asm/enumextension/RuntimeEnumExtender;");
    return type;
}

// Java: private static final Type ARRAYS = Type.getType("Ljava/util/Arrays;")
static LIBMATTI_ASM_Type *ARRAYS(void)
{
    static LIBMATTI_ASM_Type *type;
    if (type == NULL) type = LIBMATTI_ASM_Type_GetType("Ljava/util/Arrays;");
    return type;
}

// Java: Type.INT_TYPE
static const LIBMATTI_ASM_Type *INT_TYPE(void)
{
    static LIBMATTI_ASM_Type *type;
    if (type == NULL) type = LIBMATTI_ASM_Type_GetType("I");
    return type;
}

// Java: private static final int ENUM_FLAGS = ACC_PUBLIC | ACC_STATIC | ACC_FINAL | ACC_ENUM
#define ENUM_FLAGS (LIBMATTI_ASM_ACC_PUBLIC | LIBMATTI_ASM_ACC_STATIC | LIBMATTI_ASM_ACC_FINAL | LIBMATTI_ASM_ACC_ENUM)
// Java: private static final int ARRAY_FLAGS = ACC_PRIVATE | ACC_STATIC | ACC_FINAL | ACC_SYNTHETIC
#define ARRAY_FLAGS (LIBMATTI_ASM_ACC_PRIVATE | LIBMATTI_ASM_ACC_STATIC | LIBMATTI_ASM_ACC_FINAL | LIBMATTI_ASM_ACC_SYNTHETIC)
// Java: private static final int EXT_INFO_FLAGS = ACC_PRIVATE | ACC_STATIC | ACC_FINAL
#define EXT_INFO_FLAGS (LIBMATTI_ASM_ACC_PRIVATE | LIBMATTI_ASM_ACC_STATIC | LIBMATTI_ASM_ACC_FINAL)

// Java: private static Map<String, List<EnumPrototype>> prototypes = Map.of();
typedef struct
{
    char *enumName;
    LIBMATTI_FML_EnumPrototype *prototypes;
    size_t count;
} prototype_entry;

static prototype_entry *prototypes;
static size_t prototypeCount;

// Java: prototypes.get(internalName) / prototypes.containsKey(internalName)
static prototype_entry *find_entry(const char *enumName)
{
    for (size_t i = 0; i < prototypeCount; i++)
        if (strcmp(prototypes[i].enumName, enumName) == 0) return &prototypes[i];
    return NULL;
}

// Java: map.computeIfAbsent(proto.enumName(), ignored -> new ArrayList<>())
static prototype_entry *compute_if_absent(const char *enumName)
{
    prototype_entry *entry = find_entry(enumName);
    if (entry != NULL) return entry;

    prototypes = realloc(prototypes, sizeof(*prototypes) * (prototypeCount + 1));
    entry = &prototypes[prototypeCount++];
    entry->enumName = strdup(enumName);
    entry->prototypes = NULL;
    entry->count = 0;
    return entry;
}

// Java: the List<EnumPrototype> append
static void entry_add(prototype_entry *entry, const LIBMATTI_FML_EnumPrototype *prototype)
{
    entry->prototypes = realloc(entry->prototypes, sizeof(*entry->prototypes) * (entry->count + 1));
    entry->prototypes[entry->count++] = *prototype;
}

// Java: @Override public ProcessorName name()
static LIBMATTI_NEOFORGESPI_ProcessorName *name(LIBMATTI_NEOFORGESPI_ClassProcessor *self)
{
    (void) self;
    return (LIBMATTI_NEOFORGESPI_ProcessorName *) LIBMATTI_NEOFORGESPI_ClassProcessorIds_RUNTIME_ENUM_EXTENDER();
}

// Java: @Override public Set<ProcessorName> runsBefore() { return Set.of(ClassProcessorIds.MIXIN); }
static const LIBMATTI_NEOFORGESPI_ProcessorName **runs_before(LIBMATTI_NEOFORGESPI_ClassProcessor *self, size_t *count)
{
    static const LIBMATTI_NEOFORGESPI_ProcessorName *names[1];
    (void) self;
    names[0] = LIBMATTI_NEOFORGESPI_ClassProcessorIds_MIXIN();
    *count = 1;
    return names;
}

// Java: @Override public OrderingHint orderingHint() { return OrderingHint.EARLY; }
static LIBMATTI_NEOFORGESPI_ClassProcessor_OrderingHint ordering_hint(LIBMATTI_NEOFORGESPI_ClassProcessor *self)
{
    (void) self;
    return LIBMATTI_NEOFORGESPI_ClassProcessor_OrderingHint_EARLY;
}

// Java: @Override public boolean handlesClass(SelectionContext context)
static int handles_class(LIBMATTI_NEOFORGESPI_ClassProcessor *self,
                         const LIBMATTI_NEOFORGESPI_ClassProcessor_SelectionContext *context)
{
    (void) self;
    if (context->empty) return 0;

    // Java: prototypes.containsKey(context.type().getInternalName())
    char *internalName = LIBMATTI_ASM_Type_GetInternalName(context->type);
    int found = find_entry(internalName) != NULL;
    free(internalName);
    return found;
}

// Java: classNode.interfaces.contains(MARKER_IFACE.getInternalName())
static int has_interface(const LIBMATTI_ASMT_ClassNode *classNode, const char *internalName)
{
    for (size_t i = 0; i < classNode->interfaceCount; i++)
        if (strcmp(classNode->interfaces[i], internalName) == 0) return 1;
    return 0;
}

// Java: public static MethodInsnNode findFirstStaticMethodCall(MethodNode method, String owner, String name, String descriptor)
LIBMATTI_ASMT_MethodInsnNode *LIBMATTI_FML_RuntimeEnumExtender_FindFirstStaticMethodCall(
    const LIBMATTI_ASMT_MethodNode *method, const char *owner, const char *name, const char *descriptor)
{
    for (size_t i = 0; i < method->instructions.size; i++)
    {
        LIBMATTI_ASMT_AbstractInsnNode *node = LIBMATTI_ASM_InsnList_Get(&method->instructions, i);
        if (node->type == LIBMATTI_ASMT_METHOD_INSN && node->opcode == LIBMATTI_ASM_INVOKESTATIC)
        {
            LIBMATTI_ASMT_MethodInsnNode *methodInsnNode = (LIBMATTI_ASMT_MethodInsnNode *) node;
            if (strcmp(methodInsnNode->owner, owner) == 0 &&
                strcmp(methodInsnNode->name, name) == 0 &&
                strcmp(methodInsnNode->desc, descriptor) == 0)
            {
                return methodInsnNode;
            }
        }
    }
    return NULL;
}

// Java: public static AbstractInsnNode findFirstInstructionBefore(MethodNode method, int opCode, int startIndex)
LIBMATTI_ASMT_AbstractInsnNode *LIBMATTI_FML_RuntimeEnumExtender_FindFirstInstructionBefore(
    const LIBMATTI_ASMT_MethodNode *method, int opCode, long long startIndex)
{
    // Java: for (int i = Math.max(method.instructions.size() - 1, startIndex); i >= 0; i--)
    long long start = (long long) method->instructions.size - 1;
    if (startIndex > start) start = startIndex;

    for (long long i = start; i >= 0; i--)
    {
        LIBMATTI_ASMT_AbstractInsnNode *ain = LIBMATTI_ASM_InsnList_Get(&method->instructions, (size_t) i);
        if (ain->opcode == opCode) return ain;
    }
    return NULL;
}

// Java: private static Optional<MethodNode> tryFindMethod(ClassNode classNode, Predicate<MethodNode> predicate)
static LIBMATTI_ASMT_MethodNode *try_find_method(const LIBMATTI_ASMT_ClassNode *classNode, const char *name,
                                                 const char *desc)
{
    for (size_t i = 0; i < classNode->methodCount; i++)
    {
        LIBMATTI_ASMT_MethodNode *mth = classNode->methods[i];
        if (strcmp(mth->name, name) != 0) continue;
        if (desc != NULL && strcmp(mth->desc, desc) != 0) continue;
        return mth;
    }
    return NULL;
}

// Java: private static MethodNode findMethod(ClassNode classNode, Predicate<MethodNode> predicate)
static LIBMATTI_ASMT_MethodNode *find_method(const LIBMATTI_ASMT_ClassNode *classNode, const char *name,
                                             const char *desc)
{
    LIBMATTI_ASMT_MethodNode *mth = try_find_method(classNode, name, desc);
    if (mth == NULL)
    {
        // Java: orElseThrow()
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "NoSuchElementException: No method {} in class {}", name,
                                 classNode->name);
    }
    return mth;
}

// Java: private static FieldNode findField(ClassNode classNode, Predicate<FieldNode> predicate)
static LIBMATTI_ASMT_FieldNode *find_values_array(const LIBMATTI_ASMT_ClassNode *classNode, const char *arrayDescriptor)
{
    for (size_t i = 0; i < classNode->fieldCount; i++)
    {
        LIBMATTI_ASMT_FieldNode *field = classNode->fields[i];
        if ((field->access & ARRAY_FLAGS) == ARRAY_FLAGS && strcmp(field->desc, arrayDescriptor) == 0) return field;
    }
    return NULL;
}

// Java: private static void clearMethod(MethodNode mth)
static void clear_method(LIBMATTI_ASMT_MethodNode *mth)
{
    LIBMATTI_ASM_InsnList_Clear(&mth->instructions);
    // Java: mth.localVariables.clear(); - the port does not model local variables
    free(mth->tryCatchBlocks);
    mth->tryCatchBlocks = NULL;
    mth->tryCatchBlockCount = 0;
}

// Java: classNode.invisibleAnnotations.stream().filter(anno -> anno.desc.equals(annoType.getDescriptor())).findFirst().orElse(null)
static LIBMATTI_ASMT_AnnotationNode *find_annotation(LIBMATTI_ASMT_AnnotationNode *const *annotations, size_t count,
                                                     const LIBMATTI_ASM_Type *annoType)
{
    const char *descriptor = LIBMATTI_ASM_Type_GetDescriptor(annoType);
    for (size_t i = 0; i < count; i++)
        if (strcmp(annotations[i]->desc, descriptor) == 0) return annotations[i];
    return NULL;
}

// Java: private static int getVanillaEntryCount(ClassNode classNode, Type classType)
static int get_vanilla_entry_count(const LIBMATTI_ASMT_ClassNode *classNode, const LIBMATTI_ASM_Type *classType)
{
    const char *descriptor = LIBMATTI_ASM_Type_GetDescriptor(classType);
    int count = 0;
    for (size_t i = 0; i < classNode->fieldCount; i++)
    {
        LIBMATTI_ASMT_FieldNode *field = classNode->fields[i];
        if ((field->access & ENUM_FLAGS) == ENUM_FLAGS && strcmp(field->desc, descriptor) == 0) count++;
        else break;
    }
    return count;
}

// Java: private static int getParameterIndexFromAnnotation(ClassNode classNode, Type annoType)
static int get_parameter_index_from_annotation(const LIBMATTI_ASMT_ClassNode *classNode, const LIBMATTI_ASM_Type *annoType)
{
    if (classNode->invisibleAnnotations == NULL) return -1;

    LIBMATTI_ASMT_AnnotationNode *annotation = find_annotation(classNode->invisibleAnnotations,
                                                              classNode->invisibleAnnotationCount, annoType);
    if (annotation == NULL) return -1;
    if (annotation->values == NULL) return 0;

    for (size_t i = 0; i < annotation->valueCount; i++)
        if (strcmp(annotation->valueNames[i], "value") == 0)
            return LIBMATTI_ASMT_AnnotationValue_AsInt(annotation->values[i]);
    return 0;
}

// Java: private static boolean isAllowedConstructor(MethodNode mth)
static int is_allowed_constructor(const LIBMATTI_ASMT_MethodNode *mth)
{
    if (mth->invisibleAnnotations == NULL) return 1;

    LIBMATTI_ASMT_AnnotationNode *annotation = find_annotation(mth->invisibleAnnotations,
                                                              mth->invisibleAnnotationCount, RESERVED_ANNOTATION());
    return annotation == NULL;
}

// Java: private static AbstractInsnNode findValuesArrayCreation(Type classType, MethodNode clinit)
static LIBMATTI_ASMT_AbstractInsnNode *find_values_array_creation(const LIBMATTI_ASM_Type *classType,
                                                                 const LIBMATTI_ASMT_MethodNode *clinit)
{
    char *internalName = LIBMATTI_ASM_Type_GetInternalName(classType);
    for (size_t i = 0; i < clinit->instructions.size; i++)
    {
        LIBMATTI_ASMT_AbstractInsnNode *ain = LIBMATTI_ASM_InsnList_Get(&clinit->instructions, i);
        if (ain->opcode != LIBMATTI_ASM_ANEWARRAY || ain->type != LIBMATTI_ASMT_TYPE_INSN) continue;

        LIBMATTI_ASMT_TypeInsnNode *tin = (LIBMATTI_ASMT_TypeInsnNode *) ain;
        if (strcmp(tin->desc, internalName) == 0)
        {
            // Java: return tin.getPrevious();
            free(internalName);
            return ain->prev;
        }
    }
    free(internalName);
    // Java: throw new NoSuchElementException("Failed to locate values array creation in enum " + classType)
    LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "NoSuchElementException: Failed to locate values array creation in enum");
    return NULL;
}

// Java: private static FieldInsnNode findValuesArrayStore(Type classType, ClassNode classNode, MethodNode mth, String owner)
static LIBMATTI_ASMT_FieldInsnNode *find_values_array_store(const LIBMATTI_ASM_Type *classType,
                                                           const LIBMATTI_ASMT_ClassNode *classNode,
                                                           const LIBMATTI_ASMT_MethodNode *mth, const char *owner)
{
    // Java: Type.getType("[" + classType.getDescriptor()).getDescriptor()
    const char *typeDescriptor = LIBMATTI_ASM_Type_GetDescriptor(classType);
    size_t arrayLength = strlen(typeDescriptor) + 2;
    char *arrayDescriptor = malloc(arrayLength);
    snprintf(arrayDescriptor, arrayLength, "[%s", typeDescriptor);

    LIBMATTI_ASMT_FieldNode *valuesArray = find_values_array(classNode, arrayDescriptor);
    free(arrayDescriptor);
    if (valuesArray == NULL) return NULL;

    for (size_t i = 0; i < mth->instructions.size; i++)
    {
        LIBMATTI_ASMT_AbstractInsnNode *ain = LIBMATTI_ASM_InsnList_Get(&mth->instructions, i);
        if (ain->opcode != LIBMATTI_ASM_PUTSTATIC || ain->type != LIBMATTI_ASMT_FIELD_INSN) continue;

        LIBMATTI_ASMT_FieldInsnNode *fin = (LIBMATTI_ASMT_FieldInsnNode *) ain;
        if (strcmp(fin->desc, valuesArray->desc) == 0 && strcmp(fin->name, valuesArray->name) == 0 &&
            strcmp(fin->owner, owner) == 0)
        {
            return fin;
        }
    }
    // Java: throw new NoSuchElementException()
    LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "NoSuchElementException: Failed to locate values array store");
    return NULL;
}

// Java: private static void loadConstructorParams(ListGeneratorAdapter generator, int idParamIdx, int nameParamIdx, int ordinal, EnumPrototype proto)
static int load_constructor_params(LIBMATTI_FML_ListGeneratorAdapter *generator, int idParamIdx, int nameParamIdx,
                                   int ordinal, const LIBMATTI_FML_EnumPrototype *proto)
{
    // Java: Type[] argTypes = Type.getType(proto.fullCtorDesc()).getArgumentTypes();
    LIBMATTI_ASM_Type *methodType = LIBMATTI_ASM_Type_GetType(proto->fullCtorDesc);
    size_t argCount = 0;
    LIBMATTI_ASM_Type **argTypes = LIBMATTI_ASM_Type_GetArgumentTypes(methodType, &argCount);
    LIBMATTI_ASM_Type_Free(methodType);

    int result = 1;

    switch (proto->ctorParams->kind)
    {
        case LIBMATTI_FML_ENUMPARAMETERS_FIELD_REFERENCE:
        {
            const LIBMATTI_ASM_Type *owner = proto->ctorParams->owner;
            const char *fieldName = proto->ctorParams->memberName;
            for (size_t idx = 2; idx < argCount; idx++)
            {
                if ((int) idx - 2 == idParamIdx)
                {
                    LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushInt(generator, ordinal);
                    continue;
                }

                LIBMATTI_ASM_COMMONS_GeneratorAdapter_GetStatic(generator, owner, fieldName, ENUM_PROXY());
                LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushInt(generator, (int) idx - 2);
                LIBMATTI_ASM_COMMONS_Method *getParameter = LIBMATTI_ASM_COMMONS_Method_New(
                    LIBMATTI_FML_EnumProxy_GET_PARAMETER_NAME, LIBMATTI_FML_EnumProxy_GET_PARAMETER_DESCRIPTOR);
                LIBMATTI_ASM_COMMONS_GeneratorAdapter_InvokeVirtual(generator, ENUM_PROXY(), getParameter);
                LIBMATTI_ASM_COMMONS_Method_Free(getParameter);
                LIBMATTI_ASM_COMMONS_GeneratorAdapter_Unbox(generator, argTypes[idx]);
                if ((int) idx - 2 == nameParamIdx)
                {
                    LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushString(generator, proto->owningMod);
                    LIBMATTI_ASM_COMMONS_Method *validate = LIBMATTI_ASM_COMMONS_Method_New(
                        "validateNameParameter", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
                    LIBMATTI_ASM_COMMONS_GeneratorAdapter_InvokeStatic(generator, EXTENDER(), validate);
                    LIBMATTI_ASM_COMMONS_Method_Free(validate);
                }
            }
            break;
        }
        case LIBMATTI_FML_ENUMPARAMETERS_METHOD_REFERENCE:
        {
            const LIBMATTI_ASM_Type *owner = proto->ctorParams->owner;
            const char *methodName = proto->ctorParams->memberName;
            for (size_t idx = 2; idx < argCount; idx++)
            {
                if ((int) idx - 2 == idParamIdx)
                {
                    LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushInt(generator, ordinal);
                    continue;
                }

                LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushInt(generator, (int) idx - 2);
                LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushType(generator, argTypes[idx]);
                LIBMATTI_ASM_COMMONS_Method *reference = LIBMATTI_ASM_COMMONS_Method_New(
                    methodName, "(ILjava/lang/Class;)Ljava/lang/Object;");
                LIBMATTI_ASM_COMMONS_GeneratorAdapter_InvokeStatic(generator, owner, reference);
                LIBMATTI_ASM_COMMONS_Method_Free(reference);
                LIBMATTI_ASM_COMMONS_GeneratorAdapter_Unbox(generator, argTypes[idx]);
                if ((int) idx - 2 == nameParamIdx)
                {
                    LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushString(generator, proto->owningMod);
                    LIBMATTI_ASM_COMMONS_Method *validate = LIBMATTI_ASM_COMMONS_Method_New(
                        "validateNameParameter", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
                    LIBMATTI_ASM_COMMONS_GeneratorAdapter_InvokeStatic(generator, EXTENDER(), validate);
                    LIBMATTI_ASM_COMMONS_Method_Free(validate);
                }
            }
            break;
        }
        case LIBMATTI_FML_ENUMPARAMETERS_CONSTANT:
        {
            const LIBMATTI_FML_EnumParameters_Constant *paramList = proto->ctorParams->params;
            for (size_t idx = 2; idx < argCount; idx++)
            {
                if ((int) idx - 2 == idParamIdx)
                {
                    // Java: if (!(paramList.get(idx - 2) instanceof Integer i) || i != -1) throw new IllegalArgumentException(...)
                    if (paramList[idx - 2].type != LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_INTEGER ||
                        paramList[idx - 2].intValue != -1)
                    {
                        LIBMATTI_ML_Logger_Error(LOGGER(), NULL,
                                                 "IllegalArgumentException: Expected -1 as ID parameter at index {} in "
                                                 "parameters for field '{}' in enum '{}' specified by mod '{}'",
                                                 (int) idx - 2, proto->fieldName, proto->enumName, proto->owningMod);
                        result = 0;
                        break;
                    }
                    LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushInt(generator, ordinal);
                    continue;
                }

                if ((int) idx - 2 == nameParamIdx)
                {
                    // Java: if (!(paramList.get(idx - 2) instanceof String str)) throw new IllegalArgumentException(...)
                    if (paramList[idx - 2].type != LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_STRING)
                    {
                        LIBMATTI_ML_Logger_Error(LOGGER(), NULL,
                                                 "IllegalArgumentException: Expected String at index {} in parameters "
                                                 "for field '{}' in enum '{}' specified by mod '{}'",
                                                 (int) idx - 2, proto->fieldName, proto->enumName, proto->owningMod);
                        result = 0;
                        break;
                    }
                    LIBMATTI_FML_RuntimeEnumExtender_ValidateNameParameter(paramList[idx - 2].stringValue,
                                                                          proto->owningMod);
                }

                // Java: switch (paramList.get(idx - 2))
                const LIBMATTI_FML_EnumParameters_Constant *constant = &paramList[idx - 2];
                switch (constant->type)
                {
                    case LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_NULL:
                        LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushString(generator, NULL);
                        break;
                    case LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_STRING:
                        LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushString(generator, constant->stringValue);
                        break;
                    case LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_CHARACTER:
                    case LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_BYTE:
                    case LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_SHORT:
                    case LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_INTEGER:
                        LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushInt(generator, constant->intValue);
                        break;
                    case LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_LONG:
                        LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushLong(generator, constant->longValue);
                        break;
                    case LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_FLOAT:
                        LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushFloat(generator, constant->floatValue);
                        break;
                    case LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_DOUBLE:
                        LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushDouble(generator, constant->doubleValue);
                        break;
                    case LIBMATTI_FML_ENUMPARAMETERS_CONSTANT_BOOLEAN:
                        LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushBool(generator, constant->intValue != 0);
                        break;
                    default:
                        // Java: default -> throw new IllegalArgumentException("Unsupported constant type ...")
                        LIBMATTI_ML_Logger_Error(LOGGER(), NULL,
                                                 "IllegalArgumentException: Unsupported constant type in parameters for "
                                                 "field '{}' in enum '{}' specified by mod '{}'",
                                                 proto->fieldName, proto->enumName, proto->owningMod);
                        result = 0;
                        break;
                }
                if (!result) break;
            }
            break;
        }
    }

    for (size_t i = 0; i < argCount; i++) LIBMATTI_ASM_Type_Free(argTypes[i]);
    free(argTypes);
    return result;
}

// Java: private static List<FieldNode> createEnumEntries(Type classType, ListGeneratorAdapter generator, Set<String> ctors, int idParamIdx, int nameParamIdx, int vanillaEntryCount, List<EnumPrototype> prototypes)
static LIBMATTI_ASMT_FieldNode **create_enum_entries(const LIBMATTI_ASM_Type *classType,
                                                    LIBMATTI_FML_ListGeneratorAdapter *generator, const char **ctors,
                                                    size_t ctorCount, int idParamIdx, int nameParamIdx,
                                                    int vanillaEntryCount,
                                                    const LIBMATTI_FML_EnumPrototype *protos, size_t protoCount,
                                                    size_t *outCount)
{
    LIBMATTI_ASMT_FieldNode **enumFields = malloc(sizeof(*enumFields) * (protoCount > 0 ? protoCount : 1));
    size_t enumFieldCount = 0;
    int ordinal = vanillaEntryCount;

    for (size_t i = 0; i < protoCount; i++)
    {
        const LIBMATTI_FML_EnumPrototype *proto = &protos[i];

        // Java: if (!ctors.contains(proto.fullCtorDesc())) throw new IllegalArgumentException(...)
        int allowed = 0;
        for (size_t j = 0; j < ctorCount; j++)
            if (strcmp(ctors[j], proto->fullCtorDesc) == 0) { allowed = 1; break; }
        if (!allowed)
        {
            LIBMATTI_ML_Logger_Error(LOGGER(), NULL,
                                     "IllegalArgumentException: Invalid, non-existant or disallowed constructor '{}' "
                                     "for field '{}' in enum '{}' specified by mod '{}'",
                                     proto->ctorDesc, proto->fieldName, proto->enumName, proto->owningMod);
            for (size_t j = 0; j < enumFieldCount; j++) LIBMATTI_ASMT_FieldNode_Free(enumFields[j]);
            free(enumFields);
            *outCount = 0;
            return NULL;
        }

        const char *fieldName = proto->fieldName;
        LIBMATTI_ASMT_FieldNode *field = LIBMATTI_ASMT_FieldNode_New(ENUM_FLAGS, fieldName,
                                                                     LIBMATTI_ASM_Type_GetDescriptor(classType), NULL,
                                                                     NULL);
        enumFields[enumFieldCount++] = field;

        // Java: NEW_FIELD = new EnumType(fieldName, ordinal, ...);
        LIBMATTI_ASM_COMMONS_GeneratorAdapter_NewInstance(generator, classType);
        LIBMATTI_ASM_COMMONS_GeneratorAdapter_Dup(generator);
        LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushString(generator, fieldName);
        LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushInt(generator, ordinal);
        if (!load_constructor_params(generator, idParamIdx, nameParamIdx, ordinal, proto))
        {
            for (size_t j = 0; j < enumFieldCount; j++) LIBMATTI_ASMT_FieldNode_Free(enumFields[j]);
            free(enumFields);
            *outCount = 0;
            return NULL;
        }
        LIBMATTI_ASM_COMMONS_Method *constructor = LIBMATTI_ASM_COMMONS_Method_New(
            LIBMATTI_ASM_COMMONS_METHOD_CONSTRUCTOR_NAME, proto->fullCtorDesc);
        LIBMATTI_ASM_COMMONS_GeneratorAdapter_InvokeConstructor(generator, classType, constructor);
        LIBMATTI_ASM_COMMONS_Method_Free(constructor);
        LIBMATTI_ASM_COMMONS_GeneratorAdapter_PutStatic(generator, classType, field->name, classType);

        ordinal++;
    }

    *outCount = enumFieldCount;
    return enumFields;
}

// Java: private static void buildExtensionInfo(ClassNode classNode, Type classType, ListGeneratorAdapter generator, FieldNode infoField, int vanillaCount, int moddedCount)
static void build_extension_info(const LIBMATTI_ASMT_ClassNode *classNode, const LIBMATTI_ASM_Type *classType,
                                 LIBMATTI_FML_ListGeneratorAdapter *generator,
                                 const LIBMATTI_ASMT_FieldNode *infoField, int vanillaCount, int moddedCount)
{
    const char *netCheckValue = NULL;
    if (classNode->visibleAnnotations != NULL)
    {
        LIBMATTI_ASMT_AnnotationNode *annotation = find_annotation(classNode->visibleAnnotations,
                                                                   classNode->visibleAnnotationCount,
                                                                   NETWORKED_ANNOTATION());
        if (annotation != NULL)
        {
            if (annotation->values == NULL)
            {
                // Java: throw new IllegalStateException("Expected values on NetworkedEnum annotation")
                LIBMATTI_ML_Logger_Error(LOGGER(), NULL,
                                         "IllegalStateException: Expected values on NetworkedEnum annotation");
                return;
            }

            for (size_t i = 0; i < annotation->valueCount; i++)
            {
                if (strcmp(annotation->valueNames[i], LIBMATTI_FML_NETWORKEDENUM_VALUE) == 0)
                {
                    // Java: String[] value = (String[]) anno.values.get(i + 1); return Optional.of(value[1]);
                    netCheckValue = LIBMATTI_ASMT_AnnotationValue_EnumConst(annotation->values[i]);
                    break;
                }
            }

            if (netCheckValue == NULL)
            {
                // Java: throw new IllegalStateException("Expected NetworkedEnum.NetworkCheck value on NetworkedEnum annotation")
                LIBMATTI_ML_Logger_Error(LOGGER(), NULL,
                                         "IllegalStateException: Expected NetworkedEnum.NetworkCheck value on "
                                         "NetworkedEnum annotation");
                return;
            }
        }
    }

    LIBMATTI_ASM_COMMONS_GeneratorAdapter_NewInstance(generator, EXT_INFO());
    LIBMATTI_ASM_COMMONS_GeneratorAdapter_Dup(generator);
    LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushBool(generator, moddedCount > 0);
    LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushInt(generator, vanillaCount);
    LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushInt(generator, vanillaCount + moddedCount);
    if (netCheckValue != NULL)
    {
        LIBMATTI_ASM_COMMONS_GeneratorAdapter_GetStatic(generator, NET_CHECK(), netCheckValue, NET_CHECK());
    }
    else
    {
        LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushType(generator, NULL);
    }
    LIBMATTI_ASM_COMMONS_Method *constructor = LIBMATTI_ASM_COMMONS_Method_New(
        LIBMATTI_ASM_COMMONS_METHOD_CONSTRUCTOR_NAME, LIBMATTI_FML_ExtensionInfo_CONSTRUCTOR_DESCRIPTOR);
    LIBMATTI_ASM_COMMONS_GeneratorAdapter_InvokeConstructor(generator, EXT_INFO(), constructor);
    LIBMATTI_ASM_COMMONS_Method_Free(constructor);
    LIBMATTI_ASM_COMMONS_GeneratorAdapter_PutStatic(generator, classType, infoField->name, EXT_INFO());
}

// Java: private static void returnValuesToExtender(Type classType, ListGeneratorAdapter generator, List<EnumPrototype> protos, List<FieldNode> entries)
static void return_values_to_extender(const LIBMATTI_ASM_Type *classType, LIBMATTI_FML_ListGeneratorAdapter *generator,
                                      const LIBMATTI_FML_EnumPrototype *protos, size_t protoCount,
                                      LIBMATTI_ASMT_FieldNode **entries)
{
    for (size_t i = 0; i < protoCount; i++)
    {
        if (protos[i].ctorParams->kind != LIBMATTI_FML_ENUMPARAMETERS_FIELD_REFERENCE) continue;

        LIBMATTI_ASMT_FieldNode *field = entries[i];
        LIBMATTI_ASM_COMMONS_GeneratorAdapter_GetStatic(generator, protos[i].ctorParams->owner,
                                                        protos[i].ctorParams->memberName, ENUM_PROXY());
        LIBMATTI_ASM_COMMONS_GeneratorAdapter_GetStatic(generator, classType, field->name, classType);
        LIBMATTI_ASM_COMMONS_Method *setValue = LIBMATTI_ASM_COMMONS_Method_New(
            LIBMATTI_FML_EnumProxy_SET_VALUE_NAME, LIBMATTI_FML_EnumProxy_SET_VALUE_DESCRIPTOR);
        LIBMATTI_ASM_COMMONS_GeneratorAdapter_InvokeVirtual(generator, ENUM_PROXY(), setValue);
        LIBMATTI_ASM_COMMONS_Method_Free(setValue);
    }
}

// Java: private static void appendValuesArray(Type classType, ListGeneratorAdapter generator, List<FieldNode> enumEntries)
static void append_values_array(const LIBMATTI_ASM_Type *classType, LIBMATTI_FML_ListGeneratorAdapter *generator,
                                LIBMATTI_ASMT_FieldNode **enumEntries, size_t entryCount)
{
    // Java: values = Arrays.copyOf(values, values.length + listSize);
    LIBMATTI_ASM_COMMONS_GeneratorAdapter_Dup(generator);
    LIBMATTI_ASM_COMMONS_GeneratorAdapter_ArrayLength(generator);
    LIBMATTI_ASM_COMMONS_GeneratorAdapter_PushInt(generator, (int) entryCount);
    LIBMATTI_ASM_COMMONS_GeneratorAdapter_Math(generator, LIBMATTI_ASM_COMMONS_ADD, INT_TYPE());
    LIBMATTI_ASM_COMMONS_Method *copyOf = LIBMATTI_ASM_COMMONS_Method_New(
        "copyOf", "([Ljava/lang/Object;I)[Ljava/lang/Object;");
    LIBMATTI_ASM_COMMONS_GeneratorAdapter_InvokeStatic(generator, ARRAYS(), copyOf);
    LIBMATTI_ASM_COMMONS_Method_Free(copyOf);

    // Java: generator.checkCast(Type.getType("[" + classType.getDescriptor()));
    const char *typeDescriptor = LIBMATTI_ASM_Type_GetDescriptor(classType);
    size_t arrayLength = strlen(typeDescriptor) + 2;
    char *arrayDescriptor = malloc(arrayLength);
    snprintf(arrayDescriptor, arrayLength, "[%s", typeDescriptor);
    LIBMATTI_ASM_Type *arrayType = LIBMATTI_ASM_Type_GetType(arrayDescriptor);
    LIBMATTI_ASM_COMMONS_GeneratorAdapter_CheckCast(generator, arrayType);
    LIBMATTI_ASM_Type_Free(arrayType);
    free(arrayDescriptor);

    // Java: values[NEW_FIELD.ordinal()] = NEW_FIELD;
    for (size_t i = 0; i < entryCount; i++)
    {
        LIBMATTI_ASMT_FieldNode *entry = enumEntries[i];
        LIBMATTI_ASM_COMMONS_GeneratorAdapter_Dup(generator);
        LIBMATTI_ASM_COMMONS_GeneratorAdapter_GetStatic(generator, classType, entry->name, classType);
        LIBMATTI_ASM_COMMONS_GeneratorAdapter_Dup(generator);
        LIBMATTI_ASM_COMMONS_Method *ordinal = LIBMATTI_ASM_COMMONS_Method_New("ordinal", "()I");
        LIBMATTI_ASM_COMMONS_GeneratorAdapter_InvokeVirtual(generator, classType, ordinal);
        LIBMATTI_ASM_COMMONS_Method_Free(ordinal);
        LIBMATTI_ASM_COMMONS_GeneratorAdapter_Swap(generator);
        LIBMATTI_ASM_COMMONS_GeneratorAdapter_ArrayStore(generator, classType);
    }
}

// Java: @Override public ComputeFlags processClass(TransformationContext context)
static LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags process_class(
    LIBMATTI_NEOFORGESPI_ClassProcessor *self, LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext *context)
{
    (void) self;

    LIBMATTI_ASMT_ClassNode *classNode = LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_Node(context);
    const LIBMATTI_ASM_Type *classType = LIBMATTI_NEOFORGESPI_ClassProcessor_TransformationContext_Type(context);

    if ((classNode->access & LIBMATTI_ASM_ACC_ENUM) == 0 ||
        !has_interface(classNode, LIBMATTI_FML_IExtensibleEnum_INTERNAL_NAME))
    {
        char *className = LIBMATTI_ASM_Type_GetClassName(classType);
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL,
                                 "IllegalStateException: Tried to extend non-enum class or non-extensible enum: {}",
                                 className);
        free(className);
        return LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_NO_REWRITE;
    }

    char *internalName = LIBMATTI_ASM_Type_GetInternalName(classType);
    prototype_entry *entry = find_entry(internalName);
    if (entry == NULL || entry->count == 0)
    {
        free(internalName);
        return LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_NO_REWRITE;
    }

    LIBMATTI_ASMT_MethodNode *clinit = find_method(classNode, "<clinit>", NULL);
    LIBMATTI_ASMT_MethodNode *values = try_find_method(classNode, "$values", NULL);
    int valuesPresent = values != NULL;
    LIBMATTI_ASMT_MethodNode *getExtInfo = find_method(classNode, "getExtensionInfo",
                                                       LIBMATTI_FML_ExtensionInfo_GETTER_DESCRIPTOR);
    if (clinit == NULL || getExtInfo == NULL)
    {
        free(internalName);
        return LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_NO_REWRITE;
    }

    // Java: Set<String> ctors = classNode.methods.stream().filter(name.equals("<init>")).filter(isAllowedConstructor).map(desc).collect(toSet());
    const char **ctors = NULL;
    size_t ctorCount = 0;
    for (size_t i = 0; i < classNode->methodCount; i++)
    {
        LIBMATTI_ASMT_MethodNode *mth = classNode->methods[i];
        if (strcmp(mth->name, LIBMATTI_ASM_COMMONS_METHOD_CONSTRUCTOR_NAME) != 0) continue;
        if (!is_allowed_constructor(mth)) continue;
        ctors = realloc(ctors, sizeof(*ctors) * (ctorCount + 1));
        ctors[ctorCount++] = mth->desc;
    }

    int vanillaEntryCount = get_vanilla_entry_count(classNode, classType);
    int idParamIdx = get_parameter_index_from_annotation(classNode, INDEXED_ANNOTATION());
    int nameParamIdx = get_parameter_index_from_annotation(classNode, NAMED_ANNOTATION());

    if (idParamIdx != -1 && idParamIdx == nameParamIdx)
    {
        char *className = LIBMATTI_ASM_Type_GetClassName(classType);
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL,
                                 "IllegalStateException: ID and name parameter cannot have the same index on enum {}",
                                 className);
        free(className);
        free(ctors);
        free(internalName);
        return LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_NO_REWRITE;
    }

    LIBMATTI_ASMT_FieldNode *infoField = LIBMATTI_ASMT_FieldNode_New(EXT_INFO_FLAGS, "FML$ENUM_EXT_INFO",
                                                                    LIBMATTI_FML_ExtensionInfo_DESCRIPTOR, NULL, NULL);
    LIBMATTI_ASMT_ClassNode_InsertField(classNode, classNode->fieldCount, infoField);

    clear_method(getExtInfo);
    LIBMATTI_ASMT_InsnList *getExtInfoInsnList = &getExtInfo->instructions;
    LIBMATTI_ASM_InsnList_Add(getExtInfoInsnList,
                              (LIBMATTI_ASMT_AbstractInsnNode *) LIBMATTI_ASMT_FieldInsnNode_New(
                                  LIBMATTI_ASM_GETSTATIC, internalName, infoField->name, infoField->desc));
    LIBMATTI_ASM_InsnList_Add(getExtInfoInsnList,
                              (LIBMATTI_ASMT_AbstractInsnNode *) LIBMATTI_ASMT_InsnNode_New(LIBMATTI_ASM_ARETURN));

    LIBMATTI_ASMT_InsnList *clinitList = calloc(1, sizeof(*clinitList));
    LIBMATTI_FML_ListGeneratorAdapter *clinitGenerator = LIBMATTI_FML_ListGeneratorAdapter_New(clinitList);
    size_t enumEntryCount = 0;
    LIBMATTI_ASMT_FieldNode **enumEntries = create_enum_entries(classType, clinitGenerator, ctors, ctorCount, idParamIdx,
                                                               nameParamIdx, vanillaEntryCount, entry->prototypes,
                                                               entry->count, &enumEntryCount);
    free(ctors);
    if (enumEntries == NULL)
    {
        LIBMATTI_FML_ListGeneratorAdapter_Free(clinitGenerator);
        free(clinitList);
        free(internalName);
        return LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_NO_REWRITE;
    }

    if (valuesPresent)
    {
        // Java: javac
        LIBMATTI_ASMT_MethodInsnNode *valuesInsn = LIBMATTI_FML_RuntimeEnumExtender_FindFirstStaticMethodCall(
            clinit, internalName, values->name, values->desc);
        // Java: clinit.instructions.insertBefore($valuesInsn, ...) - the port skips a missing anchor
        if (valuesInsn != NULL)
            LIBMATTI_ASM_InsnList_InsertBeforeList(&clinit->instructions,
                                                   (LIBMATTI_ASMT_AbstractInsnNode *) valuesInsn, clinitList);
    }
    else
    {
        // Java: ECJ
        LIBMATTI_ASMT_AbstractInsnNode *firstValuesArrayInsn = find_values_array_creation(classType, clinit);
        if (firstValuesArrayInsn != NULL)
            LIBMATTI_ASM_InsnList_InsertBeforeList(&clinit->instructions, firstValuesArrayInsn, clinitList);
    }
    LIBMATTI_FML_ListGeneratorAdapter_Free(clinitGenerator);
    free(clinitList);

    LIBMATTI_ASMT_InsnList *tailList = calloc(1, sizeof(*tailList));
    LIBMATTI_FML_ListGeneratorAdapter *tailGenerator = LIBMATTI_FML_ListGeneratorAdapter_New(tailList);
    build_extension_info(classNode, classType, tailGenerator, infoField, vanillaEntryCount, (int) entry->count);
    return_values_to_extender(classType, tailGenerator, entry->prototypes, entry->count, enumEntries);
    LIBMATTI_ASMT_AbstractInsnNode *clinitRetNode = LIBMATTI_FML_RuntimeEnumExtender_FindFirstInstructionBefore(
        clinit, LIBMATTI_ASM_RETURN, (long long) clinit->instructions.size - 1);
    if (clinitRetNode != NULL) LIBMATTI_ASM_InsnList_InsertBeforeList(&clinit->instructions, clinitRetNode, tailList);
    LIBMATTI_FML_ListGeneratorAdapter_Free(tailGenerator);
    free(tailList);

    for (size_t i = 0; i < enumEntryCount; i++)
        LIBMATTI_ASMT_ClassNode_InsertField(classNode, (size_t) vanillaEntryCount + i, enumEntries[i]);

    LIBMATTI_ASMT_InsnList *appendList = calloc(1, sizeof(*appendList));
    LIBMATTI_FML_ListGeneratorAdapter *appendValuesGenerator = LIBMATTI_FML_ListGeneratorAdapter_New(appendList);
    append_values_array(classType, appendValuesGenerator, enumEntries, enumEntryCount);
    if (valuesPresent)
    {
        // Java: javac
        LIBMATTI_ASMT_AbstractInsnNode *valuesAretInsn = LIBMATTI_FML_RuntimeEnumExtender_FindFirstInstructionBefore(
            values, LIBMATTI_ASM_ARETURN, (long long) values->instructions.size - 1);
        if (valuesAretInsn != NULL)
            LIBMATTI_ASM_InsnList_InsertBeforeList(&values->instructions, valuesAretInsn, appendList);
    }
    else
    {
        // Java: ECJ
        LIBMATTI_ASMT_FieldInsnNode *putStaticInsn = find_values_array_store(classType, classNode, clinit,
                                                                            internalName);
        if (putStaticInsn != NULL)
            LIBMATTI_ASM_InsnList_InsertBeforeList(&clinit->instructions,
                                                   (LIBMATTI_ASMT_AbstractInsnNode *) putStaticInsn, appendList);
    }
    LIBMATTI_FML_ListGeneratorAdapter_Free(appendValuesGenerator);
    free(appendList);

    free(enumEntries);
    free(internalName);

    return LIBMATTI_NEOFORGESPI_ClassProcessor_ComputeFlags_COMPUTE_FRAMES;
}

LIBMATTI_FML_RuntimeEnumExtender *LIBMATTI_FML_RuntimeEnumExtender_New(void)
{
    LIBMATTI_FML_RuntimeEnumExtender *extender = calloc(1, sizeof(LIBMATTI_FML_RuntimeEnumExtender));
    extender->processor.name = name;
    extender->processor.runsBefore = runs_before;
    extender->processor.orderingHint = ordering_hint;
    extender->processor.handlesClass = handles_class;
    extender->processor.processClass = process_class;
    return extender;
}

void LIBMATTI_FML_RuntimeEnumExtender_Free(LIBMATTI_FML_RuntimeEnumExtender *extender)
{
    free(extender);
}

// Java: public static void loadEnumPrototypes(Map<IModInfo, JarResource> paths)
void LIBMATTI_FML_RuntimeEnumExtender_LoadEnumPrototypes(const LIBMATTI_NEOFORGESPI_IModInfo **mods,
                                                        const LIBMATTI_FML_JarResource **resources, size_t count)
{
    for (size_t i = 0; i < prototypeCount; i++)
    {
        free(prototypes[i].enumName);
        free(prototypes[i].prototypes);
    }
    free(prototypes);
    prototypes = NULL;
    prototypeCount = 0;

    // Java: paths.entrySet().stream().map(entry -> EnumPrototype.load(...)).flatMap(List::stream).sorted()
    LIBMATTI_FML_EnumPrototype *all = NULL;
    size_t allCount = 0;
    for (size_t i = 0; i < count; i++)
    {
        size_t loaded = 0;
        LIBMATTI_FML_EnumPrototype *loadedPrototypes = LIBMATTI_FML_EnumPrototype_Load(mods[i], resources[i], &loaded);
        if (loaded == 0)
        {
            free(loadedPrototypes);
            continue;
        }

        all = realloc(all, sizeof(*all) * (allCount + loaded));
        for (size_t j = 0; j < loaded; j++) all[allCount++] = loadedPrototypes[j];
        free(loadedPrototypes);
    }

    // Java: a stable sort of Comparable
    for (size_t i = 1; i < allCount; i++)
    {
        LIBMATTI_FML_EnumPrototype current = all[i];
        size_t j = i;
        while (j > 0 && LIBMATTI_FML_EnumPrototype_Compare(&all[j - 1], &current) > 0)
        {
            all[j] = all[j - 1];
            j--;
        }
        all[j] = current;
    }

    // Java: reduce into HashMap<String, List<EnumPrototype>>
    for (size_t i = 0; i < allCount; i++) entry_add(compute_if_absent(all[i].enumName), &all[i]);
    free(all);

    // Java: the duplicate detection over the map entries
    size_t kept = 0;
    for (size_t i = 0; i < prototypeCount; i++)
    {
        prototype_entry *entry = &prototypes[i];

        LIBMATTI_FML_EnumPrototype **distinctPrototypes = NULL;
        size_t distinctCount = 0;
        int foundDupe = 0;
        for (size_t j = 0; j < entry->count; j++)
        {
            LIBMATTI_FML_EnumPrototype *proto = &entry->prototypes[j];

            LIBMATTI_FML_EnumPrototype *prevProto = NULL;
            for (size_t k = 0; k < distinctCount; k++)
            {
                if (strcmp(distinctPrototypes[k]->fieldName, proto->fieldName) == 0)
                {
                    prevProto = distinctPrototypes[k];
                    break;
                }
            }

            if (prevProto == NULL)
            {
                distinctPrototypes = realloc(distinctPrototypes, sizeof(*distinctPrototypes) * (distinctCount + 1));
                distinctPrototypes[distinctCount++] = proto;
                continue;
            }

            foundDupe = 1;
            const char *args[] = {proto->fieldName, proto->enumName, proto->owningMod, prevProto->owningMod};
            LIBMATTI_FML_ModLoader_AddLoadingIssue(
                LIBMATTI_FML_ModLoadingIssue_Error("fml.modloadingissue.enumextender.duplicate", args, 4));
        }
        free(distinctPrototypes);

        if (foundDupe)
        {
            // Java: erroredEnums.forEach(prototypes::remove)
            for (size_t j = 0; j < entry->count; j++) LIBMATTI_FML_EnumPrototype_Free(&entry->prototypes[j]);
            free(entry->enumName);
            free(entry->prototypes);
            continue;
        }

        if (kept != i) prototypes[kept] = *entry;
        kept++;
    }
    prototypeCount = kept;
}

// Java: public static String validateNameParameter(String fieldName, String owningMod)
const char *LIBMATTI_FML_RuntimeEnumExtender_ValidateNameParameter(const char *fieldName, const char *owningMod)
{
    // Java: if (!fieldName.startsWith(owningMod + ":")) throw new IllegalArgumentException(...)
    size_t prefixLength = strlen(owningMod) + 1;
    char *prefix = malloc(prefixLength + 1);
    snprintf(prefix, prefixLength + 1, "%s:", owningMod);
    if (strncmp(fieldName, prefix, prefixLength) != 0)
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL,
                                 "IllegalArgumentException: Name parameter must be prefixed by mod ID: '{}' provided by "
                                 "mod '{}'",
                                 fieldName, owningMod);
    }
    free(prefix);
    return fieldName;
}
