// Throwaway harness for the modscan port: scans real javac output and checks that
// getAnnotatedBy(Mod/EventBusSubscriber) and the values the rest of FML reads are right.

#include "libmatti/net/neoforged/fml/javafmlmod/AutomaticEventSubscriber.h"
#include "libmatti/net/neoforged/fml/javafmlmod/FMLJavaModLanguageProvider.h"
#include "libmatti/net/neoforged/fml/loading/modscan/ModClassVisitor.h"
#include "libmatti/net/neoforged/neoforgespi/language/ModFileScanData.h"
#include "libmatti/org/objectweb/asm/ClassReader.h"
#include "libmatti/org/objectweb/asm/Opcodes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void check(int condition, const char *what)
{
    printf("%s: %s\n", condition ? "ok  " : "FAIL", what);
    if (!condition) failures++;
}

static const char *annotation_value(const LIBMATTI_NEOFORGESPI_AnnotationData *data, const char *key)
{
    for (size_t i = 0; i < data->annotationDataCount; i++)
        if (strcmp(data->annotationDataKeys[i], key) == 0) return data->annotationDataValues[i];
    return NULL;
}

// Java: the records carry Type objects; the harness compares their class names
static const char *type_class_name(const LIBMATTI_ASM_Type *type)
{
    static char buffer[256];
    char *name = LIBMATTI_ASM_Type_GetClassName(type);
    snprintf(buffer, sizeof(buffer), "%s", name != NULL ? name : "");
    free(name);
    return buffer;
}

static void scan(const char *path, LIBMATTI_NEOFORGESPI_ModFileScanData *result)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL)
    {
        printf("FAIL: cannot read %s\n", path);
        failures++;
        return;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    unsigned char *bytes = malloc((size_t) length);
    fread(bytes, 1, (size_t) length, file);
    fclose(file);

    LIBMATTI_ASM_ClassReader *reader =
        LIBMATTI_ASM_ClassReader_New(bytes, (size_t) length, LIBMATTI_ASM_SKIP_CODE | LIBMATTI_ASM_SKIP_DEBUG);
    LIBMATTI_ASMT_ClassNode *node = LIBMATTI_ASMT_ClassNode_New();
    LIBMATTI_ASM_ClassReader_Accept(reader, node);
    LIBMATTI_ASM_ClassReader_Free(reader);
    free(bytes);

    LIBMATTI_FML_ModClassVisitor *classVisitor = LIBMATTI_FML_ModClassVisitor_New();
    LIBMATTI_FML_ModClassVisitor_VisitNode(classVisitor, node);
    LIBMATTI_FML_ModClassVisitor_BuildData(classVisitor, result);
    LIBMATTI_FML_ModClassVisitor_Free(classVisitor);

    LIBMATTI_ASMT_ClassNode_Free(node);
}

