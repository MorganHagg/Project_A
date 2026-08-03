#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UAbility;
class ACharacter;
class UEffectHandler;

// Now carries the hit location for whatever bound this delegate.
DECLARE_DELEGATE_OneParam(FOnProjectileHit, FVector);

UCLASS()
class PROJECT_A_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	AProjectile();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	void Travel(float DeltaTime);

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

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

	UFUNCTION()
	void SetMyAbility(UAbility* Ability);
	UPROPERTY(VisibleAnywhere)
	UAbility* MyAbility;

	UFUNCTION()
	void SetMyCaster(ACharacter* Caster);
	UPROPERTY(VisibleAnywhere)
	ACharacter* MyCaster;

	FVector Destination;
	float Speed;
	int TaskID;

	// Called when the projectile reaches its destination (no impact) or is
	// destroyed for any other reason. HitLocation is the projectile's
	// current location at that point.
	void OnFinished(FVector HitLocation);

	UFUNCTION()
	void HandleComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	FOnProjectileHit OnHit;

protected:
	// Actors already counted as a penetration hit, so lingering overlaps
	// across multiple ticks don't get double-counted.
	UPROPERTY()
	TSet<TObjectPtr<AActor>> AlreadyHitActors;

	// How many hits have been penetrated so far this flight.
	int32 PenetrationsSoFar = 0;
};