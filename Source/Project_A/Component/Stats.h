#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Stats.generated.h"

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
	FName Name;

	UPROPERTY(BlueprintReadOnly)
	float Max = 0.f;

	float ReturnRatio() const
	{
		return Max > 0.f ? Value / Max : 0.f;
	}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_A_API UStats : public UActorComponent
{
	GENERATED_BODY()

public:
	UStats();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	FResource Health = FResource(100.f, TEXT("Health"));
	
};