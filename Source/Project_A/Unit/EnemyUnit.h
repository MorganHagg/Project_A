#pragma once
#include "CoreMinimal.h"
#include "UnitBase.h"
#include "EnemyUnit.generated.h"

class UBehaviorTree;
class AAIController;

UCLASS()
class PROJECT_A_API AEnemyUnit : public AUnitBase
{
	GENERATED_BODY()

public:
	void SetBehaviorTree(UBehaviorTree* InTree);

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY()
	TObjectPtr<AAIController> AIController;

	virtual void SetupUnit(UUnitDataBase* SpawnData) override;
	
};