#pragma once
#include "CoreMinimal.h"
#include "Ability.h"
#include "../Misc/IntervalTicker.h"
#include "../Interfaces/AbilityLifecycle.h"
#include "AbilityActor.generated.h"

class UAbilitySlot;
class ACharacter;
class UEffectHandler;
class AUnitBase;

UCLASS()
class PROJECT_A_API AAbilityActor : public AAbility, public IAbilityLifecycle
{
	GENERATED_BODY()

public:
	AAbilityActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// -- Tunables --

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Duration = 0.f;	// 0 = lasts infinite
	float DurationTimer = 0.f;

	UPROPERTY(EditAnywhere)
	FIntervalTicker Ticker;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Magnitude = 0.f;

	// -- Event tags --

	// Tags reported to MyAbility on overlap (see HandleOverlap).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability", meta = (Categories = "Ability,Event"))
	FGameplayTagContainer OverlapEventTags;

	// -- Functions --

	// Where/how this actor places itself when spawned - queried once on the still-deferred
	// instance (before FinishSpawning/BeginPlay/OnActivate), so a Blueprint override can decide
	// spawn placement using MyCaster, already set by then. Must be overridden per concrete
	// Blueprint (AA_Firewall, etc.) - the native default crashes immediately, naming the offending
	// class, rather than silently spawning at the origin. Named GetSpawnTransform, not
	// GetTransform, to avoid shadowing the existing AActor::GetTransform(). Declared here rather
	// than on AAbility - see the note in Ability.h.
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	FTransform GetSpawnTransform();

	// Notifies (reports FinishEventTags, broadcasts OnFinish) before firing OnEnd, so talents/
	// OnFinish listeners reacting to the finish can still take effect before OnEnd runs - then
	// destroys the actor. Does not call Super::Finish(), since that would destroy before OnEnd
	// could fire; duplicates only the guard check, not the notify/destroy logic.
	virtual void Finish() override;

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
