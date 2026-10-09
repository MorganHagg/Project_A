#include "Projectile.h"
#include "../Unit/UnitBase.h"
#include "AbilitySlot.h"
#include "Gameframework/Character.h"
#include "Components/StaticMeshComponent.h"

AProjectile::AProjectile()
{
}

FTransform AProjectile::GetSpawnTransform_Implementation()
{
	checkf(false, TEXT("%s must override GetSpawnTransform()"), *GetClass()->GetName());
	return FTransform::Identity;
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	// Runs until Travel() finishes it - no duration.
	Ticker.Start(0.f);
	MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::HandleComponentBeginOverlap);

	// GetActorLocation/GetActorForwardVector already reflect GetSpawnTransform's result by now
	// (FinishSpawning placed the actor before BeginPlay ran). Travel()/Finish() are otherwise
	// unchanged - Destination stays a plain mutable field for the rest of the flight, so anything
	// holding a live reference (e.g. a talent grabbing this projectile off the Cast payload) can
	// redirect it mid-flight simply by writing a new Destination.
	Destination = GetActorLocation() + GetActorForwardVector() * MaxDistance;

	ReportCast();
	IAbilityLifecycle::Execute_OnActivate(this);
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Travel(DeltaTime);

	bool bFinished = false;
	const int32 TicksDue = Ticker.Advance(DeltaTime, bFinished);
	for (int32 TickIndex = 0; TickIndex < TicksDue && !bHasFinished; ++TickIndex)
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

	OnHit(HitUnit, OverlapLocation);

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

	SetActorLocation(HitLocation, false);

	const FAbilityEventPayload Payload = NotifyFinish();

	IAbilityLifecycle::Execute_OnEnd(this, Payload.Location);

	Destroy();
}

void AProjectile::OnHoldEnded_Implementation()
{
	Finish(GetActorLocation());
}

UAbilitySlot* AProjectile::GetAbility()
{
	return MyAbility;
}
