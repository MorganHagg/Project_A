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
	Friendly,
	Hostile,
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

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bModifyEndsAbility = true;
	
	UPROPERTY(BlueprintReadOnly)
	AUnitBase* MyCaster;

	UPROPERTY(BlueprintReadOnly)
	APlayerController* MyController;

	UWorld* World;
	FActorSpawnParameters SpawnParams;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float CoolDown = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Cost = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MagnitudeMultiplier = 1.f;

	// GetterFunctions

	float GetCoolDown();
	float GetCost();
	float GetMagnitude();

	UPROPERTY(EditAnywhere)
	FIntervalTicker Ticker;

	// --------------------------------------------------------------
	// Lifecycle
	// --------------------------------------------------------------

	// Sets up the ability for later use
	void SetupAbility(AUnitBase* NewCaster);
	
	// Activates this ability. Calls into OnActivate (Blueprint-implementable).
	virtual void ActivateAbility();

	// Ends this ability. Calls into OnEnd (Blueprint-implementable).
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void EndAbility();

	// Called if an ability is no longer needed
	void KillAbility();

	// --------------------------------------------------------------
	// Blueprint-buildable effect library
	// --------------------------------------------------------------
	UFUNCTION(BlueprintCallable)
	void Execute_Target(const FGameplayEffect& Effect, AUnitBase* Target);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	TArray<ACharacter*> Execute_AOE(const FGameplayEffect& Effect, FVector Location, float Radius, ETargetSelection TargetSelection);

	UFUNCTION(BlueprintCallable, meta = (Latent, LatentInfo = "LatentInfo"), Category = "Ability")
	void Execute_Projectile(FLatentActionInfo LatentInfo, const FGameplayEffect& Effect, UStaticMesh* Mesh, FVector Target, float Speed,
		int32 PenetrationCount, FVector& OutLocation);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	AAbilityActor* Execute_Summon(const FGameplayEffect& Effect, TSubclassOf<AAbilityActor> NewActor, FTransform Transform);

	void TickAbility(float DeltaTime);

	virtual FName GetAbilityName() const { return AbilityName; }
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FName AbilityName = FName("NO_NAME_ABILITY");

	// Root tag identifying this ability (e.g. "Ability.Fireball"). Event tags reported
	// automatically by this class (Cast, TargetHit, Finish) are composed from this root.
	// Auto-derived from AbilityName in SetupAbility ("Ability." + AbilityName) - not directly
	// editable, so there's only one place (AbilityName) to author the ability's identity.
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayTag AbilityTag;

protected:
	UPROPERTY()
	bool bHasEnded = false;	// Small guard against double end

public:
	// Composes AbilityTag + "." + Suffix into a registered event tag (e.g. "TargetHit" -> "Ability.Fireball.TargetHit").
	// Public so callers outside UAbility (e.g. AAbilityActor::HandleOverlap) can compose their own
	// event tags from the owning ability's AbilityTag.
	FGameplayTag ComposeEventTag(const TCHAR* Suffix) const;

	// --------------------------------------------------------------
	// Talent delegation
	// --------------------------------------------------------------
	// Reports a tagged event (with contextual Payload) to the caster's
	// PlayerUnit, which fans it out to any listening talents. Called by
	// this Ability's own Execute_* library, or directly by the Ability
	// Blueprint (e.g. from OnActivate/OnEnd for Cast/Finish events).
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void ReportAbilityEvent(FGameplayTag EventTag, FAbilityEventPayload Payload);
};