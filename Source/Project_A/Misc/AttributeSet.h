#pragma once
#include "Runtime/Core/Public/CoreMinimal.h"
#include "AttributeSet.generated.h"

UENUM(BlueprintType)
enum class EAttributeType : uint8
{
	Strength,
	Intellect,
	Armour,
	MagicResist,
	Speed,
	Crit,

	Count		// Gives the number of entries - Used in AUnitBase's constructor
};

USTRUCT(BlueprintType)
struct FResource
{
	GENERATED_BODY()

	FResource() = default;
	FResource(float InResource)
		: Value(InResource), Max(InResource) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Max = 0.f;

	float ReturnRatio() const
	{
		return Max > 0.f ? Value / Max : 0.f;
	}
};
