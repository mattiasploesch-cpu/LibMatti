// Port of net.neoforged.neoforge.client.network.registration.ClientNetworkRegistry.
// The client-side payload handler maps live with the game port; the port keeps the setup
// lifecycle (guards against the main registry and double setup, fires the client event).

#ifndef MATTICRAFT_NEOFORGE_CLIENT_NETWORK_REGISTRATION_CLIENTNETWORKREGISTRY_H
#define MATTICRAFT_NEOFORGE_CLIENT_NETWORK_REGISTRATION_CLIENTNETWORKREGISTRY_H

// Java: public static void setup()
void LIBMATTI_NEOFORGE_ClientNetworkRegistry_Setup(void);

// Java: private static boolean setupClient
int LIBMATTI_NEOFORGE_ClientNetworkRegistry_IsSetup(void);

#endif //MATTICRAFT_NEOFORGE_CLIENT_NETWORK_REGISTRATION_CLIENTNETWORKREGISTRY_H
