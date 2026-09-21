//
// Created by administrator on 09.09.26.
//
// Port of cpw.mods.jarhandling.impl.SecureJarVerifier.

#ifndef MATTICRAFT_SECUREJARVERIFIER_H
#define MATTICRAFT_SECUREJARVERIFIER_H

#include "libmatti/bsl/sjh/jarhandling/SecureJar.h"

// Java: class SecureJarVerifier
// TODO: jar verification (Signature, certificates, PKCS7). Not implemented.
typedef struct
{
    int placeholder;
} LIBMATTI_JH_SecureJarVerifier;

// Java: static boolean hasSignatures(JarVerifier) - TODO stub returning 0
int LIBMATTI_JH_SecureJarVerifier_HasSignatures(void);

#endif //MATTICRAFT_SECUREJARVERIFIER_H