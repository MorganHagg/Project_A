#include "EnemyUnit.h"
#include "AIController.h"
#include "../DataAsset/EnemyUnitData.h"

void AEnemyUnit::SetBehaviorTree(UBehaviorTree* InTree)
{
	BehaviorTree = InTree;
	if (AIController && BehaviorTree)
		AIController->RunBehaviorTree(BehaviorTree);
}

void AEnemyUnit::Initiate(UUnitDataBase* SpawnData)
{
	Super::Initiate(SpawnData);
	if (UEnemyUnitData* EnemyData = Cast<UEnemyUnitData>(SpawnData))
	{
		if (EnemyData->BehaviorTree)
			SetBehaviorTree(EnemyData->BehaviorTree);
	}
}
