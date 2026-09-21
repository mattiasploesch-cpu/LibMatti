// Port of net.neoforged.fml.classloading.transformation.TransformerClassWriter.

#include "libmatti/net/neoforged/fml/classloading/transformation/TransformerClassWriter.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/java/lang/Class.h"
#include "libmatti/org/objectweb/asm/ClassReader.h"
#include "libmatti/org/objectweb/asm/Opcodes.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: private static final Map<String, String> CLASS_PARENTS
//       private static final Map<String, Set<String>> CLASS_HIERARCHIES
//       private static final Map<String, Boolean> IS_INTERFACE
typedef struct
{
    char *name;
    char *parent;
    int isInterface;
    char **hierarchies;
    size_t hierarchyCount;
} HierarchyEntry;

static HierarchyEntry *hierarchies = NULL;
static size_t hierarchyCount = 0;

static HierarchyEntry *hierarchy_for(const char *name)
{
    for (size_t i = 0; i < hierarchyCount; i++)
        if (strcmp(hierarchies[i].name, name) == 0) return &hierarchies[i];
    return NULL;
}

static HierarchyEntry *hierarchy_add(const char *name)
{
    HierarchyEntry *existing = hierarchy_for(name);
    if (existing != NULL) return existing;

    hierarchies = realloc(hierarchies, sizeof(*hierarchies) * (hierarchyCount + 1));
    HierarchyEntry *entry = &hierarchies[hierarchyCount++];
    entry->name = strdup(name);
    entry->parent = NULL;
    entry->isInterface = 0;
    entry->hierarchies = NULL;
    entry->hierarchyCount = 0;
    return entry;
}static void hierarchy_add_super(HierarchyEntry *entry, const char *superName)
{
    if (superName == NULL) return;
    for (size_t i = 0; i < entry->hierarchyCount; i++)
        if (strcmp(entry->hierarchies[i], superName) == 0) return;


    entry->hierarchies = realloc(entry->hierarchies, sizeof(*entry->hierarchies) * (entry->hierarchyCount + 1));
    entry->hierarchies[entry->hierarchyCount++] = strdup(superName);
}

// Java: private void computeHierarchyFromClass(String name, Class<?> clazz)
static void compute_hierarchy_from_class(const char *className, LIBMATTI_JL_Class *clazz);

LIBMATTI_FML_TransformerClassWriter *LIBMATTI_FML_TransformerClassWriter_New(
    int writerFlags, LIBMATTI_ASMT_ClassNode *clazzAccessor,
    LIBMATTI_FML_ClassHierarchyRecomputationContext *recomputationContext)
{
    LIBMATTI_FML_TransformerClassWriter *writer = calloc(1, sizeof(LIBMATTI_FML_TransformerClassWriter));
    writer->writer = LIBMATTI_ASM_ClassWriter_New(writerFlags, clazzAccessor);
    writer->clazzAccessor = clazzAccessor;
    writer->computedThis = 0;
    writer->recomputationContext = recomputationContext;
    return writer;
}

void LIBMATTI_FML_TransformerClassWriter_Free(LIBMATTI_FML_TransformerClassWriter *writer)
{
    if (writer == NULL) return;
    LIBMATTI_ASM_ClassWriter_Free(writer->writer);
    free(writer);
}

// Java: SuperCollectingVisitor.visit(...) and the computeHierarchyFromClass tail
static void compute_hierarchy_from_names(const char *name, const char *superName, int isInterface,
                                         const char *const *interfaces, size_t interfaceCount)
{
    HierarchyEntry *entry = hierarchy_add(name);

    if (superName != NULL)
    {
        entry->parent = strdup(superName);
        if (hierarchy_for(superName) == NULL)
            LIBMATTI_FML_TransformerClassWriter_ComputeHierarchy(NULL, superName);
        hierarchy_add_super(entry, name);
        HierarchyEntry *parentEntry = hierarchy_for(superName);
        if (parentEntry != NULL)
            for (size_t i = 0; i < parentEntry->hierarchyCount; i++)
                hierarchy_add_super(entry, parentEntry->hierarchies[i]);
    }
    else
    {
        hierarchy_add_super(entry, "java/lang/Object");
    }

    entry->isInterface = isInterface;

    for (size_t i = 0; i < interfaceCount; i++)
    {
        if (hierarchy_for(interfaces[i]) == NULL) LIBMATTI_FML_TransformerClassWriter_ComputeHierarchy(NULL, interfaces[i]);
        hierarchy_add_super(entry, interfaces[i]);
        HierarchyEntry *interfaceEntry = hierarchy_for(interfaces[i]);
        if (interfaceEntry != NULL)
            for (size_t j = 0; j < interfaceEntry->hierarchyCount; j++)
                hierarchy_add_super(entry, interfaceEntry->hierarchies[j]);
    }
}

