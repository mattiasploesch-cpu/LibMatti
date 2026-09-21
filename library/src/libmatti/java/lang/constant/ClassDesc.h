// Port of java.lang.constant.ClassDesc.
// of(String) and the binary-name validation are ported, which is everything the
// ported code (NameValidation.validateClassName) uses; the descriptor itself is
// never materialised by the ported code.

#ifndef MATTICRAFT_JL_CONSTANT_CLASSDESC_H
#define MATTICRAFT_JL_CONSTANT_CLASSDESC_H

// Java: public static ClassDesc of(String name)
// Java throws IllegalArgumentException for an invalid binary class name; the C port reports that as 0.
int LIBMATTI_JLC_ClassDesc_Of(const char *name);

#endif //MATTICRAFT_JL_CONSTANT_CLASSDESC_H
