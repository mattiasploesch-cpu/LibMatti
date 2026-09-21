// Port of net.minecraft.client.DeltaTracker (the 1.21.11 timer lives here as
// DeltaTracker.Timer; net.minecraft.util.Timer was removed). Java's interface
// and its two implementations collapse into one struct: the DefaultValue field
// doubles as the fixed value, the Timer fields carry the running state.

#ifndef MATTICRAFT_NET_MINECRAFT_CLIENT_DELTATRACKER_H
#define MATTICRAFT_NET_MINECRAFT_CLIENT_DELTATRACKER_H

// Java: FloatUnaryOperator targetMsptProvider - takes the default ms per tick
// and returns the target (Minecraft.getTickTargetMillis honours an override;
// NULL means identity, like the normal run).
typedef float (*LIBMATTI_MC_DeltaTrackerTargetMspt)(float msPerTick);

typedef struct LIBMATTI_MC_DeltaTracker
{
    // Java: private final float msPerTick
    float msPerTick;
    LIBMATTI_MC_DeltaTrackerTargetMspt targetMsptProvider;

    float deltaTicks;
    float deltaTickResidual;
    float realtimeDeltaTicks;
    float pausedDeltaTickResidual;
    long long lastMs;
    long long lastUiMs;
    int paused;
    int frozen;

    // Java: DeltaTracker.DefaultValue (ZERO/ONE) - fixed value when the tracker
    // is a constant; -1.0f means "the running Timer fields above are live".
    float fixedValue;
} LIBMATTI_MC_DeltaTracker;

// Java: DeltaTracker.ZERO / DeltaTracker.ONE (static, shared)
LIBMATTI_MC_DeltaTracker *LIBMATTI_MC_DeltaTracker_Zero(void);
LIBMATTI_MC_DeltaTracker *LIBMATTI_MC_DeltaTracker_One(void);

// Java: new DeltaTracker.Timer(tickRate, lastMs, targetMsptProvider) with
// lastMs = 0 at construction (Minecraft's field initializer).
LIBMATTI_MC_DeltaTracker *LIBMATTI_MC_DeltaTracker_Timer_New(float tickRate, LIBMATTI_MC_DeltaTrackerTargetMspt targetMsptProvider);
void LIBMATTI_MC_DeltaTracker_Free(LIBMATTI_MC_DeltaTracker *tracker);

// Java: public int advanceTime(long timeMs, boolean runGameTime)
int LIBMATTI_MC_DeltaTracker_AdvanceTime(LIBMATTI_MC_DeltaTracker *tracker, long long timeMs, int runGameTime);

// Java: public void updatePauseState(boolean paused)
void LIBMATTI_MC_DeltaTracker_UpdatePauseState(LIBMATTI_MC_DeltaTracker *tracker, int paused);

// Java: public void updateFrozenState(boolean frozen)
void LIBMATTI_MC_DeltaTracker_UpdateFrozenState(LIBMATTI_MC_DeltaTracker *tracker, int frozen);

// Java: public float getGameTimeDeltaTicks()
float LIBMATTI_MC_DeltaTracker_GetGameTimeDeltaTicks(const LIBMATTI_MC_DeltaTracker *tracker);

// Java: public float getGameTimeDeltaPartialTick(boolean runsNormally)
float LIBMATTI_MC_DeltaTracker_GetGameTimeDeltaPartialTick(const LIBMATTI_MC_DeltaTracker *tracker, int runsNormally);

// Java: public float getRealtimeDeltaTicks()
float LIBMATTI_MC_DeltaTracker_GetRealtimeDeltaTicks(const LIBMATTI_MC_DeltaTracker *tracker);

#endif //MATTICRAFT_NET_MINECRAFT_CLIENT_DELTATRACKER_H
