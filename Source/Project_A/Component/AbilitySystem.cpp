#include "AbilitySystem.h"
#include "../Ability/Ability.h"
#include "GameFramework/Character.h"

void UAbilitySystem::BeginPlay()
{
	Super::BeginPlay();
	MyOwner = GetOwner();
}

void UAbilitySystem::AddAbility(int32 Slot, TSubclassOf<UAbility> AbilityClass)
{
	if (!AbilityClass)
	{
		UE_LOG(LogTemp, Error, TEXT("AbilityClass is null!"));
		return;
	}

	if (Slot < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Slot is negative: %d"), Slot);
		return;
	}

	GrantedAbilities.Add(Slot, AbilityClass);
}

void UAbilitySystem::RemoveAbility(int32 Slot)
{
	GrantedAbilities.Remove(Slot);
}

UAbility* UAbilitySystem::ActivateAbility(int32 Slot)
{
	TSubclassOf<UAbility>* AbilityClass = GrantedAbilities.Find(Slot);

	if (AbilityClass && *AbilityClass && MyOwner)
	{
		UAbility* NewAbility = NewObject<UAbility>(this, *AbilityClass);
		NewAbility->ActivateAbility(Cast<ACharacter>(MyOwner));
		ActiveAbility = NewAbility;
		return ActiveAbility;
	}

	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, TEXT("Activate ability failed."));
	return nullptr;
}

void UAbilitySystem::EndActiveAbility()
{
	if (ActiveAbility)
	{
		ActiveAbility->EndAbility();
		ActiveAbility = nullptr;
	}
}