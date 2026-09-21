// Port of net.neoforged.fml.startup.Client (the entrypoint of the neoforge
// launch target).
//
// Java: Client.main returns void - the JVM would exit with 0 once it returns.
// The port's Client_Main returns the status the game main returned, so the
// launch chain can propagate the game's exit code as the process exit status.

#ifndef MATTICRAFT_FML_CLIENT_H
#define MATTICRAFT_FML_CLIENT_H

#include "libmatti/net/neoforged/fml/startup/Entrypoint.h"
#include "libmatti/net/neoforged/fml/loading/ProgramArgs.h"
#include "libmatti/net/neoforged/fml/startup/FatalErrorReporting.h"

// Java: public static void main(String[] args) -> the port returns the game's exit status
int LIBMATTI_FML_Client_Main(int argc, char *argv[]);

typedef struct
{
    int unused;
} LIBMATTI_FML_Client;

// Java: private Client() {}
LIBMATTI_FML_Client *LIBMATTI_FML_Client_New(void);
void LIBMATTI_FML_Client_Free(LIBMATTI_FML_Client *client);

#endif //MATTICRAFT_FML_CLIENT_H
