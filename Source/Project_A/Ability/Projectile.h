#pragma once
#include "CoreMinimal.h"
#include "AbilityProduct.h"
#include "Projectile.generated.h"

class UAbility;
class ACharacter;
class UEffectHandler;
class AUnitBase;

// Delegates used internally by UAbility::Execute_Projectile's latent action - unrelated to talent delegation.
DECLARE_DELEGATE_OneParam(FOnProjectileHit, FVector);
DECLARE_DELEGATE_OneParam(FOnProjectileFinished, FVector);

UCLASS()
class PROJECT_A_API AProjectile : public AAbilityProduct
{
	GENERATED_BODY()

public:
	AProjectile();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	void Travel(float DeltaTime);

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

	UAbility* GetAbility();

	// Unhide AAbilityProduct::Finish() - Finish(FVector) below has a different signature and
	// would otherwise shadow it.
	using AAbilityProduct::Finish;

	FVector Destination;
	float Speed;
	int TaskID;

	FOnProjectileHit OnPenetrateHit;   // fired per penetrating hit — apply effects
	FOnProjectileFinished OnFinished;  // fired exactly once — resolves the latent action

	// Tag reported to MyAbility on hit (set by UAbility::Execute_Projectile). Finish uses the
	// inherited FinishEventTag.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	FGameplayTag HitEventTag;

	// Snaps to the given hit location (the actual finish point may be a sweep impact point,
	// different from GetActorLocation()), then defers to the shared AAbilityProduct::Finish().
	FVector Finish(FVector HitLocation);

	UFUNCTION()
	void HandleComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	UPROPERTY()
	TSet<TObjectPtr<AActor>> AlreadyHitActors;

	// How many hits have been penetrated so far this flight.
	int32 PenetrationsSoFar = 0;
};
