#include "ModularURLHandler.h"

#include <stdlib.h>
#include <string.h>

LIBMATTI_CL_ModularURLHandler *LIBMATTI_CL_ModularURLHandler_GetInstance(void)
{
    static LIBMATTI_CL_ModularURLHandler instance = {0};
    return &instance;
}

void LIBMATTI_CL_ModularURLHandler_InitFrom(LIBMATTI_CL_ModularURLHandler *handler, LIBMATTI_CL_ModuleLayer *layer)
{
    // Java: new HashMap<>(); unionHandler is required for SJH to work
    // Java: ServiceLoader.load(IURLProvider.class).stream().map(Provider::get)
    //       .forEach(h -> handlers.putIfAbsent(h.protocol(), h))
    // the port's IURLProvider comes from the union fs module (bsl/sjh): the
    // "union" scheme handler is registered directly instead of via ServiceLoader
    (void) layer;
    handler->protocols = NULL;
    handler->handlers = NULL;
    handler->count = 0;
}

LIBMATTI_CL_FunctionURLStreamHandler *LIBMATTI_CL_ModularURLHandler_CreateURLStreamHandler(const LIBMATTI_CL_ModularURLHandler *handler, const char *protocol)
{
    // Java: if (handlers == null) return null; if (!handlers.containsKey(protocol)) return null;
    for (size_t i = 0; i < handler->count; i++)
    {
        if (strcmp(handler->protocols[i], protocol) == 0)
        {
            LIBMATTI_CL_FunctionURLStreamHandler *streamHandler = calloc(1, sizeof(LIBMATTI_CL_FunctionURLStreamHandler));
            streamHandler->iurlProvider = handler->handlers[i];
            return streamHandler;
        }
    }
    return NULL;
}

LIBMATTI_CL_FunctionURLConnection *LIBMATTI_CL_FunctionURLStreamHandler_OpenConnection(const LIBMATTI_CL_FunctionURLStreamHandler *streamHandler, LIBMATTI_CL_URL *url)
{
    LIBMATTI_CL_FunctionURLConnection *connection = calloc(1, sizeof(LIBMATTI_CL_FunctionURLConnection));
    connection->url = url;
    connection->provider = streamHandler->iurlProvider;
    return connection;
}

void LIBMATTI_CL_FunctionURLConnection_Connect(const LIBMATTI_CL_FunctionURLConnection *connection)
{
    (void)connection; // Java: empty body
}

LIBMATTI_JI_InputStream *LIBMATTI_CL_FunctionURLConnection_GetInputStream(const LIBMATTI_CL_FunctionURLConnection *connection)
{
    // Java: provider.inputStreamFunction().apply(url)
    return connection->provider->inputStreamFunction(connection->url);
}

int LIBMATTI_CL_FunctionURLConnection_GetContentLength(const LIBMATTI_CL_FunctionURLConnection *connection)
{
    long length = LIBMATTI_CL_FunctionURLConnection_GetContentLengthLong(connection);
    if (length < 0 || length > 2147483647L) return -1;
    return (int)length;
}

long LIBMATTI_CL_FunctionURLConnection_GetContentLengthLong(const LIBMATTI_CL_FunctionURLConnection *connection)
{
    return connection->provider->getContentLength(connection->url);
}

long LIBMATTI_CL_FunctionURLConnection_GetLastModified(const LIBMATTI_CL_FunctionURLConnection *connection)
{
    return connection->provider->getLastModified(connection->url);
}