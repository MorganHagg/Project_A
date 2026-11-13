#include "UnitAnimInstance.h"
#include "GameFramework/Actor.h"

void UUnitAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// Null in some editor contexts (e.g. thumbnail rendering).
	const AActor* Owner = GetOwningActor();
	if (!Owner)
	{
		return;
	}

	FVector Velocity = Owner->GetVelocity();
	Velocity.Z = 0.f;

	Speed = Velocity.Size();
	bIsMoving = Speed > MovingSpeedThreshold;

	if (bIsMoving)
	{
		const FVector LocalVelocity = Owner->GetActorRotation().UnrotateVector(Velocity);
		Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));
	}
}
