#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Ability.generated.h"

// Enums
UENUM(BlueprintType)
enum class EAbilityState : uint8
{
	None,
	Pressed,
	Effect2_Charging,
	Effect3_Modified
};

UENUM(BlueprintType)
enum class EAbilityActivationType : uint8
{
	None,
	Interactive,        // Complex hold/modify system (Effect1/2/3) 
	Instant,           // Single immediate effect
	Passive            // Always active, no input needed
};

class ACharacter;

UCLASS()
class PROJECT_A_API UAbility : public UObject
{
	GENERATED_BODY()
public:
	UAbility();
	
	bool ShowDebugg = false;		// Used when debugging

	UPROPERTY()
	ACharacter* MyCaster;
	UPROPERTY()
	ACharacter* MyTarget;

	FString AbilityUUID;
	FString GetAbilityUUID();

	// Activating all abilities
	void ActivateAbility(ACharacter* NewCaster);

	// On right-click while hold
	void DoModify() {CurrentState = EAbilityState::Effect3_Modified;};

	// Ending all abilities
	void EndAbility();

	EAbilityState GetCurrentState();
	
protected:
	UPROPERTY(BlueprintReadOnly)
	EAbilityState CurrentState = EAbilityState::None;
	EAbilityActivationType AbilityType = EAbilityActivationType::None;
	FTimerHandle InputTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ClickDelay = 0.3f;

	// Child functions
	virtual void OnTap() {UE_LOG(LogTemp, Warning, TEXT("Click tap"));};
	virtual void OnHold() {};
	virtual void OnHoldEnd() {};
	virtual void OnModify() {};
	virtual void OnInstant() {};
	virtual void OnPassive() {};

	
	float PressStartTime = 0.f;
	//float clickDelay = 0.2f;	// delay to distinguish tap vs hold
	FName AbilityName = FName("NO_NAME_ABILITY");
	
	FTimerHandle ThresholdTimerHandle;

	virtual EAbilityActivationType GetAbilityType() const 
	{ 
		return EAbilityActivationType::None; 
	}
    
	// Virtual function that child classes override to specify their name
	virtual FName GetAbilityName() const
	{
		return FName("NO_NAME_ABILITY");
	}
    
	// Add these function declarations:
	void ThresholdMet();
};
