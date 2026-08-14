#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PlayerSpawnDataRow.generated.h"

class UAbility;
class UAnimInstance;

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