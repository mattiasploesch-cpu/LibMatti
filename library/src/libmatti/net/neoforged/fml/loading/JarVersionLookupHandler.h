// Port of net.neoforged.fml.loading.JarVersionLookupHandler.

#ifndef MATTICRAFT_FML_LOADING_JARVERSIONLOOKUPHANDLER_H
#define MATTICRAFT_FML_LOADING_JARVERSIONLOOKUPHANDLER_H

#include "libmatti/java/lang/Class.h"

// Java: public static Optional<String> getVersion(Class<?> clazz)
// 1 when a version was found; *out points at the version (the caller copies it).
int LIBMATTI_FML_JarVersionLookupHandler_GetVersion(const LIBMATTI_JL_Class *clazz, const char **out);

#endif //MATTICRAFT_FML_LOADING_JARVERSIONLOOKUPHANDLER_H
