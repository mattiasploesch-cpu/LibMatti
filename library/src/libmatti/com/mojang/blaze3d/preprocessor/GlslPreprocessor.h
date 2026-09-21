// Port of com.mojang.blaze3d.preprocessor.GlslPreprocessor.
// Java splits the source into a String list through the moj_import regex and
// emits #line directives; the C port keeps the two-pass structure (imports
// first, then version lifting) over one output buffer with line bookkeeping.

#ifndef MATTICRAFT_BLAZE3D_PREPROCESSOR_GLSLPREPROCESSOR_H
#define MATTICRAFT_BLAZE3D_PREPROCESSOR_GLSLPREPROCESSOR_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Java: public abstract class GlslPreprocessor - the import resolver hook.
// location is the raw import path; quoted imports get the "relative to this
// file" prefix behaviour through the prefix argument, angle imports resolve
// against shaders/include/. Returns a malloc'ed string or NULL.
typedef char *(*LIBMATTI_B3D_GlslPreprocessor_ApplyImport)(void *userdata, const char *prefix,
                                                           const char *location);

// Java: public final String process(String) - resolves #moj_import and lifts
// every #version to the maximum. Returns a malloc'ed buffer, *outSize set.
char *LIBMATTI_B3D_GlslPreprocessor_Process(const char *source,
                                             LIBMATTI_B3D_GlslPreprocessor_ApplyImport applyImport,
                                             void *applyImportUserdata,
                                             size_t *outSize);

// Java: public static String injectDefines(String, ShaderDefines) - inserts
// the define block after the first line. Returns a malloc'ed string; defines
// is a NULL-terminated array of "NAME value" (or "NAME") directive lines.
char *LIBMATTI_B3D_GlslPreprocessor_InjectDefines(const char *source, const char *const *defines);

#ifdef __cplusplus
}
#endif

#endif
