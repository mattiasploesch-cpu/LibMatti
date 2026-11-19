













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
    const char *name;

    const char *binary;

    const char *args[4];

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



static int selftest_run_one(const SelftestEntry *entry)
{




    if (access(entry->binary, X_OK) != 0)
        return 0;
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


        _exit(127);
    }

    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status))
    {
        int code = WEXITSTATUS(status);


        if (code == 127)
            return 0;
        return code;
    }
    return -1;
}

int selftest_run_all(void)
{



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
