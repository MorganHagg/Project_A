#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FPlayerUnitParams.generated.h"

USTRUCT(BlueprintType)
struct FPlayerUnitParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMesh> Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> AnimationBlueprint = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UAbility>> DefaultAbilities;
};