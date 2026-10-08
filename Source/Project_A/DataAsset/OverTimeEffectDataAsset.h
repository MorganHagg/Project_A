#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "../Ability/Ability.h"
#include "OverTimeEffectDataAsset.generated.h"

class UOverTimeEffect;

// Config for one UOverTimeEffectSlot - copied onto the slot when a unit is set up (see
// UAbilitySystem::InstantiateOverTimeEffects), where talents can then change it.
UCLASS(BlueprintType)
class PROJECT_A_API UOverTimeEffectDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Identity: the slot's tag is "Effect.<EffectName>" (registered by Abilities.SyncTags).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName EffectName = FName("NO_NAME_EFFECT");

	// 0 = permanent - lasts until removed or the unit dies.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Duration = 0.f;

	// Time between OnTicks, the first one Interval after application. 0 = never ticks.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Interval = 0.f;

	// 0 = a new application is ignored while one is present. N = up to N stacks; at the limit the
	// oldest fully ends before the new one is applied (so 1 = refresh). -1 = unlimited.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 StackLimit = 1;

	// True: stacks (and StackLimit) are counted per caster. False: shared by every caster.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bMultipleCaster = true;

	// What the effect does: applied once on application when Interval is 0, otherwise on every
	// tick. Stat changes are undone when the effect stops; damage/heal stays. Magnitude 0 = nothing.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayEffect Effect;

	// Mitigation for Effect's damage (see AUnitBase::MitigateDamage).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EAbilityType AbilityType = EAbilityType::Magic;

	// Optional Blueprint subclass for behaviour Effect can't describe. Empty = the generic UOverTimeEffect.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UOverTimeEffect> EffectClass;
};
