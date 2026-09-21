// Port of net.neoforged.fml.util.PathPrettyPrinting.

#ifndef MATTICRAFT_FML_UTIL_PATHPRETTYPRINTING_H
#define MATTICRAFT_FML_UTIL_PATHPRETTYPRINTING_H

// Java: public static void addRoot(Path root)
void LIBMATTI_FML_PathPrettyPrinting_AddRoot(const char *root);
// Java: public static void addSubstitution(Path root, String prefix, String suffix)
void LIBMATTI_FML_PathPrettyPrinting_AddSubstitution(const char *root, const char *prefix, const char *suffix);
// Java: public static String prettyPrint(Path path)
char *LIBMATTI_FML_PathPrettyPrinting_PrettyPrint(const char *path);

#endif //MATTICRAFT_FML_UTIL_PATHPRETTYPRINTING_H
