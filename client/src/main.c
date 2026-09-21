//
// Created by administrator on 08.09.26.
//

#include "libmatti/main.h"

#include "demo-service.h"
#include "selftest.h"

#include <stdio.h>
#include <string.h>

// Manifest of the matticraft artifact, in the ELF section ".matti_manifest" that
// the jar contents reader exposes as "META-INF/MANIFEST.MF". ModLauncher reads
// its own artifact metadata from it (Java: IEnvironment.class.getPackage()
// .getSpecificationVersion() / .getImplementationVersion()).
__attribute__((used, section(".matti_manifest")))
const char MATTICRAFT_MANIFEST[] =
    "Manifest-Version: 1.0\n"
    "Implementation-Version: 11.0\n"
    "Specification-Version: 11.0\n"
    "\n";

int main(int argc, char *argv[], char *envp[])
{
    // The selftest phase: the tests of libmatti live in the client, so one run
    // of matticraft builds and executes every harness (the same commands ctest
    // runs). Skipped with --no-tests, or compiled out for runClient (the plain
    // client target has no test harnesses as dependencies).
#if MATTICRAFT_CLIENT_ONLY
    int runSelftests = 0;
#else
    int runSelftests = 1;
#endif
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--no-tests") == 0) runSelftests = 0;

    // C has no META-INF/services: declare the demo providers before the
    // launcher reads the registry inside start_from_bsl.
    demo_register_services();

    if (runSelftests && selftest_run_all() != 0)
    {
        fprintf(stderr, "matticraft: selftests failed, not starting the launcher\n");
        fprintf(stderr, "matticraft: run with --no-tests to start anyway\n");
        return 1;
    }

    // Java: BootstrapLauncher.run(...) ends in ModLauncher, which hands over to
    // the launch target's game main (net.minecraft.client.main.Main). The
    // process exit status is the game's: start_from_bsl propagates the status
    // the game main returned through the launcher chain.
    return start_from_bsl(argc, argv, envp);
}
