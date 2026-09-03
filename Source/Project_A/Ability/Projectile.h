#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Misc/GameplayEffect.h"
#include "Projectile.generated.h"

class UAbility;
class ACharacter;
class UEffectHandler;
class AUnitBase;

// Delegates
DECLARE_DELEGATE_OneParam(FOnProjectileHit, FVector);
DECLARE_DELEGATE_OneParam(FOnProjectileFinished, FVector);

// Broadcast to any listening talent (see UTalentBase::BindToProjectile).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProjectileHitDelegate, AUnitBase*, HitUnit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProjectileFinishDelegate, FVector, Location);

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

	bool bHasFinished = false;
	
	UFUNCTION()
	void SetMyAbility(UAbility* Ability);
	UPROPERTY(VisibleAnywhere)
	UAbility* MyAbility;
	UAbility* GetAbility();

	UFUNCTION()
	void SetMyCaster(ACharacter* Caster);
	UPROPERTY(VisibleAnywhere)
	ACharacter* MyCaster;

	UPROPERTY(VisibleAnywhere)
	FGameplayEffect MyEffect;

	FVector Destination;
	float Speed;
	int TaskID;
	
	FOnProjectileHit OnHit;           // fired per penetrating hit — apply effects
	FOnProjectileFinished OnFinished; // fired exactly once — resolves the latent action

	UPROPERTY(BlueprintAssignable, Category = "Projectile")
	FOnProjectileHitDelegate OnProjectileHit;

	UPROPERTY(BlueprintAssignable, Category = "Projectile")
	FOnProjectileFinishDelegate OnProjectileFinish;

	void Finish(FVector HitLocation);
	
	UFUNCTION()
	void HandleComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	UPROPERTY()
	TSet<TObjectPtr<AActor>> AlreadyHitActors;

	// How many hits have been penetrated so far this flight.
	int32 PenetrationsSoFar = 0;
};