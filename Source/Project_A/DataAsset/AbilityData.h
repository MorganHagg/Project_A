#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "../Ability/Ability.h"
#include "AbilityData.generated.h"

UCLASS()
class PROJECT_A_API UAbilityData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UAbility* Ability;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Magnitude;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Cost;
	
};