void LIBMATTI_FML_TransformerClassWriter_ComputeHierarchyForClassNode(LIBMATTI_FML_TransformerClassWriter *writer,
                                                                     LIBMATTI_ASMT_ClassNode *clazzNode)
{
    // Java: if (!CLASS_HIERARCHIES.containsKey(clazzNode.name)) clazzNode.accept(new SuperCollectingVisitor())
    if (hierarchy_for(clazzNode->name) != NULL) return;
    (void) writer;

    compute_hierarchy_from_names(clazzNode->name, clazzNode->superName,
                                 (clazzNode->access & LIBMATTI_ASM_ACC_INTERFACE) != 0,
                                 (const char *const *) clazzNode->interfaces, clazzNode->interfaceCount);
}

// Java: private void computeHierarchyFromFile(String className)
static void compute_hierarchy_from_file(LIBMATTI_FML_TransformerClassWriter *writer, const char *className)
{
    char *dotted = strdup(className);
    for (char *c = dotted; *c != '\0'; c++) if (*c == '/') *c = '.';

    size_t length = 0;
    unsigned char *classData = LIBMATTI_FML_ClassHierarchyRecomputationContext_UpToFrames(
        writer->recomputationContext, dotted, &length);

    if (classData == NULL)
    {
        // Java: catch (ClassNotFoundException e) -> computeHierarchyFromClass on the parent loader's class
        LIBMATTI_JL_Class *parentClass = LIBMATTI_FML_ClassHierarchyRecomputationContext_LocateParentClass(
            writer->recomputationContext, dotted);
        if (parentClass != NULL)
        {
            compute_hierarchy_from_class(className, parentClass);
            free(dotted);
            return;
        }
        LIBMATTI_ML_Logger_Fatal(LOGGER(), NULL, "Failed to find class {} ", className);
        free(dotted);
        return;
    }

    // Java: classReader.accept(new SuperCollectingVisitor(), SKIP_CODE | SKIP_DEBUG | SKIP_FRAMES)
    LIBMATTI_ASM_ClassReader *reader = LIBMATTI_ASM_ClassReader_New(
        classData, length, LIBMATTI_ASM_SKIP_CODE | LIBMATTI_ASM_SKIP_DEBUG | LIBMATTI_ASM_SKIP_FRAMES);
    LIBMATTI_ASMT_ClassNode *node = LIBMATTI_ASMT_ClassNode_New();
    LIBMATTI_ASM_ClassReader_Accept(reader, node);

    compute_hierarchy_from_names(node->name, node->superName, (node->access & LIBMATTI_ASM_ACC_INTERFACE) != 0,
                                (const char *const *) node->interfaces, node->interfaceCount);

    LIBMATTI_ASMT_ClassNode_Free(node);
    LIBMATTI_ASM_ClassReader_Free(reader);
    free(classData);
    free(dotted);
}

// Java: private void computeHierarchyFromClass(String name, Class<?> clazz)
static void compute_hierarchy_from_class(const char *className, LIBMATTI_JL_Class *clazz)
{
    HierarchyEntry *entry = hierarchy_add(className);
    // Java: Class<?> superClass = clazz.getSuperclass(); if (superClass != null) { ... }
    LIBMATTI_JL_Class *superClass = LIBMATTI_JL_Class_GetSuperclass(clazz);
    if (superClass != NULL)
    {
        char *superName = strdup(LIBMATTI_JL_Class_GetName(superClass));
        for (char *c = superName; *c != '\0'; c++) if (*c == '.') *c = '/';
        entry->parent = strdup(superName);
        if (hierarchy_for(superName) == NULL)
            compute_hierarchy_from_class(superName, superClass);
        hierarchy_add_super(entry, className);
        HierarchyEntry *parentEntry = hierarchy_for(superName);
        if (parentEntry != NULL)
            for (size_t i = 0; i < parentEntry->hierarchyCount; i++)
                hierarchy_add_super(entry, parentEntry->hierarchies[i]);
        free(superName);
    }
    else
    {
        hierarchy_add_super(entry, "java/lang/Object");
    }

    entry->isInterface = LIBMATTI_JL_Class_IsInterface(clazz);

    // Java: Arrays.stream(clazz.getInterfaces()).forEach(c -> { ... })
    size_t interfaceCount = 0;
    LIBMATTI_JL_Class **interfaces = LIBMATTI_JL_Class_GetInterfaces(clazz, &interfaceCount);
    for (size_t i = 0; i < interfaceCount; i++)
    {
        char *interfaceName = strdup(LIBMATTI_JL_Class_GetName(interfaces[i]));
        for (char *c = interfaceName; *c != '\0'; c++) if (*c == '.') *c = '/';
        if (hierarchy_for(interfaceName) == NULL)
            compute_hierarchy_from_class(interfaceName, interfaces[i]);
        hierarchy_add_super(entry, interfaceName);
        HierarchyEntry *interfaceEntry = hierarchy_for(interfaceName);
        if (interfaceEntry != NULL)
            for (size_t j = 0; j < interfaceEntry->hierarchyCount; j++)
                hierarchy_add_super(entry, interfaceEntry->hierarchies[j]);
        free(interfaceName);
    }
}

