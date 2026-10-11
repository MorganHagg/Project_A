#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "../Misc/AttributeType.h"
#include "UnitDataBase.generated.h"

class UAbilityData;
class UOverTimeEffectData;
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
	TArray<UAbilityData*> DefaultAbilities;

	// Over-time effects this unit's abilities can apply (by tag, see AAbility::ApplyOverTimeEffect).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<UOverTimeEffectData*> DefaultOverTimeEffects;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<EAttributeType, float> DefaultAttributes;

	// Floating health bar shown above this unit (e.g. WBP_HealthBar). Leave empty for no bar.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UHealthBarWidget> HealthBarWidgetClass;
};
