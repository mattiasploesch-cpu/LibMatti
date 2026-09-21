// Port of net.neoforged.fml.jarcontents.PathNormalization.

#ifndef MATTICRAFT_FML_JARCONTENTS_PATHNORMALIZATION_H
#define MATTICRAFT_FML_JARCONTENTS_PATHNORMALIZATION_H

// Java: public static void assertNormalized(CharSequence path) - 0 = rejected (Java: throws)
int LIBMATTI_FML_PathNormalization_AssertNormalized(const char *path);
// Java: public static boolean isNormalized(CharSequence path)
int LIBMATTI_FML_PathNormalization_IsNormalized(const char *path);
// Java: public static String normalize(CharSequence path) - returns a new string
char *LIBMATTI_FML_PathNormalization_Normalize(const char *path);
// Java: public static String normalizeFolderPrefix(CharSequence path) - returns a new string
char *LIBMATTI_FML_PathNormalization_NormalizeFolderPrefix(const char *path);

#endif //MATTICRAFT_FML_JARCONTENTS_PATHNORMALIZATION_H
