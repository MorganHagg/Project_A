#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "UnitAnimInstance.generated.h"

// Base anim instance for every unit, player and enemy. Reads the owning actor's movement each
// update and exposes it to the AnimGraph; the Animation Blueprint only picks which clips to play.
UCLASS()
class PROJECT_A_API UUnitAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// Horizontal speed in cm/s.
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Speed = 0.f;

	// Angle of the horizontal velocity relative to the unit's facing, in degrees: 0 forward, 90 right,
	// -90 left, +-180 backward. Keeps its last value while standing still, so a directional blend
	// does not snap to forward during the blend back to idle.
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Direction = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsMoving = false;

	// Speed above which the unit counts as moving.
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float MovingSpeedThreshold = 3.f;
};
