





#ifndef MATTICRAFT_JL_CONSTANT_METHODTYPEDESC_H
#define MATTICRAFT_JL_CONSTANT_METHODTYPEDESC_H

// Java: public static MethodTypeDesc ofDescriptor(String descriptor)
// Java throws IllegalArgumentException for a malformed descriptor; the C port reports that as 0.
int LIBMATTI_JLC_MethodTypeDesc_OfDescriptor(const char *descriptor);

#endif
