#include "AbilitySystem.h"
#include "../Ability/Ability.h"
#include "GameFramework/Character.h"

UAbilitySystem::UAbilitySystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	GrantedAbilities.Empty();
	ActiveAbility = nullptr;
}

void UAbilitySystem::BeginPlay()
{
	Super::BeginPlay();
	MyOwner = CastChecked<ACharacter>(GetOwner());
}

void UAbilitySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UAbilitySystem::AddAbility(TSubclassOf<UAbility> AbilityClass, EAbilityInputID InputID)
{
	if (!AbilityClass)
	{
		UE_LOG(LogTemp, Error, TEXT("AbilityClass is null!"));
		return;
	}

	if (InputID == EAbilityInputID::None)
	{
		UE_LOG(LogTemp, Error, TEXT("InputID is None, cannot assign a slot"));
		return;
	}

	// Check if this ability class is already granted anywhere
	for (const TPair<EAbilityInputID, TSubclassOf<UAbility>>& Pair : GrantedAbilities)
	{
		if (Pair.Value == AbilityClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("Ability already exists at slot %d"), static_cast<int32>(Pair.Key));
			return; // Don't add duplicate
		}
	}

	// Slot doesn't exist yet, safe to add
	GrantedAbilities.Add(InputID, AbilityClass);
}

void UAbilitySystem::RemoveAbility(TSubclassOf<UAbility> AbilityClass)
{
	if (!AbilityClass)
	{
		UE_LOG(LogTemp, Error, TEXT("AbilityClass is null!"));
		return;
	}

	for (auto It = GrantedAbilities.CreateIterator(); It; ++It)
	{
		if (It->Value == AbilityClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("Found and removed ability from slot %d"), static_cast<int32>(It->Key));
			It.RemoveCurrent();
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Ability not found in granted abilities"));
}

void UAbilitySystem::RemoveAbilityAtSlot(EAbilityInputID InputID)
{
	GrantedAbilities.Remove(InputID);
}

void UAbilitySystem::InitializeAbility(EAbilityInputID InputID)
{
	if (ActiveAbility)
		return;		// already active, ignore duplicate init
	
	TSubclassOf<UAbility>* AbilityClass = GrantedAbilities.Find(InputID);

	if (AbilityClass && *AbilityClass && MyOwner)
	{
		UAbility* NewAbility = NewObject<UAbility>(this, *AbilityClass);
		NewAbility->ActivateAbility(MyOwner);
		ActiveAbility = NewAbility;
		ActiveAbilityInputID = InputID;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red,
			TEXT("Activate ability failed."));
	}
}

void UAbilitySystem::OnAbilityInputReleased(EAbilityInputID ReleasedInputID)
{
	if (ActiveAbility != nullptr && ReleasedInputID == ActiveAbilityInputID)
	{
		ActiveAbility->EndAbility();
		ActiveAbility = nullptr;
		ActiveAbilityInputID = EAbilityInputID::None;
	}
}

void UAbilitySystem::HandleModifyInput()
{
	if (!ActiveAbility)
		return;

	ActiveAbility->DoModify();

	if (ActiveAbility->bModifyEnd)
	{
		ActiveAbility->EndAbility();
		ActiveAbility = nullptr;
		ActiveAbilityInputID = EAbilityInputID::None;
	}
}
