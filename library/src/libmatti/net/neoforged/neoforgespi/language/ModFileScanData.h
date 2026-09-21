// Port of net.neoforged.neoforgespi.language.ModFileScanData.
// The records carry real org.objectweb.asm.Type objects (see asm/Type.h); the annotation
// model (java.lang.annotation.Annotation/ElementType) is keyed by the annotation's class
// name like the Class-based lookups of Java.

#ifndef MATTICRAFT_NEOFORGESPI_LANGUAGE_MODFILESCANDATA_H
#define MATTICRAFT_NEOFORGESPI_LANGUAGE_MODFILESCANDATA_H

#include "libmatti/java/lang/annotation/Annotation.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModFileInfo.h"
#include "libmatti/org/objectweb/asm/Type.h"

#include <stddef.h>

// Java: java.lang.annotation.ElementType
typedef enum
{
    LIBMATTI_NEOFORGESPI_ElementType_TYPE,
    LIBMATTI_NEOFORGESPI_ElementType_FIELD,
    LIBMATTI_NEOFORGESPI_ElementType_METHOD,
    LIBMATTI_NEOFORGESPI_ElementType_PARAMETER,
    LIBMATTI_NEOFORGESPI_ElementType_CONSTRUCTOR,
    LIBMATTI_NEOFORGESPI_ElementType_LOCAL_VARIABLE,
    LIBMATTI_NEOFORGESPI_ElementType_ANNOTATION_TYPE,
    LIBMATTI_NEOFORGESPI_ElementType_PACKAGE,
    LIBMATTI_NEOFORGESPI_ElementType_TYPE_PARAMETER,
    LIBMATTI_NEOFORGESPI_ElementType_TYPE_USE,
    LIBMATTI_NEOFORGESPI_ElementType_MODULE,
    LIBMATTI_NEOFORGESPI_ElementType_RECORD_COMPONENT
} LIBMATTI_NEOFORGESPI_ElementType;

// Java: record ClassData(Type clazz, Type parent, Set<Type> interfaces)
typedef struct
{
    LIBMATTI_ASM_Type *clazz;        // owned
    LIBMATTI_ASM_Type *parent;       // owned, NULL for Object's superclass-less case
    LIBMATTI_ASM_Type **interfaces;  // owned, each element owned
    size_t interfaceCount;
} LIBMATTI_NEOFORGESPI_ClassData;

// Java: record AnnotationData(Type annotationType, ElementType targetType, Type clazz, String memberName, Map<String, Object> annotationData)
typedef struct
{
    LIBMATTI_ASM_Type *annotationType; // owned
    LIBMATTI_NEOFORGESPI_ElementType targetType;
    LIBMATTI_ASM_Type *clazz;          // owned
    char *memberName;                  // owned
    char **annotationDataKeys;         // owned
    char **annotationDataValues;       // owned
    size_t annotationDataCount;
} LIBMATTI_NEOFORGESPI_AnnotationData;

// Java: public class ModFileScanData
typedef struct LIBMATTI_NEOFORGESPI_ModFileScanData
{
    // Java: Set<AnnotationData> annotations (LinkedHashSet)
    LIBMATTI_NEOFORGESPI_AnnotationData *annotations;
    size_t annotationCount;
    // Java: Set<ClassData> classes (LinkedHashSet)
    LIBMATTI_NEOFORGESPI_ClassData *classes;
    size_t classCount;
    // Java: List<IModFileInfo> modFiles
    LIBMATTI_NEOFORGESPI_IModFileInfo **modFiles;
    size_t modFileCount;
} LIBMATTI_NEOFORGESPI_ModFileScanData;

LIBMATTI_NEOFORGESPI_ModFileScanData *LIBMATTI_NEOFORGESPI_ModFileScanData_New(void);
void LIBMATTI_NEOFORGESPI_ModFileScanData_Free(LIBMATTI_NEOFORGESPI_ModFileScanData *scanData);

// Java: public Set<ClassData> getClasses()
LIBMATTI_NEOFORGESPI_ClassData *LIBMATTI_NEOFORGESPI_ModFileScanData_GetClasses(
    const LIBMATTI_NEOFORGESPI_ModFileScanData *scanData, size_t *count);
// Java: public Set<AnnotationData> getAnnotations()
LIBMATTI_NEOFORGESPI_AnnotationData *LIBMATTI_NEOFORGESPI_ModFileScanData_GetAnnotations(
    const LIBMATTI_NEOFORGESPI_ModFileScanData *scanData, size_t *count);
// Java: public Stream<AnnotationData> getAnnotatedBy(Class<? extends Annotation> type, ElementType elementType)
// 'annotationType' is the annotation class name the ASM Type of the annotation resolves to
LIBMATTI_NEOFORGESPI_AnnotationData **LIBMATTI_NEOFORGESPI_ModFileScanData_GetAnnotatedBy(
    const LIBMATTI_NEOFORGESPI_ModFileScanData *scanData, const char *annotationType,
    LIBMATTI_NEOFORGESPI_ElementType elementType, size_t *count);
// Java: classes.add(ClassData) / annotations.add(AnnotationData)
void LIBMATTI_NEOFORGESPI_ModFileScanData_AddClass(LIBMATTI_NEOFORGESPI_ModFileScanData *scanData,
                                                   const LIBMATTI_NEOFORGESPI_ClassData *classData);
void LIBMATTI_NEOFORGESPI_ModFileScanData_AddAnnotation(LIBMATTI_NEOFORGESPI_ModFileScanData *scanData,
                                                        const LIBMATTI_NEOFORGESPI_AnnotationData *annotation);
// Java: public void addModFileInfo(IModFileInfo info)
void LIBMATTI_NEOFORGESPI_ModFileScanData_AddModFileInfo(LIBMATTI_NEOFORGESPI_ModFileScanData *scanData,
                                                         LIBMATTI_NEOFORGESPI_IModFileInfo *info);
// Java: public List<IModFileInfo> getIModInfoData()
LIBMATTI_NEOFORGESPI_IModFileInfo **LIBMATTI_NEOFORGESPI_ModFileScanData_GetIModInfoData(
    const LIBMATTI_NEOFORGESPI_ModFileScanData *scanData, size_t *count);

#endif //MATTICRAFT_NEOFORGESPI_LANGUAGE_MODFILESCANDATA_H
