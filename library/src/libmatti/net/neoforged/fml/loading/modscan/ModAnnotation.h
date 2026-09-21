// Port of net.neoforged.fml.loading.modscan.ModAnnotation.
// Java keeps the annotation values as Object (String, Integer, ..., Type, EnumHolder,
// Map and List); the port tags them like the class file element_value
// (see org.objectweb.asm.tree.AnnotationNode).

#ifndef MATTICRAFT_FML_LOADING_MODSCAN_MODANNOTATION_H
#define MATTICRAFT_FML_LOADING_MODSCAN_MODANNOTATION_H

#include "libmatti/net/neoforged/neoforgespi/language/ModFileScanData.h"
#include "libmatti/org/objectweb/asm/Type.h"

#include <stddef.h>

// Java: public record EnumHolder(String desc, String value)
typedef struct
{
    char *desc;
    char *value;
} LIBMATTI_FML_ModAnnotation_EnumHolder;

// Java: the Objects the values Map can hold. Byte/Short/Character/Integer are separate
// objects in Java, the port keeps them all as INT because ASM visits them as a Number.
typedef enum
{
    LIBMATTI_FML_ModAnnotationValue_STRING,
    LIBMATTI_FML_ModAnnotationValue_INT,
    LIBMATTI_FML_ModAnnotationValue_LONG,
    LIBMATTI_FML_ModAnnotationValue_DOUBLE,
    LIBMATTI_FML_ModAnnotationValue_FLOAT,
    LIBMATTI_FML_ModAnnotationValue_BOOLEAN,
    LIBMATTI_FML_ModAnnotationValue_TYPE,
    LIBMATTI_FML_ModAnnotationValue_ENUM,
    // Java: Map<String, Object> - the values of a child annotation
    LIBMATTI_FML_ModAnnotationValue_ANNOTATION,
    // Java: List<Object> - the arrayList after endArray()
    LIBMATTI_FML_ModAnnotationValue_LIST
} LIBMATTI_FML_ModAnnotationValueKind;

typedef struct LIBMATTI_FML_ModAnnotationValues LIBMATTI_FML_ModAnnotationValues;
typedef struct LIBMATTI_FML_ModAnnotationValue LIBMATTI_FML_ModAnnotationValue;
typedef struct LIBMATTI_FML_ModAnnotation LIBMATTI_FML_ModAnnotation;
typedef struct LIBMATTI_FML_ModAnnotationList LIBMATTI_FML_ModAnnotationList;

// Java: Map<String, Object> - and the list an array annotation is built into
// (its names are NULL, because ASM calls visit(null, value) for the elements).
struct LIBMATTI_FML_ModAnnotationValues
{
    char **names;
    LIBMATTI_FML_ModAnnotationValue **values;
    size_t count;
};

struct LIBMATTI_FML_ModAnnotationValue
{
    LIBMATTI_FML_ModAnnotationValueKind kind;
    // Java: String
    char *stringValue;
    // Java: Integer/Boolean / Long
    long longValue;
    // Java: Double / Float
    double doubleValue;
    // Java: EnumHolder
    LIBMATTI_FML_ModAnnotation_EnumHolder *enumHolder;
    // Java: the Map of a child annotation / the List of an array
    LIBMATTI_FML_ModAnnotationValues *container;
};

// Java: public class ModAnnotation
struct LIBMATTI_FML_ModAnnotation
{
    LIBMATTI_NEOFORGESPI_ElementType type;
    LIBMATTI_ASM_Type *asmType;
    char *member;
    // Java: private final Map<String, Object> values
    LIBMATTI_FML_ModAnnotationValues *values;
    // Java: private ArrayList<Object> arrayList / private String arrayName
    LIBMATTI_FML_ModAnnotationValues *arrayList;
    char *arrayName;
};

// Java: the LinkedList<ModAnnotation> the ModClassVisitor fills from the visits
// (addFirst on a visit, removeFirst/addLast at the end of a sub annotation).
// The ModClassVisitor owns the annotations of this list.
struct LIBMATTI_FML_ModAnnotationList
{
    LIBMATTI_FML_ModAnnotation **items;
    size_t count;
};

void LIBMATTI_FML_ModAnnotationList_AddFirst(LIBMATTI_FML_ModAnnotationList *list,
                                             LIBMATTI_FML_ModAnnotation *annotation);
void LIBMATTI_FML_ModAnnotationList_AddLast(LIBMATTI_FML_ModAnnotationList *list,
                                            LIBMATTI_FML_ModAnnotation *annotation);
// Java: LinkedList.removeFirst()
LIBMATTI_FML_ModAnnotation *LIBMATTI_FML_ModAnnotationList_RemoveFirst(LIBMATTI_FML_ModAnnotationList *list);
// Java: LinkedList.getFirst()
LIBMATTI_FML_ModAnnotation *LIBMATTI_FML_ModAnnotationList_GetFirst(const LIBMATTI_FML_ModAnnotationList *list);
void LIBMATTI_FML_ModAnnotationList_Free(LIBMATTI_FML_ModAnnotationList *list);

