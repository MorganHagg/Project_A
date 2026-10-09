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

	// Activates this ability. Calls into OnActivate (Blueprint-implementable).
	virtual void ActivateAbility();

	// Ends this ability. Calls into OnEnd (Blueprint-implementable).
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void EndAbility();

	// Called if an ability is no longer needed
	void KillAbility();

protected:
	UPROPERTY()
	bool bHasEnded = false;	// Small guard against double end

public:
	// --------------------------------------------------------------
	// Talent delegation
	// --------------------------------------------------------------
	// Reports a tagged event (with contextual Payload) to the caster's
	// PlayerUnit, which fans it out to any listening talents.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void ReportAbilityEvent(FGameplayTagContainer EventTags, FAbilityEventPayload Payload);
};
