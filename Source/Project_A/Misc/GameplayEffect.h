#pragma once
#include "Runtime/Core/Public/CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayEffect.generated.h"

UENUM(BlueprintType)
enum class EEffectAttribute : uint8
{
	Health,
	Strength,
	Intellect,
	Armour,
	MagicResist,
	Speed,
	Crit
};

UENUM(BlueprintType)
enum class EEffectOperation : uint8
{
	Subtract,
	Add,
	Modify
};

USTRUCT(BlueprintType)
struct FGameplayEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEffectAttribute Attribute = EEffectAttribute::Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEffectOperation Operation = EEffectOperation::Add;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Magnitude = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Interval = 0.f;
};
