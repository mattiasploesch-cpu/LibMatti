//
// The selftest phase: see selftest.h. The harnesses are built into the same
// build tree as matticraft; this file runs them with their fixture arguments
// (the same commands the ctest entries use) and reports each result.
//
// The fixture .so files come straight from the test targets, so the paths are
// fixed by the build: <build>/tests/<fixture>.so. The absolute build and source
// folder reach this file as compile definitions from client/CMakeLists.txt.
//
// The runClient target skips this phase entirely (--no-tests in its run
// configuration): it has no test harnesses as build dependencies.
//

#include "selftest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef MATTI_BUILD_DIR
#define MATTI_BUILD_DIR "."
#endif

#ifndef MATTI_SOURCE_DIR
#define MATTI_SOURCE_DIR "."
#endif

typedef struct
{
    const char *name;    // reported in the summary line
    const char *binary;  // the harness binary, relative to the build folder
    const char *args[4]; // fixture arguments, NULL terminated
} SelftestEntry;

static const SelftestEntry SELFTESTS[] = {
    {"modscan", MATTI_BUILD_DIR "/tests/test_modscan",
     {MATTI_SOURCE_DIR "/tests/modscan/out/example/ExampleMod.class",
      MATTI_SOURCE_DIR "/tests/modscan/out/example/ExampleSubscriber.class", NULL, NULL}},
    {"soscan", MATTI_BUILD_DIR "/tests/test_soscan",
     {MATTI_BUILD_DIR "/tests/classmod-fixture.so", NULL, NULL, NULL}},
    {"mixinfacade", MATTI_BUILD_DIR "/tests/test_mixinfacade",
     {MATTI_BUILD_DIR "/tests/mixina-fixture.so",
      MATTI_BUILD_DIR "/tests/mixinb-fixture.so", NULL, NULL}},
    {"m4proof", MATTI_BUILD_DIR "/tests/test_m4proof",
     {MATTI_BUILD_DIR "/tests/classmod-fixture.so", NULL, NULL, NULL}},
};

#define SELFTEST_COUNT (sizeof(SELFTESTS) / sizeof(SELFTESTS[0]))

// The executable's own directory: the harnesses sit next to it in the build tree.
static int selftest_run_one(const SelftestEntry *entry)
{
    char *argv[7] = {(char *) entry->binary};
    int argc = 1;
    for (int i = 0; i < 4 && entry->args[i] != NULL; i++)
        argv[argc++] = (char *) entry->args[i];
    argv[argc] = NULL;

    pid_t pid = fork();
    if (pid < 0) return -1;
    if (pid == 0)
    {
        execv(entry->binary, argv);
        // execv only returns on failure
        _exit(127);
    }

    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return -1;
}

int selftest_run_all(void)
{
    // The harness binaries are the test targets of the full matticraft build; runClient
    // (no test dependencies) starts directly instead of probing for missing binaries.
    printf("[SELFTEST] running %d libmatti test harness(es)\n", (int) SELFTEST_COUNT);

    int failed = 0;
    for (size_t i = 0; i < SELFTEST_COUNT; i++)
    {
        const SelftestEntry *entry = &SELFTESTS[i];
        int code = selftest_run_one(entry);
        if (code == 0)
            printf("[SELFTEST] %s: ok\n", entry->name);
        else
        {
            printf("[SELFTEST] %s: FAILED (exit %d)\n", entry->name, code);
            failed++;
        }
    }

    if (failed == 0)
        printf("[SELFTEST] all %d harness(es) passed\n", (int) SELFTEST_COUNT);
    else
        printf("[SELFTEST] %d of %d harness(es) FAILED\n", failed, (int) SELFTEST_COUNT);
    return failed;
}
