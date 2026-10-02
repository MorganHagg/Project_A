#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "../Misc/AttributeSet.h"
#include "UnitDataBase.generated.h"

class UAbilityDataAsset;
class USkeletalMesh;
class UHealthBarWidget;

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

	// Floating health bar shown above this unit (e.g. WBP_HealthBar). Leave empty for no bar.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UHealthBarWidget> HealthBarWidgetClass;
};
