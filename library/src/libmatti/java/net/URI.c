//
// Created by administrator on 09.09.26.
//

#include "URI.h"

#include <stdlib.h>
#include <string.h>

static char *uri_substr(const char *start, const char *end)
{
    size_t len = (size_t)(end - start);
    char *result = malloc(len + 1);
    memcpy(result, start, len);
    result[len] = '\0';
    return result;
}

LIBMATTI_JN_URI *LIBMATTI_JN_URI_Create(const char *uri)
{
    if (uri == NULL) return NULL; // Java: new URI(null) -> NullPointerException

    LIBMATTI_JN_URI *result = calloc(1, sizeof(LIBMATTI_JN_URI));
    result->raw = strdup(uri);
    result->port = -1;

    const char *end = uri + strlen(uri);
    const char *rest = uri;

    // fragment: everything after '#'
    const char *fragment = strchr(rest, '#');
    if (fragment != NULL)
        result->fragment = uri_substr(fragment + 1, end);

    // query: '?' before the fragment
    const char *queryEnd = fragment != NULL ? fragment : end;
    const char *query = rest;
    while (query < queryEnd && *query != '?') query++;
    if (query < queryEnd)
        result->query = uri_substr(query + 1, queryEnd);

    const char *mainEnd = query < queryEnd ? query : queryEnd;

    // scheme: up to the first ':' before any '/', '?' or '#'
    const char *colon = rest;
    while (colon < mainEnd && *colon != ':' && *colon != '/' && *colon != '?' && *colon != '#') colon++;

    const char *authorityStart = rest;
    if (colon < mainEnd && *colon == ':')
    {
        result->scheme = uri_substr(rest, colon);
        authorityStart = colon + 1;
    }

    // authority: '//' up to the next '/', '?' or '#'
    const char *pathStart = authorityStart;
    if (authorityStart + 1 < mainEnd && authorityStart[0] == '/' && authorityStart[1] == '/')
    {
        const char *authEnd = authorityStart + 2;
        while (authEnd < mainEnd && *authEnd != '/' && *authEnd != '?' && *authEnd != '#') authEnd++;

        const char *auth = authorityStart + 2;
        const char *at = auth;
        while (at < authEnd && *at != '@') at++;
        if (at < authEnd)
        {
            result->userInfo = uri_substr(auth, at);
            auth = at + 1;
        }

        const char *portSep = auth;
        while (portSep < authEnd && *portSep != ':') portSep++;
        if (portSep < authEnd)
        {
            result->host = uri_substr(auth, portSep);
            // port must be digits, otherwise invalid (Java: URISyntaxException)
            for (const char *p = portSep + 1; p < authEnd; p++)
            {
                if (*p < '0' || *p > '9')
                {
                    LIBMATTI_JN_URI_Free(result);
                    return NULL;
                }
            }
            result->port = atoi(portSep + 1);
        }
        else
        {
            result->host = uri_substr(auth, authEnd);
        }

        result->authority = uri_substr(authorityStart + 2, authEnd);
        pathStart = authEnd;
    }

    result->schemeSpecificPart = uri_substr(authorityStart, mainEnd);

    if (pathStart < mainEnd)
        result->path = uri_substr(pathStart, mainEnd);

    return result;
}

void LIBMATTI_JN_URI_Free(LIBMATTI_JN_URI *uri)
{
    if (uri == NULL) return;

    free(uri->raw);
    free(uri->scheme);
    free(uri->schemeSpecificPart);
    free(uri->authority);
    free(uri->userInfo);
    free(uri->host);
    free(uri->path);
    free(uri->query);
    free(uri->fragment);
    free(uri);
}

const char *LIBMATTI_JN_URI_ToString(const LIBMATTI_JN_URI *uri)
{
    return uri->raw;
}

const char *LIBMATTI_JN_URI_Scheme(const LIBMATTI_JN_URI *uri)
{
    return uri->scheme;
}

const char *LIBMATTI_JN_URI_SchemeSpecificPart(const LIBMATTI_JN_URI *uri)
{
    return uri->schemeSpecificPart;
}

const char *LIBMATTI_JN_URI_Authority(const LIBMATTI_JN_URI *uri)
{
    return uri->authority;
}

const char *LIBMATTI_JN_URI_UserInfo(const LIBMATTI_JN_URI *uri)
{
    return uri->userInfo;
}

const char *LIBMATTI_JN_URI_Host(const LIBMATTI_JN_URI *uri)
{
    return uri->host;
}

int LIBMATTI_JN_URI_Port(const LIBMATTI_JN_URI *uri)
{
    return uri->port;
}

const char *LIBMATTI_JN_URI_Path(const LIBMATTI_JN_URI *uri)
{
    return uri->path;
}

const char *LIBMATTI_JN_URI_Query(const LIBMATTI_JN_URI *uri)
{
    return uri->query;
}

const char *LIBMATTI_JN_URI_Fragment(const LIBMATTI_JN_URI *uri)
{
    return uri->fragment;
}