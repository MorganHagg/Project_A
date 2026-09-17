#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "../Misc/IntervalTicker.h"
#include "../Interfaces/AbilityLifecycle.h"
#include "AbilityEventPayload.h"
#include "AbilitySlot.generated.h"

// Forward declarations
class AUnitBase;
class AAbility;
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

// ============================================================================
// UAbilitySlot
// A thin translation layer between AbilitySystem/input and the actual action
// (Ability). Holds only cast-bookkeeping (cooldown, cost) - fully generic, no
// per-ability branching or Blueprint subclass. Its job is done once it has
// spawned the Ability instance for this cast.
// ============================================================================

UCLASS(Blueprintable)
class PROJECT_A_API UAbilitySlot : public UObject, public IAbilityLifecycle
{
	GENERATED_BODY()

public:
	UAbilitySlot();

	// --------------------------------------------------------------
	// Context
	// --------------------------------------------------------------

	UPROPERTY(BlueprintReadOnly)
	AUnitBase* MyCaster;

	UPROPERTY(BlueprintReadOnly)
	APlayerController* MyController;

	UWorld* World;
	FActorSpawnParameters SpawnParams;

	// --------------------------------------------------------------
	// Identity
	// --------------------------------------------------------------

	// Vestigial - AbilitySlot is now fully generic (one C++ class, no per-ability Blueprint
	// subclass), so nothing sets this to anything but the default anymore. Identity now lives on
	// Ability (the spawned product) instead - see AAbility::AbilityName. Scheduled for removal
	// once nothing references it (pending Task 3 of the Ability rework).
	virtual FName GetAbilityName() const { return AbilityName; }
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FName AbilityName = FName("NO_NAME_ABILITY");

	// Vestigial along with AbilityName above - always invalid now (ComposeEventTags below already
	// guards on IsValid()), kept only until Task 3 removes both.
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayTag AbilityTag;

	// --------------------------------------------------------------
	// Tunables
	// --------------------------------------------------------------

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bModifyEndsAbility = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float CoolDown = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Cost = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MagnitudeMultiplier = 1.f;

	UPROPERTY(EditAnywhere)
	FIntervalTicker Ticker;

	// Which Ability (product) class this slot spawns when activated. Copied from the
	// UAbilityDataAsset this instance was built from - see UAbilitySystem::InstantiateAbilities.
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	TSubclassOf<AAbility> ProductClass;

	float GetCoolDown();
	float GetCost();
	float GetMagnitude();

	// --------------------------------------------------------------
	// Lifecycle
	// --------------------------------------------------------------

	// Sets up the ability for later use
	void SetupAbility(AUnitBase* NewCaster);

	// Activates this ability. Calls into OnActivate (Blueprint-implementable).
	virtual void ActivateAbility();

	void TickAbility(float DeltaTime);

	// Ends this ability. Calls into OnEnd (Blueprint-implementable).
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void EndAbility();

	// Called if an ability is no longer needed
	void KillAbility();

protected:
	UPROPERTY()
	bool bHasEnded = false;	// Small guard against double end

private:
	// Shared selection test for Target_Single/Target_AOE: does Unit match TargetSelection?
	// PlayerUnit/EnemyUnit are absolute type checks (IsA), not relative to MyCaster - an
	// EnemyUnit-selection ability never matches another AEnemyUnit regardless of who cast it.
	static bool MatchesSelection(const AUnitBase* Unit, ETargetSelection TargetSelection);

public:
	// --------------------------------------------------------------
	// Blueprint-buildable effect library
	// --------------------------------------------------------------

	// Radius used by Target_Single's nearest-match search. Internal only - not exposed to
	// Blueprint or the Details panel.
	float TargetAcceptanceRadius = 10.f;

	// Finds the closest AUnitBase to Location (within TargetAcceptanceRadius) matching
	// TargetSelection. Pure target-finder - does not apply an effect or report an event; pair
	// with ApplyEffect for that.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	AUnitBase* Target_Single(ETargetSelection TargetSelection, FVector Location);

	// Applies Effect to Target's EffectHandler. No reporting - just the effect application, so
	// callers (e.g. AAbility::ApplyEffect) can pair it with their own reporting logic.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void ApplyEffect(AUnitBase* Target, const FGameplayEffect& Effect);

	// Finds every AUnitBase within Radius of Location matching TargetSelection. Pure
	// target-finder - does not apply an effect or report an event; pair with ApplyEffect for that.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	TArray<AUnitBase*> Target_AOE(ETargetSelection TargetSelection, FVector Location, float Radius);

	// --------------------------------------------------------------
	// Talent delegation
	// --------------------------------------------------------------
	// Reports a tagged event (with contextual Payload) to the caster's
	// PlayerUnit, which fans it out to any listening talents.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void ReportAbilityEvent(FGameplayTagContainer EventTags, FAbilityEventPayload Payload);

	// Builds the tag set reported for one event: this ability's identity tag (AbilityTag, e.g.
	// "Ability.Fireball") plus the shared, ability-agnostic event-category tag ("Event." + Suffix,
	// e.g. "Event.TargetHit"). Reporting both as independent facets (rather than one composed
	// "Ability.Fireball.TargetHit" tag) lets a talent's RequiredTags AND them together - e.g.
	// {Event.Crit} to react to any ability's crit, or {Ability.Fireball, Event.Crit} for Fireball's
	// only. Public so callers outside UAbilitySlot (e.g. AAbilityActor::HandleOverlap) can compose their
	// own event tag sets from the owning ability's AbilityTag.
	FGameplayTagContainer ComposeEventTags(const TCHAR* Suffix) const;
};
