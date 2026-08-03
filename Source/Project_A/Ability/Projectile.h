#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UAbility;

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

	UFUNCTION()
	void SetMyAbility(UAbility* Ability);
	UPROPERTY(VisibleAnywhere)
	UAbility* MyAbility;

	FVector Destination;
	float Speed = 1000.f;
	int TaskID;

	// Called when the projectile reaches its destination (no impact) or is
	// destroyed for any other reason. HitLocation is the projectile's
	// current location at that point.
	void OnFinished(FVector HitLocation);

	UFUNCTION()
	void HandleComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	FOnProjectileHit OnHit;
};