void LIBMATTI_FML_TransformerClassWriter_ComputeHierarchy(LIBMATTI_FML_TransformerClassWriter *writer,
                                                         const char *className)
{
    // Java: if (CLASS_HIERARCHIES.containsKey(className)) return;
    if (hierarchy_for(className) != NULL) return;
    if (writer == NULL) return;

    // Java: Class<?> clz = recomputationContext.findLoadedClass(className.replace('/', '.'));
    //       if (clz != null) computeHierarchyFromClass(className, clz); else computeHierarchyFromFile(className);
    char *dotted = strdup(className);
    for (char *c = dotted; *c != '\0'; c++) if (*c == '/') *c = '.';
    LIBMATTI_JL_Class *clz = LIBMATTI_FML_ClassHierarchyRecomputationContext_FindLoadedClass(
        writer->recomputationContext, dotted);
    free(dotted);
    if (clz != NULL)
        compute_hierarchy_from_class(className, clz);
    else
        compute_hierarchy_from_file(writer, className);
}

// Java: private Set<String> getSupers(String typeName)
static char **get_supers(const char *typeName, size_t *count)
{
    HierarchyEntry *entry = hierarchy_for(typeName);
    if (entry == NULL)
    {
        *count = 0;
        return NULL;
    }
    *count = entry->hierarchyCount;
    return entry->hierarchies;
}

static int contains(char **items, size_t count, const char *value)
{
    for (size_t i = 0; i < count; i++) if (strcmp(items[i], value) == 0) return 1;
    return 0;
}

char *LIBMATTI_FML_TransformerClassWriter_GetCommonSuperClass(LIBMATTI_FML_TransformerClassWriter *writer,
                                                             const char *type1, const char *type2)
{
    // Java: if (!computedThis) { computeHierarchy(clazzAccessor); computedThis = true; }
    if (!writer->computedThis)
    {
        LIBMATTI_FML_TransformerClassWriter_ComputeHierarchyForClassNode(writer, writer->clazzAccessor);
        writer->computedThis = 1;
    }

    if (hierarchy_for(type1) == NULL) LIBMATTI_FML_TransformerClassWriter_ComputeHierarchy(writer, type1);
    if (hierarchy_for(type2) == NULL) LIBMATTI_FML_TransformerClassWriter_ComputeHierarchy(writer, type2);

    size_t count1 = 0;
    size_t count2 = 0;
    char **supers1 = get_supers(type1, &count1);
    char **supers2 = get_supers(type2, &count2);

    // Java: if (getSupers(type2).contains(type1)) return type1; if (getSupers(type1).contains(type2)) return type2;
    if (contains(supers2, count2, type1)) return strdup(type1);
    if (contains(supers1, count1, type2)) return strdup(type2);

    HierarchyEntry *entry1 = hierarchy_for(type1);
    HierarchyEntry *entry2 = hierarchy_for(type2);
    // Java: if (isIntf(type1) || isIntf(type2)) return "java/lang/Object"
    if ((entry1 != NULL && entry1->isInterface) || (entry2 != NULL && entry2->isInterface))
        return strdup("java/lang/Object");

    // Java: String type = type1; do { type = getSuper(type); } while (!getSupers(type2).contains(type));
    const char *current = type1;
    do
    {
        HierarchyEntry *entry = hierarchy_for(current);
        if (entry == NULL || entry->parent == NULL) return strdup("java/lang/Object");
        current = entry->parent;
    }
    while (!contains(supers2, count2, current));

    return strdup(current);
}
