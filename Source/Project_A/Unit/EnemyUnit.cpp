#include "EnemyUnit.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "../DataAsset/EnemyUnitData.h"

void AEnemyUnit::SetBehaviorTree(UBehaviorTree* InTree)
{
	BehaviorTree = InTree;
	if (AIController && BehaviorTree)
		AIController->RunBehaviorTree(BehaviorTree);
}

void AEnemyUnit::SetupUnit(UUnitDataBase* SpawnData)
{
	Super::SetupUnit(SpawnData);
	if (UEnemyUnitData* EnemyData = Cast<UEnemyUnitData>(SpawnData))
	{
		if (EnemyData->BehaviorTree)
			SetBehaviorTree(EnemyData->BehaviorTree);
	}
}


void AEnemyUnit::HandleDeath()
{
	Super::HandleDeath();

	// Uses the possessing controller, not the AIController member - that member is never assigned.
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		AI->StopMovement();
		if (AI->BrainComponent)
		{
			AI->BrainComponent->StopLogic(TEXT("Dead"));
		}
	}
}
