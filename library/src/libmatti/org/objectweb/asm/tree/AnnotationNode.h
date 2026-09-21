// Port of org.objectweb.asm.tree.AnnotationNode.
// Java stores all annotation values in one alternating List<Object> values
// (element name, value, element name, value, ...); the C port keeps the names and
// the values in two parallel arrays.
// The element_value union of the class file format is modelled as a tag plus the
// fields belonging to that tag.

#ifndef MATTICRAFT_ASMT_ANNOTATIONNODE_H
#define MATTICRAFT_ASMT_ANNOTATIONNODE_H

#include <stddef.h>

// Java: the Object the AnnotationNode stores for one element value
typedef struct LIBMATTI_ASMT_AnnotationNode LIBMATTI_ASMT_AnnotationNode;
typedef struct LIBMATTI_ASMT_AnnotationValue LIBMATTI_ASMT_AnnotationValue;

struct LIBMATTI_ASMT_AnnotationValue
{
    // Java: the class file element_value tag
    char tag;
    // Java: Integer / Byte / Short / Character / Boolean
    int intValue;
    // Java: Long / Double
    long long longValue;
    double doubleValue;
    // Java: String
    char *stringValue;
    // Java: Type (the 'c' tag, or the descriptor of a 't' tag)
    char *classDescriptor;
    // Java: String[2] - the enum constant type descriptor and its name ('e' tag)
    char *enumType;
    char *enumConst;
    // Java: AnnotationNode ('@' tag)
    LIBMATTI_ASMT_AnnotationNode *annotation;
    // Java: List<Object> ('[' tag)
    LIBMATTI_ASMT_AnnotationValue **arrayValues;
    size_t arrayValueCount;
};

// Java: public class AnnotationNode
struct LIBMATTI_ASMT_AnnotationNode
{
    // Java: public String desc
    char *desc;
    // Java: public List<Object> values - the element names of the alternating list
    char **valueNames;
    // Java: public List<Object> values - the element values of the alternating list
    LIBMATTI_ASMT_AnnotationValue **values;
    size_t valueCount;
};

// Java: public AnnotationNode(String descriptor)
LIBMATTI_ASMT_AnnotationNode *LIBMATTI_ASMT_AnnotationNode_New(const char *descriptor);
void LIBMATTI_ASMT_AnnotationNode_Free(LIBMATTI_ASMT_AnnotationNode *node);

// Java: public void visit(String name, Object value) - the port adds the parsed value directly
void LIBMATTI_ASMT_AnnotationNode_AddValue(LIBMATTI_ASMT_AnnotationNode *node, const char *name,
                                           LIBMATTI_ASMT_AnnotationValue *value);

// Java: public String desc
const char *LIBMATTI_ASMT_AnnotationNode_Desc(const LIBMATTI_ASMT_AnnotationNode *node);
// Java: values.get(i) - the element name
const char *LIBMATTI_ASMT_AnnotationNode_ValueNameAt(const LIBMATTI_ASMT_AnnotationNode *node, size_t index);
// Java: values.get(i) - the element value
LIBMATTI_ASMT_AnnotationValue *LIBMATTI_ASMT_AnnotationNode_ValueAt(const LIBMATTI_ASMT_AnnotationNode *node,
                                                                   size_t index);
// Java: the values.get(i) loop that looks the element up by name
LIBMATTI_ASMT_AnnotationValue *LIBMATTI_ASMT_AnnotationNode_FindValue(const LIBMATTI_ASMT_AnnotationNode *node,
                                                                     const char *name);

// Java: ((Integer) values.get(i + 1))
int LIBMATTI_ASMT_AnnotationValue_AsInt(const LIBMATTI_ASMT_AnnotationValue *value);
// Java: ((Boolean) values.get(i + 1))
int LIBMATTI_ASMT_AnnotationValue_AsBoolean(const LIBMATTI_ASMT_AnnotationValue *value);
// Java: (String) values.get(i + 1); caller frees
char *LIBMATTI_ASMT_AnnotationValue_AsString(const LIBMATTI_ASMT_AnnotationValue *value);
// Java: ((String[]) values.get(i + 1))[0] - the enum constant type descriptor
const char *LIBMATTI_ASMT_AnnotationValue_EnumType(const LIBMATTI_ASMT_AnnotationValue *value);
// Java: ((String[]) values.get(i + 1))[1] - the enum constant name
const char *LIBMATTI_ASMT_AnnotationValue_EnumConst(const LIBMATTI_ASMT_AnnotationValue *value);
// Java: (Type) values.get(i + 1) - the class descriptor
const char *LIBMATTI_ASMT_AnnotationValue_ClassDescriptor(const LIBMATTI_ASMT_AnnotationValue *value);
// Java: (AnnotationNode) values.get(i + 1)
LIBMATTI_ASMT_AnnotationNode *LIBMATTI_ASMT_AnnotationValue_Annotation(const LIBMATTI_ASMT_AnnotationValue *value);
// Java: (List<Object>) values.get(i + 1) - the array element count
size_t LIBMATTI_ASMT_AnnotationValue_ArrayCount(const LIBMATTI_ASMT_AnnotationValue *value);
// Java: (List<Object>) values.get(i + 1) - the array element at the given index
LIBMATTI_ASMT_AnnotationValue *LIBMATTI_ASMT_AnnotationValue_ArrayAt(const LIBMATTI_ASMT_AnnotationValue *value,
                                                                    size_t index);

void LIBMATTI_ASMT_AnnotationValue_Free(LIBMATTI_ASMT_AnnotationValue *value);

#endif //MATTICRAFT_ASMT_ANNOTATIONNODE_H
