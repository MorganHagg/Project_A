#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AbilityLifecycle.generated.h"

class AUnitBase;

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
	
	// Per-ability hit logic (e.g. Fireball applying its damage). Called by AAbility::OnHit before the
	// hit is reported to talents and OnHitDelegate. Not named OnHit - that would clash with
	// AAbility::OnHit on every implementing Ability.
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnTargetHit(AUnitBase* Target, FVector Location);
	virtual void OnTargetHit_Implementation(AUnitBase* Target, FVector Location) {}
	
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnTick();
	virtual void OnTick_Implementation() {}

	// The hold that spawned this ability ended (released, ended by Modify, or the caster died).
	// Called by UAbilitySlot::EndAbility. AAbilityActor/AProjectile default to finishing; override
	// to keep living instead (e.g. a wall that stops growing).
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnHoldEnded();
	virtual void OnHoldEnded_Implementation() {}

	// Location is where the ability/product ended - passed through from Finish()/EndAbility() so
	// Blueprint doesn't need to query it separately (it may no longer be meaningful by the time
	// this fires, e.g. after MyCaster is cleared).
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnEnd(FVector Location);
	virtual void OnEnd_Implementation(FVector Location) {}

};