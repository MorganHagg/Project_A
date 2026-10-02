#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "../Ability/Ability.h"
#include "AbilityDataAsset.generated.h"

// Config for one AbilitySlot instance - only cross-ability tunables that mean the same thing for
// every ability (cooldown, cost, magnitude) plus which Ability class this slot spawns when
// activated. Product-specific tunables (Speed, PenetrationCount, etc.) live as class defaults on
// ProductClass itself, not here.
UCLASS(BlueprintType)
class PROJECT_A_API UAbilityDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CoolDown = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Cost = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MagnitudeMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EAbilityType AbilityType = EAbilityType::Magic;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AAbility> ProductClass;
};
