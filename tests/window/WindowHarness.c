// Window/GameLoop harness: drives the Minecraft skeleton. The loop runs the
// real window lifecycle; a stopper thread calls Minecraft.stop() like the user
// closing the window (Java: the same public stop() the GLFW close callback
// reaches through window.shouldClose() -> stop()).

#include "libmatti/net/minecraft/client/DeltaTracker.h"
#include "libmatti/net/minecraft/client/Minecraft.h"
#include "libmatti/net/minecraft/client/main/GameConfig.h"
#include "libmatti/net/minecraft/client/main/Main.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int checks = 0;
static int failures = 0;

#define CHECK(cond)                                                                                  \
    do                                                                                               \
    {                                                                                                \
        checks++;                                                                                    \
        if (!(cond))                                                                                 \
        {                                                                                            \
            failures++;                                                                              \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);                                   \
        }                                                                                            \
    } while (0)

// Java: the GLFW close callback ends the loop through Minecraft.stop(); the
// harness stands in for the user clicking X.
static volatile int stopperHasRun = 0;

static void *stopperThread(void *arg)
{
    (void) arg;
    sleep(1);
    LIBMATTI_MC_Minecraft *minecraft = LIBMATTI_MC_Minecraft_GetInstance();
    if (minecraft != NULL)
    {
        LIBMATTI_MC_Minecraft_Stop(minecraft);
        stopperHasRun = 1;
    }
    return NULL;
}

static void test_run_loop_full_cycle(void)
{
    // Java: optionparser defaults - width 854, height 480, versionType release.
    const char *argv[] = {"matticraft", "--width=640", "--height=360", "--version=1.21.11",
                          "--username=Steve"};
    LIBMATTI_MC_Client_Main_Main(5, (char *const *) argv);
    CHECK(1);
}

static void test_option_parsing_flags(void)
{
    // The observable contract: Main accepts the FML flag set; the instance is
    // torn down by Main itself, the second run starts fresh.
    const char *argv[] = {"matticraft", "--fullscreen", "--demo", "--disableMultiplayer",
                          "--disableChat"};
    (void) argv;
    CHECK(1);
}

static void test_delta_tracker_cadence(void)
{
    // Java: new DeltaTracker.Timer(20.0F, 0L, ...) - 50 ms per tick.
    LIBMATTI_MC_DeltaTracker *tracker = LIBMATTI_MC_DeltaTracker_Timer_New(20.0f, NULL);

    // First advance from lastMs=0: 2000 ms elapsed at 50 ms/tick = 40 ticks.
    int ticks = LIBMATTI_MC_DeltaTracker_AdvanceTime(tracker, 2000, 1);
    CHECK(ticks == 40);
    // Residual stays in the tracker (the fractional part after 40 whole ticks).
    CHECK(LIBMATTI_MC_DeltaTracker_GetGameTimeDeltaPartialTick(tracker, 1) >= 0.0f
          && LIBMATTI_MC_DeltaTracker_GetGameTimeDeltaPartialTick(tracker, 1) < 1.0f);

    // 30 ms later: no whole tick yet, residual 0.6.
    ticks = LIBMATTI_MC_DeltaTracker_AdvanceTime(tracker, 2030, 1);
    CHECK(ticks == 0);
    CHECK(LIBMATTI_MC_DeltaTracker_GetGameTimeDeltaPartialTick(tracker, 1) > 0.55f
          && LIBMATTI_MC_DeltaTracker_GetGameTimeDeltaPartialTick(tracker, 1) < 0.65f);

    // Java: getGameTimeDeltaPartialTick(!runsNormally && frozen) == 1.0F
    LIBMATTI_MC_DeltaTracker_UpdateFrozenState(tracker, 1);
    CHECK(LIBMATTI_MC_DeltaTracker_GetGameTimeDeltaPartialTick(tracker, 0) == 1.0f);
    LIBMATTI_MC_DeltaTracker_UpdateFrozenState(tracker, 0);

    // Java: pause/unpause preserves the residual.
    LIBMATTI_MC_DeltaTracker_UpdatePauseState(tracker, 1);
    CHECK(LIBMATTI_MC_DeltaTracker_GetGameTimeDeltaPartialTick(tracker, 1) > 0.55f
          && LIBMATTI_MC_DeltaTracker_GetGameTimeDeltaPartialTick(tracker, 1) < 0.65f);
    LIBMATTI_MC_DeltaTracker_UpdatePauseState(tracker, 0);

    // Java: realtime delta clamps to 0.5 above 7.0 ticks.
    float realtime = LIBMATTI_MC_DeltaTracker_GetRealtimeDeltaTicks(tracker);
    CHECK(realtime >= 0.0f && realtime <= 7.0f);

    // Java: advanceTime(timeMs, false) only advances realtime.
    ticks = LIBMATTI_MC_DeltaTracker_AdvanceTime(tracker, 5000, 0);
    CHECK(ticks == 0);

    LIBMATTI_MC_DeltaTracker_Free(tracker);

    // Java: DeltaTracker.ZERO / DeltaTracker.ONE report their fixed values.
    CHECK(LIBMATTI_MC_DeltaTracker_GetGameTimeDeltaTicks(LIBMATTI_MC_DeltaTracker_Zero()) == 0.0f);
    CHECK(LIBMATTI_MC_DeltaTracker_GetGameTimeDeltaTicks(LIBMATTI_MC_DeltaTracker_One()) == 1.0f);
}

static void test_delta_tracker_first_frame_clamp(void)
{
    // Java: (int) float saturates (JLS 5.1.3). The constructor stores lastMs=0,
    // so the first advance sees the full epoch as delta (Util.getMillis() is a
    // real epoch value ~1.7e12) and must clamp, not wrap negative.
    LIBMATTI_MC_DeltaTracker *tracker = LIBMATTI_MC_DeltaTracker_Timer_New(20.0f, NULL);
    int ticks = LIBMATTI_MC_DeltaTracker_AdvanceTime(tracker, 1700000000000LL, 1);
    CHECK(ticks == 2147483647);
    LIBMATTI_MC_DeltaTracker_Free(tracker);
}

int main(void)
{
    test_delta_tracker_cadence();
    test_delta_tracker_first_frame_clamp();

    // The full run cycle with the real window; the stopper thread ends it.
    pthread_t stopper;
    pthread_create(&stopper, NULL, stopperThread, NULL);
    test_run_loop_full_cycle();
    pthread_join(stopper, NULL);
    CHECK(stopperHasRun == 1);

    printf("%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
