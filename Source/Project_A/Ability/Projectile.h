#pragma once
#include "CoreMinimal.h"
#include "AbilityProduct.h"
#include "../Misc/IntervalTicker.h"
#include "../Interfaces/AbilityLifecycle.h"
#include "Projectile.generated.h"

class UAbility;
class ACharacter;
class UEffectHandler;
class AUnitBase;

// Delegates used internally by UAbility::Execute_Projectile's latent action - unrelated to talent delegation.
DECLARE_DELEGATE_OneParam(FOnProjectileHit, FVector);
DECLARE_DELEGATE_OneParam(FOnProjectileFinished, FVector);

// Abstract: never spawned directly - Execute_Projectile always takes a TSubclassOf<AProjectile>
// naming a concrete Blueprint subclass, and marking this Abstract keeps the base class itself out
// of that picker (same pattern as AAbilityProduct).
UCLASS(Abstract)
class PROJECT_A_API AProjectile : public AAbilityProduct, public IAbilityLifecycle
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
	// Interval = 0 fires OnTick continuously. No Duration equivalent: a projectile's end is
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

	// -- Flight state --

	FVector Destination;
	float Speed;
	int TaskID;

	// -- Delegates --

	FOnProjectileHit OnPenetrateHit;   // fired per penetrating hit — apply effects
	FOnProjectileFinished OnFinished;  // fired exactly once — resolves the latent action

	// -- Functions --

	void Travel(float DeltaTime);

	UFUNCTION()
	void HandleComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Unhide AAbilityProduct::Finish() - Finish(FVector) below has a different signature and
	// would otherwise shadow it.
	using AAbilityProduct::Finish;

	// Notifies (reports FinishEventTags, broadcasts OnFinish) before firing OnEnd, then destroys -
	// same ordering as AAbilityActor::Finish(). Snaps to the given hit location first (the actual
	// finish point may be a sweep impact point, different from GetActorLocation()). Does not call
	// Super::Finish(), since that would destroy before OnEnd could fire.
	void Finish(FVector HitLocation);

	UAbility* GetAbility();

protected:
	UPROPERTY()
	TSet<TObjectPtr<AActor>> AlreadyHitActors;

	// How many hits have been penetrated so far this flight.
	int32 PenetrationsSoFar = 0;
};