int main(int argc, char **argv)
{
    if (argc < 3) return 2;

    LIBMATTI_NEOFORGESPI_ModFileScanData *result = LIBMATTI_NEOFORGESPI_ModFileScanData_New();
    scan(argv[1], result);
    scan(argv[2], result);

    size_t count = 0;
    for (size_t i = 0; i < result->annotationCount; i++)
    {
        const LIBMATTI_NEOFORGESPI_AnnotationData *dump = &result->annotations[i];
        printf("dump: %s target=%d clazz=%s member=%s\n", type_class_name(dump->annotationType), (int) dump->targetType,
               type_class_name(dump->clazz), dump->memberName);
        for (size_t k = 0; k < dump->annotationDataCount; k++)
            printf("dump:     [%s] = [%s]\n", dump->annotationDataKeys[k], dump->annotationDataValues[k]);
    }

    LIBMATTI_NEOFORGESPI_AnnotationData **annotations = LIBMATTI_NEOFORGESPI_ModFileScanData_GetAnnotatedBy(
        result, "net.neoforged.fml.common.Mod", LIBMATTI_NEOFORGESPI_ElementType_TYPE, &count);
    check(count == 1, "one @Mod annotation (ElementType.TYPE)");
    if (count == 1)
    {
        const LIBMATTI_NEOFORGESPI_AnnotationData *data = annotations[0];
        check(strcmp(type_class_name(data->clazz), "example.ExampleMod") == 0, "@Mod clazz is example.ExampleMod");
        check(strcmp(data->memberName, "example.ExampleMod") == 0, "@Mod member is the class itself");
        check(strcmp(annotation_value(data, "value"), "examplemod") == 0, "@Mod value is examplemod");
        check(strcmp(annotation_value(data, "depends"), "othermod,thirdmod") == 0,
              "@Mod depends is rendered as a comma separated list");

        // Java: getDepends / getSides read the same values the C port hands over
        LIBMATTI_DIST_Dist sides[2];
        check(LIBMATTI_FML_AutomaticEventSubscriber_GetSides(annotation_value(data, "dist"), sides, 2) == 1 &&
                  sides[0] == LIBMATTI_DIST_CLIENT,
              "@Mod dist yields CLIENT");

        const char *depends[2];
        check(LIBMATTI_FML_FMLJavaModLanguageProvider_GetDepends(annotation_value(data, "depends"), depends, 2) == 2 &&
                  strcmp(depends[0], "othermod") == 0 && strcmp(depends[1], "thirdmod") == 0,
              "@Mod depends yields othermod and thirdmod");
        for (size_t i = 0; i < 2; i++) free((void *) depends[i]);
    }
    free(annotations);

    // A class annotation with a scalar, an enum, an array and a nested annotation
    annotations = LIBMATTI_NEOFORGESPI_ModFileScanData_GetAnnotatedBy(
        result, "example.Complex", LIBMATTI_NEOFORGESPI_ElementType_TYPE, &count);
    check(count == 1, "one @Complex annotation");
    if (count == 1)
    {
        const LIBMATTI_NEOFORGESPI_AnnotationData *data = annotations[0];
        check(strcmp(annotation_value(data, "count"), "7") == 0, "@Complex count is 7");
        check(strcmp(annotation_value(data, "flag"), "false") == 0, "@Complex flag is false");
        check(strcmp(annotation_value(data, "choice"), "B") == 0, "@Complex choice is the enum constant B");
        check(strcmp(annotation_value(data, "tags"), "x,y") == 0, "@Complex tags is the list x,y");
        check(strcmp(annotation_value(data, "nested"), "{name=inner}") == 0,
              "@Complex nested is the child annotation");
    }
    free(annotations);

    annotations = LIBMATTI_NEOFORGESPI_ModFileScanData_GetAnnotatedBy(
        result, "net.neoforged.fml.common.EventBusSubscriber", LIBMATTI_NEOFORGESPI_ElementType_TYPE, &count);
    check(count == 1, "one @EventBusSubscriber annotation");
    if (count == 1)
    {
        const LIBMATTI_NEOFORGESPI_AnnotationData *data = annotations[0];
        check(strcmp(type_class_name(data->clazz), "example.ExampleSubscriber") == 0, "@EventBusSubscriber clazz");
        check(strcmp(annotation_value(data, "modid"), "examplemod") == 0, "@EventBusSubscriber modid");
        check(annotation_value(data, "value") == NULL, "@EventBusSubscriber value defaults are not in the file");
    }
    free(annotations);

    annotations = LIBMATTI_NEOFORGESPI_ModFileScanData_GetAnnotatedBy(
        result, "net.neoforged.bus.api.SubscribeEvent", LIBMATTI_NEOFORGESPI_ElementType_METHOD, &count);
    check(count == 2, "two @SubscribeEvent methods");
    for (size_t i = 0; i < count; i++)
    {
        const LIBMATTI_NEOFORGESPI_AnnotationData *data = annotations[i];
        if (strcmp(type_class_name(data->clazz), "example.ExampleMod") == 0)
        {
            check(strcmp(data->memberName, "onSomething(Ljava/lang/String;)V") == 0,
                  "method member is name + descriptor");
            check(annotation_value(data, "priority") == NULL, "the default priority is not in the file");
        }
        else
        {
            check(strcmp(type_class_name(data->clazz), "example.ExampleSubscriber") == 0, "the second @SubscribeEvent clazz");
            check(strcmp(annotation_value(data, "priority"), "HIGHEST") == 0, "the enum priority value");
            check(strcmp(annotation_value(data, "receiveCanceled"), "true") == 0, "the boolean value");
        }
    }
    free(annotations);

    annotations = LIBMATTI_NEOFORGESPI_ModFileScanData_GetAnnotatedBy(
        result, "net.neoforged.bus.api.SubscribeEvent", LIBMATTI_NEOFORGESPI_ElementType_FIELD, &count);
    check(count == 1, "one @SubscribeEvent field");
    if (count == 1)
    {
        check(strcmp(annotations[0]->memberName, "counter") == 0, "field member is the field name");
        check(strcmp(annotation_value(annotations[0], "priority"), "HIGH") == 0, "field enum priority");
    }
    free(annotations);

    // ClassData comes from the same pass
    size_t classCount = 0;
    LIBMATTI_NEOFORGESPI_ClassData *classes =
        LIBMATTI_NEOFORGESPI_ModFileScanData_GetClasses(result, &classCount);
    check(classCount == 2, "two ClassData entries");
    for (size_t i = 0; i < classCount; i++)
    {
        if (strcmp(type_class_name(classes[i].clazz), "example.ExampleMod") != 0) continue;
        check(strcmp(type_class_name(classes[i].parent), "java.lang.Object") == 0, "ExampleMod's parent");
        check(classes[i].interfaceCount == 1 &&
                  strcmp(type_class_name(classes[i].interfaces[0]), "java.lang.Runnable") == 0,
              "ExampleMod's interface");
    }

    LIBMATTI_NEOFORGESPI_ModFileScanData_Free(result);
    printf("%s\n", failures == 0 ? "ALL OK" : "FAILURES");
    return failures != 0;
}
