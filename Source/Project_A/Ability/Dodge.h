#pragma once

#include "CoreMinimal.h"
#include "Ability.h"
#include "Dodge.generated.h"


UCLASS()
class PROJECT_A_API UDodge : public UAbility
{
	GENERATED_BODY()

	// Activation Type
	EAbilityActivationType GetAbilityType() const override{
		return EAbilityActivationType::Instant;	}

	// Ability Name
	FName GetAbilityName() const override { 
		return FName("Dodge");	}
	
	void OnInstant() override;
};
