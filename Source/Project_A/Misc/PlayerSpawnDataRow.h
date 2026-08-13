#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerSpawnDataRow.generated.h"

USTRUCT(BlueprintType)
struct PROJECT_A_API FPlayerSpawnDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMesh> Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UAnimInstance> AnimationBlueprint = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSubclassOf<UAbility>> DefaultAbilities;
};