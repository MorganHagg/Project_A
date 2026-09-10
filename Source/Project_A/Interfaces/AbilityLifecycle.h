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

	// Location is where the ability/product ended - passed through from Finish()/EndAbility() so
	// Blueprint doesn't need to query it separately (it may no longer be meaningful by the time
	// this fires, e.g. after MyCaster is cleared).
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnEnd(FVector Location);
	virtual void OnEnd_Implementation(FVector Location) {}
};
