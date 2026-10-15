#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "../Ability/Ability.h"
#include "../Misc/AttributeEffect.h"
#include "AbilityData.generated.h"

class UOverTimeEffect;

// What kind of granted ability a UAbilityData describes. Decides which fields are shown here.
// Active: cast from an input, spawns AbilityClass. Proc: never cast; its Effect is fetched by other
// abilities (AAbility::GetEffect), e.g. Ignite. Passive: its Effect is applied to its own unit at setup
// and lasts until the unit dies (see UEffectComponent::ApplyPassive) - Duration is hidden.
UENUM(BlueprintType)
enum class EAbilityKind : uint8
{
	Active,
	Proc,
	Passive
};

// Config for one UGrantedAbility instance - copied onto it when a unit is set up (see
// UAbilityComponent::InstantiateAbilities), where talents can then change it. Ability-specific
// tunables (Speed, PenetrationCount, etc.) live as class defaults on AbilityClass itself, not here.
UCLASS(BlueprintType)
class PROJECT_A_API UAbilityData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EAbilityKind Kind = EAbilityKind::Active;

	// Identity for Proc/Passive: the granted ability's tag is "Ability.<AbilityName>" (registered by
	// Abilities.SyncTags). Active abilities take theirs from AbilityClass's AbilityName instead.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "Kind != EAbilityKind::Active", EditConditionHides))
	FName AbilityName = FName("NO_NAME_ABILITY");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "Kind == EAbilityKind::Active", EditConditionHides))
	float CoolDown = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "Kind == EAbilityKind::Active", EditConditionHides))
	float Cost = 0.f;

	// Hold slots only: the Ability spawns on release (with the charge time) instead of on press.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "Kind == EAbilityKind::Active", EditConditionHides))
	bool bActivateOnRelease = false;

	// Hold slots only: cap on the charge time (how long the hold has been held, see
	// UGrantedAbility::GetChargeTime and AAbility::ChargeTime) - holding longer doesn't charge further.
	// 0 = no cap.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "Kind == EAbilityKind::Active", EditConditionHides))
	float MaxChargeTime = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "Kind == EAbilityKind::Active", EditConditionHides))
	TSubclassOf<AAbility> AbilityClass;

	// This ability's effect, fetched with AAbility::GetEffect and applied with ApplyEffect. Instant
	// when Duration and Interval are 0, timed otherwise. Passive: always permanent (Duration hidden);
	// Interval > 0 applies it on every tick.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FAttributeEffect Effect;

	// Damage only: how much of the caster's stat is added to Effect's damage - Intellect for Magic,
	// Strength for Physical, none for True (see UEffectComponent::CalculateOutgoingDamage).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0", EditCondition = "Kind != EAbilityKind::Passive", EditConditionHides))
	float Coefficient = 1.f;

	// Timed Effect only. 0 = a new application is ignored while one is present. N = up to N stacks;
	// at the limit the oldest fully ends before the new one is applied (so 1 = refresh). -1 = unlimited.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "Kind != EAbilityKind::Passive", EditConditionHides))
	int32 StackLimit = 1;

	// Timed Effect only. True: stacks (and StackLimit) are counted per caster. False: shared by every caster.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "Kind != EAbilityKind::Passive", EditConditionHides))
	bool bMultipleCaster = true;

	// Timed Effect and Passive. Optional Blueprint subclass for behaviour Effect can't describe. Empty =
	// the generic UOverTimeEffect.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UOverTimeEffect> EffectClass;

#if WITH_EDITOR
	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

private:
	// Hides Effect's Duration for a Passive (see FAttributeEffect::bHideDuration).
	void SyncEffectDisplay();
#endif
};
