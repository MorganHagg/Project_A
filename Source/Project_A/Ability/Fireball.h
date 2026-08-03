#pragma once
#include "CoreMinimal.h"
#include "Ability.h"
#include "Fireball.generated.h"

UCLASS()
class PROJECT_A_API UFireball : public UAbility
{
	GENERATED_BODY()
	
protected:
	// Activation Type
	EAbilityActivationType GetAbilityType() const override { 
		return EAbilityActivationType::Interactive;	}

	// Ability Name
	FName GetAbilityName() const override { 
		return FName("Fireball");	}

	void OnTap() override;
	void OnHold() override;
	void OnHoldEnd() override;
	void OnModify() override;
};
