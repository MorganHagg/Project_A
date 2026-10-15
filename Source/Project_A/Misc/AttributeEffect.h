#pragma once
#include "Runtime/Core/Public/CoreMinimal.h"
#include "AttributeType.h"
#include "AttributeEffect.generated.h"

// Selects which of the target's stats mitigates an effect's damage (see
// UEffectComponent::MitigateDamage): Magic -> MagicResist, Physical -> Armour, True -> none. Also
// selects the caster's scaling stat (see UEffectComponent::CalculateOutgoingDamage): Magic ->
// Intellect, Physical -> Strength, True -> none.
// TrueDamage rather than True - UHT rejects enum entries named true/false in any case.
UENUM(BlueprintType)
enum class EDamageType : uint8
{
	Magic,
	Physical,
	TrueDamage	UMETA(DisplayName = "True")
};

// Percentage operations take whole percentages (Magnitude 10 = 10%) - see
// UAttributeComponent::ModifyAttributePercent.
UENUM(BlueprintType)
enum class EEffectOperation : uint8
{
	Add,
	Subtract,
	AddPercentage,
	SubtractPercentage
};

class UGrantedAbility;

// One change to one attribute. Duration and Interval both 0 = instant. Otherwise it is timed and
// applied through a UOverTimeEffect: Interval 0 applies it once for Duration, Interval > 0 on every
// tick. Stat changes made by a timed effect are undone when it ends; damage/heal stays.
USTRUCT(BlueprintType)
struct FAttributeEffect
{
	GENERATED_BODY()

	bool IsTimed() const { return Duration > 0.f || Interval > 0.f; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAttributeType Attribute = EAttributeType::Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEffectOperation Operation = EEffectOperation::Subtract;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Magnitude = 0.f;

	// Mitigation for this effect's damage (only used by Subtract on Health).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDamageType DamageType = EDamageType::Magic;

	// Timed effects only. 0 = permanent - lasts until removed or the unit dies.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bHideDuration", EditConditionHides))
	float Duration = 0.f;

	// Timed effects only. Time between applications, the first one Interval after application.
	// 0 = applied once, when the effect is applied.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Interval = 0.f;

	// The granted ability this effect came from (set by UGrantedAbility::GetEffect). Gives a timed
	// effect its identity, stacking rules, effect class and magnitude multiplier. Empty for an effect
	// made inline: it is applied as-is, and if timed, stacks without limit and reports no events.
	UPROPERTY()
	TWeakObjectPtr<UGrantedAbility> Source;

	// Output only: set by UEffectComponent on the applied effect it returns when this damage crit.
	UPROPERTY(BlueprintReadOnly)
	bool bCrit = false;

#if WITH_EDITORONLY_DATA
	// Editor display only: hides Duration where its owner makes it meaningless (a Passive's Effect is
	// always permanent). An EditCondition can only read fields of this struct, so the owner sets this -
	// see UAbilityData::SyncEffectDisplay.
	UPROPERTY()
	bool bHideDuration = false;
#endif
};
