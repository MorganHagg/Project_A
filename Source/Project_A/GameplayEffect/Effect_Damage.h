#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Effect_Damage.generated.h"

UCLASS()
class PROJECT_A_API UEffect_Damage : public UGameplayEffect
{
	GENERATED_BODY()

public:

	// Effect Name
	FName GetEffectName() const override
	{
		return FName("Effect_Damage");
	}
	
	
};
