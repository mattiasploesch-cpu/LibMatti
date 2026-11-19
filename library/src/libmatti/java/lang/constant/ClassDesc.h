





#ifndef MATTICRAFT_JL_CONSTANT_CLASSDESC_H
#define MATTICRAFT_JL_CONSTANT_CLASSDESC_H

// Java: public static ClassDesc of(String name)
// Java throws IllegalArgumentException for an invalid binary class name; the C port reports that as 0.
int LIBMATTI_JLC_ClassDesc_Of(const char *name);

#endif
