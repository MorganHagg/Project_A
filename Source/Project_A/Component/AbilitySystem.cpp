#include "AbilitySystem.h"
#include "../Ability/Ability.h"
#include "../Actor/UnitBase.h"

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

	GrantedAbilities.SetNum(FMath::Max(GrantedAbilities.Num(), Slot + 1));
	GrantedAbilities[Slot] = AbilityClass;
}

void UAbilitySystem::RemoveAbility(int32 Slot)
{
	if (GrantedAbilities.IsValidIndex(Slot))
	{
		GrantedAbilities[Slot] = nullptr;
	}
}

UAbility* UAbilitySystem::InitiateAbility(int32 Slot)
{
	if (GrantedAbilities.IsValidIndex(Slot) &&
		GrantedAbilities[Slot] &&
		MyOwner)
	{
		UAbility* NewAbility = NewObject<UAbility>(
			this,
			GrantedAbilities[Slot]);

		NewAbility->InitiateAbility(Cast<ACharacter>(MyOwner));
		return NewAbility;
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
