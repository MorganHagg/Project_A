#include "Projectile.h"
#include "../Unit/UnitBase.h"
#include "Ability.h"
#include "Gameframework/Character.h"
#include "Components/StaticMeshComponent.h"

AProjectile::AProjectile()
{
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	Ticker.IntervalTimer = Ticker.Interval;
	MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::HandleComponentBeginOverlap);
	IAbilityLifecycle::Execute_OnActivate(this);
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Travel(DeltaTime);

	if (Ticker.ShouldTick(DeltaTime))
	{
		IAbilityLifecycle::Execute_OnTick(this);
	}
}

void AProjectile::Travel(float DeltaTime)
{
	FVector Direction = (Destination - GetActorLocation()).GetSafeNormal();
	FVector NewLocation = GetActorLocation() + Direction * Speed * DeltaTime;

	SetActorLocation(NewLocation, true);
	SetActorRotation(Direction.Rotation());

	if (FVector::Dist(NewLocation, Destination) < 25.f)
	{
		Finish(NewLocation);
	}
}

void AProjectile::HandleComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bReactToImpact || OtherActor == MyCaster || AlreadyHitActors.Contains(OtherActor))
	{
		return;
	}

	AUnitBase* HitUnit = Cast<AUnitBase>(OtherActor);
	if (!HitUnit)
	{
		return;
	}

	FVector OverlapLocation = bFromSweep ? FVector(SweepResult.ImpactPoint) : GetActorLocation();

	HitTarget(HitUnit, OverlapLocation);

	AlreadyHitActors.Add(OtherActor);

	if (PenetrationCount == -1 || PenetrationsSoFar < PenetrationCount)
	{
		PenetrationsSoFar++;
		OnPenetrateHit.ExecuteIfBound(OverlapLocation);
		return;
	}

	Finish(OverlapLocation);
}

void AProjectile::Finish(FVector HitLocation)
{
	if (bHasFinished)
	{
		return;
	}

	OnFinished.ExecuteIfBound(HitLocation);
	SetActorLocation(HitLocation, false);

	const FAbilityEventPayload Payload = NotifyFinish();

	IAbilityLifecycle::Execute_OnEnd(this, Payload.Location);

	Destroy();
}

UAbility* AProjectile::GetAbility()
{
	return MyAbility;
}
