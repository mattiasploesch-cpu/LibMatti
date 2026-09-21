// Port of cpw.mods.modlauncher.TransformerClassWriter.
// Java's writer only computes frames when COMPUTE_FRAMES is set and then asks
// getCommonSuperClass for the hierarchy. The C ClassWriter does not recompute
// frames (see ClassWriter.h), so the hierarchy helpers below are provided for
// callers that need them, but the writer itself never calls them.

#include "libmatti/cpw/modlauncher/TransformerClassWriter.h"

#include "LogManager.h"
#include "LogMarkers.h"
#include "libmatti/org/objectweb/asm/ClassReader.h"
#include "libmatti/org/objectweb/asm/Opcodes.h"

#include <stdlib.h>
#include <string.h>

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
}

static void hierarchy_add_super(HierarchyEntry *entry, const char *superName)
{
    if (superName == NULL) return;
    for (size_t i = 0; i < entry->hierarchyCount; i++)
        if (strcmp(entry->hierarchies[i], superName) == 0) return;

    entry->hierarchies = realloc(entry->hierarchies, sizeof(*entry->hierarchies) * (entry->hierarchyCount + 1));
    entry->hierarchies[entry->hierarchyCount++] = strdup(superName);
}

// Java: static ClassWriter createClassWriter(int mlFlags, ClassTransformer classTransformer, ClassNode clazzAccessor)
LIBMATTI_ASM_ClassWriter *LIBMATTI_ML_TransformerClassWriter_CreateClassWriter(
    int mlFlags, LIBMATTI_ML_ClassTransformer *classTransformer, LIBMATTI_ASMT_ClassNode *clazzAccessor)
{
    (void)classTransformer;
    (void)clazzAccessor;

    // Java: final int writerFlag = mlFlags & ~ILaunchPluginService.ComputeFlags.SIMPLE_REWRITE
    int writerFlag = mlFlags & ~LIBMATTI_MLS_COMPUTE_SIMPLE_REWRITE;

    // Java: only the TransformerClassWriter is used when COMPUTE_FRAMES is set
    return LIBMATTI_ASM_ClassWriter_New(writerFlag, clazzAccessor);
}

