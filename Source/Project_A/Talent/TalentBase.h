#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TalentBase.generated.h"

class UAbility;

UCLASS(Blueprintable)
class PROJECT_A_API UTalentBase : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void BindToAbility(UAbility* Ability);

	UFUNCTION(BlueprintImplementableEvent)
	void OnSetup();
	
	UPROPERTY(BlueprintReadWrite)
	UAbility* MyAbility;

	UFUNCTION(BlueprintImplementableEvent)
	void HandleAbilityHit(UAbility* Ability, AUnitBase* HitActor);
	
};


/*
Declare the delegate type above the class: DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityHit, UAbility*, Ability, AUnitBase*, HitUnit);
Forward-declare any custom types used in the delegate params (e.g. class AUnitBase;) if not already declared
Add the delegate as a property: UPROPERTY(BlueprintAssignable, Category = "Ability") FOnAbilityHit OnHit;
Broadcast it where the event happens: OnHit.Broadcast(this, HitUnit);
To bind from another class, declare a matching UFUNCTION() handler with the exact same param types, then call Ability->OnHit.AddDynamic(this, &UTalentBase::HandleAbilityHit);
Rebuild (full rebuild, not just compile) so UHT regenerates the .generated.h file with the new delegate/property
*/