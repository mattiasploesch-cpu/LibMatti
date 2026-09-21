// Port of net.minecraft.client.DeltaTracker. See DeltaTracker.h.

#include "libmatti/net/minecraft/client/DeltaTracker.h"

#include <stdlib.h>

static LIBMATTI_MC_DeltaTracker zeroTracker = {0.0f, NULL, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0, 0.0f};
static LIBMATTI_MC_DeltaTracker oneTracker = {0.0f, NULL, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0, 1.0f};

LIBMATTI_MC_DeltaTracker *LIBMATTI_MC_DeltaTracker_Zero(void)
{
    return &zeroTracker;
}

LIBMATTI_MC_DeltaTracker *LIBMATTI_MC_DeltaTracker_One(void)
{
    return &oneTracker;
}

LIBMATTI_MC_DeltaTracker *LIBMATTI_MC_DeltaTracker_Timer_New(float tickRate, LIBMATTI_MC_DeltaTrackerTargetMspt targetMsptProvider)
{
    LIBMATTI_MC_DeltaTracker *tracker = calloc(1, sizeof(LIBMATTI_MC_DeltaTracker));
    // Java: this.msPerTick = 1000.0F / tickRate; this.lastUiMs = this.lastMs = lastMs (0)
    tracker->msPerTick = 1000.0f / tickRate;
    tracker->targetMsptProvider = targetMsptProvider;
    tracker->fixedValue = -1.0f;
    return tracker;
}

void LIBMATTI_MC_DeltaTracker_Free(LIBMATTI_MC_DeltaTracker *tracker)
{
    if (tracker == NULL || tracker == &zeroTracker || tracker == &oneTracker) return;
    free(tracker);
}

// Java: private int advanceGameTime(long timeMs)
static int advanceGameTime(LIBMATTI_MC_DeltaTracker *tracker, long long timeMs)
{
    float target = tracker->msPerTick;
    if (tracker->targetMsptProvider != NULL)
        target = tracker->targetMsptProvider(target);
    tracker->deltaTicks = (float) (timeMs - tracker->lastMs) / target;
    tracker->lastMs = timeMs;
    tracker->deltaTickResidual = tracker->deltaTickResidual + tracker->deltaTicks;
    // Java: (int) float saturates at Integer.MAX_VALUE (JLS 5.1.3); the first
    // advance from lastMs=0 produces a huge residual and must clamp, not wrap.
    int ticks;
    if (tracker->deltaTickResidual >= 2147483647.0f)
        ticks = 2147483647;
    else if (tracker->deltaTickResidual <= -2147483648.0f)
        ticks = -2147483648;
    else
        ticks = (int) tracker->deltaTickResidual;
    tracker->deltaTickResidual -= (float) ticks;
    return ticks;
}

// Java: private void advanceRealTime(long timeMs)
static void advanceRealTime(LIBMATTI_MC_DeltaTracker *tracker, long long timeMs)
{
    tracker->realtimeDeltaTicks = (float) (timeMs - tracker->lastUiMs) / tracker->msPerTick;
    tracker->lastUiMs = timeMs;
}

int LIBMATTI_MC_DeltaTracker_AdvanceTime(LIBMATTI_MC_DeltaTracker *tracker, long long timeMs, int runGameTime)
{
    if (tracker->fixedValue >= 0.0f)
    {
        // Java: DefaultValue always reports the fixed value; advanceTime is not
        // overridden there, Minecraft never calls it on ZERO/ONE.
        return (int) tracker->fixedValue;
    }

    advanceRealTime(tracker, timeMs);
    return runGameTime ? advanceGameTime(tracker, timeMs) : 0;
}

void LIBMATTI_MC_DeltaTracker_UpdatePauseState(LIBMATTI_MC_DeltaTracker *tracker, int paused)
{
    if (paused)
    {
        // Java: private void pause() { if (!paused) pausedDeltaTickResidual = deltaTickResidual; paused = true; }
        if (!tracker->paused)
            tracker->pausedDeltaTickResidual = tracker->deltaTickResidual;
        tracker->paused = 1;
    }
    else
    {
        // Java: private void unPause() { if (paused) deltaTickResidual = pausedDeltaTickResidual; paused = false; }
        if (tracker->paused)
            tracker->deltaTickResidual = tracker->pausedDeltaTickResidual;
        tracker->paused = 0;
    }
}

void LIBMATTI_MC_DeltaTracker_UpdateFrozenState(LIBMATTI_MC_DeltaTracker *tracker, int frozen)
{
    tracker->frozen = frozen;
}

float LIBMATTI_MC_DeltaTracker_GetGameTimeDeltaTicks(const LIBMATTI_MC_DeltaTracker *tracker)
{
    if (tracker->fixedValue >= 0.0f) return tracker->fixedValue;
    return tracker->deltaTicks;
}

float LIBMATTI_MC_DeltaTracker_GetGameTimeDeltaPartialTick(const LIBMATTI_MC_DeltaTracker *tracker, int runsNormally)
{
    if (tracker->fixedValue >= 0.0f) return tracker->fixedValue;
    // Java: if (!runsNormally && frozen) return 1.0F
    if (!runsNormally && tracker->frozen) return 1.0f;
    return tracker->paused ? tracker->pausedDeltaTickResidual : tracker->deltaTickResidual;
}

float LIBMATTI_MC_DeltaTracker_GetRealtimeDeltaTicks(const LIBMATTI_MC_DeltaTracker *tracker)
{
    if (tracker->fixedValue >= 0.0f) return tracker->fixedValue;
    // Java: return realtimeDeltaTicks > 7.0F ? 0.5F : realtimeDeltaTicks
    return tracker->realtimeDeltaTicks > 7.0f ? 0.5f : tracker->realtimeDeltaTicks;
}
