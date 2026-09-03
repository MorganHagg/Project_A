#pragma once
#include "CoreMinimal.h"
#include "IntervalTicker.generated.h"

USTRUCT()
struct FIntervalTicker
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    float Interval = 0.0f;

    UPROPERTY()
    float IntervalTimer = 0.0f;
    
    bool ShouldTick(float DeltaTime)
    {
        if (Interval == 0.0f)
            return false;
        
        IntervalTimer -= DeltaTime;

        if (IntervalTimer <= 0.f)
        {
            IntervalTimer = Interval;
            return true;
        }

        return false;
    }
};