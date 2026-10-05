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
struct FGameplayEffect;

// ============================================================================
// Enums
// ============================================================================

UENUM(BlueprintType)
enum class ETargetSelection : uint8
{
	PlayerUnit,
	EnemyUnit,
	All
};

// Selects which of the target's stats mitigates this ability's damage (see
// AUnitBase::MitigateDamage): Magic -> MagicResist, Physical -> Armour, True -> none.
// TrueDamage rather than True - UHT rejects enum entries named true/false in any case.
UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	Magic,
	Physical,
	TrueDamage	UMETA(DisplayName = "True")
};

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

	// Builds the identity tag "Ability.<InAbilityName>" - shared by BeginPlay and
	// UAbilitySystem::InstantiateAbilities (which tags each slot with its ProductClass's identity).
	static FGameplayTag ComposeAbilityTag(FName InAbilityName, bool bErrorIfNotFound = true);

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

	// Tags reported to MyAbility when OnHit is called.
	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	FGameplayTagContainer HitEventTags;

	// Tags reported through ReportAbilityEvent when this actor finishes (see Finish()). Normally
	// the owning Ability's identity tag plus Event.Finish (see ComposeEventTags) - set this
	// directly for a bespoke override.
	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	FGameplayTagContainer FinishEventTags;

	// -- Delegates --
	// Per-instance hooks - bind to these (e.g. via "Bind Event to...") on a specific reference
	// to attach extra behavior to just that instance, without going through the tag system.

	UPROPERTY(BlueprintAssignable, Category = "Ability")
	FOnAbilityEventDelegate OnHitDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Ability")
	FOnAbilityEventDelegate OnFinish;

	// -- Targeting --

	// Radius used by Target_Single's nearest-match search. Internal only - not exposed to
	// Blueprint or the Details panel.
	float TargetAcceptanceRadius = 10.f;

	// Finds the closest AUnitBase to Location (within TargetAcceptanceRadius) matching
	// TargetSelection. Pure target-finder - does not apply an effect or report an event; pair
	// with ApplyEffect for that.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	AUnitBase* Target_Single(ETargetSelection TargetSelection, FVector Location);

	// Finds every AUnitBase within Radius of Location matching TargetSelection. Pure
	// target-finder - does not apply an effect or report an event; pair with ApplyEffect for that.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	TArray<AUnitBase*> Target_AOE(ETargetSelection TargetSelection, FVector Location, float Radius);

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

	// Builds the tag set reported for one event: this ability's identity tag (AbilityTag, e.g.
	// "Ability.Fireball") plus the shared, ability-agnostic event-category tag ("Event." + Suffix,
	// e.g. "Event.TargetHit"). Reporting both as independent facets (rather than one composed
	// "Ability.Fireball.TargetHit" tag) lets a talent's RequiredTags AND them together - e.g.
	// {Event.Crit} to react to any ability's crit, or {Ability.Fireball, Event.Crit} for Fireball's
	// only.
	FGameplayTagContainer ComposeEventTags(const TCHAR* Suffix) const;

	// Builds a payload for Target/Location and reports HitEventTags - the shared "this hit
	// something" entry point for both AProjectile (called from HandleComponentBeginOverlap) and
	// AAbilityActor (called manually, e.g. from an animation notify on a melee swing, or from
	// OnTick for interval damage - the system doesn't distinguish why OnHit was called).
	// Runs IAbilityLifecycle::OnTargetHit (the ability's own hit logic) first, then reports
	// HitEventTags and broadcasts OnHitDelegate, so listeners see the hit after its effects. Not
	// overridable - per-ability logic goes in OnTargetHit, so the talent/delegate notification
	// can't be skipped. Auto-composes HitEventTags from this ability's own AbilityTag +
	// Event.TargetHit unless a bespoke set is already set. The first effect applied via ApplyEffect
	// during OnTargetHit is carried in the payload's AppliedEffect.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void OnHit(AUnitBase* Target, FVector Location);

	// Applies Effect to Target's EffectHandler. Reports nothing itself; when called during OnHit's
	// OnTargetHit hook, the first such effect is included in the hit's payload.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void ApplyEffect(AUnitBase* Target, const FGameplayEffect& Effect);

	// Applies the over-time effect tagged EffectTag (e.g. "Effect.Burn") from MyCaster's granted
	// over-time effects to Target. Logs an error if MyCaster wasn't granted it.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void ApplyOverTimeEffect(UPARAM(meta = (Categories = "Effect")) FGameplayTag EffectTag, AUnitBase* Target);

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

private:
	// Set while OnHit runs its OnTargetHit hook, so ApplyEffect knows to record into HitAppliedEffect.
	bool bResolvingHit = false;

	// First effect applied during the current hit - copied into the TargetHit payload's AppliedEffect.
	TOptional<FGameplayEffect> HitAppliedEffect;

	// Shared selection test for Target_Single/Target_AOE: does Unit match TargetSelection?
	// PlayerUnit/EnemyUnit are absolute type checks (IsA), not relative to MyCaster - an
	// EnemyUnit-selection ability never matches another AEnemyUnit regardless of who cast it.
	static bool MatchesSelection(const AUnitBase* Unit, ETargetSelection TargetSelection);
};
