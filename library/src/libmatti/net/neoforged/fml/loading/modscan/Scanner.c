// Port of net.neoforged.fml.loading.modscan.Scanner.

#include "libmatti/net/neoforged/fml/loading/modscan/Scanner.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/loading/LogMarkers.h"
#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFileInfo.h"
#include "libmatti/net/neoforged/fml/loading/modscan/ModClassVisitor.h"
#include "libmatti/org/objectweb/asm/ClassReader.h"
#include "libmatti/org/objectweb/asm/Opcodes.h"

#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

typedef struct
{
    LIBMATTI_NEOFORGESPI_ModFileScanData *result;
    LIBMATTI_FML_ModFile *fileToScan;
} ScanUserdata;

static int endsWith(const char *value, const char *suffix)
{
    size_t valueLength = strlen(value);
    size_t suffixLength = strlen(suffix);
    return suffixLength <= valueLength && strcmp(value + valueLength - suffixLength, suffix) == 0;
}

// Java: ModClassVisitor mcv = new ModClassVisitor(); cr.accept(mcv, SKIP_CODE | SKIP_DEBUG);
//       mcv.buildData(result.getClasses(), result.getAnnotations());
static void scanClass(ScanUserdata *userdata, const char *relativePath, LIBMATTI_FML_JarResource *resource)
{
    size_t length = 0;
    unsigned char *bytes = LIBMATTI_FML_JarResource_Open(resource, &length);
    if (bytes == NULL) return;

    LIBMATTI_ASM_ClassReader *reader =
        LIBMATTI_ASM_ClassReader_New(bytes, length, LIBMATTI_ASM_SKIP_CODE | LIBMATTI_ASM_SKIP_DEBUG);
    LIBMATTI_ASMT_ClassNode *node = LIBMATTI_ASMT_ClassNode_New();
    LIBMATTI_ASM_ClassReader_Accept(reader, node);

    LIBMATTI_ASM_ClassReader_Free(reader);
    free(bytes);

    if (node->name == NULL)
    {
        // Java logs the ModFile itself; the port's logger only substitutes strings.
        LIBMATTI_ML_Logger_Error(LOGGER(), &LIBMATTI_FML_MARKER_SCAN, "Exception scanning {} path {}",
                                 LIBMATTI_NEOFORGESPI_IModFile_GetFilePath(
                                     LIBMATTI_FML_ModFile_AsModFile(userdata->fileToScan)),
                                 relativePath);
        LIBMATTI_ASMT_ClassNode_Free(node);
        return;
    }

    LIBMATTI_FML_ModClassVisitor *classVisitor = LIBMATTI_FML_ModClassVisitor_New();
    LIBMATTI_FML_ModClassVisitor_VisitNode(classVisitor, node);
    LIBMATTI_FML_ModClassVisitor_BuildData(classVisitor, userdata->result);
    LIBMATTI_FML_ModClassVisitor_Free(classVisitor);

    LIBMATTI_ASMT_ClassNode_Free(node);
}

static void visit(const char *relativePath, LIBMATTI_FML_JarResource *resource, void *userdata)
{
    // Java: if (relativePath.endsWith(".class"))
    if (!endsWith(relativePath, ".class")) return;

    scanClass(userdata, relativePath, resource);
}

LIBMATTI_FML_Scanner *LIBMATTI_FML_Scanner_New(LIBMATTI_FML_ModFile *fileToScan)
{
    LIBMATTI_FML_Scanner *scanner = calloc(1, sizeof(LIBMATTI_FML_Scanner));
    scanner->fileToScan = fileToScan;
    return scanner;
}

void LIBMATTI_FML_Scanner_Free(LIBMATTI_FML_Scanner *scanner)
{
    free(scanner);
}

// Java: public ModFileScanData scan()
LIBMATTI_NEOFORGESPI_ModFileScanData *LIBMATTI_FML_Scanner_Scan(const LIBMATTI_FML_Scanner *scanner)
{
    LIBMATTI_NEOFORGESPI_ModFileScanData *result = LIBMATTI_NEOFORGESPI_ModFileScanData_New();

    // Java: result.addModFileInfo(fileToScan.getModFileInfo());
    LIBMATTI_NEOFORGESPI_ModFileScanData_AddModFileInfo(
        result, LIBMATTI_FML_ModFile_GetModFileInfo(scanner->fileToScan));

    // Java: fileToScan.getContents().visitContent((relativePath, resource) -> { ... })
    ScanUserdata userdata = {result, scanner->fileToScan};
    LIBMATTI_FML_JarContents_VisitContent(LIBMATTI_FML_ModFile_GetContents(scanner->fileToScan), visit, &userdata);

    return result;
}
