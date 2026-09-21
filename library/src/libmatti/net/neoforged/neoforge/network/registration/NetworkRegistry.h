// Port of net.neoforged.neoforge.network.registration.NetworkRegistry.
// The payload handling/negotiation runs over the Minecraft Connection objects, which are the
// game port's part; the port keeps the registration state machine (setup phase, the payload
// registrations collected through RegisterPayloadHandlersEvent, and the post-setup guard).

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

#endif //MATTICRAFT_NEOFORGE_NETWORK_REGISTRATION_NETWORKREGISTRY_H