// Java: static AnnotationData fromModAnnotation(Type clazz, ModAnnotation annotation)
// clazz is the class name of the annotated class; the caller frees the result's strings.
LIBMATTI_NEOFORGESPI_AnnotationData LIBMATTI_FML_ModAnnotation_FromModAnnotation(
    const char *clazz, const LIBMATTI_FML_ModAnnotation *annotation);

// Java: public ModAnnotation(ElementType type, Type asmType, String member)
// Java keeps the Type reference; the annotation takes ownership of it here.
LIBMATTI_FML_ModAnnotation *LIBMATTI_FML_ModAnnotation_New(LIBMATTI_NEOFORGESPI_ElementType type,
                                                           LIBMATTI_ASM_Type *asmType, const char *member);
// Java: public ModAnnotation(Type asmType, ModAnnotation parent)
LIBMATTI_FML_ModAnnotation *LIBMATTI_FML_ModAnnotation_NewChildAnnotation(LIBMATTI_ASM_Type *asmType,
                                                                         const LIBMATTI_FML_ModAnnotation *parent);
void LIBMATTI_FML_ModAnnotation_Free(LIBMATTI_FML_ModAnnotation *annotation);

// Java: getType() / getASMType() / getMember() / getValues()
LIBMATTI_NEOFORGESPI_ElementType LIBMATTI_FML_ModAnnotation_GetType(const LIBMATTI_FML_ModAnnotation *annotation);
const LIBMATTI_ASM_Type *LIBMATTI_FML_ModAnnotation_GetASMType(const LIBMATTI_FML_ModAnnotation *annotation);
const char *LIBMATTI_FML_ModAnnotation_GetMember(const LIBMATTI_FML_ModAnnotation *annotation);
const LIBMATTI_FML_ModAnnotationValues *LIBMATTI_FML_ModAnnotation_GetValues(
    const LIBMATTI_FML_ModAnnotation *annotation);

// Java: public void addArray(String name)
void LIBMATTI_FML_ModAnnotation_AddArray(LIBMATTI_FML_ModAnnotation *annotation, const char *name);
// Java: public void addProperty(String key, Object value) - the value becomes owned here
void LIBMATTI_FML_ModAnnotation_AddProperty(LIBMATTI_FML_ModAnnotation *annotation, const char *key,
                                            LIBMATTI_FML_ModAnnotationValue *value);
// Java: public void addEnumProperty(String key, String enumName, String value)
void LIBMATTI_FML_ModAnnotation_AddEnumProperty(LIBMATTI_FML_ModAnnotation *annotation, const char *key,
                                                const char *enumName, const char *value);
// Java: public void endArray()
void LIBMATTI_FML_ModAnnotation_EndArray(LIBMATTI_FML_ModAnnotation *annotation);
// Java: public ModAnnotation addChildAnnotation(String name, String desc)
LIBMATTI_FML_ModAnnotation *LIBMATTI_FML_ModAnnotation_AddChildAnnotation(LIBMATTI_FML_ModAnnotation *annotation,
                                                                         const char *name, const char *desc);

// Java: the Objects AnnotationVisitor.visit gets
LIBMATTI_FML_ModAnnotationValue *LIBMATTI_FML_ModAnnotationValue_OfString(const char *value);
LIBMATTI_FML_ModAnnotationValue *LIBMATTI_FML_ModAnnotationValue_OfInt(int value);
LIBMATTI_FML_ModAnnotationValue *LIBMATTI_FML_ModAnnotationValue_OfLong(long value);
LIBMATTI_FML_ModAnnotationValue *LIBMATTI_FML_ModAnnotationValue_OfDouble(double value);
LIBMATTI_FML_ModAnnotationValue *LIBMATTI_FML_ModAnnotationValue_OfFloat(float value);
LIBMATTI_FML_ModAnnotationValue *LIBMATTI_FML_ModAnnotationValue_OfBoolean(int value);
LIBMATTI_FML_ModAnnotationValue *LIBMATTI_FML_ModAnnotationValue_OfType(const char *descriptor);

// Java: ModFileScanData.AnnotationData carries the values as Strings in the port;
// this renders one value for that map (caller frees).
char *LIBMATTI_FML_ModAnnotationValue_ToString(const LIBMATTI_FML_ModAnnotationValue *value);
void LIBMATTI_FML_ModAnnotationValue_Free(LIBMATTI_FML_ModAnnotationValue *value);
void LIBMATTI_FML_ModAnnotationValues_Free(LIBMATTI_FML_ModAnnotationValues *values);

#endif //MATTICRAFT_FML_LOADING_MODSCAN_MODANNOTATION_H
