#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "../Misc/AttributeType.h"
#include "UnitDataBase.generated.h"

class UAbilityData;
class USkeletalMesh;
class UHealthBarWidget;

// The three abilities one input casts: a tap (released before the hold threshold), a hold, and a
// Modify (the other input pressed while this one is held). Any of them can be left empty.
USTRUCT(BlueprintType)
struct FAbilityInputSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAbilityData> Tap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAbilityData> Hold;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAbilityData> Modify;
};

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
	FAbilityInputSet Primary;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FAbilityInputSet Secondary;

	// Granted abilities not bound to an input - procs and passives, e.g. Ignite, whose Effect other
	// abilities fetch by tag (AAbility::GetEffect).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<UAbilityData*> ExtraAbilities;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<EAttributeType, float> DefaultAttributes;

	// Floating health bar shown above this unit (e.g. WBP_HealthBar). Leave empty for no bar.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UHealthBarWidget> HealthBarWidgetClass;
};
