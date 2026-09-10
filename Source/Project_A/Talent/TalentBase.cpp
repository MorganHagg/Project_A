#include "TalentBase.h"

#include "Project_A/Unit/PlayerUnit.h"
#include "Project_A/Component/AbilitySystem.h"
#include "Project_A/Ability/Ability.h"

void UTalentBase::BindToPlayerUnit(APlayerUnit* PlayerUnit)
{
	MyPlayerUnit = PlayerUnit;
	if (!PlayerUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to bind Talent to a null PlayerUnit"));
		return;
	}

	PlayerUnit->OnAbilityEvent.AddDynamic(this, &UTalentBase::HandleAbilityEvent);

	OnSetup();
}

void UTalentBase::HandleAbilityEvent(FGameplayTag EventTag, FAbilityEventPayload Payload)
{
	if (EventTag.MatchesTag(ListenTag))
	{
		OnAbilityEvent(Payload);
	}
}

UAbility* UTalentBase::GetAbility(TSubclassOf<UAbility> AbilityClass) const
{
	if (!AbilityClass)
	{
		UE_LOG(LogTemp, Error, TEXT("UTalentBase::GetAbility called with a null AbilityClass."));
		return nullptr;
	}

	if (!MyPlayerUnit)
	{
		UE_LOG(LogTemp, Error, TEXT("UTalentBase::GetAbility called before BindToPlayerUnit."));
		return nullptr;
	}

	UAbilitySystem* AbilitySystem = MyPlayerUnit->FindComponentByClass<UAbilitySystem>();
	if (!AbilitySystem)
	{
		UE_LOG(LogTemp, Error, TEXT("UTalentBase::GetAbility - %s has no AbilitySystem component."), *MyPlayerUnit->GetName());
		return nullptr;
	}

	for (UAbility* Ability : AbilitySystem->GrantedAbilities)
	{
		if (Ability && Ability->GetClass() == AbilityClass)
		{
			return Ability;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("UTalentBase::GetAbility - %s does not have an ability of class %s."),
		*MyPlayerUnit->GetName(), *AbilityClass->GetName());
	return nullptr;
}