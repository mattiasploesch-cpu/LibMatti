// Port of net.neoforged.neoforgespi.language.ModFileScanData.

#include "libmatti/net/neoforged/neoforgespi/language/ModFileScanData.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_NEOFORGESPI_ModFileScanData *LIBMATTI_NEOFORGESPI_ModFileScanData_New(void)
{
    return calloc(1, sizeof(LIBMATTI_NEOFORGESPI_ModFileScanData));
}

void LIBMATTI_NEOFORGESPI_ModFileScanData_Free(LIBMATTI_NEOFORGESPI_ModFileScanData *scanData)
{
    if (scanData == NULL)
        return;

    for (size_t i = 0; i < scanData->annotationCount; i++)
    {
        LIBMATTI_NEOFORGESPI_AnnotationData *annotation = &scanData->annotations[i];
        LIBMATTI_ASM_Type_Free(annotation->annotationType);
        LIBMATTI_ASM_Type_Free(annotation->clazz);
        free(annotation->memberName);
        for (size_t k = 0; k < annotation->annotationDataCount; k++)
        {
            free(annotation->annotationDataKeys[k]);
            free(annotation->annotationDataValues[k]);
        }
        free(annotation->annotationDataKeys);
        free(annotation->annotationDataValues);
    }
    free(scanData->annotations);

    for (size_t i = 0; i < scanData->classCount; i++)
    {
        LIBMATTI_NEOFORGESPI_ClassData *classData = &scanData->classes[i];
        LIBMATTI_ASM_Type_Free(classData->clazz);
        LIBMATTI_ASM_Type_Free(classData->parent);
        for (size_t k = 0; k < classData->interfaceCount; k++)
            LIBMATTI_ASM_Type_Free(classData->interfaces[k]);
        free(classData->interfaces);
    }
    free(scanData->classes);

    free(scanData->modFiles);
    free(scanData);
}

// Java: public Set<ClassData> getClasses()
LIBMATTI_NEOFORGESPI_ClassData *LIBMATTI_NEOFORGESPI_ModFileScanData_GetClasses(
    const LIBMATTI_NEOFORGESPI_ModFileScanData *scanData, size_t *count)
{
    *count = scanData->classCount;
    return scanData->classes;
}

// Java: public Set<AnnotationData> getAnnotations()
LIBMATTI_NEOFORGESPI_AnnotationData *LIBMATTI_NEOFORGESPI_ModFileScanData_GetAnnotations(
    const LIBMATTI_NEOFORGESPI_ModFileScanData *scanData, size_t *count)
{
    *count = scanData->annotationCount;
    return scanData->annotations;
}

// Java: public Stream<AnnotationData> getAnnotatedBy(Class<? extends Annotation> type, ElementType elementType)
// The Class lookup compares the annotation class name (the ASM Type's class name)
LIBMATTI_NEOFORGESPI_AnnotationData **LIBMATTI_NEOFORGESPI_ModFileScanData_GetAnnotatedBy(
    const LIBMATTI_NEOFORGESPI_ModFileScanData *scanData, const char *annotationType,
    LIBMATTI_NEOFORGESPI_ElementType elementType, size_t *count)
{
    LIBMATTI_NEOFORGESPI_AnnotationData **result = NULL;
    *count = 0;

    for (size_t i = 0; i < scanData->annotationCount; i++)
    {
        LIBMATTI_NEOFORGESPI_AnnotationData *annotation = &scanData->annotations[i];
        char *className = LIBMATTI_ASM_Type_GetClassName(annotation->annotationType);
        int matches = className != NULL && strcmp(className, annotationType) == 0;
        free(className);
        if (annotation->targetType == elementType && matches)
        {
            result = realloc(result, sizeof(*result) * (*count + 1));
            result[(*count)++] = annotation;
        }
    }

    return result;
}

// Java: classes.add(ClassData)
void LIBMATTI_NEOFORGESPI_ModFileScanData_AddClass(LIBMATTI_NEOFORGESPI_ModFileScanData *scanData,
                                                   const LIBMATTI_NEOFORGESPI_ClassData *classData)
{
    scanData->classes = realloc(scanData->classes, sizeof(*scanData->classes) * (scanData->classCount + 1));
    scanData->classes[scanData->classCount++] = *classData;
}

// Java: annotations.add(AnnotationData)
void LIBMATTI_NEOFORGESPI_ModFileScanData_AddAnnotation(LIBMATTI_NEOFORGESPI_ModFileScanData *scanData,
                                                        const LIBMATTI_NEOFORGESPI_AnnotationData *annotation)
{
    scanData->annotations =
        realloc(scanData->annotations, sizeof(*scanData->annotations) * (scanData->annotationCount + 1));
    scanData->annotations[scanData->annotationCount++] = *annotation;
}

// Java: public void addModFileInfo(IModFileInfo info)
void LIBMATTI_NEOFORGESPI_ModFileScanData_AddModFileInfo(LIBMATTI_NEOFORGESPI_ModFileScanData *scanData,
                                                         LIBMATTI_NEOFORGESPI_IModFileInfo *info)
{
    scanData->modFiles = realloc(scanData->modFiles, sizeof(*scanData->modFiles) * (scanData->modFileCount + 1));
    scanData->modFiles[scanData->modFileCount++] = info;
}

// Java: public List<IModFileInfo> getIModInfoData()
LIBMATTI_NEOFORGESPI_IModFileInfo **LIBMATTI_NEOFORGESPI_ModFileScanData_GetIModInfoData(
    const LIBMATTI_NEOFORGESPI_ModFileScanData *scanData, size_t *count)
{
    *count = scanData->modFileCount;
    return scanData->modFiles;
}
