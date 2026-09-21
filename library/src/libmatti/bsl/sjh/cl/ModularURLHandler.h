// Port of cpw.mods.cl.ModularURLHandler (java.net.URLStreamHandlerFactory).

#ifndef MATTICRAFT_MODULARURLHANDLER_H
#define MATTICRAFT_MODULARURLHANDLER_H

#include "libmatti/java/io/InputStream.h"
#include "libmatti/java/net/URI.h"

#include <stddef.h>

// Java: java.net.URL - not ported; URI used as stand-in
typedef LIBMATTI_JN_URI LIBMATTI_CL_URL;

typedef struct LIBMATTI_CL_ModuleLayer LIBMATTI_CL_ModuleLayer;

// Java: interface IURLProvider
typedef struct
{
    const char *(*protocol)(void);
    LIBMATTI_JI_InputStream *(*inputStreamFunction)(LIBMATTI_CL_URL *url);
    long (*getLastModified)(LIBMATTI_CL_URL *url); // Java: default 0
    long (*getContentLength)(LIBMATTI_CL_URL *url); // Java: default -1
} LIBMATTI_CL_IURLProvider;

// Java: static class FunctionURLStreamHandler extends URLStreamHandler
typedef struct
{
    LIBMATTI_CL_IURLProvider *iurlProvider;
} LIBMATTI_CL_FunctionURLStreamHandler;

// Java: static class FunctionURLConnection extends URLConnection
typedef struct
{
    LIBMATTI_CL_URL *url;
    LIBMATTI_CL_IURLProvider *provider;
} LIBMATTI_CL_FunctionURLConnection;

// Java: class ModularURLHandler implements URLStreamHandlerFactory
typedef struct
{
    // Java: Map<String, IURLProvider> handlers
    char **protocols;
    LIBMATTI_CL_IURLProvider **handlers;
    size_t count;
} LIBMATTI_CL_ModularURLHandler;

// Java: INSTANCE
LIBMATTI_CL_ModularURLHandler *LIBMATTI_CL_ModularURLHandler_GetInstance(void);

// Java: static void initFrom(@Nullable ModuleLayer layer)
void LIBMATTI_CL_ModularURLHandler_InitFrom(LIBMATTI_CL_ModularURLHandler *handler, LIBMATTI_CL_ModuleLayer *layer);

// Java: URLStreamHandler createURLStreamHandler(String protocol); NULL when not found
LIBMATTI_CL_FunctionURLStreamHandler *LIBMATTI_CL_ModularURLHandler_CreateURLStreamHandler(const LIBMATTI_CL_ModularURLHandler *handler, const char *protocol);

// Java: URLConnection openConnection(URL u)
LIBMATTI_CL_FunctionURLConnection *LIBMATTI_CL_FunctionURLStreamHandler_OpenConnection(const LIBMATTI_CL_FunctionURLStreamHandler *streamHandler, LIBMATTI_CL_URL *url);

// Java: void connect() {}
void LIBMATTI_CL_FunctionURLConnection_Connect(const LIBMATTI_CL_FunctionURLConnection *connection);
// Java: InputStream getInputStream()
LIBMATTI_JI_InputStream *LIBMATTI_CL_FunctionURLConnection_GetInputStream(const LIBMATTI_CL_FunctionURLConnection *connection);
// Java: int getContentLength()
int LIBMATTI_CL_FunctionURLConnection_GetContentLength(const LIBMATTI_CL_FunctionURLConnection *connection);
// Java: long getContentLengthLong()
long LIBMATTI_CL_FunctionURLConnection_GetContentLengthLong(const LIBMATTI_CL_FunctionURLConnection *connection);
// Java: long getLastModified()
long LIBMATTI_CL_FunctionURLConnection_GetLastModified(const LIBMATTI_CL_FunctionURLConnection *connection);

#endif //MATTICRAFT_MODULARURLHANDLER_H