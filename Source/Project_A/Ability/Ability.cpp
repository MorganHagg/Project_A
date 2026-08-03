#include "Ability.h"
#include "Gameframework/Character.h"
#include "Gameframework/CharacterMovementComponent.h"

UAbility::UAbility()
{
	CurrentState = EAbilityState::None;
	AbilityUUID = FGuid::NewGuid().ToString();
}

FString UAbility::GetAbilityUUID()
{
	return AbilityUUID;
}

void UAbility::ActivateAbility(ACharacter* NewCaster)
{
	UE_LOG(LogTemp, Log, TEXT("UAbility::ActivateAbility()"));
	MyCaster = NewCaster;
	
	switch (GetAbilityType())
	{
	case EAbilityActivationType::Interactive:   
		{
			UWorld* World = GetWorld();
			if (!World)
			{
				UE_LOG(LogTemp, Error, TEXT("GetWorld() returned null in ActivateAbility"));
				return;
			}
            
			PressStartTime = World->GetTimeSeconds();
			CurrentState = EAbilityState::Pressed;

			if (MyCaster)
			{
				World->GetTimerManager().SetTimer(
					ThresholdTimerHandle,
					this,
					&UAbility::ThresholdMet,
					ClickDelay,
					false
				);
			}
			break;
		}
        
	case EAbilityActivationType::Instant:
		{
			OnInstant();
			break;
		}
        
	case EAbilityActivationType::Passive:
		{
			OnPassive();
			break;
		}
        
	case EAbilityActivationType::None:
	default:
		{
			UE_LOG(LogTemp, Error, TEXT("%s has no activation type set"), *AbilityName.ToString());
			break;
		}
	}
}

void UAbility::EndAbility()
{
	UWorld* World = GetWorld();
	if (World)
	{
		float HoldTime = World->GetTimeSeconds() - PressStartTime;      //TODO: Check if this is needed
		World->GetTimerManager().ClearTimer(ThresholdTimerHandle);    
	}
    
	if (CurrentState == EAbilityState::Effect3_Modified)
	{
		OnModify();
	}
    
	if (CurrentState == EAbilityState::Effect2_Charging)
	{
		OnHoldEnd();
	}
	else if (CurrentState == EAbilityState::Pressed)
	{
		OnTap();
	}
}

EAbilityState UAbility::GetCurrentState()
{
	return CurrentState;
}

void UAbility::ThresholdMet()
{
	CurrentState = EAbilityState::Effect2_Charging;
	OnHold();
}

