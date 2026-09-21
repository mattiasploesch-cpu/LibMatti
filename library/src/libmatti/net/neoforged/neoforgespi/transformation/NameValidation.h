// Port of net.neoforged.neoforgespi.transformation.NameValidation (package-private in Java).

#ifndef MATTICRAFT_NEOFORGESPI_TRANSFORMATION_NAMEVALIDATION_H
#define MATTICRAFT_NEOFORGESPI_TRANSFORMATION_NAMEVALIDATION_H

// Java: static void validateClassName(String name)
// Java throws IllegalArgumentException; the C port reports that as 0.
int LIBMATTI_NEOFORGESPI_NameValidation_ValidateClassName(const char *name);
// Java: static void validateUnqualified(String name)
int LIBMATTI_NEOFORGESPI_NameValidation_ValidateUnqualified(const char *name);
// Java: static void validateMethod(String name, String descriptor)
int LIBMATTI_NEOFORGESPI_NameValidation_ValidateMethod(const char *name, const char *descriptor);

#endif //MATTICRAFT_NEOFORGESPI_TRANSFORMATION_NAMEVALIDATION_H
