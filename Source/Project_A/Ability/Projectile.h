#pragma once
#include "CoreMinimal.h"
#include "Ability.h"
#include "../Misc/IntervalTicker.h"
#include "../Interfaces/AbilityLifecycle.h"
#include "Projectile.generated.h"

class UAbilitySlot;
class ACharacter;
class UEffectHandler;
class AUnitBase;

DECLARE_DELEGATE_OneParam(FOnProjectileHit, FVector);

// Abstract: never spawned directly - AbilitySlot::ActivateAbility's generic dispatch always
// spawns via ProductClass, naming a concrete Blueprint subclass, and marking this Abstract keeps
// the base class itself out of that picker (same pattern as AAbility).
UCLASS(Abstract)
class PROJECT_A_API AProjectile : public AAbility, public IAbilityLifecycle
{
	GENERATED_BODY()

public:
	AProjectile();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// -- Tunables --

	// Gates OnTick (IAbilityLifecycle), not Travel - movement stays frame-accurate every Tick.
	// Interval = 0 never fires OnTick. No Duration equivalent: a projectile's end is
	// already fully determined by distance to Destination inside Travel(), not a timer.
	UPROPERTY(EditAnywhere)
	FIntervalTicker Ticker;

	// If true, the projectile finishes as soon as it hits something (and
	// reports the hit location). If false, it only finishes by reaching
	// Destination, same as before.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bReactToImpact = true;

	// 0 = stop on first hit (default). Positive N = penetrate N hits before
	// stopping on hit N+1. -1 = infinite penetration, only stops by reaching
	// Destination.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	int32 PenetrationCount = 0;

	// How far this projectile travels (from its spawn point, in the direction GetSpawnTransform's
	// rotation faced) before finishing on its own, if nothing stops it first.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float MaxDistance = 2000.f;

	// -- Flight state --

	FVector Destination;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float Speed = 900.f;
	int TaskID;

	// -- Delegates --

	FOnProjectileHit OnPenetrateHit;   // fired per penetrating hit — apply effects

	// -- Functions --

	// Where/how this projectile places itself when spawned - queried once on the still-deferred
	// instance (before FinishSpawning/BeginPlay/OnActivate), so a Blueprint override can decide
	// spawn placement (e.g. "5m in front of the caster") using MyCaster, already set by then. Must
	// be overridden per concrete Blueprint (Projectile_Fireball, etc.) - the native default crashes
	// immediately, naming the offending class, rather than silently spawning at the origin. Named
	// GetSpawnTransform, not GetTransform, to avoid shadowing the existing AActor::GetTransform().
	// Declared here rather than on AAbility - see the note in Ability.h.
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	FTransform GetSpawnTransform();

	void Travel(float DeltaTime);

	UFUNCTION()
	void HandleComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Unhide AAbility::Finish() - Finish(FVector) below has a different signature and
	// would otherwise shadow it.
	using AAbility::Finish;

	// Notifies (reports FinishEventTags, broadcasts OnFinish) before firing OnEnd, then destroys -
	// same ordering as AAbilityActor::Finish(). Snaps to the given hit location first (the actual
	// finish point may be a sweep impact point, different from GetActorLocation()). Does not call
	// Super::Finish(), since that would destroy before OnEnd could fire.
	void Finish(FVector HitLocation);

	// Default: finishes where it is. Override in Blueprint to keep flying after the hold ends.
	virtual void OnHoldEnded_Implementation() override;

	UAbilitySlot* GetAbility();

protected:
	UPROPERTY()
	TSet<TObjectPtr<AActor>> AlreadyHitActors;

	// How many hits have been penetrated so far this flight.
	int32 PenetrationsSoFar = 0;
};
