#pragma once
#include "CoreMinimal.h"
#include "AttributeSet.generated.h"

USTRUCT(BlueprintType)
struct FStats
{
	GENERATED_BODY()

	FStats() = default;
	FStats(float InStat, const FName& InName)
		: Value(InStat), Name(InName) {}

	UPROPERTY(BlueprintReadOnly)
	float Value = 0.f;

	UPROPERTY(BlueprintReadOnly)
	FName Name;
};

USTRUCT(BlueprintType)
struct FResource
{
	GENERATED_BODY()

	FResource() = default;
	FResource(float InStat, const FName& InName)
		: Value(InStat), Max(InStat), Name(InName) {}

	UPROPERTY(BlueprintReadOnly)
	float Value = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float Max = 0.f;

	UPROPERTY(BlueprintReadOnly)
	FName Name;

	float ReturnRatio() const
	{
		return Max > 0.f ? Value / Max : 0.f;
	}
};
