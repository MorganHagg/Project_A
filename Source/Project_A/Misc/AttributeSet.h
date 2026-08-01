#pragma once
#include "CoreMinimal.h"
#include "AttributeSet.generated.h"

USTRUCT(BlueprintType)
struct FResource
{
	GENERATED_BODY()
	FResource() {}
	FResource(float InMax, FString InName)
		: Current(InMax), Max(InMax), Name(InName) {}
	
	
	UPROPERTY(BlueprintReadOnly)
	float Current;
	
	UPROPERTY(BlueprintReadOnly)
	float Max;

	UPROPERTY(BlueprintReadOnly)
	FString Name;
	
	float ReturnRatio() { return Current/Max;};
};

UCLASS()
class PROJECT_A_API UAttributeSet : public UActorComponent
{
	GENERATED_BODY()
	UAttributeSet(){
		Health = FResource(100, TEXT("Health"));
	};

public: 
	UPROPERTY(BlueprintReadOnly)
	FResource Health;
	
};