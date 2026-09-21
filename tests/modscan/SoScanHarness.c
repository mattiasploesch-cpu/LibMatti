// Throwaway harness 2: reads the real mod .so through the port's JarContents and runs the
// same class scan the Scanner runs, so the ELF-section entry names reach the scan as .class.

#include "libmatti/net/neoforged/fml/jarcontents/JarContents.h"
#include "libmatti/net/neoforged/fml/loading/modscan/ModClassVisitor.h"
#include "libmatti/net/neoforged/neoforgespi/language/ModFileScanData.h"
#include "libmatti/org/objectweb/asm/ClassReader.h"
#include "libmatti/org/objectweb/asm/Opcodes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;
static LIBMATTI_NEOFORGESPI_ModFileScanData *result;
static int classFiles;

static int endsWith(const char *value, const char *suffix)
{
    size_t valueLength = strlen(value);
    size_t suffixLength = strlen(suffix);
    return suffixLength <= valueLength && strcmp(value + valueLength - suffixLength, suffix) == 0;
}

static void visit(const char *relativePath, LIBMATTI_FML_JarResource *resource, void *userdata)
{
    (void) userdata;
    if (!endsWith(relativePath, ".class")) return;

    size_t length = 0;
    unsigned char *bytes = LIBMATTI_FML_JarResource_Open(resource, &length);
    if (bytes == NULL) return;
    classFiles++;

    LIBMATTI_ASM_ClassReader *reader =
        LIBMATTI_ASM_ClassReader_New(bytes, length, LIBMATTI_ASM_SKIP_CODE | LIBMATTI_ASM_SKIP_DEBUG);
    LIBMATTI_ASMT_ClassNode *node = LIBMATTI_ASMT_ClassNode_New();
    LIBMATTI_ASM_ClassReader_Accept(reader, node);
    LIBMATTI_ASM_ClassReader_Free(reader);
    free(bytes);

    if (node->name == NULL)
    {
        printf("FAIL: %s is not a class file\n", relativePath);
        failures++;
        LIBMATTI_ASMT_ClassNode_Free(node);
        return;
    }

    LIBMATTI_FML_ModClassVisitor *classVisitor = LIBMATTI_FML_ModClassVisitor_New();
    LIBMATTI_FML_ModClassVisitor_VisitNode(classVisitor, node);
    LIBMATTI_FML_ModClassVisitor_BuildData(classVisitor, result);
    LIBMATTI_FML_ModClassVisitor_Free(classVisitor);

    LIBMATTI_ASMT_ClassNode_Free(node);
}

int main(int argc, char **argv)
{
    if (argc < 2) return 2;

    LIBMATTI_FML_JarContents *contents = LIBMATTI_FML_JarContents_OfPath(argv[1]);
    if (contents == NULL)
    {
        printf("FAIL: cannot open %s\n", argv[1]);
        return 1;
    }

    result = LIBMATTI_NEOFORGESPI_ModFileScanData_New();
    LIBMATTI_FML_JarContents_VisitContent(contents, visit, NULL);

    printf("%s: %d .class entr%s found in the .so\n", classFiles > 0 ? "ok  " : "FAIL", classFiles,
           classFiles == 1 ? "y" : "ies");
    if (classFiles == 0) failures++;

    size_t count = 0;
    LIBMATTI_NEOFORGESPI_AnnotationData **annotations = LIBMATTI_NEOFORGESPI_ModFileScanData_GetAnnotatedBy(
        result, "net.neoforged.fml.common.Mod", LIBMATTI_NEOFORGESPI_ElementType_TYPE, &count);
    printf("%s: @Mod annotations from the .so: %zu\n", count == 1 ? "ok  " : "FAIL", count);
    if (count != 1) failures++;

    if (count == 1)
    {
        const LIBMATTI_NEOFORGESPI_AnnotationData *data = annotations[0];
        printf("dump: clazz=%s member=%s\n", data->clazz, data->memberName);
        for (size_t i = 0; i < data->annotationDataCount; i++)
            printf("dump:     [%s] = [%s]\n", data->annotationDataKeys[i], data->annotationDataValues[i]);
    }

    free(annotations);
    LIBMATTI_NEOFORGESPI_ModFileScanData_Free(result);
    LIBMATTI_FML_JarContents_Close(contents);

    printf("%s\n", failures == 0 ? "ALL OK" : "FAILURES");
    return failures != 0;
}