static void compute_hierarchy_from_names(const char *name, const char *parent, const char *superName,
                                         const char *const *interfaces, size_t interfaceCount)
{
    HierarchyEntry *entry = hierarchy_add(name);

    if (superName != NULL)
    {
        entry->parent = strdup(superName);
        if (hierarchy_for(superName) == NULL) LIBMATTI_ML_TransformerClassWriter_ComputeHierarchy(NULL, superName);
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

    for (size_t i = 0; i < interfaceCount; i++)
    {
        if (hierarchy_for(interfaces[i]) == NULL)
            LIBMATTI_ML_TransformerClassWriter_ComputeHierarchy(NULL, interfaces[i]);
        hierarchy_add_super(entry, interfaces[i]);
        HierarchyEntry *interfaceEntry = hierarchy_for(interfaces[i]);
        if (interfaceEntry != NULL)
            for (size_t j = 0; j < interfaceEntry->hierarchyCount; j++)
                hierarchy_add_super(entry, interfaceEntry->hierarchies[j]);
    }

    (void)parent;
}

void LIBMATTI_ML_TransformerClassWriter_ComputeHierarchyForClassNode(LIBMATTI_ML_ClassTransformer *classTransformer,
                                                                    LIBMATTI_ASMT_ClassNode *clazzNode)
{
    if (hierarchy_for(clazzNode->name) != NULL) return;

    compute_hierarchy_from_names(clazzNode->name, NULL, clazzNode->superName,
                                 (const char *const *)clazzNode->interfaces, clazzNode->interfaceCount);
    HierarchyEntry *entry = hierarchy_for(clazzNode->name);
    if (entry != NULL) entry->isInterface = (clazzNode->access & LIBMATTI_ASM_ACC_INTERFACE) != 0;
    (void)classTransformer;
}

// Java: private void computeHierarchy(String className)
void LIBMATTI_ML_TransformerClassWriter_ComputeHierarchy(LIBMATTI_ML_ClassTransformer *classTransformer,
                                                         const char *className)
{
    if (hierarchy_for(className) != NULL) return;

    // Java: Class<?> clz = classTransformer.getTransformingClassLoader().getLoadedClass(className.replace('/','.'))
    //       if (clz != null) computeHierarchyFromClass(...) else computeHierarchyFromFile(className)
    // findLoadedClass is JVM-backed (external), so the C port always reads the class file.
    if (classTransformer == NULL) return;

    LIBMATTI_ML_TransformingClassLoader *loader =
        LIBMATTI_ML_ClassTransformer_GetTransformingClassLoader(classTransformer);

    char *dotted = strdup(className);
    for (char *c = dotted; *c != '\0'; c++) if (*c == '/') *c = '.';

    size_t length = 0;
    unsigned char *classData = LIBMATTI_ML_TransformingClassLoader_BuildTransformedClassNodeFor(
        loader, dotted, LIBMATTI_MLA_ITransformerActivity_COMPUTING_FRAMES_REASON, &length);

    if (classData == NULL)
    {
        // Java: catch (ClassNotFoundException e) -> LOGGER.fatal("Failed to find class {}", className)
        LIBMATTI_ML_Logger_Fatal(LIBMATTI_ML_LogManager_GetLogger(), &LIBMATTI_ML_MARKER_MODLAUNCHER,
                                 "Failed to find class {} ", className);
        free(dotted);
        return;
    }

    // Java: classReader.accept(new SuperCollectingVisitor(), SKIP_CODE | SKIP_DEBUG | SKIP_FRAMES)
    LIBMATTI_ASM_ClassReader *reader = LIBMATTI_ASM_ClassReader_New(
        classData, length, LIBMATTI_ASM_SKIP_CODE | LIBMATTI_ASM_SKIP_DEBUG | LIBMATTI_ASM_SKIP_FRAMES);
    LIBMATTI_ASMT_ClassNode *node = LIBMATTI_ASMT_ClassNode_New();
    LIBMATTI_ASM_ClassReader_Accept(reader, node);

    compute_hierarchy_from_names(node->name, NULL, node->superName, (const char *const *)node->interfaces,
                                 node->interfaceCount);
    HierarchyEntry *entry = hierarchy_for(node->name);
    if (entry != NULL) entry->isInterface = (node->access & LIBMATTI_ASM_ACC_INTERFACE) != 0;

    LIBMATTI_ASMT_ClassNode_Free(node);
    LIBMATTI_ASM_ClassReader_Free(reader);
    free(classData);
    free(dotted);
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

// Java: protected String getCommonSuperClass(String type1, String type2)
char *LIBMATTI_ML_TransformerClassWriter_GetCommonSuperClass(LIBMATTI_ML_ClassTransformer *classTransformer,
                                                             const char *type1, const char *type2)
{
    if (hierarchy_for(type1) == NULL) LIBMATTI_ML_TransformerClassWriter_ComputeHierarchy(classTransformer, type1);
    if (hierarchy_for(type2) == NULL) LIBMATTI_ML_TransformerClassWriter_ComputeHierarchy(classTransformer, type2);

    size_t count1 = 0;
    size_t count2 = 0;
    char **supers1 = get_supers(type1, &count1);
    char **supers2 = get_supers(type2, &count2);

    if (contains(supers2, count2, type1)) return strdup(type1);
    if (contains(supers1, count1, type2)) return strdup(type2);

    HierarchyEntry *entry1 = hierarchy_for(type1);
    HierarchyEntry *entry2 = hierarchy_for(type2);

    if ((entry1 != NULL && entry1->isInterface) || (entry2 != NULL && entry2->isInterface))
        return strdup("java/lang/Object");

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
