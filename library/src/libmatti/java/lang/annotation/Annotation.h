// Port of the java.lang.annotation model the scan data needs.
// Java's Annotation is a marker interface; annotations are compared by their class and the
// ElementType positions them. The C port keys an annotation by its class name (the ASM Type's
// class name) and exposes the ElementType enum used by the scan records.

#ifndef MATTICRAFT_JAVA_LANG_ANNOTATION_ANNOTATION_H
#define MATTICRAFT_JAVA_LANG_ANNOTATION_ANNOTATION_H

// Java: package java.lang.annotation - the interface the Class<? extends Annotation> lookups
// use; the port's name-based key is the annotation's binary class name
#define LIBMATTI_JL_ANNOTATION_INTERFACE_NAME "java.lang.annotation.Annotation"

// Java: public enum RetentionPolicy { SOURCE, CLASS, RUNTIME }
typedef enum
{
    LIBMATTI_JL_RetentionPolicy_SOURCE = 0,
    LIBMATTI_JL_RetentionPolicy_CLASS,
    LIBMATTI_JL_RetentionPolicy_RUNTIME
} LIBMATTI_JL_RetentionPolicy;

// Java: an annotation instance the scan data hands around (binary class name + the retention
// the visitor saw); the values stay with the AnnotationData record
typedef struct
{
    char *className;   // owned
    LIBMATTI_JL_RetentionPolicy retention;
} LIBMATTI_JL_Annotation;

LIBMATTI_JL_Annotation *LIBMATTI_JL_Annotation_New(const char *className, LIBMATTI_JL_RetentionPolicy retention);
void LIBMATTI_JL_Annotation_Free(LIBMATTI_JL_Annotation *annotation);

#endif //MATTICRAFT_JAVA_LANG_ANNOTATION_ANNOTATION_H
