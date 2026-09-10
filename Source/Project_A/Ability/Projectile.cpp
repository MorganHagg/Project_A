#include "Projectile.h"
#include "../Unit/UnitBase.h"
#include "Ability.h"
#include "Gameframework/Character.h"
#include "../Component/EffectHandler.h"
#include "Components/StaticMeshComponent.h"

AProjectile::AProjectile()
{
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::HandleComponentBeginOverlap);
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Travel(DeltaTime);
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

UAbility* AProjectile::GetAbility()
{
	return MyAbility;
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

	if (UEffectHandler* EffectHandler = HitUnit->FindComponentByClass<UEffectHandler>())
	{
		EffectHandler->AddEffect(MyEffect);
	}

	FVector OverlapLocation = bFromSweep ? FVector(SweepResult.ImpactPoint) : GetActorLocation();

	FAbilityEventPayload Payload;
	Payload.Ability = MyAbility;
	Payload.AbilityProduct = this;
	Payload.Target = HitUnit;
	Payload.Location = OverlapLocation;
	Payload.AppliedEffect = MyEffect;
	Payload.Magnitude = MyEffect.Magnitude;
	ReportAbilityEvent(HitEventTag, Payload);

	AlreadyHitActors.Add(OtherActor);

	if (PenetrationCount == -1 || PenetrationsSoFar < PenetrationCount)
	{
		PenetrationsSoFar++;
		OnPenetrateHit.ExecuteIfBound(OverlapLocation);
		return;
	}

	Finish(OverlapLocation);
}

FVector AProjectile::Finish(FVector HitLocation)
{
	// OnFinished must only ever fire once (it resolves Execute_Projectile's latent action) -
	// Super::Finish()'s guard only protects its own body, not this one, so check bHasFinished
	// (inherited, protected) here first.
	if (!bHasFinished)
	{
		OnFinished.ExecuteIfBound(HitLocation);
	}

	SetActorLocation(HitLocation, false);
	return Super::Finish();
}
