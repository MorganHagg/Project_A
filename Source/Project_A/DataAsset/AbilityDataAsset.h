#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "../Ability/Ability.h"
#include "AbilityDataAsset.generated.h"

// Config for one AbilitySlot instance - only cross-ability tunables that mean the same thing for
// every ability (cooldown, cost) plus which Ability class this slot spawns when
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

	// Hold slots only: the Ability spawns on release (with the charge time) instead of on press.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bActivateOnRelease = false;

	// Hold slots only: cap on the charge time (how long the hold has been held, see
	// UAbilitySlot::GetChargeTime and AAbility::ChargeTime) - holding longer doesn't charge further.
	// 0 = no cap.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxChargeTime = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AAbility> ProductClass;
};
