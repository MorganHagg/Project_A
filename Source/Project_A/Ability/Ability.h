#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "../Misc/GameplayEffect.h"
#include "AbilityEventPayload.h"
#include "Ability.generated.h"

class UAbilitySlot;
class ACharacter;
class AUnitBase;

// ============================================================================
// AAbility
// Shared base for anything a cast produces (AProjectile, AAbilityActor) - the
// actual action: a fireball, a firewall, a summoned pet. Holds the parts that
// are identical between subclasses: caster/ability/effect context, tagged
// event reporting, and a generic "this finished" event. Movement, collision,
// duration/ticking, and any lifecycle interface are left entirely to the
// concrete subclasses.
// ============================================================================

UCLASS(Abstract)
class PROJECT_A_API AAbility : public AActor
{
	GENERATED_BODY()

public:
	AAbility();

protected:
	// Composes AbilityTag from AbilityName, checkf-guarded against NO_NAME_ABILITY. Runs on
	// FinishSpawning (after GetSpawnTransform has already been queried on the deferred instance,
	// before OnActivate) - Cast reporting needs AbilityTag valid, GetSpawnTransform doesn't.
	virtual void BeginPlay() override;

public:
	// -- Identity --

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FName AbilityName = FName("NO_NAME_ABILITY");

	// Root tag identifying this ability (e.g. "Ability.Fireball"). Auto-derived from AbilityName
	// in BeginPlay - not directly editable, so there's only one place (AbilityName) to author
	// this ability's identity.
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayTag AbilityTag;

	// -- Context --

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* MeshComponent;

	UFUNCTION()
	void SetMyAbility(UAbilitySlot* Ability);
	UPROPERTY(BlueprintReadOnly)
	UAbilitySlot* MyAbility;

	UFUNCTION()
	void SetMyCaster(ACharacter* Caster);
	UPROPERTY(BlueprintReadOnly)
	ACharacter* MyCaster;

	// -- Event tags --

	// Tags reported to MyAbility when HitTarget is called.
	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	FGameplayTagContainer HitEventTags;

	// Tags reported through ReportAbilityEvent when this actor finishes (see Finish()). Normally
	// the owning Ability's identity tag plus Event.Finish (see UAbilitySlot::ComposeEventTags) - set
	// this directly for a bespoke override.
	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	FGameplayTagContainer FinishEventTags;

	// -- Delegates --
	// Per-instance hooks - bind to these (e.g. via "Bind Event to...") on a specific reference
	// to attach extra behavior to just that instance, without going through the tag system.

	UPROPERTY(BlueprintAssignable, Category = "Ability")
	FOnAbilityEventDelegate OnHit;

	UPROPERTY(BlueprintAssignable, Category = "Ability")
	FOnAbilityEventDelegate OnFinish;

	// -- Functions --

	// Note: GetSpawnTransform (the spawn-placement hook) is declared separately on AProjectile and
	// AAbilityActor, not here - a shared declaration on this grandparent class hit a reproducible
	// Unreal 5.8 Blueprint compiler bug where the override reference fails to resolve on reload for
	// any Blueprint two native levels down that also implements IAbilityLifecycle (confirmed via a
	// clean rebuild + brand-new test assets, both still failing; a direct child of AAbility alone
	// was unaffected). AbilitySlot::ActivateAbility Casts to the concrete type to call it instead.

	// Reports a tagged event (with contextual Payload) to this actor's owning Ability,
	// which forwards it up to the caster's PlayerUnit.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void ReportAbilityEvent(FGameplayTagContainer EventTags, FAbilityEventPayload Payload);

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

	// Reports FinishEventTags, broadcasts OnFinish, and destroys this actor.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void Finish();

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
