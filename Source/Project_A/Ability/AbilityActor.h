#pragma once
#include "CoreMinimal.h"
#include "AbilityProduct.h"
#include "../Misc/IntervalTicker.h"
#include "../Interfaces/AbilityLifecycle.h"
#include "AbilityActor.generated.h"

class UAbility;
class ACharacter;
class UEffectHandler;
class AUnitBase;

UCLASS()
class PROJECT_A_API AAbilityActor : public AAbilityProduct, public IAbilityLifecycle
{
	GENERATED_BODY()

public:
	AAbilityActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Duration = 0.f;	// 0 = lasts infinite
	float DurationTimer = 0.f;

	UPROPERTY(EditAnywhere)
	FIntervalTicker Ticker;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Magnitude = 0.f;

	// Notifies (reports FinishEventTags, broadcasts OnFinish) before firing OnEnd, so talents/
	// OnFinish listeners reacting to the finish can still take effect before OnEnd runs - then
	// destroys the actor. Does not call Super::Finish(), since that would destroy before OnEnd
	// could fire; duplicates only the guard check, not the notify/destroy logic.
	virtual void Finish() override;

	// Tags reported to MyAbility on overlap (see HandleOverlap).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability", meta = (Categories = "Ability,Event"))
	FGameplayTagContainer OverlapEventTags;

	// Bound to MeshComponent->OnComponentBeginOverlap. Sorts OtherActor into the payload: a unit
	// goes into Target, another AbilityProduct (e.g. a Projectile passing through) goes into
	// OverlappedProduct - then reports OverlapEventTags.
	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// All AUnitBase actors currently overlapping MeshComponent. bIsHelpful selects which side:
	// true -> APlayerUnit only, false (default) -> AEnemyUnit only.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	TArray<AUnitBase*> GetOverlappingUnits(bool bIsHelpful = false);
};
