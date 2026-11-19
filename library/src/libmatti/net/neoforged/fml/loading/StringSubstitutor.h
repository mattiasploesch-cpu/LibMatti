



#ifndef MATTICRAFT_FML_LOADING_STRINGSUBSTITUTOR_H
#define MATTICRAFT_FML_LOADING_STRINGSUBSTITUTOR_H

// Java: net.neoforged.fml.loading.moddiscovery.ModFile
typedef struct LIBMATTI_FML_ModFile LIBMATTI_FML_ModFile;

// Java: public static String replace(String in, ModFile file) - returns a new string
char *LIBMATTI_FML_StringSubstitutor_Replace(const char *in, LIBMATTI_FML_ModFile *file);

#endif
