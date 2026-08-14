#pragma once
#include "CoreMinimal.h"
#include "PlayerSpawnDataRow.h"
#include "UnitSpawnDataRow.generated.h"

class UBehaviorTree;

USTRUCT(BlueprintType)
struct PROJECT_A_API FUnitSpawnDataRow : public FPlayerSpawnDataRow
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UBehaviorTree> BehaviorTree = nullptr;
};