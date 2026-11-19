





#ifndef MATTICRAFT_NEOFORGE_NETWORK_REGISTRATION_NETWORKREGISTRY_H
#define MATTICRAFT_NEOFORGE_NETWORK_REGISTRATION_NETWORKREGISTRY_H

// Java: public static final List<Integer> SUPPORTED_COMMON_NETWORKING_VERSIONS = List.of(1)
#define LIBMATTI_NEOFORGE_NetworkRegistry_SUPPORTED_COMMON_NETWORKING_VERSIONS 1

// Java: public static void setup()
void LIBMATTI_NEOFORGE_NetworkRegistry_Setup(void);

// Java: protected static boolean setup
int LIBMATTI_NEOFORGE_NetworkRegistry_IsSetup(void);

// Java: public static <T, B> void register(...) - throws after the registration phase
int LIBMATTI_NEOFORGE_NetworkRegistry_Register(const char *payloadId, const char *version, int optional);

#endif
