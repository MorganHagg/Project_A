#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/LatentActionManager.h"
#include "GameplayTagContainer.h"
#include "../Misc/IntervalTicker.h"
#include "../Interfaces/AbilityLifecycle.h"
#include "AbilityEventPayload.h"
#include "Ability.generated.h"

// Forward declarations
class AUnitBase;
class AAbilityActor;
class AProjectile;
class UStaticMesh;
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
// Latent action used by RunEffect_Projectile - completes when the projectile
// reports a hit via its OnHit delegate.
// ============================================================================

class FEffect_ProjectileAction : public FPendingLatentAction
{
public:
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;
	bool bComplete = false;
	FVector& OutLocation;

	FEffect_ProjectileAction(const FLatentActionInfo& Info, FVector& InOutLocation)
		: ExecutionFunction(Info.ExecutionFunction)
		, OutputLink(Info.Linkage)
		, CallbackTarget(Info.CallbackTarget)
		, OutLocation(InOutLocation)
	{}

	void Finish(FVector Location)
	{
		OutLocation = Location;
		bComplete = true;
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		Response.FinishAndTriggerIf(bComplete, ExecutionFunction, OutputLink, CallbackTarget);
	}
};

// ============================================================================
// UAbility
// An individual ability. Does not know whether it was activated via tap,
// hold, or modify - that decision is made by the controller, which picks
// which UAbility subclass to activate. This class only knows how to
// activate and end itself.
// ============================================================================

UCLASS(Blueprintable)
class PROJECT_A_API UAbility : public UObject, public IAbilityLifecycle
{
	GENERATED_BODY()

public:
	UAbility();

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

	virtual FName GetAbilityName() const { return AbilityName; }
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FName AbilityName = FName("NO_NAME_ABILITY");

	// Root tag identifying this ability (e.g. "Ability.Fireball"). Event tags reported
	// automatically by this class (Cast, TargetHit, Finish) are composed from this root.
	// Auto-derived from AbilityName in SetupAbility ("Ability." + AbilityName) - not directly
	// editable, so there's only one place (AbilityName) to author the ability's identity.
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
	// callers (e.g. AAbilityProduct::ApplyEffect) can pair it with their own reporting logic.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void ApplyEffect(AUnitBase* Target, const FGameplayEffect& Effect);

	// Finds every AUnitBase within Radius of Location matching TargetSelection. Pure
	// target-finder - does not apply an effect or report an event; pair with ApplyEffect for that.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	TArray<AUnitBase*> Target_AOE(ETargetSelection TargetSelection, FVector Location, float Radius);

	UFUNCTION(BlueprintCallable, meta = (Latent, LatentInfo = "LatentInfo"), Category = "Ability")
	void Execute_Projectile(FLatentActionInfo LatentInfo, TSubclassOf<AProjectile> NewProjectile, FVector Target, float Speed,
		int32 PenetrationCount, FVector& OutLocation);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	AAbilityActor* Execute_Summon(TSubclassOf<AAbilityActor> NewActor, FTransform Transform);

	// --------------------------------------------------------------
	// Talent delegation
	// --------------------------------------------------------------
	// Reports a tagged event (with contextual Payload) to the caster's
	// PlayerUnit, which fans it out to any listening talents. Called by
	// this Ability's own Execute_* library, or directly by the Ability
	// Blueprint (e.g. from OnActivate/OnEnd for Cast/Finish events).
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void ReportAbilityEvent(FGameplayTagContainer EventTags, FAbilityEventPayload Payload);

	// Builds the tag set reported for one event: this ability's identity tag (AbilityTag, e.g.
	// "Ability.Fireball") plus the shared, ability-agnostic event-category tag ("Event." + Suffix,
	// e.g. "Event.TargetHit"). Reporting both as independent facets (rather than one composed
	// "Ability.Fireball.TargetHit" tag) lets a talent's RequiredTags AND them together - e.g.
	// {Event.Crit} to react to any ability's crit, or {Ability.Fireball, Event.Crit} for Fireball's
	// only. Public so callers outside UAbility (e.g. AAbilityActor::HandleOverlap) can compose their
	// own event tag sets from the owning ability's AbilityTag.
	FGameplayTagContainer ComposeEventTags(const TCHAR* Suffix) const;
};
