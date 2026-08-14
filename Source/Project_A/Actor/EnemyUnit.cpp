#include "EnemyUnit.h"
#include "AIController.h"

void AEnemyUnit::SetBehaviorTree(UBehaviorTree* InTree)
{
	BehaviorTree = InTree;
	if (AIController && BehaviorTree)
		AIController->RunBehaviorTree(BehaviorTree);
}