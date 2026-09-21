//
// Created by administrator on 09.09.26.
//
// Port of cpw.mods.jarhandling.impl.JarSigningData.

#ifndef MATTICRAFT_JARSIGNINGDATA_H
#define MATTICRAFT_JARSIGNINGDATA_H

#include "libmatti/bsl/sjh/jarhandling/SecureJar.h"

// Java: class JarSigningData
// TODO: JAR signature verification (JCA: Signature, MessageDigest, CodeSigner,
// certificates). Requires a C crypto library. Until then the SecureJar status
// methods return NONE and hasSecurityData() returns 0.
typedef struct
{
    int placeholder; // TODO: signing state
} LIBMATTI_JH_JarSigningData;

#endif //MATTICRAFT_JARSIGNINGDATA_H