#pragma once
#include "CoreMinimal.h"
#include "IntervalTicker.generated.h"

// Shared tick timing for everything that ticks on an interval over a duration (AAbilityActor,
// AProjectile, UOverTimeEffect). The first tick happens one Interval after Start, and a run of
// Duration ticks exactly floor(Duration / Interval) times - a tick landing exactly on the end
// still fires, nothing fires after the last whole interval. Ticks are counted from elapsed time
// rather than a resetting timer, so the count doesn't depend on frame rate, and a long frame
// spanning several intervals reports all of them.
USTRUCT()
struct FIntervalTicker
{
    GENERATED_BODY()

    // 0 = never ticks.
    UPROPERTY(EditAnywhere)
    float Interval = 0.0f;

    // Resets progress. InDuration = 0 runs forever (never finishes).
    void Start(float InDuration)
    {
        Duration = InDuration;
        Elapsed = 0.0f;
        TicksFired = 0;
    }

    // Advances by DeltaTime and returns how many ticks are due this frame. bOutFinished is set
    // once Duration has elapsed (never for Duration 0) - callers fire the returned ticks first,
    // then end.
    int32 Advance(float DeltaTime, bool& bOutFinished)
    {
        Elapsed += DeltaTime;
        bOutFinished = Duration > 0.0f && Elapsed >= Duration;

        if (Interval <= 0.0f)
        {
            return 0;
        }

        // Once finished, the end of the run counts as reached even if float accumulation leaves
        // Elapsed a hair short of the last interval - so a tick landing on the end always fires.
        // The tolerance keeps e.g. 15 / 5 from flooring to 2.
        int32 TicksDue = bOutFinished
            ? FMath::FloorToInt32(Duration / Interval + KINDA_SMALL_NUMBER)
            : FMath::FloorToInt32(Elapsed / Interval);

        if (Duration > 0.0f)
        {
            TicksDue = FMath::Min(TicksDue, FMath::FloorToInt32(Duration / Interval + KINDA_SMALL_NUMBER));
        }

        const int32 NewTicks = FMath::Max(TicksDue - TicksFired, 0);
        TicksFired += NewTicks;
        return NewTicks;
    }

private:
    float Duration = 0.0f;
    float Elapsed = 0.0f;
    int32 TicksFired = 0;
};
