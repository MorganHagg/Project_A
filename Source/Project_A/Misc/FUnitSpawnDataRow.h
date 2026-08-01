#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FUnitSpawnDataRow.generated.h"

USTRUCT(BlueprintType)
struct FFUnitSpawnDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBehaviorTree* BehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UAbility>> DefaultAbilities;
};