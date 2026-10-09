#pragma once
#include "Runtime/Core/Public/CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayEffect.generated.h"

// Selects which of the target's stats mitigates an effect's damage (see
// AUnitBase::MitigateDamage): Magic -> MagicResist, Physical -> Armour, True -> none.
// TrueDamage rather than True - UHT rejects enum entries named true/false in any case.
UENUM(BlueprintType)
enum class EAbilityType : uint8
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

// One instant change to one attribute. Anything lasting over time is a UOverTimeEffect, which
// applies these as it goes.
USTRUCT(BlueprintType)
struct FGameplayEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAttributeType Attribute = EAttributeType::Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEffectOperation Operation = EEffectOperation::Subtract;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Magnitude = 0.f;

	// Mitigation for this effect's damage (only used by Subtract on Health).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAbilityType AbilityType = EAbilityType::Magic;
};
