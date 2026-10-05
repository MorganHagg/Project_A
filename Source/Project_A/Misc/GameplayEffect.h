#pragma once
#include "Runtime/Core/Public/CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayEffect.generated.h"

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
};
