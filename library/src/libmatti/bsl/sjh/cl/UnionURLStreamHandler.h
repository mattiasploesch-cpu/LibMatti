// Port of cpw.mods.cl.UnionURLStreamHandler (ModularURLHandler.IURLProvider).

#ifndef MATTICRAFT_UNIONURLSTREAMHANDLER_H
#define MATTICRAFT_UNIONURLSTREAMHANDLER_H

#include "libmatti/bsl/sjh/cl/ModularURLHandler.h"
#include "libmatti/bsl/sjh/niofs/union/UnionPath.h"

// Java: protocol() -> "union"
const char *LIBMATTI_CL_UnionURLStreamHandler_Protocol(void);

// Java: inputStreamFunction() - Paths.get(u.toURI()) instanceof UnionPath -> buildInputStream()
LIBMATTI_JI_InputStream *LIBMATTI_CL_UnionURLStreamHandler_InputStream(LIBMATTI_CL_URL *url);

// Java: getLastModified(URL) - Files.getLastModifiedTime(Paths.get(u.toURI())).toMillis()
long LIBMATTI_CL_UnionURLStreamHandler_GetLastModified(LIBMATTI_CL_URL *url);

// Java: getContentLength(URL) - Files.size(Paths.get(u.toURI()))
long LIBMATTI_CL_UnionURLStreamHandler_GetContentLength(LIBMATTI_CL_URL *url);

// The IURLProvider instance for the union protocol (Java: static unionHandler)
LIBMATTI_CL_IURLProvider *LIBMATTI_CL_UnionURLStreamHandler_GetProvider(void);

#endif //MATTICRAFT_UNIONURLSTREAMHANDLER_H