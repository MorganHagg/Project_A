#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "../Interfaces/AbilityLifecycle.h"
#include "AbilityEventPayload.h"
#include "Ability.h"
#include "../DataAsset/AbilityData.h"
#include "GrantedAbility.generated.h"

// Forward declarations
class AUnitBase;
class AAbility;
class UOverTimeEffect;

// ============================================================================
// UGrantedAbility
// One ability a unit was granted, built from a UAbilityData. An Active one is the translation
// layer between AbilityComponent/input and the actual action (Ability): cast-bookkeeping
// (cooldown, cost, charging) and spawning the Ability for each cast. Every kind also holds an
// Effect that any of the caster's abilities can fetch (AAbility::GetEffect) - e.g. Ignite.
// Fully generic, no per-ability branching or Blueprint subclass.
// ============================================================================

UCLASS(Blueprintable)
class PROJECT_A_API UGrantedAbility : public UObject, public IAbilityLifecycle
{
	GENERATED_BODY()

public:
	UGrantedAbility();

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

	// See UAbilityData::bActivateOnRelease / MaxChargeTime.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bActivateOnRelease = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MaxChargeTime = 0.f;

	// Which Ability class this granted ability spawns when activated. Copied from the
	// UAbilityData this instance was built from - see UAbilityComponent::InstantiateAbilities.
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	TSubclassOf<AAbility> AbilityClass;

	// This granted ability's identity, "Ability.<AbilityName>" - from AbilityClass for an Active ability,
	// from the data asset's AbilityName otherwise. Set by UAbilityComponent::InstantiateAbilities; used
	// to find it (UAbilityComponent::FindGrantedAbility).
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayTag AbilityTag;

	// Copied from the UAbilityData. Passive ones are applied at setup (UAbilityComponent::ApplyPassives).
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	EAbilityKind Kind = EAbilityKind::Active;

	// -- Effect (see UAbilityData) --

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Effect")
	FAttributeEffect Effect;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Effect")
	float Coefficient = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Effect")
	int32 StackLimit = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Effect")
	bool bMultipleCaster = true;

	// Never null after InstantiateAbilities - empty in the data asset means the generic UOverTimeEffect.
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UOverTimeEffect> EffectClass;

	// Effect, ready to apply: Source set to this granted ability. Unscaled - stat scaling, the magnitude
	// multiplier and crit are applied by UEffectComponent on each application, from Source.
	UFUNCTION(BlueprintPure, Category = "Effect")
	FAttributeEffect GetEffect();

	// -- Magnitude modifiers --
	// Percent bonuses to every application of Effect, read live when it is applied - including each
	// tick of timed effects already running. Owned by Source, kept until RemoveMagnitudeModifiers(Source).

	// Percent in whole numbers (20 = +20%). Several modifiers add up.
	UFUNCTION(BlueprintCallable, Category = "Effect")
	void AddMagnitudeModifier(UObject* Source, float Percent);

	UFUNCTION(BlueprintCallable, Category = "Effect")
	void RemoveMagnitudeModifiers(UObject* Source);

	// 1 + the sum of active magnitude modifiers / 100.
	UFUNCTION(BlueprintPure, Category = "Effect")
	float GetMagnitudeMultiplier() const;

	// --------------------------------------------------------------
	// Lifecycle
	// --------------------------------------------------------------

	// Sets up the ability for later use
	void SetupAbility(AUnitBase* NewCaster);

	// Activates this ability: spawns its Ability, unless bActivateOnRelease (then it spawns on release).
	// InModifiedAbility is set on the spawned Ability's ModifiedAbility (see AControllerBase::ApplyModify).
	virtual void ActivateAbility(AAbility* InModifiedAbility = nullptr);

	// Starts charging - called when this granted ability becomes the active hold (UAbilityComponent::SetActiveAbility).
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
	// Spawns AbilityClass with its ChargeTime and ModifiedAbility set, and returns it.
	AAbility* SpawnAbility(float ChargeTime, AAbility* InModifiedAbility = nullptr);

	bool IsCasterDead() const;

	bool bIsCharging = false;
	float ChargeStartTime = 0.f;

	struct FMagnitudeModifier
	{
		TWeakObjectPtr<const UObject> Source;
		float Percent = 0.f;
	};
	TArray<FMagnitudeModifier> MagnitudeModifiers;

public:
	// --------------------------------------------------------------
	// Talent delegation
	// --------------------------------------------------------------
	// Reports a tagged event (with contextual Payload) to the caster's
	// PlayerUnit, which fans it out to any listening talents.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void ReportAbilityEvent(FGameplayTagContainer EventTags, FAbilityEventPayload Payload);
};
