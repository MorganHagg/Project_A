#pragma once
#include "CoreMinimal.h"
#include "UnitDataBase.h"
#include "EnemyUnitData.generated.h"

class UAbility;
class UBehaviorTree;
class USkeletalMesh;

UCLASS(BlueprintType)
class PROJECT_A_API UEnemyUnitData : public UUnitDataBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBehaviorTree* BehaviorTree;
};