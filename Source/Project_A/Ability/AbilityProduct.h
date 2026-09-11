#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "../Misc/GameplayEffect.h"
#include "AbilityEventPayload.h"
#include "AbilityProduct.generated.h"

class UAbility;
class ACharacter;
class AUnitBase;

// ============================================================================
// AAbilityProduct
// Shared base for anything an Ability produces that isn't the ability itself
// (AProjectile, AAbilityActor). Holds the parts that are identical between
// them: caster/ability/effect context, tagged event reporting, and a generic
// "this finished" event. Movement, collision, duration/ticking, and any
// lifecycle interface are left entirely to the concrete subclasses.
// ============================================================================

UCLASS(Abstract)
class PROJECT_A_API AAbilityProduct : public AActor
{
	GENERATED_BODY()

public:
	AAbilityProduct();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* MeshComponent;

	UFUNCTION()
	void SetMyAbility(UAbility* Ability);
	UPROPERTY(BlueprintReadOnly)
	UAbility* MyAbility;

	UFUNCTION()
	void SetMyCaster(ACharacter* Caster);
	UPROPERTY(BlueprintReadOnly)
	ACharacter* MyCaster;

	// Tags reported through ReportAbilityEvent when this actor finishes (see Finish()). Normally
	// the owning Ability's identity tag plus Event.Finish (see UAbility::ComposeEventTags) - set
	// this directly for a bespoke override.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability", meta = (Categories = "Ability,Event"))
	FGameplayTagContainer FinishEventTags;

	// Reports a tagged event (with contextual Payload) to this actor's owning Ability,
	// which forwards it up to the caster's PlayerUnit.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void ReportAbilityEvent(FGameplayTagContainer EventTags, FAbilityEventPayload Payload);

	// Per-instance hooks - bind to these (e.g. via "Bind Event to...") on a specific reference
	// to attach extra behavior to just that instance, without going through the tag system.
	UPROPERTY(BlueprintAssignable, Category = "Ability")
	FOnAbilityEventDelegate OnHit;

	UPROPERTY(BlueprintAssignable, Category = "Ability")
	FOnAbilityEventDelegate OnFinish;

	// Reports FinishEventTags, broadcasts OnFinish, and destroys this actor.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void Finish();

	// Tags reported to MyAbility when HitTarget is called.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability", meta = (Categories = "Ability,Event"))
	FGameplayTagContainer HitEventTags;

	// Builds a payload for Target/Location and reports HitEventTags - the shared "this hit
	// something" entry point for both AProjectile (called from HandleComponentBeginOverlap) and
	// AAbilityActor (called manually, e.g. from an animation notify on a melee swing, or from
	// OnTick for interval damage - the system doesn't distinguish why HitTarget was called).
	// Purely a report - pair with ApplyEffect if the hit should also deliver a GameplayEffect.
	// BlueprintNativeEvent so a Blueprint subclass (e.g. Projectile_Fireball) can override it to
	// run its own bespoke logic (call Parent: HitTarget to still get the native report).
	// Auto-composes HitEventTags from the owning Ability's AbilityTag + Event.TargetHit unless a
	// bespoke set is already set.
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void HitTarget(AUnitBase* Target, FVector Location);
	virtual void HitTarget_Implementation(AUnitBase* Target, FVector Location);

	// Forwards to MyAbility->ApplyEffect - lets AProjectile/AAbilityActor apply a GameplayEffect
	// to a target without reaching through MyAbility themselves.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void ApplyEffect(AUnitBase* Target, const FGameplayEffect& Effect);

protected:
	UPROPERTY()
	bool bHasFinished = false;

	// Sets bHasFinished, builds the finish payload, reports FinishEventTags, and broadcasts
	// OnFinish - everything Finish() does except actually destroying the actor. Split out so
	// AAbilityActor::Finish() can insert its OnEnd hook between notifying and destroying, instead
	// of OnEnd firing before talents/OnFinish listeners get a chance to react. Callers must still
	// check bHasFinished themselves first - this does not guard against being called twice.
	FAbilityEventPayload NotifyFinish();
};
