#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "../Misc/AttributeSet.h"
#include "UnitDataBase.generated.h"

class UAbilityDataAsset;
class USkeletalMesh;

UCLASS()
class PROJECT_A_API UUnitDataBase : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	
	UUnitDataBase();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMesh> Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAnimInstance> AnimationBlueprint;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<UAbilityDataAsset*> DefaultAbilities;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<EAttributeType, float> DefaultAttributes;
};
