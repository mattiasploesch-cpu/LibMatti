// Port of javax.lang.model.SourceVersion (identifier and keyword checking).

#ifndef MATTICRAFT_JAVAX_LANG_MODEL_SOURCEVERSION_H
#define MATTICRAFT_JAVAX_LANG_MODEL_SOURCEVERSION_H

// Java: public static boolean isIdentifier(CharSequence name)
int LIBMATTI_SV_SourceVersion_IsIdentifier(const char *name);
// Java: public static boolean isKeyword(CharSequence name)
int LIBMATTI_SV_SourceVersion_IsKeyword(const char *name);
// Java: public static boolean isName(CharSequence name)
int LIBMATTI_SV_SourceVersion_IsName(const char *name);

#endif //MATTICRAFT_JAVAX_LANG_MODEL_SOURCEVERSION_H
