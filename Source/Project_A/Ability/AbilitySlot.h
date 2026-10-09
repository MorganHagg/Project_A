#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "../Interfaces/AbilityLifecycle.h"
#include "AbilityEventPayload.h"
#include "Ability.h"
#include "AbilitySlot.generated.h"

// Forward declarations
class AUnitBase;
class AAbility;

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

	UWorld* World;

	// --------------------------------------------------------------
	// Tunables
	// --------------------------------------------------------------

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bModifyEndsAbility = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float CoolDown = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Cost = 0.f;

	// See UAbilityDataAsset::bActivateOnRelease / MaxChargeTime.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bActivateOnRelease = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MaxChargeTime = 0.f;

	// Which Ability (product) class this slot spawns when activated. Copied from the
	// UAbilityDataAsset this instance was built from - see UAbilitySystem::InstantiateAbilities.
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	TSubclassOf<AAbility> ProductClass;

	// This slot's identity: ProductClass's "Ability.<AbilityName>" tag. Set by
	// UAbilitySystem::InstantiateAbilities; used by talents to find the slot (UAbilitySystem::FindSlot).
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayTag AbilityTag;

	// --------------------------------------------------------------
	// Lifecycle
	// --------------------------------------------------------------

	// Sets up the ability for later use
	void SetupAbility(AUnitBase* NewCaster);

	// Activates this ability: spawns its Ability, unless bActivateOnRelease (then it spawns on release).
	// InModifiedAbility is set on the spawned Ability's ModifiedAbility (see AControllerBase::ApplyModify).
	virtual void ActivateAbility(AAbility* InModifiedAbility = nullptr);

	// Starts charging - called when this slot becomes the active hold (UAbilitySystem::SetActiveAbility).
	// Every hold charges; bActivateOnRelease only decides when the Ability spawns.
	void StartCharging();

	// The hold was released: ends the hold, then with bActivateOnRelease spawns the Ability with the
	// charge time. Otherwise the same as EndAbility.
	void ReleaseAbility();

	// Ends this ability: stops charging, writes the final charge time to the live Ability from the last
	// activation (AAbility::ChargeTime) and runs its OnHoldEnded (by default it finishes), then calls
	// into OnEnd (Blueprint-implementable). A bActivateOnRelease charge is dropped without firing.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void EndAbility();

	// The live Ability spawned by the last activation, or null.
	UFUNCTION(BlueprintPure, Category = "Ability")
	AAbility* GetSpawnedAbility() const { return SpawnedAbility.Get(); }

	UFUNCTION(BlueprintPure, Category = "Ability")
	bool IsCharging() const { return bIsCharging; }

	// Seconds the current hold has charged, capped at MaxChargeTime. 0 when not charging.
	UFUNCTION(BlueprintPure, Category = "Ability")
	float GetChargeTime() const;

	// Called if an ability is no longer needed
	void KillAbility();

protected:
	UPROPERTY()
	bool bHasEnded = false;	// Small guard against double end

	// The Ability spawned by the last ActivateAbility. Weak: it destroys itself when it finishes.
	TWeakObjectPtr<AAbility> SpawnedAbility;

private:
	// Spawns ProductClass with its ChargeTime and ModifiedAbility set, and returns it.
	AAbility* SpawnAbility(float ChargeTime, AAbility* InModifiedAbility = nullptr);

	bool IsCasterDead() const;

	bool bIsCharging = false;
	float ChargeStartTime = 0.f;

public:
	// --------------------------------------------------------------
	// Talent delegation
	// --------------------------------------------------------------
	// Reports a tagged event (with contextual Payload) to the caster's
	// PlayerUnit, which fans it out to any listening talents.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void ReportAbilityEvent(FGameplayTagContainer EventTags, FAbilityEventPayload Payload);
};
