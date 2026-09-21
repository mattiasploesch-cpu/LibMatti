#ifndef MATTICRAFT_JAVA_NET_URL_H
#define MATTICRAFT_JAVA_NET_URL_H

// Port of java.net.URL.
typedef struct
{
    char *value;
} LIBMATTI_JN_URL;

// Java: public URL(String spec)
LIBMATTI_JN_URL *LIBMATTI_JN_URL_New(const char *value);
void LIBMATTI_JN_URL_Free(LIBMATTI_JN_URL *url);

// Java: public String toString()
const char *LIBMATTI_JN_URL_ToString(const LIBMATTI_JN_URL *url);

#endif //MATTICRAFT_JAVA_NET_URL_H
