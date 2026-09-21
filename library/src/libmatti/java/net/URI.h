//
// Created by administrator on 09.09.26.
//
// Port of java.net.URI.

#ifndef MATTICRAFT_URI_H
#define MATTICRAFT_URI_H

typedef struct
{
    char *raw; // Java: toString()
    char *scheme;
    char *schemeSpecificPart;
    char *authority;
    char *userInfo;
    char *host;
    int port; // -1 = undefined (Java: port() == -1)
    char *path;
    char *query;
    char *fragment;
} LIBMATTI_JN_URI;

// Java: new URI(String) - NULL on parse failure (Java: URISyntaxException)
LIBMATTI_JN_URI *LIBMATTI_JN_URI_Create(const char *uri);
void LIBMATTI_JN_URI_Free(LIBMATTI_JN_URI *uri);

const char *LIBMATTI_JN_URI_ToString(const LIBMATTI_JN_URI *uri);
const char *LIBMATTI_JN_URI_Scheme(const LIBMATTI_JN_URI *uri);
const char *LIBMATTI_JN_URI_SchemeSpecificPart(const LIBMATTI_JN_URI *uri);
const char *LIBMATTI_JN_URI_Authority(const LIBMATTI_JN_URI *uri);
const char *LIBMATTI_JN_URI_UserInfo(const LIBMATTI_JN_URI *uri);
const char *LIBMATTI_JN_URI_Host(const LIBMATTI_JN_URI *uri);
int LIBMATTI_JN_URI_Port(const LIBMATTI_JN_URI *uri);
const char *LIBMATTI_JN_URI_Path(const LIBMATTI_JN_URI *uri);
const char *LIBMATTI_JN_URI_Query(const LIBMATTI_JN_URI *uri);
const char *LIBMATTI_JN_URI_Fragment(const LIBMATTI_JN_URI *uri);

#endif //MATTICRAFT_URI_H