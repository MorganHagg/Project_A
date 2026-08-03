#pragma once

#include "CoreMinimal.h"
#include "Ability.h"
#include "Fireblast.generated.h"

UCLASS()
class PROJECT_A_API UFireblast : public UAbility
{
	GENERATED_BODY()

protected:
	// Activation Type
	EAbilityActivationType GetAbilityType() const override
	{
		return EAbilityActivationType::Interactive;
	}

	// Ability Name
	FName GetAbilityName() const override
	{
		return FName("Fireblast");
	}

	void OnTap() override;
	void OnHold() override;
	void OnHoldEnd() override;
	void OnModify() override;
};
