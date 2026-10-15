#pragma once
#include "CoreMinimal.h"
#include "UnitBase.h"
#include "EnemyUnit.generated.h"

class UBehaviorTree;
class AAIController;

// HideDropdown: not offered in class pickers (e.g. UEnemyUnitData::CustomUnitClass, where empty already
// means this class). Not Abstract - UUnitSpawner spawns it directly. Not inherited by subclasses.
UCLASS(HideDropdown)
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

	// Also stops the AI controller's behavior tree and pathing.
	virtual void HandleDeath() override;
	
};