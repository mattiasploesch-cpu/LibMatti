// Port of java.lang.reflect.Modifier.

#ifndef MATTICRAFT_JAVA_LANG_REFLECT_MODIFIER_H
#define MATTICRAFT_JAVA_LANG_REFLECT_MODIFIER_H

// Java: public static final int PUBLIC = 0x00000001;
#define LIBMATTI_JL_Modifier_PUBLIC 0x00000001
// Java: public static final int PRIVATE = 0x00000002;
#define LIBMATTI_JL_Modifier_PRIVATE 0x00000002
// Java: public static final int PROTECTED = 0x00000004;
#define LIBMATTI_JL_Modifier_PROTECTED 0x00000004
// Java: public static final int STATIC = 0x00000008;
#define LIBMATTI_JL_Modifier_STATIC 0x00000008
// Java: public static final int FINAL = 0x00000010;
#define LIBMATTI_JL_Modifier_FINAL 0x00000010
// Java: public static final int SYNCHRONIZED = 0x00000020;
#define LIBMATTI_JL_Modifier_SYNCHRONIZED 0x00000020
// Java: public static final int VOLATILE = 0x00000040;
#define LIBMATTI_JL_Modifier_VOLATILE 0x00000040
// Java: public static final int TRANSIENT = 0x00000080;
#define LIBMATTI_JL_Modifier_TRANSIENT 0x00000080
// Java: public static final int NATIVE = 0x00000100;
#define LIBMATTI_JL_Modifier_NATIVE 0x00000100
// Java: public static final int INTERFACE = 0x00000200;
#define LIBMATTI_JL_Modifier_INTERFACE 0x00000200
// Java: public static final int ABSTRACT = 0x00000400;
#define LIBMATTI_JL_Modifier_ABSTRACT 0x00000400
// Java: public static final int STRICT = 0x00000800;
#define LIBMATTI_JL_Modifier_STRICT 0x00000800

// Java: public static boolean isPublic(int mod)
int LIBMATTI_JL_Modifier_IsPublic(int mod);
// Java: public static boolean isStatic(int mod)
int LIBMATTI_JL_Modifier_IsStatic(int mod);
// Java: public static boolean isFinal(int mod)
int LIBMATTI_JL_Modifier_IsFinal(int mod);
// Java: public static boolean isAbstract(int mod)
int LIBMATTI_JL_Modifier_IsAbstract(int mod);
// Java: public static boolean isInterface(int mod)
int LIBMATTI_JL_Modifier_IsInterface(int mod);

#endif //MATTICRAFT_JAVA_LANG_REFLECT_MODIFIER_H
