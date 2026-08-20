#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AbilityLifecycle.generated.h"

UINTERFACE()
class UAbilityLifecycle : public UInterface
{
	GENERATED_BODY()
};
class IAbilityLifecycle
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnActivate();
	virtual void OnActivate_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnTick();
	virtual void OnTick_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnEnd();
	virtual void OnEnd_Implementation() {}
};
