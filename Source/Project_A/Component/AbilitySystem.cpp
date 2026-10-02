#include "AbilitySystem.h"
#include "../Ability/AbilitySlot.h"
#include "../Unit/UnitBase.h"
#include "../DataAsset/UnitDataBase.h"
#include "../DataAsset/AbilityDataAsset.h"
#include "AttributeComponent.h"

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

void UAbilitySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (ActiveAbility)
		ActiveAbility->TickAbility(DeltaTime);
}

void UAbilitySystem::InstantiateAbilities(const UUnitDataBase* UnitData)
{
	if (!UnitData)
	{
		return;
	}

	for (UAbilityDataAsset* AbilityData : UnitData->DefaultAbilities)
	{
		if (!AbilityData)
		{
			continue;
		}

		// One generic class now - identity/behavior come entirely from the copied DataAsset
		// values and ProductClass, not from a per-ability subclass.
		UAbilitySlot* NewAbility = NewObject<UAbilitySlot>(this);
		NewAbility->CoolDown = AbilityData->CoolDown;
		NewAbility->Cost = AbilityData->Cost;
		NewAbility->MagnitudeMultiplier = AbilityData->MagnitudeMultiplier;
		NewAbility->ProductClass = AbilityData->ProductClass;
		NewAbility->AbilityType = AbilityData->AbilityType;
		NewAbility->SetupAbility(MyOwner);
		GrantedAbilities.Add(NewAbility);
	}
}

UAbilitySlot* UAbilitySystem::InitiateAbility(int32 Slot)
{
	if (IsOwnerDead())
	{
		return nullptr;
	}

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

void UAbilitySystem::SetActiveAbility(UAbilitySlot* NewActiveAbility)
{
	if (IsOwnerDead())
	{
		return;
	}

	if (NewActiveAbility)
	{
		ActiveAbility = NewActiveAbility;
		SetComponentTickEnabled(true);
	}
	else
		UE_LOG(LogTemp, Error, TEXT(
			"AbilitySystem::ActivateAbility doesn't have valid *NewActiveAbility."));
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

bool UAbilitySystem::IsOwnerDead() const
{
	return MyOwner && MyOwner->AttributeComponent && MyOwner->AttributeComponent->IsDead();
}
