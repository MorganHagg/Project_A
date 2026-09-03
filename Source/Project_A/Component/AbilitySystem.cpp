#include "AbilitySystem.h"
#include "../Ability/Ability.h"
#include "../Unit/UnitBase.h"
#include "../DataAsset/UnitDataBase.h"

UAbilitySystem::UAbilitySystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UAbilitySystem::BeginPlay()
{
	Super::BeginPlay();
	MyOwner = CastChecked<AUnitBase>(GetOwner());
}

void UAbilitySystem::InstantiateAbilities(const UUnitDataBase* UnitData)
{
	if (!UnitData)
	{
		return;
	}

	for (TSubclassOf<UAbility> AbilityClass : UnitData->DefaultAbilities)
	{
		if (!AbilityClass)
		{
			continue;
		}

		UAbility* NewAbility = NewObject<UAbility>(this, AbilityClass);
		NewAbility->SetupAbility(MyOwner);
		GrantedAbilities.Add(NewAbility);
	}
}

bool UAbilitySystem::SwapAbility(TSubclassOf<UAbility> OldAbilityClass, TSubclassOf<UAbility> NewAbilityClass)
{
	if (!NewAbilityClass)
	{
		UE_LOG(LogTemp, Error, TEXT("NewAbilityClass is null!"));
		return false;
	}

	for (UAbility*& Ability : GrantedAbilities)
	{
		if (Ability && Ability->GetClass() == OldAbilityClass)
		{
			Ability = NewObject<UAbility>(this, NewAbilityClass);
			Ability->SetupAbility(MyOwner);
			return true;
		}
	}

	return false;
}

UAbility* UAbilitySystem::InitiateAbility(int32 Slot)
{
	if (GrantedAbilities.IsValidIndex(Slot) &&
		GrantedAbilities[Slot] &&
		MyOwner)
	{
		GrantedAbilities[Slot]->ActivateAbility();
		return GrantedAbilities[Slot];	
	}

	GEngine->AddOnScreenDebugMessage(
		-1,
		1,
		FColor::Red,
		TEXT("Activate ability failed."));

	return nullptr;
}

void UAbilitySystem::SetActiveAbility(UAbility* NewActiveAbility)
{
	if (NewActiveAbility)
	{
		ActiveAbility = NewActiveAbility;
		SetComponentTickEnabled(true);
	}
	else
		UE_LOG(LogTemp, Error, TEXT(
			"AbilitySystem::ActivateAbility doesn't have valid *NewActiveAbility."));
}

void UAbilitySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (ActiveAbility)
		ActiveAbility->TickAbility(DeltaTime);
}


void UAbilitySystem::EndActiveAbility()
{
	if (ActiveAbility)
	{
		ActiveAbility->EndAbility();
		ActiveAbility = nullptr;
		SetComponentTickEnabled(false);
	}
}
